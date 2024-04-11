/*
libpsxav: MDEC video + SPU/XA-ADPCM audio library

Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
Copyright (c) 2023 spicyjpeg

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "adpcm.h"

#include <stdint.h>
#include <string.h>

#define SHIFT_RANGE_4BPS 12

#define ADPCM_FILTER_COUNT 5
#define XA_ADPCM_FILTER_COUNT 4
#define SPU_ADPCM_FILTER_COUNT 5

static const int16_t filter_k1[ADPCM_FILTER_COUNT] = { 0, 60, 115, 98, 122 };
static const int16_t filter_k2[ADPCM_FILTER_COUNT] = { 0, 0, -52, -55, -60 };

static int
find_min_shift(const psx_audio_encoder_channel_state_t *state, int16_t *samples, int sample_limit,
  int pitch, int filter, int shift_range)
{
	// Assumption made:
	//
	// There is value in shifting right one step further to allow the nibbles to clip.
	// However, given a possible shift value, there is no value in shifting one step less.
	//
	// Having said that, this is not a completely accurate model of the encoder,
	// so maybe we will need to shift one step less.
	//
	int prev1 = state->prev1;
	int prev2 = state->prev2;
	int k1 = filter_k1[filter];
	int k2 = filter_k2[filter];

	int right_shift = 0;

	int32_t s_min = 0;
	int32_t s_max = 0;
	for (int i = 0; i < 28; i++) {
		int32_t raw_sample = (i >= sample_limit) ? 0 : samples[i * pitch];
		int32_t previous_values = (k1 * prev1 + k2 * prev2 + (1 << 5)) >> 6;
		int32_t sample = raw_sample - previous_values;
		if (sample < s_min) {
			s_min = sample;
		}
		if (sample > s_max) {
			s_max = sample;
		}
		prev2 = prev1;
		prev1 = raw_sample;
	}
	while (right_shift < shift_range && (s_max >> right_shift) > (+0x7FFF >> shift_range)) {
		right_shift += 1;
	};
	while (right_shift < shift_range && (s_min >> right_shift) < (-0x8000 >> shift_range)) {
		right_shift += 1;
	};

	int min_shift = shift_range - right_shift;
	// assert(0 <= min_shift && min_shift <= shift_range);
	return min_shift;
}

static uint8_t
attempt_to_encode(psx_audio_encoder_channel_state_t *outstate,
  const psx_audio_encoder_channel_state_t *instate, int16_t *samples, int sample_limit, int pitch,
  uint8_t *data, int data_shift, int data_pitch, int filter, int sample_shift, int shift_range)
{
	uint8_t sample_mask = 0xFFFF >> shift_range;
	uint8_t nondata_mask = ~(sample_mask << data_shift);

	int min_shift = sample_shift;
	int k1 = filter_k1[filter];
	int k2 = filter_k2[filter];

	uint8_t hdr = (min_shift & 0x0F) | (filter << 4);

	if (outstate != instate) {
		memcpy(outstate, instate, sizeof(psx_audio_encoder_channel_state_t));
	}

	outstate->mse = 0;

	for (int i = 0; i < 28; i++) {
		int32_t sample = ((i >= sample_limit) ? 0 : samples[i * pitch]) + outstate->qerr;
		int32_t previous_values = (k1 * outstate->prev1 + k2 * outstate->prev2 + (1 << 5)) >> 6;
		int32_t sample_enc = sample - previous_values;
		sample_enc <<= min_shift;
		sample_enc += (1 << (shift_range - 1));
		sample_enc >>= shift_range;
		if (sample_enc < (-0x8000 >> shift_range)) {
			sample_enc = -0x8000 >> shift_range;
		}
		if (sample_enc > (+0x7FFF >> shift_range)) {
			sample_enc = +0x7FFF >> shift_range;
		}
		sample_enc &= sample_mask;

		int32_t sample_dec = (int16_t)((sample_enc & sample_mask) << shift_range);
		sample_dec >>= min_shift;
		sample_dec += previous_values;
		if (sample_dec > +0x7FFF) {
			sample_dec = +0x7FFF;
		}
		if (sample_dec < -0x8000) {
			sample_dec = -0x8000;
		}
		int64_t sample_error = sample_dec - sample;

		// assert(sample_error < (1 << 30));
		// assert(sample_error > -(1 << 30));

		data[i * data_pitch] = (data[i * data_pitch] & nondata_mask) | (sample_enc << data_shift);
		// FIXME: dithering is hard to predict
		// outstate->qerr += sample_error;
		outstate->mse += ((uint64_t)sample_error) * (uint64_t)sample_error;

		outstate->prev2 = outstate->prev1;
		outstate->prev1 = sample_dec;
	}

	return hdr;
}

static uint8_t
encode(psx_audio_encoder_channel_state_t *state, int16_t *samples, int sample_limit, int pitch,
  uint8_t *data, int data_shift, int data_pitch, int filter_count, int shift_range)
{
	psx_audio_encoder_channel_state_t proposed;
	int64_t best_mse = ((int64_t)1 << (int64_t)50);
	int best_filter = 0;
	int best_sample_shift = 0;

	for (int filter = 0; filter < filter_count; filter++) {
		int true_min_shift = find_min_shift(state, samples, sample_limit, pitch, filter,
		  shift_range);

		// Testing has shown that the optimal shift can be off the true minimum shift
		// by 1 in *either* direction.
		// This is NOT the case when dither is used.
		int min_shift = true_min_shift - 1;
		int max_shift = true_min_shift + 1;
		if (min_shift < 0) {
			min_shift = 0;
		}
		if (max_shift > shift_range) {
			max_shift = shift_range;
		}

		for (int sample_shift = min_shift; sample_shift <= max_shift; sample_shift++) {
			// ignore header here
			attempt_to_encode(&proposed, state, samples, sample_limit, pitch, data, data_shift,
			  data_pitch, filter, sample_shift, shift_range);

			if (best_mse > proposed.mse) {
				best_mse = proposed.mse;
				best_filter = filter;
				best_sample_shift = sample_shift;
			}
		}
	}

	// now go with the encoder
	return attempt_to_encode(state, state, samples, sample_limit, pitch, data, data_shift,
	  data_pitch, best_filter, best_sample_shift, shift_range);
}

uint32_t
psx_audio_spu_get_buffer_size(int sample_count)
{
	return ((sample_count + 27) / 28) << 4;
}

uint32_t
psx_audio_spu_get_buffer_size_per_block(void)
{
	return 16;
}

uint32_t
psx_audio_spu_get_samples_per_block(void)
{
	return 28;
}

int
psx_audio_spu_encode(psx_audio_encoder_channel_state_t *state, int16_t *samples, int sample_count,
  int pitch, uint8_t *output)
{
	uint8_t prebuf[28];
	uint8_t *buffer = output;

	for (int i = 0; i < sample_count; i += 28, buffer += 16) {
		buffer[0] = encode(state, samples + i * pitch, sample_count - i, pitch, prebuf, 0, 1,
		  SPU_ADPCM_FILTER_COUNT, SHIFT_RANGE_4BPS);
		buffer[1] = 0;

		for (int j = 0; j < 28; j += 2) {
			buffer[2 + (j >> 1)] = (prebuf[j] & 0x0F) | (prebuf[j + 1] << 4);
		}
	}

	return buffer - output;
}

int
psx_audio_spu_encode_simple(int16_t *samples, int sample_count, uint8_t *output, int loop_start)
{
	psx_audio_encoder_channel_state_t state;
	memset(&state, 0, sizeof(psx_audio_encoder_channel_state_t));
	int length = psx_audio_spu_encode(&state, samples, sample_count, 1, output);

	if (length >= 32) {
		if (loop_start < 0) {
			// output[1] = PSX_AUDIO_SPU_LOOP_START;
			output[length - 16 + 1] = PSX_AUDIO_SPU_LOOP_END;
		} else {
			psx_audio_spu_set_flag_at_sample(output, loop_start, PSX_AUDIO_SPU_LOOP_START);
			output[length - 16 + 1] = PSX_AUDIO_SPU_LOOP_REPEAT;
		}
	} else if (length >= 16) {
		output[1] = PSX_AUDIO_SPU_LOOP_START | PSX_AUDIO_SPU_LOOP_END;
		if (loop_start >= 0)
			output[1] |= PSX_AUDIO_SPU_LOOP_REPEAT;
	}

	return length;
}

void
psx_audio_spu_set_flag_at_sample(uint8_t *spu_data, int sample_pos, int flag)
{
	int buffer_pos = (sample_pos / 28) << 4;
	spu_data[buffer_pos + 1] = flag;
}
