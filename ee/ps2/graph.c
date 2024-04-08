#include "graph.h"

#include "gs_regs.h"
#include "intc_regs.h"
#include "kernel.h"
#include "util.h"

struct GsCrtState {
	int16 inter, mode, ff;
};

struct GsCrtState gsCrtState;

void
graphReset(int inter, int mode, int ff)
{
	gsCrtState.inter = inter;
	gsCrtState.mode = mode;
	gsCrtState.ff = ff;

	write64(GS_R_CSR, GS_CSR_RESET);
	GsPutIMR(0xff00);
	SetGsCrt(inter & 1, mode & 0xff, ff & 1);
}

void
graphWaitVSync(void)
{
	uint32 oldintr = DIntr();
	write32(INTC_R_I_STAT, INTC_I_VBON);
	__asm__ volatile("sync");
	if (oldintr)
		EIntr();

	// wait for vbon flag to come back on
	while ((read32(INTC_R_I_STAT) & INTC_I_VBON) == 0)
		;

	oldintr = DIntr();
	write32(INTC_R_I_STAT, INTC_I_VBON);
	__asm__ volatile("sync");
	if (oldintr)
		EIntr();
}
