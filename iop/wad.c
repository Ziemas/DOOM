#include "wad.h"

#include "list.h"
#include "sound_data.h"
#include "sysmem.h"

#include <intrman.h>
#include <ioman.h>
#include <stdio.h>
#include <sysclib.h>

LIST_HEAD(wad_files);
struct wad_file {
	int fd;
	int lump_count;
	struct list_head wad_list;
	struct filelump lumps[0];
};

int
imp_AddFile(char *filename)
{
	struct wadinfo header;
	struct wad_file *wad;
	int fd, i, istate;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("failed to open wad\n");
		return -1;
	}

	read(fd, &header, sizeof(header));
	if (memcmp(&header.identification, "IWAD", 4) == 0 &&
	  memcmp(&header.identification, "PWAD", 4) == 0) {
		printf("unknown file format\n");
		return -1;
	}

	CpuSuspendIntr(&istate);
	wad = AllocSysMemory(ALLOC_LAST, sizeof(*wad) + sizeof(struct filelump) * header.numlumps,
	  NULL);
	CpuResumeIntr(istate);

	if (!wad) {
		printf("WAD alloc failed (too many lumps?)\n");
		return -1;
	}

	wad->fd = fd;
	wad->lump_count = header.numlumps;
	list_init(&wad->wad_list);

	lseek(fd, header.infotableofs, SEEK_SET);
	read(fd, wad->lumps, header.numlumps * sizeof(struct filelump));

	list_insert(&wad_files, &wad->wad_list);

	return 0;
}

static u32
toupper4_swar(u32 chars)
{
	u32 mask;
	u32 one = 0x01010101;
	u32 guard = one * 0x80;
	mask = ((((chars + one * 0x1F) & guard) >> 7) * 0xFF) &
	  ~((((chars + one * 0x05) & guard) >> 7) * 0xFF);
	chars -= mask & (one * 0x20);

	return chars;
}

int
imp_FindLump(const char *name, struct lumphandle *handle)
{
	struct wad_file *wad;
	struct filelump *file;
	u32 lname[2];
	u32 *wname;

	lname[0] = toupper4_swar(*(u32 *)&name[0]);
	lname[1] = toupper4_swar(*(u32 *)&name[4]);

	list_for_each (wad, &wad_files, wad_list) {
		file = wad->lumps;

		for (int i = 0; i < wad->lump_count; i++, file++) {
			wname = (u32 *)file->name;

			if (wname[0] == lname[0] && wname[1] == lname[1]) {
				handle->fd = wad->fd;
				handle->position = file->filepos;
				handle->size = file->size;

				return 1;
			}
		}
	}

	return 0;
}

int
imp_ReadLump(struct lumphandle *lump, u8 *dest)
{
	lseek(lump->fd, lump->position, SEEK_SET);
	read(lump->fd, dest, lump->size);

	return 0;
}

int
imp_IdentifyVersion()
{
	imp_AddFile("mass:doom.wad");
	imp_AddFile("host:doom.wad");

	return 0;
}
