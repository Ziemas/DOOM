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

void
I_InitSound()
{
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
	//impcom_PlaySong(h, musicnum, looping);
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
