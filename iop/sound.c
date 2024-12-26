#include "sound.h"

#include <ioman.h>
#include <libsd.h>
#include <list.h>
#include <sysclib.h>

// just writing this down so i don't forget it
// might not use it for this project
#define ANGLE_BITS (1 << 8)
u8 angle_deg_to_bin(int deg) {
	return (deg * ANGLE_BITS) / 360; 
}
int angle_bin_to_deg(u16 angle) {
	return ((angle * 360) + (ANGLE_BITS / 2)) / ANGLE_BITS;
}

#define SPU_WAIT_FOR_TRANSFER 1
#define SAMPLE_PAD 16
#define SPU_BASE 0x5010

struct impSound sound_storage[48];

LIST_HEAD(playing_sounds);
LIST_HEAD(free_sounds);
static u32 spu_alloc;

void
imp_InitSound()
{
	spu_alloc = SPU_BASE;
	memset(sound_storage, 0, sizeof(sound_storage));

	for (int i = 0; i < 48; i++) {
		list_init(&sound_storage[i].list);
		list_insert(&free_sounds, &sound_storage[i].list);
	}
}
