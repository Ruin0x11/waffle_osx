/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  debug.h
    �f�o�b�O�p���[�`��
=======================================================================*/

#ifndef _debug_h
#define _debug_h

//#define DEBUGLOG		// �ł΂������O���Ƃ�H
#define LOGFILE			// �t�@�C���ɂƂ�H
//#define EVERYCLOSE		// ����t�@�C���N���[�Y����H�i�n���O���鎞�̃f�o�b�O�p�j

//#define WAITMOUSE while(!Button()){}while(Button()){}
#define WAITMOUSE

//#define FLAG_ERROR	// �t���O�̃����W�G���[���x������ (->flags.cpp)

extern bool IsLogging;

void DebugStart(void);
void DebugEnd(void);

void dprintf(char* s);
void dprintf(char* s, int arg1);
void dprintf(char* s, int arg1, int arg2);
void dprintf(char* s, int arg1, int arg2, int arg3);
void dprintf(char* s, int arg1, int arg2, int arg3, int arg4);
void dprintf(char* s, int arg1, int arg2, int arg3, int arg4, int arg5);

#endif
