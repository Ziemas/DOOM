#ifndef GIF_H_
#define GIF_H_

#include "util.h"

#define GIF_BLOCK_SIZE 0x7FFF

#define GIF_PACKED    0
#define GIF_REGLIST   1
#define GIF_IMAGE     2
#define GIF_PACKED_AD 0x0e

#define GIF_TAG_NREG  GENMASK(63, 60)
#define GIF_TAG_FLG   GENMASK(59, 58)
#define GIF_TAG_PRIM  GENMASK(57, 47)
#define GIF_TAG_PRE   BIT(46)
#define GIF_TAG_EOP   BIT(15)
#define GIF_TAG_NLOOP GENMASK(14, 0)

#define GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg)                           \
        (FIELD_PREP(GIF_TAG_NLOOP, (nloop)) | FIELD_PREP(GIF_TAG_EOP, (eop)) |  \
            FIELD_PREP(GIF_TAG_PRE, (pre)) | FIELD_PREP(GIF_TAG_PRIM, (prim)) | \
            FIELD_PREP(GIF_TAG_FLG, (flg)) | FIELD_PREP(GIF_TAG_NREG, (nreg)))

#endif // GIF_H_
