#include "imp.h"

#include <sifrpc.h>
#include <stddef.h>
#include <stdio.h>
#include <sysmem.h>
#include <thbase.h>

s32 rpcThreadId = 0;

static void *messageBuffer = NULL;
static u32 *returnBuffer = NULL;
static u32 *retBufPos = NULL;

static void
impInit(u8 *)
{
	printf("--- IMP INIT ---\n");
	*retBufPos = 0;
}

static void (*commandFunc[IMP_CMD_MAX])(u8 *) = {
	[IMP_CMD_INIT] = impInit,
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
	  messageBuffer, NULL, NULL, &qd);
	sceSifRpcLoop(&qd);

	return 0;
}

int
_start()
{
	iop_thread_t param;

	messageBuffer = AllocSysMemory(0, 0x1000, 0);
	returnBuffer = AllocSysMemory(0, 0x420, 0);

	printf("imp hello\n");
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

	return 0;
}
