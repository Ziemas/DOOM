#ifndef SOUND_H_
#define SOUND_H_

#include "list.h"
#include "types.h"

struct sound_header {
	u16 fmt;
	u16 rate;
	u32 sample_count;
	u8 samples[0];
};

struct impSound {
	struct list_head list;
	struct sfxinfo *sfx;
	u32 tag;
	int vol;
};

void imp_InitSound();

#endif // SOUND_H_
