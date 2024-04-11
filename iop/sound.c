#include "sound.h"

#include "adpcm.h"
#include "ioman.h"
#include "list.h"
#include "sound_data.h"
#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "wad.h"

#include <libsd.h>

#define SPU_WAIT_FOR_TRANSFER 1

#define MAX_SOUND_SIZE 1024 * 16

u8 buf[MAX_SOUND_SIZE];
s16 pcm[MAX_SOUND_SIZE];
u8 adpcm[MAX_SOUND_SIZE] __attribute__((aligned(32)));

#define SAMPLE_PAD 16
struct sound_header {
	u16 fmt;
	u16 rate;
	u32 sample_count;
	u8 pad[16];
	u8 samples[0];
};

s16 channel = 0;
s16 voice = 0;
#define SPU_DST (0x3800 << 1)

static void
initRegs()
{
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x1000);
	sceSdSetParam(SD_VOICE(channel, (voice)) | SD_VPARAM_ADSR1,
	  SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
	sceSdSetParam(SD_VOICE(channel, (voice)) | SD_VPARAM_ADSR2,
	  SD_SET_ADSR2(SD_ADSR_SR_EXPi, 0x7f, SD_ADSR_RR_LINEARd, 0x10));

	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLR, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLL, 0x3fff);
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_PITCH, 0x1000);
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR1,
	  SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
	sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR2,
	  SD_SET_ADSR2(SD_ADSR_SR_EXPi, 0x7f, SD_ADSR_RR_LINEARd, 0x10));

	sceSdSetParam(0 | SD_PARAM_MMIX, 0xffff);
	sceSdSetParam(1 | SD_PARAM_MMIX, 0xffff);
	sceSdSetSwitch(channel | SD_SWITCH_VMIXL, 0xffff);
	sceSdSetSwitch(channel | SD_SWITCH_VMIXR, 0xffff);
	sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
	sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
	sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);
}

static inline u32
rate_to_pitch(u32 sample_rate)
{
	return (sample_rate << 12) / 48000;
}

static inline void
pcm8to16(u8 *src, s16 *dest, u32 size)
{
	while (size-- > 0) {
		*dest = (s16)(*src - 0x80) << 8;
		src++;
		dest++;
	}
}

static unsigned int
set_kon(void *)
{
	static int phase = 0;
	iop_sys_clock_t clock = { 0 };
	USec2SysClock(1000000, &clock);

	u32 kon = (1 << voice);

	if (phase) {
		sceSdSetSwitch(channel | SD_SWITCH_KON, kon);
	} else {
		sceSdSetSwitch(channel | SD_SWITCH_KOFF, kon);
	}

	phase = !phase;

	return clock.lo;
}

int
test()
{
	struct lumphandle lump;

	if (imp_FindLump("dsshotgn", &lump)) {
		printf("found lump for dsshotgn!\n");
		printf("size %d\n", lump.size);
		printf("pos %d\n", lump.position);
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
	u32 trans = sceSdVoiceTrans(1, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, adpcm, (u32 *)SPU_DST,
	  adp_len);
	if (trans < 0) {
		printf("Bad transfer\n");
		return 1;
	}

	u32 err = sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
	if (err < 0) {
		printf("failed to wait for transfer %d", err);
	}

	int fd = open("host:dump.bin", O_RDWR | O_CREAT);
	write(fd, adpcm, adp_len);

	printf("rate: %d\n", hdr->rate);
	printf("pitch: %04x\n", rate_to_pitch(hdr->rate));
	sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST);
	sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, rate_to_pitch(hdr->rate));
	sceSdSetSwitch(channel | SD_SWITCH_KON, 1 << voice);

	iop_sys_clock_t clock = { 0 };
	USec2SysClock(1000000, &clock);
	SetAlarm(&clock, set_kon, NULL);

	return 0;
}

void
imp_InitSound()
{
	initRegs();
	test();
}
