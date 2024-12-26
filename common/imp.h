#ifndef IMP_H_
#define IMP_H_

#include "types.h"
#define ALIGN_UP(n, a) (((n) + a) & ~(((typeof(n))(a)) - 1))
#define FOURCC(a,b,c,d) ( (uint32) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)) )

struct impSoundEntry {
	u32 start_addr;
	u32 size;
	u32 pitch;
	u16 flags;
	s16 volume;
};

struct impSoundBank {
	u32 magic;
	u32 total_size;
	u32 adpcm_size;
	struct impSoundEntry sounds[0];
};

#define IMP_MESSAGE_RPC 0x123457
#define MAX_CMDBUF_ENTRIES 64

struct impCommandReturn {
	void (*done)(u32 ret, u64 data);
	u64 data;
};

enum impCmd {
	IMP_CMD_GARBAGE, // 0 is invalid
	IMP_CMD_BATCH,
	IMP_CMD_INIT,
	IMP_CMD_SOUND_INIT,
	IMP_CMD_PLAYSND,
	IMP_CMD_PLAYSONG,
	IMP_CMD_UPDATESND,
	IMP_CMD_SETSFXVOL,
	IMP_CMD_SETMUSICVOL,
	IMP_CMD_MAX,
};

struct playSound {
	u32 handle;
	int id;
	int vol;
	int pitch;
	int sep;
	int priority;
};

struct updateSound {
	u32 handle;
	int vol;
	int sep;
	int pitch;
};

struct setVol {
	int vol;
};

struct playSong {
	u32 handle;
	u32 music;
	int looping;
};

struct handle {
	u32 handle;
};

struct impCommand {
	s32 cmdId;
	union {
		struct playSound snd;
		struct updateSound upd;
		struct setVol vol;
		struct playSong song;
		struct handle handle;
	};
};

struct impCommandBuffer {
	u32 num_commands;
	struct impCommand cmd[MAX_CMDBUF_ENTRIES];
} __attribute__((aligned(64)));

#endif // IMP_H_
