#include "dma.h"

#include "dmac_regs.h"
#include "gif_regs.h"
#include "gs_regs.h"
#include "util.h"
#include "vif_regs.h"

#include <kernel.h>
#include <stdio.h>

void
dmaInit(void)
{
	dmaResetPath();
	dmaResetDma();

	write32(GIF_R_MODE, 0);
	// write32(GIF_R_MODE, GIF_MODE_IMT); // IMT intermittet mode
}

void
dmaResetPath()
{
	// VIFcode reset routine
	static const uint32 init_vif_regs[] __attribute__((aligned(128))) = {
		0x01000404,
		0x20000000,
		0x00000000,
		0x05000000,
		0x06000000,
		0x03000000,
		0x02000000,
		0x04000000,
	};

	write32(VIF1_R_FBRST, VIF_FBRST_RST); // reset vif1
	write32(VIF1_R_ERR, VIF_ERR_ME0);	  // mask tag mismatch

	// Reset VU1 by setting RS1 in FBRST
	__asm__ volatile("sync          \n"
					 "cfc2 $2, $28  \n"
					 "ori $2, 0x200 \n"
					 "ctc2 $2, $28  \n"
					 "sync.p        \n"
					 :
					 :
					 : "2");

	// Put init_vif_regs into the VIF1 FIFO
	// If only TImode worked so we could do this from C...
	// idk if it has to be lq/sq though, maybe ld/sd works
	__asm__ volatile("lq $2, 0(%0)    \n"
					 "sq $2, 0(%1)    \n"
					 "lq $3, 0x10(%0) \n"
					 "sq $3, 0(%1)    \n"
					 :
					 : "d"(init_vif_regs), "d"(VIF1_R_FIFO)
					 : "2", "3");

	write32(GIF_R_CTRL, GIF_CTRL_RST); // reset gif
}

void
dmaResetDma()
{
	uint32 stat;

	write32(DMA_BASE_VIF0 + DMA_R_CHCR, 0);
	write32(DMA_BASE_VIF0 + DMA_R_MADR, 0);
	write32(DMA_BASE_VIF0 + DMA_R_TADR, 0);
	write32(DMA_BASE_VIF0 + DMA_R_SADR, 0);
	write32(DMA_BASE_VIF0 + DMA_R_ASR0, 0);
	write32(DMA_BASE_VIF0 + DMA_R_ASR1, 0);

	write32(DMA_BASE_VIF1 + DMA_R_CHCR, 0);
	write32(DMA_BASE_VIF1 + DMA_R_MADR, 0);
	write32(DMA_BASE_VIF1 + DMA_R_TADR, 0);
	write32(DMA_BASE_VIF1 + DMA_R_SADR, 0);
	write32(DMA_BASE_VIF1 + DMA_R_ASR0, 0);
	write32(DMA_BASE_VIF1 + DMA_R_ASR1, 0);

	write32(DMA_BASE_GIF + DMA_R_CHCR, 0);
	write32(DMA_BASE_GIF + DMA_R_MADR, 0);
	write32(DMA_BASE_GIF + DMA_R_TADR, 0);
	write32(DMA_BASE_GIF + DMA_R_SADR, 0);
	write32(DMA_BASE_GIF + DMA_R_ASR0, 0);
	write32(DMA_BASE_GIF + DMA_R_ASR1, 0);

	write32(DMA_R_STAT, 0xff1f);
	stat = read32(DMA_R_STAT);
	write32(DMA_R_STAT, stat & 0xff1f0000);

	write32(DMA_R_CTRL, 0);
	write32(DMA_R_PCR, 0);
	write32(DMA_R_SQWC, 0);
	write32(DMA_R_RBOR, 0);
	write32(DMA_R_RBSR, 0);

	set32(DMA_R_CTRL, DMA_CTRL_DMAE);
}

void
dmaPause(enum DMA_CHAN chan)
{
	uint32 oldintr = DIntr();
	uint32 enabler;

	enabler = read32(DMA_R_ENABLER);
	if ((enabler & DMA_ENABLER_CPND) == 0) {
		set32(DMA_R_ENABLER, DMA_ENABLER_CPND);
	}

	clear32(chan + DMA_R_CHCR, DMA_CHCR_STR);

	write32(DMA_R_ENABLER, enabler);

	if (oldintr)
		EIntr();
}

void
dmaWait(enum DMA_CHAN chan)
{
	int32 timeout = 0xffffff;

	while (read32(chan + DMA_R_CHCR) & DMA_CHCR_STR) {
		if (timeout < 0) {
			printf("dma sync timeout\n");
			dmaPause(chan);
		}
		timeout--;
	}
}

uint32
dmaCheckAddr(uint32 data)
{
	// Test if addr is on scratchpad
	if ((data >> 28) == 7) {
		// if so, set SPR bit
		data = (data & 0xfffffff) | BIT(31);
	}

	return data;
}

// VPU Control register
#define CCR_VPU_STAT 29
#define CCR_VPU_STAT_VBS1 BIT(8)

int
dmaSyncPath()
{
	uint32 vifmask;

	/* TODO bc0f */

	while (read32(DMA_BASE_VIF1 + DMA_R_CHCR) & DMA_CHCR_STR)
		;

	while (read32(DMA_BASE_GIF + DMA_R_CHCR) & DMA_CHCR_STR)
		;

	vifmask = VIF1_STAT_FQC | VIF1_STAT_VPS;
	while (read32(VIF1_R_STAT) & vifmask)
		;

	// check vbs1
	while (CFC2(CCR_VPU_STAT) & CCR_VPU_STAT_VBS1)
		;

	while (read32(GIF_R_STAT) & GIF_STAT_APATH)
		;

	return 0;
}

void
dmaSend(enum DMA_CHAN chan, struct dmaList *list)
{
	uint32 chcr = 0, addr = 0;

	FlushCache(0);

	addr = dmaCheckAddr((uint32)list->start);
	dmaWait(chan);

	write32(chan + DMA_R_TADR, (uint32)addr);
	write32(chan + DMA_R_QWC, 0);

	chcr |= FIELD_PREP(DMA_CHCR_DIR, DMA_CHCR_DIR_FROM_MEM);
	chcr |= FIELD_PREP(DMA_CHCR_MOD, DMA_CHCR_MOD_CHAIN);
	chcr |= FIELD_PREP(DMA_CHCR_TTE, 1);
	chcr |= FIELD_PREP(DMA_CHCR_TIE, 1);
	chcr |= FIELD_PREP(DMA_CHCR_STR, 1);
	write32(chan + DMA_R_CHCR, chcr);
}
