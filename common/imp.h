#ifndef IMP_H_
#define IMP_H_

#include "types.h"

#define IMP_MESSAGE_RPC 0x123457
#define MAX_CMDBUF_ENTRIES 256

struct impCommandReturn {
	void (*done)(u32 ret, u64 data);
	u64 data;
};

enum impCmd {
	IMP_CMD_INIT,
	IMP_CMD_PLAYSND,
	IMP_CMD_MAX,
};

struct playSound {
	u8 id;
};

struct impCommand {
	s32 cmdId;
	union {
		struct playSound snd;
	};
};

struct impCommandBuffer {
	int num_commands;
	// char buffer[4092];
	struct impCommand cmd[MAX_CMDBUF_ENTRIES];
};

#endif // IMP_H_
