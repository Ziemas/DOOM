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
static u32 returnBuffer[MAX_CMDBUF_ENTRIES] __attribute__((aligned(64)));
static u32 *retBufPos = NULL;

static void
imp_c_Init(u8 *)
{
	printf("--- IMP INIT ---\n");

	*retBufPos = 0;
}

/* Separate sound init, so we can get some files before starting the encoder */
static void
imp_c_InitSound(u8 *)
{
	imp_InitSound();

	*retBufPos = 0;
}

static void (*commandFunc[IMP_CMD_MAX])(u8 *) = {
	[IMP_CMD_INIT] = imp_c_Init,
	[IMP_CMD_SOUND_INIT] = imp_c_InitSound,
};

static void *
imp_MessageParser(u32 command, void *data, s32 size)
{
	retBufPos = &returnBuffer[1];
	commandFunc[command](data);
	retBufPos[1] = -1;

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
	int istate;

	if (sceSdInit(0) < 0) {
		printf("SD INIT FAILED");
		return 0;
	}

	// CpuSuspendIntr(&istate);
	// messageBuffer = AllocSysMemory(ALLOC_FIRST, 0x1000, NULL);
	// returnBuffer = AllocSysMemory(ALLOC_FIRST, 0x420, NULL);
	// CpuResumeIntr(istate);

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

	imp_IdentifyVersion();

	imp_InitSound();

	return 0;
}
