/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  debug.h
    デバッグ用ルーチン
=======================================================================*/

#ifndef _debug_h
#define _debug_h

//#define DEBUGLOG		// でばっぐログをとる？
#define LOGFILE			// ファイルにとる？
//#define EVERYCLOSE		// 毎回ファイルクローズする？（ハングする時のデバッグ用）

//#define WAITMOUSE while(!Button()){}while(Button()){}
#define WAITMOUSE

//#define FLAG_ERROR	// フラグのレンジエラーを警告する (->flags.cpp)

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
