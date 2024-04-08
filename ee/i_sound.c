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

#include <fcntl.h>
#include <math.h>
#include <sifrpc.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// Timer stuff. Experimental.
#include "i_sound.h"
#include "w_wad.h"

static SifRpcClientData_t imp_client_data;

// wasteful for the sync buffer...
struct impCommandBuffer imp_syncbuf __attribute__((aligned(64)));
struct impCommandBuffer imp_cmd_buf1 __attribute__((aligned(64)));
struct impCommandBuffer imp_cmd_buf2 __attribute__((aligned(64)));
u32 imp_return_values1[MAX_CMDBUF_ENTRIES] __attribute__((aligned));
u32 imp_return_values2[MAX_CMDBUF_ENTRIES] __attribute__((aligned));
u32 buf_pos[2];


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
}

// MUSIC API - dummy. Some code from DOS version.
void
I_SetMusicVolume(int volume)
{
	// Internal state variable.
	snd_MusicVolume = volume;
	// Now set volume on output device.
	// Whatever( snd_MusciVolume );
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
	return 0;
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
}

void
I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
}

void
I_ShutdownSound(void)
{
}

void
I_InitSound()
{
	int i;

	SifInitRpc(0);

	do {
		if (SifBindRpc(&imp_client_data, IMP_MESSAGE_RPC, 0) < 0) {
			printf("error: sceSifBindRpc in %s, at line %d\n", __FILE__, __LINE__);
			while (1)
				;
		}

		i = 10000;
		while (i--)
			;
	} while (imp_client_data.server == NULL);
}

void
I_InitMusic(void)
{
}

void
I_ShutdownMusic(void)
{
}

void
I_PlaySong(int handle, int looping)
{
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
