#ifndef SOUND_H_
#define SOUND_H_

#include "list.h"
#include "types.h"

struct sound_header {
	u16 fmt;
	u16 rate;
	u32 sample_count;
	u8 pad[16];
	u8 samples[0];
};

struct impSound {
	struct list_head list;

	union {
		u32 handle;
		struct {
			bool in_use : 1;
			int index : 15;
			int id : 16;
		};
	};

	struct sfxinfo *sfx;
	int vol;
};

void imp_InitSound();

#endif // SOUND_H_
