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
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "d_main.h"
#include "doomdef.h"
#include "i_system.h"
#include "v_video.h"

#include <SDL2/SDL.h>

SDL_Renderer *rend;
SDL_Window *win;
SDL_Palette *pal;
SDL_Surface *screen_surface;

void
I_ShutdownGraphics(void)
{
}

//
// I_StartFrame
//
void
I_StartFrame(void)
{
	// er?
}

static int
TranslateKey(SDL_KeyboardEvent key)
{
	int c;

	switch (key.keysym.sym) {
	case SDLK_UP:
		c = KEY_UPARROW;
		break;
	case SDLK_DOWN:
		c = KEY_DOWNARROW;
		break;
	case SDLK_LEFT:
		c = KEY_LEFTARROW;
		break;
	case SDLK_RIGHT:
		c = KEY_RIGHTARROW;
		break;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		c = KEY_RCTRL;
		break;
	case SDLK_LALT:
	case SDLK_RALT:
		c = KEY_RALT;
		break;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		c = KEY_RSHIFT;
		break;
	default:
		c = key.keysym.sym;
		break;
	}

	return c;
}

void
I_GetEvent(void)
{
	SDL_Event sdl_event;
	event_t event;

	while (SDL_PollEvent(&sdl_event)) {
		switch (sdl_event.type) {
		case SDL_KEYDOWN:
			event.type = ev_keydown;
			event.data1 = TranslateKey(sdl_event.key);
			D_PostEvent(&event);
			break;
		case SDL_KEYUP:
			event.type = ev_keyup;
			event.data1 = TranslateKey(sdl_event.key);
			D_PostEvent(&event);
			break;
		case SDL_QUIT:
			I_Quit();
			break;

		default:
			break;
		}
	}
}

//
// I_StartTic
//
void
I_StartTic(void)
{

	I_GetEvent();
	// Get events
}

//
// I_UpdateNoBlit
//
void
I_UpdateNoBlit(void)
{
	// what is this?
}

//
// I_FinishUpdate
//
void
I_FinishUpdate(void)
{
	SDL_RenderClear(rend);

	memcpy(screen_surface->pixels, screens[0], SCREENWIDTH * SCREENHEIGHT);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(rend, screen_surface);
	SDL_RenderCopy(rend, texture, NULL, NULL);
	SDL_DestroyTexture(texture);

	SDL_RenderPresent(rend);
}

//
// I_ReadScreen
//
void
I_ReadScreen(byte *scr)
{
	memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void
I_SetPalette(byte *palette)
{
	SDL_Color colors[256];
	int c;

	// set the X colormap entries
	for (int i = 0; i < 256; i++) {
		c = gammatable[usegamma][*palette++];
		colors[i].r = (c << 8) + c;
		c = gammatable[usegamma][*palette++];
		colors[i].g = (c << 8) + c;
		c = gammatable[usegamma][*palette++];
		colors[i].b = (c << 8) + c;
		colors[i].a = 0;
	}

	SDL_SetPaletteColors(screen_surface->format->palette, colors, 0, 256);
}

void
I_InitGraphics(void)
{
	int flags, rflags;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		I_Error("Failed to init SDL: %s\n", SDL_GetError());
	}

	flags = SDL_WINDOW_RESIZABLE;
	win = SDL_CreateWindow("DOOM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREENWIDTH,
	  SCREENHEIGHT, flags);

	if (!win) {
		I_Error("Failed to create window: %s\n", SDL_GetError());
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	rflags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	rend = SDL_CreateRenderer(win, -1, rflags);
	if (!rend) {
		I_Error("Failed to create renderer: %s\n", SDL_GetError());
	}

	SDL_GL_SetSwapInterval(1);

	pal = SDL_AllocPalette(256);

	screen_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREENWIDTH, SCREENHEIGHT, 8, 0, 0, 0, 0);
	SDL_RenderSetLogicalSize(rend, SCREENWIDTH, SCREENHEIGHT);
}
