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
//	Fixed point implementation.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include "doomtype.h"
#include "stdlib.h"

#include <stdint.h>

#ifdef CHECK_MATH
#include <assert.h>
#include <stdio.h>
#endif

#ifdef __GNUG__
#pragma implementation "m_fixed.h"
#endif
#include "m_fixed.h"

// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul(fixed_t a, fixed_t b)
{
	fixed_t res;

	__asm__("mult %0, %1, %2 \n"
			"srl %0, %3\n"
			"mfhi $v1 \n"
			"sll $v1, %3\n"
			"or %0, $v1 \n"
			: "=d"(res)
			: "d"(a), "d"(b), "i"(FRACBITS)
			: "3");

#ifdef CHECK_MATH
	fixed_t ref = ((long long)a * (long long)b) >> FRACBITS;
	assert(ref == res);
#endif

	return res;
}

//
// FixedDiv, C version.
//
//

fixed_t
FixedDiv(fixed_t a, fixed_t b)
{
	if ((abs(a) >> 14) >= abs(b))
		return (a ^ b) < 0 ? MININT : MAXINT;
	return FixedDiv2(a, b);
}

/* borrowed from the linux kernel */
static inline uint32_t
div64_32(uint64_t n, uint32_t base)
{
	uint64_t rem = n;
	uint64_t b = base;
	uint64_t res, d = 1;
	uint32_t high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64_t)high << 32;
		rem -= (uint64_t)(high * base) << 32;
	}

	while ((int64_t)b > 0 && b < rem) {
		b = b + b;
		d = d + d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	return res;
}

fixed_t
FixedDiv2(fixed_t a, fixed_t b)
{
	int neg = (a < 0) ^ (b < 0);
	uint64_t res = div64_32(llabs((int64_t)a << 16), abs(b));

	if (neg) {
		res = -res;
	}

#ifdef CHECK_MATH
	long long c;
	c = ((long long)a << 16) / ((long long)b);
	assert(c == res);
#endif

	return res;
}
