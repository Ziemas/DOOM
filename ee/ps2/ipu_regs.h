#ifndef IPU_REGS_H_
#define IPU_REGS_H_

#include "util.h"

#define IPU_R_CMD  0x10002000
#define IPU_R_CTRL 0x10002010
#define IPU_R_BP   0x10002020
#define IPU_R_TOP  0x10002030

/* CTRL IDP */
#define IPU_IDP_8BIT  0
#define IPU_IDP_9BIT  1
#define IPU_IDP_10BIT 2

/* CTRL AS */
#define IPU_AS_ZIGZAG 0
#define IPU_AS_ALT    1

/* CTRL IVF */
#define IPU_IVF_NONINTRA 0
#define IPU_IVF_INTRA    1

/* CTRL QST */
#define IPU_QST_LINEAR    0
#define IPU_QST_NONLINEAR 1

/* CTRL MP1 */
#define IPU_MP1_MPEG2 0
#define IPU_MP1_MPEG1 1

/* CTRL PCT */
#define IPU_PCT_IPIC 1
#define IPU_PCT_PPIC 2
#define IPU_PCT_BPIC 3
#define IPU_PCT_DPIC 4

/* IPU_R_CMD WRITE Fields */
#define IPU_CMD_CODE   GENMASK(31, 28)
#define IPU_CMD_OPTION GENMASK(27, 0)

/* IPU_R_CMD READ Fields */
#define IPU_CMD_BUSY BIT(63)
#define IPU_CMD_DATA GENMASK(31, 0)

/* IPU_R_CTRL Fields */
#define IPU_CTRL_BUSY BIT(31)
#define IPU_CTRL_RST  BIT(30)
#define IPU_CTRL_PCT  GENMASK(26, 24)
#define IPU_CTRL_MP1  BIT(23)
#define IPU_CTRL_QST  BIT(22)
#define IPU_CTRL_IVF  BIT(21)
#define IPU_CTRL_AS   BIT(20)
#define IPU_CTRL_IDP  GENMASK(17, 16)
#define IPU_CTRL_SCD  BIT(15)
#define IPU_CTRL_ECD  BIT(14)
#define IPU_CTRL_CBP  GENMASK(13, 8)
#define IPU_CTRL_OFC  GENMASK(7, 4)
#define IPU_CTRL_IFC  GENMASK(3, 0)

/* IPU_R_BP Fields */
#define IPU_BP_FP  GENMASK(17, 16)
#define IPU_BP_IFC GENMASK(11, 8)
#define IPU_BP_BP  GENMASK(6, 0)

/* IPU_R_TOP Fields */
#define IPU_TOP_BUSY  BIT(63)
#define IPU_TOP_BSTOP GENMASK(31, 0)

#endif // IPU_REGS_H_
