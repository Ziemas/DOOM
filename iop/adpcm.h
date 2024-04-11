#ifndef ADPCM_H_
#define ADPCM_H_

/*
libpsxav: MDEC video + SPU/XA-ADPCM audio library

Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell

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

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	int qerr;	  // quanitisation error
	uint64_t mse; // mean square error
	int prev1, prev2;
} psx_audio_encoder_channel_state_t;

#define PSX_AUDIO_SPU_LOOP_END 1
#define PSX_AUDIO_SPU_LOOP_REPEAT 3
#define PSX_AUDIO_SPU_LOOP_START 4

uint32_t psx_audio_spu_get_buffer_size(int sample_count);
uint32_t psx_audio_spu_get_buffer_size_per_block(void);
uint32_t psx_audio_spu_get_samples_per_block(void);
int psx_audio_spu_encode(psx_audio_encoder_channel_state_t *state, int16_t *samples,
  int sample_count, int pitch, uint8_t *output);
int psx_audio_spu_encode_simple(int16_t *samples, int sample_count, uint8_t *output,
  int loop_start);
void psx_audio_spu_set_flag_at_sample(uint8_t *spu_data, int sample_pos, int flag);

#endif
