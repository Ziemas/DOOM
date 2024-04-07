#ifndef GRAPH_H_
#define GRAPH_H_

#define GS_MODE_NTSC 0x2
#define GS_MODE_PAL 0x3

#define GS_NOINTERLACE 0
#define GS_INTERLACE 1

#define GS_FIELD 0
#define GS_FRAME 1

#define GS_PSMCT32  0
#define GS_PSMCT24  1
#define GS_PSMCT16  2
#define GS_PSMCT16S 10
#define GS_PSMT8    19
#define GS_PSMT4    20
#define GS_PSMT8H   27
#define GS_PSMT4HL  36
#define GS_PSMT4HH  44
#define GS_PSMZ32   48
#define GS_PSMZ24   49
#define GS_PSMZ16   50
#define GS_PSMZ16S  58

void graphReset(int inter, int mode, int ff);
void graphWaitVSync(void);

#endif // GRAPH_H_
