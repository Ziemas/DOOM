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

#define MAX_SOUND_SIZE 1024 * 48
#define SPU_BASE 0x5010

u8 buf[MAX_SOUND_SIZE];
s16 pcm[MAX_SOUND_SIZE];
u8 adpcm[2][MAX_SOUND_SIZE] __attribute__((aligned(32)));

static s16 channel = 0;
static s16 voice = 0;

static u32 spu_alloc;

#define SAMPLE_PAD 16
struct sound_header {
	u16 fmt;
	u16 rate;
	u32 sample_count;
	u8 pad[16];
	u8 samples[0];
};

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

int
imp_EncodeSounds()
{
	struct lumphandle lump;
	struct sfxinfo *sfx;
	char name[9];
	int buf_idx = 0;
	int i;

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

		int adp_len = psx_audio_spu_encode_simple(pcm, count, adpcm[buf_idx], -1);

		u32 err = sceSdVoiceTransStatus(buf_idx, SPU_WAIT_FOR_TRANSFER);
		if (err < 0) {
			printf("failed to wait for transfer %d", err);
		}

		u32 trans = sceSdVoiceTrans(buf_idx, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, adpcm[buf_idx],
		  (u32 *)spu_alloc, adp_len);
		if (trans < 0) {
			printf("Bad transfer\n");
			return -1;
		}

		sfx->spu_addr = spu_alloc;
		sfx->spu_pitch = rate_to_pitch(hdr->rate);

		spu_alloc += adp_len;
		buf_idx = !buf_idx;
	}

	return 0;
}

void
imp_InitSound()
{
	spu_alloc = SPU_BASE;
	imp_EncodeSounds();
}
