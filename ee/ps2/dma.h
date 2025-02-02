#ifndef DMA_H_
#define DMA_H_

#include "assert.h"
#include "class.h"
#include "gif.h"
#include "types.h"
#include "vif.h"

#include <stdint.h>
#include <stdio.h>

#define UNCACHED   0x20000000
#define UNCACHED_A 0x30000000

#define UC_PTR(x) ((void*)((uintptr_t)(x) | UNCACHED))
#define UCA_PTR(x) ((void*)((uintptr_t)(x) | UNCACHED_A))

enum DMA_CHAN {
	DMA_CHAN_VIF0 = 0x10008000,
	DMA_CHAN_VIF1 = 0x10009000,
	DMA_CHAN_GIF = 0x1000A000,
};

#define DMA_TAG_ADDR GENMASK(63, 32)
#define DMA_TAG_IRQ BIT(31)
#define DMA_TAG_ID GENMASK(30, 28)
#define DMA_TAG_PCE GENMASK(27, 26)
#define DMA_TAG_QWC GENMASK(15, 0)

#define DMA_TAG_ID_REFE 0
#define DMA_TAG_ID_CNTS 0
#define DMA_TAG_ID_CNT 1
#define DMA_TAG_ID_NEXT 2
#define DMA_TAG_ID_REF 3
#define DMA_TAG_ID_REFS 4
#define DMA_TAG_ID_CALL 5
#define DMA_TAG_ID_RET 6
#define DMA_TAG_ID_END 7

struct dmaList {
	uint128 *start;
	uint128 *pos;
	uint32 limit;

	struct {
		uint8 type;
		uint128 *p;
	} stack[4];
	int8 sp;
};

void dmaInit();
void dmaResetPath();
void dmaResetDma();
void dmaPause(enum DMA_CHAN chan);
int dmaSyncPath();
void dmaSend(enum DMA_CHAN chan, struct dmaList *list);

static uint32
dmaListSize(struct dmaList *list)
{
	return list->pos - list->start;
}

static inline void
dmaStart(struct dmaList *list, uint128 *buf, uint32 size)
{
	list->start = buf;
	list->pos = buf;
	list->limit = size;
	list->sp = 0;
}

static void
dmaAdd(struct dmaList *list, uint128 q)
{
	uint32 size = dmaListSize(list);

	if (!(dmaListSize(list) < list->limit)) {
		printf("chain limit: %d %d\n", dmaListSize(list), list->limit);
	}

	assert(size < list->limit);

	*list->pos = q;
	list->pos++;
}

static inline void
dmaFinish(struct dmaList *list)
{
	dmaAdd(list, MAKE128(0x0, FIELD_PREP(DMA_TAG_ID, DMA_TAG_ID_END)));
}

// static inline uint128 *
// dmaSkip(struct dmaList *list, uint32 n)
//{
//	uint128 *p;
//
//	p = &list->pos[list->size];
//	list->size += n;
//	assert(list->size <= list->limit);
//	return p;
// }

static inline void
dmaAddD(struct dmaList *list, uint64 d0, uint64 d1)
{
	dmaAdd(list, MAKE128(d1, d0));
}

static inline void
dmaAddW(struct dmaList *list, uint32 w0, uint32 w1, uint32 w2, uint32 w3)
{
	dmaAdd(list, MAKEQ(w3, w2, w1, w0));
}

static inline void
dmaAddF(struct dmaList *list, float f0, float f1, float f2, float f3)
{
	uint32 w0 = *(uint32 *)&f0;
	uint32 w1 = *(uint32 *)&f1;
	uint32 w2 = *(uint32 *)&f2;
	uint32 w3 = *(uint32 *)&f3;
	dmaAddW(list, w0, w1, w2, w3);
}

static inline void
dmaAddGIFtag(struct dmaList *list, int nloop, int eop, int pre, int prim, int flg, int nreg,
  uint64 regs)
{
	dmaAdd(list, MAKE128(regs, GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg)));
}

static inline void
dmaAddAD(struct dmaList *list, uint64 a, uint64 d)
{
	dmaAdd(list, MAKE128(a, d));
}

static inline void
dmaRef(struct dmaList *list, void *data, uint16 qwc, uint32 w0, uint32 w1)
{
	dmaAdd(list,
	  MAKEQ(w1, w0, data, FIELD_PREP(DMA_TAG_ID, DMA_TAG_ID_REF) | FIELD_PREP(DMA_TAG_QWC, qwc)));
}

static inline void
dmaRefDirect(struct dmaList *list, void *data, uint16 qwc)
{
	dmaRef(list, data, qwc, VIF_NOP(), VIF_DIRECT(qwc));
}

static inline void
dmaCnt(struct dmaList *list, uint16 qwc, uint32 w0, uint32 w1)
{
	//	list->stack[list->sp].p = list->pos;
	//	list->stack[list->sp].type = DMA_TAG_ID_CNT;
	//	list->sp++;
	dmaAdd(list,
	  MAKEQ(w1, w0, 0, FIELD_PREP(DMA_TAG_ID, DMA_TAG_ID_CNT) | FIELD_PREP(DMA_TAG_QWC, qwc)));
	// assert(list->sp < 4);
}

// static inline void
// dmaCntEnd(struct dmaList *list)
//{
//	assert(list->stack[list->sp].type == DMA_TAG_ID_CNT);
//
//	list->sp--;
//	assert(list->sp >= 0);
// }

static inline void
dmaCntDirect(struct dmaList *list, uint16 qwc)
{
	dmaCnt(list, qwc, VIF_NOP(), VIF_DIRECT(qwc));
}

static inline void
dmaRet(struct dmaList *list, uint16 qwc, uint32 w0, uint32 w1)
{
	dmaAdd(list,
	  MAKEQ(w1, w0, 0, FIELD_PREP(DMA_TAG_ID, DMA_TAG_ID_RET) | FIELD_PREP(DMA_TAG_QWC, qwc)));
}

static inline void
dmaRetDirect(struct dmaList *list, uint16 qwc)
{
	dmaRet(list, qwc, VIF_NOP(), VIF_DIRECT(qwc));
}

// static inline void **
// dmaNext(struct dmaList *list, void *next, uint16 qwc, uint32 w0, uint32 w1)
//{
//	uint128 t = MAKEQ(w1, w0, next, DMAnext + qwc);
//	dmaAdd(list, t);
//	return (void *)(((uint32 *)&list->p[list->size - 1]) + 1);
// }
//
// static inline void **
// dmaCall(struct dmaList *list, uint16 qwc, void *addr, uint32 w0, uint32 w1)
//{
//	uint128 t = MAKEQ(w1, w0, addr, DMAcall + qwc);
//	dmaAdd(list, t);
//	return (void *)(((uint32 *)&list->p[list->size - 1]) + 1);
// }

#endif // DMA_H_
