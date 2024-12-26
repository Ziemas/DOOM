//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

#include "imp.h"
#include "imp_com.h"
#include "ps2/libpsxav.h"

#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <sifrpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// Timer stuff. Experimental.
#include "i_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "z_zone.h"

static u32 next_handle = 0;

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void
I_SetChannels()
{
}

void
I_SetSfxVolume(int volume)
{
	// Identical to DOS.
	// Basically, this should propagate
	//  the menu/config file setting
	//  to the state variable used in
	//  the mixing.
	snd_SfxVolume = volume;
	impcom_SetSfxVolume(volume);
}

// MUSIC API - dummy. Some code from DOS version.
void
I_SetMusicVolume(int volume)
{
	// Internal state variable.
	snd_MusicVolume = volume;
	// Now set volume on output device.
	// Whatever( snd_MusciVolume );
	impcom_SetMusicVolume(volume);
}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int
I_GetSfxLumpNum(sfxinfo_t *sfx)
{
	char namebuf[9];
	sprintf(namebuf, "ds%s", sfx->name);
	return W_GetNumForName(namebuf);
}

int
I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
	u32 h = next_handle++;
	impcom_StartSound(h, id, vol, sep, pitch, priority);
	return h;
}

void
I_StopSound(int handle)
{
}

int
I_SoundIsPlaying(int handle)
{
	return 0;
}

void
I_UpdateSound(void)
{
}

void
I_SubmitSound(void)
{
	impcom_FlushBuffer();
}

void
I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
}

void
I_ShutdownSound(void)
{
}

static inline u32
rate_to_pitch(u32 sample_rate)
{
	return (sample_rate << 12) / 48000;
}

/* Requires buffers are qw aligned */
static inline void
pcm8to16(s16 *dest, u8 *src, u32 count)
{
	/*
		static const u8 sub[16] __attribute__((aligned(128))) = { 0x80, 0x80, 0x80, 0x80, 0x80,
	   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };

		u32 qw = count / 16;
		u32 remaining = count % 16;

		__asm__("lq \n"
				""
				: "=m"(dest)
				: "m"(src), "r"(qw), "r"(remaining)
				:);
	*/

	for (int i = 0; i < count; i++) {
		dest[i] = ((s16)(src[i] - 0x80) << 8);
	}
}

void
I_BuildSoundBank()
{
	struct sound_header *hdr;
	s32 sample_count, size;
	sfxinfo_t *sfx;
	char nbuf[9];
	s16 *s16buf;
	u8 *bank_alloc;

	struct {
		u8 *adpcm;
		u32 size;
		u16 pitch;
	} sound[NUMSFX];

	memset(sound, 0, sizeof(sound));

	printf("starting sample conversion\n");

	for (int i = 1; i < NUMSFX; i++) {
		sfx = &S_sfx[i];

		sprintf(nbuf, "ds%.6s", sfx->name);
		int lump = W_CheckNumForName(nbuf);
		if (lump == -1) {
			continue;
		}

		hdr = W_CacheLumpNum(lump, PU_CACHE);
		// we don't care about the padding bytes
		sample_count = hdr->sample_count - 32;
		printf("sample_count %d\n", sample_count);

		s16buf = malloc(sample_count * 2);

		sound[i].size = psx_audio_spu_get_buffer_size(sample_count);
		sound[i].adpcm = malloc(sound[i].size);

		// + 16 to skip start padding
		pcm8to16(s16buf, hdr->samples + 16, sample_count);
		psx_audio_spu_encode_simple(s16buf, sample_count, sound[i].adpcm, -1);

		sound[i].pitch = rate_to_pitch(hdr->rate);

		free(s16buf);
	}

	size = ALIGN_UP(sizeof(struct impSoundBank) + sizeof(struct impSoundEntry) * NUMSFX, 16);
	for (int i = 0; i < NUMSFX; i++) {
		size += sound[i].size;
	}

	printf("total sound bank size 0x%x\n", size);
	bank_alloc = malloc(size);

	for (int i = 1; i < NUMSFX; i++) {
		free(sound[i].adpcm);
	}
}

void
I_InitSound()
{
	//I_BuildSoundBank();
}

void
I_InitMusic(void)
{
}

void
I_ShutdownMusic(void)
{
}

int
I_PlaySong(int musicnum, int looping)
{
	u32 h = next_handle++;
	// impcom_PlaySong(h, musicnum, looping);
	return h;
}

void
I_PauseSong(int handle)
{
}

void
I_ResumeSong(int handle)
{
}

void
I_StopSong(int handle)
{
}

void
I_UnRegisterSong(int handle)
{
}

int
I_RegisterSong(void *data)
{
	return 1;
}

// Is the song playing?
int
I_QrySongPlaying(int handle)
{
	return 0;
}

// Interrupt handler.
void
I_HandleSoundTimer(int ignore)
{
}

// Get the interrupt. Set duration in millisecs.
int
I_SoundSetTimer(int duration_of_tick)
{
	return 0;
}

// Remove the interrupt. Set duration to zero.
void
I_SoundDelTimer()
{
}
