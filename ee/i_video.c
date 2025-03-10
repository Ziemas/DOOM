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

#include "d_main.h"
#include "doomdef.h"
#include "i_system.h"
#include "imp_com.h"
#include "ps2/dma.h"
#include "ps2/graph.h"
#include "ps2/gs_regs.h"
#include "timer.h"
#include "v_video.h"

#include <kernel.h>
#include <libpad.h>
#include <stdbool.h>

#define OUTPUT_WIDTH 640
#define OUTPUT_HEIGHT 448

int g_fps;

uint32 gsAllocPtr;
uint32 screenStart;
uint32 clutStart;
const uint32 gsEnd = (4 * 1024 * 1024) / 4 / 64;

const uint32 xoffset = 2048 - OUTPUT_WIDTH / 2;
const uint32 yoffset = 2048 - OUTPUT_HEIGHT / 2;

struct color {
	uint8 r, g, b, a;
};

struct color palbuf[256] __attribute__((aligned(128)));

// does this really belong here
struct dmaDispBuffer {
	uint64 pmode;
	uint64 dispfb1;
	uint64 dispfb2;
	uint64 display1;
	uint64 display2;
	uint64 bgcolor;
};

struct dmaDrawBuffer {
	uint64 frame1;
	uint64 frame2;
	uint64 zbuf1;
	uint64 zbuf2;
	uint64 xyoffset1;
	uint64 xyoffset2;
	uint64 scissor1;
	uint64 scissor2;
};

struct dmaBuffers {
	struct dmaDispBuffer disp[2];
	struct dmaDrawBuffer draw[2];
};

struct dmaBuffers dma_buffers;
uint128 gif_buffer_storage[16 * 1024] __attribute__((aligned(128)));
static uint128 *gif_buffer = NULL;
#define SP_SIZE (1024 * 16)

bool pal_dirty = false;

#define fun_printf printf

static void
indent(int n)
{
	while (n--)
		fun_printf("    ");
}

static void
dumpData(uint32 *data, int qwc, int depth)
{
	while (qwc--) {
		float *fdata = (float *)data;
		indent(depth + 1);
		fun_printf("%08X: ", (uint32)data);
		fun_printf("%08X %08X %08X %08X\t", data[0], data[1], data[2], data[3]);
		fun_printf("%.3f %.3f %.3f %.3f\n", fdata[0], fdata[1], fdata[2], fdata[3]);
		data += 4;
	}
}

void
dumpDma(uint32 *packet, int data)
{
	uint32 *addr, *next;
	uint32 *stack[2];
	uint32 qwc;
	int sp = 0;
	int end = 0;
	fun_printf("packet start: %p\n", packet);
	while (!end) {
		qwc = packet[0] & 0xFFFF;
		addr = (uint32 *)packet[1];
		indent(sp);
		fun_printf("%08X: %08X %08X %08X %08X\n", (uint32)packet, packet[0], packet[1], packet[2],
		  packet[3]);
		switch ((packet[0] >> 28) & 7) {
		case 0:
			indent(sp);
			fun_printf("refe %p, %X\n", addr, qwc);
			if (data)
				dumpData(addr, qwc, sp);
			end = 1;
			next = NULL;
			break;
		case 1:
			indent(sp);
			fun_printf("cnt %p, %X\n", addr, qwc);
			if (data)
				dumpData(packet + 4, qwc, sp);
			next = packet + (1 + qwc) * 4;
			break;
		case 2:
			indent(sp);
			fun_printf("next %p, %X\n", addr, qwc);
			if (data)
				dumpData(packet + 4, qwc, sp);
			next = addr;
			break;
		case 3:
			indent(sp);
			fun_printf("ref %p, %X\n", addr, qwc);
			if (data)
				dumpData(addr, qwc, sp);
			next = packet + 4;
			break;
		case 4:
			indent(sp);
			fun_printf("refs %p, %X\n", addr, qwc);
			if (data)
				dumpData(addr, qwc, sp);
			next = packet + 4;
			break;
		case 5:
			indent(sp);
			fun_printf("call %p, %X\n", addr, qwc);
			if (data)
				dumpData(packet + 4, qwc, sp);
			stack[sp++] = packet + (1 + qwc) * 4;
			if (sp > 2) {
				fun_printf("DMA stack overflow\n\n\n");
				return;
			}
			next = addr;
			break;
		case 6:
			indent(sp);
			fun_printf("ret %p, %X\n", addr, qwc);
			if (data)
				dumpData(packet + 4, qwc, sp);
			next = stack[--sp];
			if (sp < 0) {
				fun_printf("DMA stack underflow\n\n\n");
				return;
			}
			break;
		case 7:
			indent(sp);
			fun_printf("end %p, %X\n", addr, qwc);
			if (data)
				dumpData(packet + 4, qwc, sp);
			end = 1;
			next = NULL;
			break;
		}
		packet = next;
	}
	fun_printf("packet end\n\n\n");
}

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

static char pad_data[256 * 2] __attribute__((aligned(64)));
static struct padButtonStatus buttons[2];
static int pad_state;

static int
run_pad_state()
{
	int status = padGetState(0, 0);
	int id, exid;

	if (status == PAD_STATE_DISCONN) {
		pad_state = 0;
	}

	switch (pad_state) {
	case 0:
		if (status != PAD_STATE_STABLE && status != PAD_STATE_FINDCTP1) {
			break;
		}

		id = padInfoMode(0, 0, PAD_MODECURID, 0);
		if (id == 0) {
			break;
		}
		exid = padInfoMode(0, 0, PAD_MODECUREXID, 0);
		if (exid > 0) {
			id = exid;
		}

		if (id == PAD_TYPE_DUALSHOCK) {
			pad_state = 3;
			break;
		}

		if (id == PAD_TYPE_DIGITAL) {
			pad_state = 1;
			break;
		}
		break;
	case 1:
		// Digital only
		if (padInfoMode(0, 0, PAD_MODECUREXID, 0) == 0) {
			pad_state = 4;
			break;
		}

		if (padSetMainMode(0, 0, 1, 0) == 1) {
			pad_state = 2;
		}
		break;
	case 2:
		if (padGetReqState(0, 0) == PAD_RSTAT_FAILED) {
			pad_state = 1;
		}
		if (padGetReqState(0, 0) == PAD_RSTAT_COMPLETE) {
			pad_state = 0;
		}
		break;
	case 3:
		if (status == PAD_STATE_STABLE || status == PAD_STATE_FINDCTP1) {
			buttons[1] = buttons[0];
			padRead(0, 0, &buttons[0]);
		}
		return 0;
	default:
		break;
	}

	return 1;
};

static void
test_send_key(int button_bit, int key)
{
	event_t event;
	int prev = buttons[1].btns & button_bit;
	int cur = buttons[0].btns & button_bit;
	int type = cur ? ev_keyup : ev_keydown;

	if (cur != prev) {
		event.type = type;
		event.data1 = key;
		D_PostEvent(&event);
	}
}

static int
check_button(int button)
{
	if (buttons[0].btns & button) {
		return 0;
	} else {
		return 1;
	}
}

void
I_GetEvent(void)
{
	event_t event = {};

	if (run_pad_state()) {
		return;
	}

	test_send_key(PAD_LEFT, KEY_LEFTARROW);
	test_send_key(PAD_RIGHT, KEY_RIGHTARROW);
	test_send_key(PAD_UP, KEY_UPARROW);
	test_send_key(PAD_DOWN, KEY_DOWNARROW);
	test_send_key(PAD_CROSS, KEY_ENTER);
	test_send_key(PAD_SQUARE, KEY_BACKSPACE);
	// test_send_key(PAD_TRIANGLE, KEY_ESCAPE);
	test_send_key(PAD_START, KEY_PAUSE);

	// test_send_key(PAD_R2, KEY_RCTRL);

	event.type = ev_joystick;
	event.data1 = (check_button(PAD_R2) << 0) | (check_button(PAD_SQUARE) << 1) |
	  (check_button(PAD_L1) << 2) | (check_button(PAD_CROSS) << 3);

	if ((buttons[0].ljoy_h <= 50) || (buttons[0].ljoy_h >= 200)) {
		event.data2 = buttons[0].ljoy_h - 127;
	}
	if ((buttons[0].ljoy_v <= 50) || (buttons[0].ljoy_v >= 200)) {
		event.data3 = buttons[0].ljoy_v - 127;
	}

	if ((buttons[0].rjoy_h <= 50) || (buttons[0].rjoy_h >= 200)) {
		event.data4 = buttons[0].rjoy_h - 127;
	}
	if ((buttons[0].rjoy_v <= 50) || (buttons[0].rjoy_v >= 200)) {
		event.data5 = buttons[0].rjoy_v - 127;
	}

	// printf("%d:%d\n", buttons[0].ljoy_h, buttons[0].ljoy_v);
	D_PostEvent(&event);
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

static void
SetDisp(struct dmaDispBuffer *disp)
{
	write64(GS_R_PMODE, disp->pmode);
	write64(GS_R_DISPFB1, disp->dispfb1);
	write64(GS_R_DISPLAY1, disp->display1);
	write64(GS_R_DISPFB2, disp->dispfb2);
	write64(GS_R_DISPLAY2, disp->display2);
	write64(GS_R_BGCOLOR, disp->bgcolor);
}

static inline void
SetDraw(struct dmaDrawBuffer *buf, struct dmaList *list)
{
	dmaCnt(list, 9, 0, 0);
	dmaAddGIFtag(list, 8, 1, 0, 0, GIF_PACKED, 1, GIF_PACKED_AD);
	dmaAddAD(list, GS_R_FRAME_1, buf->frame1);
	dmaAddAD(list, GS_R_FRAME_2, buf->frame2);
	dmaAddAD(list, GS_R_ZBUF_1, buf->zbuf1);
	dmaAddAD(list, GS_R_ZBUF_2, buf->zbuf2);
	dmaAddAD(list, GS_R_XYOFFSET_1, buf->xyoffset1);
	dmaAddAD(list, GS_R_XYOFFSET_2, buf->xyoffset2);
	dmaAddAD(list, GS_R_SCISSOR_1, buf->scissor1);
	dmaAddAD(list, GS_R_SCISSOR_2, buf->scissor2);
}

static void
Clear(struct dmaList *list, uint8 r, uint8 g, uint8 b)

{
	const uint32 w = OUTPUT_WIDTH;
	const uint32 h = OUTPUT_HEIGHT;
	const uint32 nstrips = w / 32;

	dmaCnt(list, 5 + nstrips * 2, 0, 0);
	dmaAddGIFtag(list, 4 + nstrips * 2, 1, 1, GS_PRIM_SPRITE, GIF_PACKED, 1, GIF_PACKED_AD);

	dmaAddAD(list, GS_R_TEST_1, GS_SET_TEST(0, 1, 0, 0, 0, 0, 1, 1));
	dmaAddAD(list, GS_R_SCISSOR_1, GS_SET_SCISSOR(0, w - 1, 0, h - 1));
	dmaAddAD(list, GS_R_PRMODE, GS_SET_PRMODE(0, 0, 0, 0, 0, 0, 0, 0));

	dmaAddAD(list, GS_R_RGBAQ, GS_SET_RGBAQ(r, g, b, 0x80, 0));

	for (int i = 0; i < nstrips; i++) {
		int x = 2048 - w / 2;
		int y = 2048 - h / 2;

		dmaAddAD(list, GS_R_XYZ2, GS_SET_XYZ((x + i * 32) << 4, y << 4, 0));
		dmaAddAD(list, GS_R_XYZ2, GS_SET_XYZ((x + (i + 1) * 32) << 4, (y + h) << 4, 0));
	}
}

struct texFmt {
	uint32 size_to_qw;
} const texInfo[] = {
	[GS_PSMCT32] = { .size_to_qw = 2 },
	[GS_PSMCT24] = { .size_to_qw = 2 },
	[GS_PSMT8] = { .size_to_qw = 4 },
	[GS_PSMT8H] = { .size_to_qw = 4 },
};

static uint32
next_pow2(uint32 v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

static void
UploadImage(struct dmaList *list, void *image, int width, int height, int psm, int dest)
{
	uint32 qw = (width * height) >> texInfo[psm].size_to_qw;
	uint32 dbw = (next_pow2(width)) / 64;
	if (dbw == 0)
		dbw = 1;
	uint32 block = qw / GIF_BLOCK_SIZE;
	uint32 remaining = qw % GIF_BLOCK_SIZE;

	dmaCnt(list, 5, 0, 0);
	dmaAddGIFtag(list, 4, 0, 0, 0, GIF_PACKED, 1, GIF_PACKED_AD);
	dmaAddAD(list, GS_R_BITBLTBUF, GS_SET_BITBLTBUF(0, 0, 0, dest, dbw, psm));
	dmaAddAD(list, GS_R_TRXPOS, GS_SET_TRXPOS(0, 0, 0, 0, 0));
	dmaAddAD(list, GS_R_TRXREG, GS_SET_TRXREG(width, height));
	dmaAddAD(list, GS_R_TRXDIR, GS_SET_TRXDIR(0));

	while (block-- > 0) {
		dmaCnt(list, 1, 0, 0);
		dmaAddGIFtag(list, GIF_BLOCK_SIZE, 1, 0, 0, GIF_IMAGE, 0, 0);
		dmaRef(list, image, GIF_BLOCK_SIZE, 0, 0);

		image = image + (GIF_BLOCK_SIZE * 16);
	}

	if (remaining) {
		dmaCnt(list, 1, 0, 0);
		dmaAddGIFtag(list, remaining, 1, 0, 0, GIF_IMAGE, 0, 0);
		dmaRef(list, image, remaining, 0, 0);
	}

	dmaCnt(list, 2, 0, 0);
	dmaAddGIFtag(list, 1, 1, 0, 0, GIF_PACKED, 1, GIF_PACKED_AD);
	dmaAddAD(list, GS_R_TEXFLUSH, 0);
}

static void
DrawFrame(struct dmaList *list)
{

	uint32 w = OUTPUT_WIDTH;
	uint32 h = OUTPUT_HEIGHT;
	int x = 2048 - w / 2;
	int y = 2048 - h / 2;

	dmaCnt(list, 8 + 1, 0, 0);
	dmaAddGIFtag(list, 8, 1, 1, GS_PRIM_SPRITE, GIF_PACKED, 1, GIF_PACKED_AD);

	dmaAddAD(list, GS_R_TEX0_1,
	  GS_SET_TEX0(screenStart, 512 / 64, GS_PSMT8H, 9, 8, 0, GS_DECAL, clutStart, GS_PSMCT32, 0, 0,
		1));

	dmaAddAD(list, GS_R_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));

	dmaAddAD(list, GS_R_PRMODE, GS_SET_PRMODE(0, 1, 0, 0, 0, 1, 0, 0));
	dmaAddAD(list, GS_R_RGBAQ, GS_SET_RGBAQ(0x0, 0x0, 0x00, 0x80, 0));

	dmaAddAD(list, GS_R_UV, GS_SET_UV(0, 0));
	dmaAddAD(list, GS_R_XYZ2, GS_SET_XYZ((x) << 4, (y) << 4, 0));

	dmaAddAD(list, GS_R_UV, GS_SET_UV(SCREENWIDTH << 4, SCREENHEIGHT << 4));
	dmaAddAD(list, GS_R_XYZ2, GS_SET_XYZ(((x + w)) << 4, ((y + h)) << 4, 0));
}

/* Code borrewde from crispy, stupid changes */
static void
updateFPS()
{
	static int lastmili;
	static int fpscount;
	uint32 busclock_sec;
	uint32 busclock_usec;
	int time, mili;

	fpscount++;
	TimerBusClock2USec(GetTimerSystemTime(), &busclock_sec, &busclock_usec);

	time = busclock_sec * 1000 + busclock_usec / 1000;
	mili = time - lastmili;
	if (mili >= 1000) {
		g_fps = (fpscount * 1000) / mili;
		fpscount = 0;
		lastmili = time;
	}
}

//
// I_FinishUpdate
//
void
I_FinishUpdate(void)
{
	struct dmaList list;
	static int f = 0;

	dmaStart(&list, gif_buffer, SP_SIZE);

	SetDraw(&dma_buffers.draw[f], &list);
	Clear(&list, 0x80, 0x0, 0x0);

	SyncDCache(screens[0], &screens[0][512 * 256]);
	UploadImage(&list, screens[0], SCREENWIDTH, SCREENHEIGHT, GS_PSMT8H, screenStart);

	if (pal_dirty) {
		UploadImage(&list, palbuf, 16, 16, GS_PSMCT32, clutStart);
		pal_dirty = false;
	}

	DrawFrame(&list);
	dmaFinish(&list);

	// dumpDma(list.start, 1);

	EE_SYNCL();
	dmaSend(DMA_CHAN_GIF, &list);

	// Eeh, surely we'd do this before sending anything
	// or editing dma lists in progress
	dmaSyncPath();

	updateFPS();
	graphWaitVSync();

	SetDisp(&dma_buffers.disp[f]);

	f ^= 1;
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
	byte *gamma = gammatable[usegamma];
	struct color *pbuf = UCA_PTR(palbuf);

#define clut_shuf(x) (((x) & ~0x18) | ((((x) & 0x08) << 1) | (((x) & 0x10) >> 1)))
	for (int i = 0; i < 256; i++) {
		pbuf[clut_shuf(i)].r = gamma[*palette++];
		pbuf[clut_shuf(i)].g = gamma[*palette++];
		pbuf[clut_shuf(i)].b = gamma[*palette++];
		pbuf[clut_shuf(i)].a = 0;
	}
#undef clut_shuf

	pal_dirty = true;
}

void
dmaInitDraw(struct dmaDrawBuffer *draw, int width, int height, int psm, int zpsm)
{
	draw->frame1 = GS_SET_FRAME(0, width / 64, psm, 0);
	draw->frame2 = draw->frame1;
	draw->zbuf1 = GS_SET_ZBUF(0, zpsm, 0);
	draw->zbuf2 = draw->zbuf1;
	draw->xyoffset1 = GS_SET_XYOFFSET((2048 - width / 2) << 4, (2048 - height / 2) << 4);
	draw->xyoffset2 = draw->xyoffset1;
	draw->scissor1 = GS_SET_SCISSOR(0, width - 1, 0, height - 1);
	draw->scissor2 = draw->scissor1;
}

void
InitDisp(struct dmaDispBuffer *disp, int width, int height, int psm)
{
	int magh, magv;
	int dx, dy;
	int dw, dh;

	dx = 636;
	dy = 25;
	magh = 2560 / width - 1;
	magv = 0;
	dw = 2560 - 1;
	dh = height - 1;
	dy *= 2;

	disp->pmode = GS_SET_PMODE(0, 1, 1, 1, 1, 0, 0x00);
	disp->bgcolor = 0x000000;
	disp->dispfb1 = 0;
	disp->dispfb2 = GS_SET_DISPFB(0, width / 64, psm, 0, 0);
	disp->display1 = 0;
	disp->display2 = GS_SET_DISPLAY(dx, dy, magh, magv, dw, dh);
}

void
InitBuffers(struct dmaBuffers *buffers, int width, int height, int psm, int zpsm)
{
	uint32 fbsz, zbsz;
	uint32 fbp, zbp;
	fbsz = (width * height * 4 / 4 + 2047) / 2048;
	zbsz = (width * height * 4 / 4 + 2047) / 2048;
	gsAllocPtr = (2 * fbsz + zbsz) * 4 * 2048;
	fbp = fbsz;
	zbp = fbsz * 2;

	// display buffers
	InitDisp(&buffers->disp[0], width, height, psm);
	buffers->disp[1] = buffers->disp[0];
	buffers->disp[1].dispfb2 |= fbp;

	// draw buffers
	dmaInitDraw(&buffers->draw[0], width, height, psm, zpsm);
	buffers->draw[0].zbuf1 |= zbp;
	buffers->draw[0].zbuf2 |= zbp;
	buffers->draw[1] = buffers->draw[0];
	buffers->draw[1].frame1 |= fbp;
	buffers->draw[1].frame2 |= fbp;

	// screenStart = gsAllocPtr / 4 / 64;
	//  Upload screen as PSMT8H in the framebuffer space
	screenStart = 0;
	clutStart = gsEnd - 10;
}

void
InitGsRegs(struct dmaList *list)
{
	dmaCnt(list, 28 + 1, 0, 0);
	dmaAddGIFtag(list, 28, 1, 0, 0, GIF_PACKED, 1, 0xe);

	dmaAddAD(list, GS_R_FRAME_1, 0);
	dmaAddAD(list, GS_R_ZBUF_1, 0);
	dmaAddAD(list, GS_R_XYOFFSET_1, 0);
	dmaAddAD(list, GS_R_SCISSOR_1, 0);
	dmaAddAD(list, GS_R_TEST_1, 0);
	dmaAddAD(list, GS_R_ALPHA_1, 0);
	dmaAddAD(list, GS_R_TEX0_1, 0);
	dmaAddAD(list, GS_R_TEX1_1, 0);
	dmaAddAD(list, GS_R_CLAMP_1, GS_SET_CLAMP(GS_CLAMP, GS_CLAMP, 0, 0, 0, 0));
	dmaAddAD(list, GS_R_COLCLAMP, GS_SET_COLCLAMP(1));
	dmaAddAD(list, GS_R_FRAME_2, 0);
	dmaAddAD(list, GS_R_ZBUF_2, 0);
	dmaAddAD(list, GS_R_XYOFFSET_2, 0);
	dmaAddAD(list, GS_R_SCISSOR_2, 0);
	dmaAddAD(list, GS_R_TEST_2, 0);
	dmaAddAD(list, GS_R_ALPHA_2, 0);
	dmaAddAD(list, GS_R_TEX0_2, 0);
	dmaAddAD(list, GS_R_TEX1_2, 0);
	dmaAddAD(list, GS_R_CLAMP_2, 0);
	dmaAddAD(list, GS_R_PRMODE, 0);
	dmaAddAD(list, GS_R_PABE, 0);
	dmaAddAD(list, GS_R_PRMODECONT, 0);
	dmaAddAD(list, GS_R_FOGCOL, 0);
	dmaAddAD(list, GS_R_TEXA, 0);
	dmaAddAD(list, GS_R_MIPTBP1_1, 0);
	dmaAddAD(list, GS_R_MIPTBP1_2, 0);
	dmaAddAD(list, GS_R_MIPTBP2_1, 0);
	dmaAddAD(list, GS_R_MIPTBP2_2, 0);
}

void
I_InitGraphics(void)
{
	struct dmaList list;

	dmaInit();
	graphReset(GS_INTERLACE, GS_MODE_NTSC, GS_FIELD);
	InitBuffers(&dma_buffers, OUTPUT_WIDTH, OUTPUT_HEIGHT, GS_PSMCT24, GS_PSMZ24);

	gif_buffer = UCA_PTR(gif_buffer_storage);
	dmaStart(&list, gif_buffer, SP_SIZE);
	InitGsRegs(&list);
	dmaFinish(&list);
	dmaSend(DMA_CHAN_GIF, &list);
	dmaSyncPath();

	padInit(0);
	padPortOpen(0, 0, pad_data);
	pad_state = 0;
}
