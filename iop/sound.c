#include "sound.h"

#include "adpcm.h"
#include "sound_data.h"
#include "stdio.h"
#include "thbase.h"
#include "wad.h"

#include <ioman.h>
#include <libsd.h>
#include <list.h>
#include <sysclib.h>

#define SPU_WAIT_FOR_TRANSFER 1
#define MAX_SOUND_SIZE 1024 * 48
#define SPU_BASE 0x5010
#define SAMPLE_PAD 16

struct impSound sound_storage[48];

LIST_HEAD(playing_sounds);
LIST_HEAD(free_sounds);

static s16 channel = 0;
static s16 voice = 0;
static u32 spu_alloc;

u8 buf[MAX_SOUND_SIZE];
s16 pcm[MAX_SOUND_SIZE];
u8 adpcm[MAX_SOUND_SIZE] __attribute__((aligned(32)));

static const int filter[5][2] = {
	{ 0, 0 },
	{ -960, 0 },
	{ 1840, 832 },
	{ -1568, 880 },
	{ -1952, 960 },
};

static void
encode_block(s16 *dest, u8 *src)
{
}

static inline u32
rate_to_pitch(u32 sample_rate)
{
	return (sample_rate << 12) / 48000;
}

static inline void
pcm8to16(u8 *src, s16 *dest, u32 size)
{
	for (int i = 0; i < size; i++) {
		dest[i] = (s16)(src[i] - 0x80) << 8;
	}
}

/* Reencodes the 8bit pcm into adpcm and uploads to the spu
 * simple bump allocation used for SPU memory.
 */

int
imp_EncodeSounds()
{
	struct lumphandle lump;
	struct sfxinfo *sfx;
	char name[9];
	int i;
	u32 sec, usec;

	iop_sys_clock_t t1, t2, res;
	GetSystemTime(&t1);

	for (i = 0; i < NUMSFX; i++) {
		sfx = &S_sfx[i];

		sprintf(name, "ds%.6s", sfx->name);
		if (!imp_FindLump(name, &lump)) {
			continue;
		}

		if (lump.size > MAX_SOUND_SIZE) {
			printf("maximum sound size exceeded, bump up the limit!\n");
			return -1;
		}

		imp_ReadLump(&lump, buf);

		struct sound_header *hdr = (struct sound_header *)buf;
		pcm8to16(&hdr->samples[0], pcm, hdr->sample_count);
		u32 count = hdr->sample_count - SAMPLE_PAD;

		int adp_len = psx_audio_spu_encode_simple(pcm, count, adpcm, -1);

		GetSystemTime(&t2);

		u32 err = sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);
		if (err < 0) {
			printf("failed to wait for transfer %d", err);
		}

		u32 trans = sceSdVoiceTrans(0, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, adpcm, (u32 *)spu_alloc,
		  adp_len);
		if (trans < 0) {
			printf("Bad transfer\n");
			return -1;
		}

		sfx->spu_addr = spu_alloc;
		sfx->spu_pitch = rate_to_pitch(hdr->rate);

		spu_alloc += adp_len;
	}

	GetSystemTime(&t2);

	res.lo = t2.lo - t1.lo;
	res.hi = t2.hi - (t1.hi + (t1.lo > t2.lo));

	SysClock2USec(&res, &sec, &usec);

	printf("Spent %llu usec encoding audio\n", usec + (sec * 1000000));

	return 0;
}

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

	imp_EncodeSounds();

	test();
}
