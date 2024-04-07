#ifndef GIF_REGS_H_
#define GIF_REGS_H_

#include "util.h"

#define GIF_R_CTRL  0x10003000
#define GIF_R_MODE  0x10003010
#define GIF_R_STAT  0x10003020
#define GIF_R_TAG0  0x10003040
#define GIF_R_TAG1  0x10003050
#define GIF_R_TAG2  0x10003060
#define GIF_R_TAG3  0x10003070
#define GIF_R_CNT   0x10003080
#define GIF_R_P3CNT 0x10003090
#define GIF_R_P3TAG 0x100030A0
#define GIF_R_FIFO  0x10006000

/* GIF_R_CTRL Fields */
#define GIF_CTRL_PSE BIT(3)
#define GIF_CTRL_RST BIT(0)

/* GIF_R_MODE Fields */
#define GIF_MODE_IMT BIT(2)
#define GIF_MODE_M3R BIT(0)

/* GIF_R_STAT Fields */
#define GIF_STAT_FQC   GENMASK(28, 24)
#define GIF_STAT_DIR   BIT(12)
#define GIF_STAT_APATH GENMASK(11, 10)
#define GIF_STAT_OPH   BIT(9)
#define GIF_STAT_P1Q   BIT(8)
#define GIF_STAT_P2Q   BIT(7)
#define GIF_STAT_P3Q   BIT(6)
#define GIF_STAT_IP3   BIT(5)
#define GIF_STAT_PSE   BIT(3)
#define GIF_STAT_IMT   BIT(2)
#define GIF_STAT_M3P   BIT(1)
#define GIF_STAT_M3R   BIT(0)

/* GIF_R_TAG0 Fields */
#define GIF_TAG0_TAG   GENMASK(31, 16)
#define GIF_TAG0_EOP   BIT(15)
#define GIF_TAG0_NLOOP GENMASK(14, 0)

/* GIF_R_TAG1 Fields */
#define GIF_TAG1_NREG GENMASK(31, 28)
#define GIF_TAG1_FLG  GENMASK(27, 26)
#define GIF_TAG1_PRIM GENMASK(25, 15)
#define GIF_TAG1_PRE  BIT(14)
#define GIF_TAG1_TAG  GENMASK(13, 0)

/* GIF_R_CNT Fields */
#define GIF_CNT_VUADDR  GENMASK(29, 20)
#define GIF_CNT_REGCNT  GENMASK(19, 16)
#define GIF_CNT_LOOPCNT GENMASK(14, 0)

/* GIF_R_P3CNT Fields */
#define GIF_P3CNT_P3CNT GENMASK(14, 0)

/* GIF_R_P3TAG Fields */
#define GIF_P3TAG_EOP     BIT(15)
#define GIF_P3TAG_LOOPCNT GENMASK(14, 0)

#endif // GIF_REGS_H_
