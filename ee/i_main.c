//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

#include "d_main.h"
#include "doomdef.h"
#include "i_system.h"
#include "m_argv.h"

#include <loadfile.h>
#include <sifrpc.h>
#include <unistd.h>

extern unsigned int size_libsd_data;
extern unsigned int libsd_data[];

extern unsigned int size_usbd_data;
extern unsigned int usbd_data[];

extern unsigned int size_bdm_data;
extern unsigned int bdm_data[];

extern unsigned int size_bdmfs_fatfs_data;
extern unsigned int bdmfs_fatfs_data[];

extern unsigned int size_usbmass_bd_data;
extern unsigned int usbmass_bd_data[];

int
main(int argc, char **argv)
{
	int module;

	myargc = argc;
	myargv = argv;

	SifInitRpc(0);

	module = SifExecModuleBuffer(libsd_data, size_libsd_data, 0, NULL, NULL);
	if (module < 0) {
		I_Error("Failed to load libsd");
	}

	module = SifExecModuleBuffer(usbd_data, size_usbd_data, 0, NULL, NULL);
	if (module < 0) {
		I_Error("Failed to load usbd");
	}

	module = SifExecModuleBuffer(bdm_data, size_bdm_data, 0, NULL, NULL);
	if (module < 0) {
		I_Error("Failed to load bdm");
	}

	module = SifExecModuleBuffer(bdmfs_fatfs_data, size_bdmfs_fatfs_data, 0, NULL, NULL);
	if (module < 0) {
		I_Error("Failed to load bdmfs_fatfs");
	}

	module = SifExecModuleBuffer(usbmass_bd_data, size_usbmass_bd_data, 0, NULL, NULL);
	if (module < 0) {
		I_Error("Failed to load usbmass");
	}

	module = SifLoadModule("host:imp.irx", 0, NULL);
	if (module < 0) {
		I_Error("Failed to load IMP");
	}

	sleep(5); // Wait for mass device

	D_DoomMain();

	return 0;
}
