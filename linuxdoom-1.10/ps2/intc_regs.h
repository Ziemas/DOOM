#ifndef INTC_REGS_H_
#define INTC_REGS_H_

#define INTC_R_I_STAT 0x1000f000
#define INTC_R_I_MASK 0x1000f010

/* Same bit layout for I_STAT/I_MASK */
#define INTC_I_GS    BIT(0)
#define INTC_I_SBUS  BIT(1)
#define INTC_I_VBON  BIT(2)
#define INTC_I_VBOFF BIT(3)
#define INTC_I_VIF0  BIT(4)
#define INTC_I_VIF1  BIT(5)
#define INTC_I_VU0   BIT(6)
#define INTC_I_VU1   BIT(7)
#define INTC_I_IPU   BIT(8)
#define INTC_I_TIM0  BIT(9)
#define INTC_I_TIM1  BIT(10)
#define INTC_I_TIM2  BIT(11)
#define INTC_I_TIM3  BIT(12)
#define INTC_I_SFIFO BIT(13)
#define INTC_I_VU0WD BIT(13)

#endif // INTC_REGS_H_
