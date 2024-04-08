#ifndef DMAC_REGS_H_
#define DMAC_REGS_H_

#include "util.h"

/* DMAC control registers */
#define DMA_R_CTRL    0x1000e000
#define DMA_R_STAT    0x1000e010
#define DMA_R_PCR     0x1000e020
#define DMA_R_SQWC    0x1000e030
#define DMA_R_RBOR    0x1000e050
#define DMA_R_RBSR    0x1000e040
#define DMA_R_STADR   0x1000e060
#define DMA_R_ENABLER 0x1000f520
#define DMA_R_ENABLEW 0x1000f590

/* Channel register bases */
#define DMA_BASE_VIF0     0x10008000
#define DMA_BASE_VIF1     0x10009000
#define DMA_BASE_GIF      0x1000A000
#define DMA_BASE_IPU_FROM 0x1000B000
#define DMA_BASE_IPU_TO   0x1000B400
#define DMA_BASE_SIF0     0x1000C000
#define DMA_BASE_SIF1     0x1000C400
#define DMA_BASE_SIF2     0x1000C800
#define DMA_BASE_SPR_FROM 0x1000D000
#define DMA_BASE_SPR_TO   0x1000D400

/* Channel register offsets */
#define DMA_R_CHCR 0x0
#define DMA_R_MADR 0x10
#define DMA_R_QWC  0x20
#define DMA_R_TADR 0x30
#define DMA_R_ASR0 0x40
#define DMA_R_ASR1 0x50
#define DMA_R_SADR 0x80

/* CHCR Fields */
#define DMA_CHCR_TAG          GENMASK(31, 16)
#define DMA_CHCR_STR          BIT(8)
#define DMA_CHCR_TIE          BIT(7)
#define DMA_CHCR_TTE          BIT(6)
#define DMA_CHCR_ASP          GENMASK(5, 4)
#define DMA_CHCR_MOD          GENMASK(3, 2)
#define DMA_CHCR_MOD_NORM     0
#define DMA_CHCR_MOD_CHAIN    1
#define DMA_CHCR_MOD_INTRL    2
#define DMA_CHCR_DIR          BIT(0)
#define DMA_CHCR_DIR_TO_MEM   0
#define DMA_CHCR_DIR_FROM_MEM 1

/* CTRL Fields */
#define DMA_CTRL_RCYC GENMASK(10, 8)
#define DMA_CTRL_STD  GENMASK(7, 6)
#define DMA_CTRL_STS  GENMASK(5, 4)
#define DMA_CTRL_MFD  GENMASK(3, 2)
#define DMA_CTRL_RELE BIT(1)
#define DMA_CTRL_DMAE BIT(0)

/* STAT Fields */
#define DMA_STAT_MEIM BIT(30)
#define DMA_STAT_SIM  BIT(29)
#define DMA_STAT_CIM  GENMASK(25, 16)
#define DMA_STAT_BEIS BIT(15)
#define DMA_STAT_MEIS BIT(14)
#define DMA_STAT_SIS  BIT(13)
#define DMA_STAT_CIS  GENMASK(9, 0)

/* PCR Fields */
#define DMA_PCR_PCE BIT(31)
#define DMA_PCR_CDE GENMASK(25, 16)
#define DMA_PCR_CPC GENMASK(9, 0)

/* SQWC Fields */
#define DMA_SQWC_TQWC GENMASK(23, 16)
#define DMA_SQWC_SQWC GENMASK(7, 0)

/* RBOR Fields */
#define DMA_RBOR_ADDR GENMASK(30, 0)

/* RBSR Fields */
#define DMA_RBSR_RMSK GENMASK(30, 4)

/* STADR Fields */
#define DMA_STADR_ADDR GENMASK(30, 0)

/* ENABLEW Fields */
#define DMA_ENABLEW_CPND BIT(16)

/* ENABLER Fields */
#define DMA_ENABLER_CPND BIT(16)

#endif // DMAC_REGS_H_
