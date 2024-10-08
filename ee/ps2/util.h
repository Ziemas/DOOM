#ifndef UTIL_H_
#define UTIL_H_

#include "types.h"

#define MASK(x) (BIT(x) - 1)

#define BIT(x) (1ULL << (x))
#define GENMASK(msb, lsb) ((BIT((msb + 1) - (lsb)) - 1) << (lsb))
#define FIELD_PREP(field, val) ((typeof(field))(val) << (__builtin_ffsll(field) - 1))
#define FIELD_GET(field, val) (((val) & (field)) >> (__builtin_ffsll(field) - 1))

#define CFC2(reg)                                                 \
	({                                                            \
		uint32 data;                                              \
		__asm__ volatile("cfc2 %0, $%1" : "=r"(data) : "i"(reg)); \
		data;                                                     \
	})

static inline uint64
read64(uint32 addr)
{
	volatile uint64 *a = (volatile uint64 *)addr;

	return *a;
}

static inline void
write64(uint32 addr, uint64 data)
{
	volatile uint64 *a = (volatile uint64 *)addr;

	*a = data;
}

static inline uint32
read32(uint32 addr)
{
	volatile uint32 *a = (volatile uint32 *)addr;

	return *a;
}

static inline void
write32(uint32 addr, uint32 data)
{
	volatile uint32 *a = (volatile uint32 *)addr;

	*a = data;
}

static inline uint32
set32(uint32 addr, uint32 set)
{
	volatile uint32 *a = (volatile uint32 *)addr;
	uint32 d = *a | set;

	*a = d;
	return d;
}

static inline uint32
clear32(uint32 addr, uint32 clear)
{
	volatile uint32 *a = (volatile uint32 *)addr;
	uint32 d = *a & ~clear;

	*a = d;
	return d;
}

static inline uint32
mask32(uint32 addr, uint32 clear, uint32 set)
{
	volatile uint32 *a = (volatile uint32 *)addr;
	uint32 d = *a;

	d = (d & ~clear) | set;

	*a = d;
	return d;
}

static inline int
poll32(uint32 addr, uint32 mask, uint32 target)
{
	while (1) {
		uint32 value = read32(addr) & mask;
		if (value == target) {
			return 0;
		}
	}
}

#endif // UTIL_H_
