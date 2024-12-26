#include "imp.h"

#include "sound.h"
#include "wad.h"

#include <intrman.h>
#include <libsd.h>
#include <sifrpc.h>
#include <stddef.h>
#include <stdio.h>
#include <sysmem.h>
#include <thbase.h>

s32 rpcThreadId = 0;

// static void *messageBuffer = NULL;
// static u32 *returnBuffer = NULL;
// static u32 *retBufPos = NULL;

static struct impCommandBuffer cmd_buf;
static u32 returnBuffer[MAX_CMDBUF_ENTRIES + 1] __attribute__((aligned(64)));

#define MARK() (printf("RPC %s called\n", __FUNCTION__))

static void
imp_c_Init(struct impCommand *, u32 *ret)
{
	printf("--- IMP INIT ---\n");

	*ret = 0;
}

static void
imp_c_PlaySound(struct impCommand *cmd, u32 *ret)
{
	struct playSound *s = &cmd->snd;

	//printf("playsnd: %d %d %d\n", s->id, s->vol, s->priority);

	*ret = 0;
}

static void
imp_c_PlaySong(struct impCommand *cmd, u32 *ret)
{
	MARK();
	*ret = 0;
}

static void
imp_c_SetSfxVol(struct impCommand *cmd, u32 *ret)
{
	MARK();
	*ret = 0;
}

static void
imp_c_SetMusicVol(struct impCommand *cmd, u32 *ret)
{
	MARK();
	*ret = 0;
}

/* Separate sound init, so we can get some files before starting the encoder */
static void
imp_c_InitSound(struct impCommand *, u32 *ret)
{
	imp_InitSound();

	*ret = 0;
}

static void (*commandFunc[IMP_CMD_MAX])(struct impCommand *, u32 *) = {
	[IMP_CMD_INIT] = imp_c_Init,
	[IMP_CMD_PLAYSND] = imp_c_PlaySound,
	[IMP_CMD_PLAYSONG] = imp_c_PlaySong,
	[IMP_CMD_SOUND_INIT] = imp_c_InitSound,
	[IMP_CMD_SETSFXVOL] = imp_c_SetSfxVol,
	[IMP_CMD_SETMUSICVOL] = imp_c_SetMusicVol,
};

static void *
imp_MessageParser(u32 command, void *data, s32 size)
{
	struct impCommandBuffer *buf = data;
	struct impCommand *cmd;

	for (int i = 0; i < buf->num_commands; i++) {
		cmd = &buf->cmd[i];
		if (cmd->cmdId >= IMP_CMD_MAX) {
			printf("%s: BAD COMMAND ID! (trash data dma'd?)", __FUNCTION__);
		}

		commandFunc[cmd->cmdId](cmd, &returnBuffer[i + 1]);
	}

	returnBuffer[0] = 0xf0f0f0f0;

	return returnBuffer;
}

s32
imp_RpcThread()
{
	SifRpcDataQueue_t qd;
	SifRpcServerData_t sd;

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd, IMP_MESSAGE_RPC, (void *(*)(int, void *, int))imp_MessageParser,
	  &cmd_buf, NULL, NULL, &qd);
	sceSifRpcLoop(&qd);

	return 0;
}

int
_start()
{
	iop_thread_t param;

	if (sceSdInit(0) < 0) {
		printf("SD INIT FAILED");
		return 0;
	}

	param.attr = TH_C;
	param.thread = (void (*)(void *))imp_RpcThread;
	param.priority = 0x3a;
	param.stacksize = 0x1000;
	param.option = 0;

	rpcThreadId = CreateThread(&param);
	if (rpcThreadId <= 0) {
		return 1;
	}

	StartThread(rpcThreadId, 0);

	//imp_IdentifyVersion();

	//imp_InitSound();

	return 0;
}
