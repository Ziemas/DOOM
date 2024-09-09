#include "imp_com.h"

#include "imp.h"
#include "kernel.h"

#include <sifrpc.h>
#include <stdio.h>
#include <string.h>

static SifRpcClientData_t sClientData;

// wasteful for the sync buffer...
static struct impCommand sSyncbuf;
static struct impCommandBuffer sCmdBuf[2];
static struct impCommandReturn sRetCmd[2][MAX_CMDBUF_ENTRIES];
static u32 sRetVal[2][MAX_CMDBUF_ENTRIES + 1] __attribute__((aligned(32)));
static u32 sBuffIdx;
static u32 sCmdPos;

/* Async send */
static void
impcom_Send(struct impCommand *cmd, void (*done)(u32 ret, u64 data), u64 udata)
{
	// Buffer full, flip early
	if (sCmdPos >= MAX_CMDBUF_ENTRIES) {
		impcom_FlushBuffer();
	}

	sRetCmd[sBuffIdx][sCmdPos].done = done;
	sRetCmd[sBuffIdx][sCmdPos].data = udata;
	sCmdBuf[sBuffIdx].cmd[sCmdPos] = *cmd;
	sCmdPos++;
}

/* Blocking */
static int
impcom_SendSync()
{
	return 0;
}

void
impcom_SetSfxVolume(int volume)
{
	struct impCommand cmd;

	cmd.cmdId = IMP_CMD_SETSFXVOL;
	cmd.vol.vol = volume;
	impcom_Send(&cmd, NULL, 0);
}

void
impcom_SetMusicVolume(int volume)
{
	struct impCommand cmd;

	cmd.cmdId = IMP_CMD_SETMUSICVOL;
	cmd.vol.vol = volume;
	impcom_Send(&cmd, NULL, 0);
}

void
impcom_StartSound_CB(u32 handle, int id, int vol, int sep, int pitch, int priority,
  void (*done)(u32 ret, u64 data), u64 udata)
{
	struct impCommand cmd;

	cmd.cmdId = IMP_CMD_PLAYSND;
	cmd.snd.handle = handle;
	cmd.snd.id = id;
	cmd.snd.vol = vol;
	cmd.snd.sep = sep;
	cmd.snd.pitch = pitch;
	cmd.snd.priority = priority;

	impcom_Send(&cmd, done, udata);
}

void
impcom_StartSound(u32 handle, int id, int vol, int sep, int pitch, int priority)
{
	impcom_StartSound_CB(handle, id, vol, sep, pitch, priority, NULL, 0);
}

void
impcom_PlaySong(u32 handle, int music, int looping)
{
	struct impCommand cmd;

	cmd.cmdId = IMP_CMD_PLAYSONG;
	cmd.song.handle = handle;
	cmd.song.music = music;
	cmd.song.looping = looping;

	impcom_Send(&cmd, NULL, 0);
}

void
impcom_FlushBuffer()
{
	if (SifCheckStatRpc(&sClientData) != 0) {
		printf("RPC BUSY\n");
		return;
	}

	if (sRetVal[sBuffIdx ^ 1][0] == 0xf0f0f0f0u) {
		int which = sBuffIdx ^ 1;

		for (int i = 0; i < sCmdBuf[which].num_commands; i++) {
			if (sRetCmd[which][i].done) {
				sRetCmd[which][i].done(sRetVal[which][i], sRetCmd[which][i].data);
			}
		}

		sRetVal[which][0] = -1;
	}

	sRetVal[sBuffIdx][0] = -1;
	sCmdBuf[sBuffIdx].num_commands = sCmdPos;

	SyncDCache((void *)&sCmdBuf[sBuffIdx],
	  (void *)&sCmdBuf[sBuffIdx] + sizeof(struct impCommandBuffer));

	SifCallRpc(&sClientData, 0, SIF_RPC_M_NOWAIT, &sCmdBuf[sBuffIdx], sizeof(sCmdBuf[0]),
	  &sRetVal[sBuffIdx], sizeof(sRetVal[0]), NULL, NULL);

	sBuffIdx ^= 1;
	sCmdPos = 0;
}

int
impcom_Init()
{
	int i;

	sBuffIdx = 0;
	sCmdPos = 0;
	memset(&sSyncbuf, 0, sizeof(sSyncbuf));
	memset(&sCmdBuf, 0, sizeof(sCmdBuf));
	memset(&sRetVal, 0, sizeof(sRetVal));

	SifInitRpc(0);

	do {
		if (SifBindRpc(&sClientData, IMP_MESSAGE_RPC, 0) < 0) {
			printf("error: sceSifBindRpc in %s, at line %d\n", __FILE__, __LINE__);
			return -1;
		}

		i = 10000;
		while (i--)
			;
	} while (sClientData.server == NULL);

	return 0;
};
