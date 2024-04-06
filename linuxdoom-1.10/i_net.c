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
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include "d_event.h"
#include "d_net.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __GNUG__
#pragma implementation "i_net.h"
#endif
#include "i_net.h"

//
// I_InitNetwork
//
void
I_InitNetwork(void)
{
	boolean trueval = true;
	int i;
	int p;
	struct hostent *hostentry; // host information entry

	doomcom = malloc(sizeof(*doomcom));
	memset(doomcom, 0, sizeof(*doomcom));

	netgame = false;
	doomcom->id = DOOMCOM_ID;
	doomcom->numplayers = doomcom->numnodes = 1;
	doomcom->deathmatch = false;
	doomcom->consoleplayer = 0;
	doomcom->ticdup = 1;
}

void
I_NetCmd(void)
{
}
