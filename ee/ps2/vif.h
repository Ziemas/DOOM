#ifndef VIF_H_
#define VIF_H_

#include "util.h"

#define VIF_CODE_NOP      0x00
#define VIF_CODE_STCYCL   0x01
#define VIF_CODE_OFFSET   0x02
#define VIF_CODE_BASE     0x03
#define VIF_CODE_IOTP     0x04
#define VIF_CODE_STMOD    0x05
#define VIF_CODE_MSKPATH3 0x06
#define VIF_CODE_MARK     0x07
#define VIF_CODE_FLUSHE   0x10
#define VIF_CODE_FLUSH    0x11
#define VIF_CODE_FLUSHA   0x13
#define VIF_CODE_MSCAL    0x14
#define VIF_CODE_MSCALF   0x15
#define VIF_CODE_MSCNT    0x17
#define VIF_CODE_STMASK   0x20
#define VIF_CODE_STROW    0x30
#define VIF_CODE_STCOL    0x31
#define VIF_CODE_MPG      0x4A
#define VIF_CODE_DIRECT   0x50
#define VIF_CODE_DIRECTHL 0x51

#define VIF_C_IRQ BIT(31)
#define VIF_C_CMD GENMASK(30, 24)
#define VIF_C_NUM GENMASK(23, 16)
#define VIF_C_IMM GENMASK(15, 0)

#define VIFC_STCYCL_WL GENMASK(15, 8)
#define VIFC_STCYCL_CL GENMASK(7, 0)

#define VIFC_OFFSET_OF GENMASK(9, 0)

#define VIFC_BASE_B GENMASK(9, 0)

#define VIFC_ITOP_ADDR GENMASK(9, 0)

#define VIFC_STMOD_MODE GENMASK(1, 0)

#define VIFC_MSKPATH3_MASK BIT(15)

#define MAKE_VIF_CODE(_cmd, _num, _immediate) \
    (((uint32)(_cmd) << 24) | ((uint32)(_num) << 16) | ((uint32)(_immediate)))

#define VIF_NOP() (MAKE_VIF_CODE(VIF_CODE_NOP, 0, 0))
#define VIF_STCYCL(WL, CL) (MAKE_VIF_CODE(VIF_CODE_STCYCL, 0, (WL) << 8 | (CL)))
#define VIF_OFFSET(off) (MAKE_VIF_CODE(VIF_CODE_OFFSET, 0, off))
#define VIF_BASE(base) (MAKE_VIF_CODE(VIF_CODE_BASE, 0, base))
#define VIF_ITOP(addr) (MAKE_VIF_CODE(VIF_CODE_ITOP, 0, addr))
#define VIF_STMASK() (MAKE_VIF_CODE(VIF_CODE_STMASK, 0, 0))
#define VIF_STMOD(mode) (MAKE_VIF_CODE(VIF_CODE_STMOD, 0, mode))
#define VIF_MSKPATH3(mask) (MAKE_VIF_CODE(VIF_CODE_MSKPATH3, 0, ((mask) & 1) << 15))
#define VIF_FLUSH() (MAKE_VIF_CODE(VIF_CODE_FLUSH, 0, 0))
#define VIF_MSCAL(addr) (MAKE_VIF_CODE(VIF_CODE_MSCAL, 0, addr))
#define VIF_MSCALF(addr) (MAKE_VIF_CODE(VIF_CODE_MSCALF, 0, addr))
#define VIF_MSCNT(addr) (MAKE_VIF_CODE(VIF_CODE_MSCNT, 0, addr))
#define VIF_DIRECT(size) (MAKE_VIF_CODE(VIF_CODE_DIRECT, 0, size))
#define VIF_UNPACK(type, nq, offset) (MAKE_VIF_CODE(VIF_CODE_UNPACK, nq, offset))

#endif // VIF_H_
