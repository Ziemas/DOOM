#ifndef IMP_COM_H_
#define IMP_COM_H_

#include "types.h"

int  impcom_Init();
void impcom_FlushBuffer();
void impcom_StartSound(u32 handle, int id, int vol, int sep, int pitch, int priority);
void impcom_PlaySong(u32 handle, int music, int looping);
void impcom_SetSfxVolume(int volume);
void impcom_SetMusicVolume(int volume);

#endif // IMP_COM_H_
