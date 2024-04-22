#include "sound.h"

#include "sound_data.h"
#include "stdio.h"
#include "thbase.h"

#include <ioman.h>
#include <libsd.h>
#include <list.h>
#include <sysclib.h>

#define SPU_WAIT_FOR_TRANSFER 1
#define SAMPLE_PAD 16
#define SPU_BASE 0x5010

struct impSound sound_storage[48];

LIST_HEAD(playing_sounds);
LIST_HEAD(free_sounds);

static s16 channel = 0;
static s16 voice = 0;
static u32 spu_alloc;

static void
test()
{
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x1000);
	// sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0x6F,
	// 0xf, 0xf));
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1,
	  SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x0, 0xf));
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2,
	  SD_SET_ADSR2(SD_ADSR_SR_LINEARi, 0x36, SD_ADSR_RR_LINEARd, 0x0));

	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLR, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLL, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_PITCH, 0x1000);
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR1,
	  SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR2,
	  SD_SET_ADSR2(SD_ADSR_SR_EXPi, 0x7f, SD_ADSR_RR_LINEARd, 0x10));

	sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
	sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

	while (1) {
		for (int i = 0; i < NUMSFX; i++) {
			if (S_sfx[i].link) {
				continue;
			}

			if (!S_sfx[i].spu_addr) {
				continue;
			}

			sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, S_sfx[i].spu_addr);
			printf("playing sound at %x\n", S_sfx[i].spu_addr);
			sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, S_sfx[i].spu_pitch);
			printf("rate %x\n", S_sfx[i].spu_pitch);
			sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

			DelayThread(2000000);
		}
	}
}

void
imp_InitSound()
{
	spu_alloc = SPU_BASE;
	memset(sound_storage, 0, sizeof(sound_storage));

	for (int i = 0; i < 48; i++) {
		list_init(&sound_storage[i].list);
		list_insert(&free_sounds, &sound_storage[i].list);
	}

	test();
}
