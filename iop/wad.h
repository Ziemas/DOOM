#ifndef WAD_H_
#define WAD_H_

#include "types.h"

struct wadinfo {
	// Should be "IWAD" or "PWAD".
	char identification[4];
	int numlumps;
	int infotableofs;
};

struct filelump {
	int filepos;
	int size;
	char name[8];
};

struct lumphandle {
	int fd;
	int position;
	int size;
};

int imp_IdentifyVersion();
int imp_ReadLump(struct lumphandle *lump, u8 *dest);
int imp_FindLump(const char *name, struct lumphandle *res);

#endif // WAD_H_
