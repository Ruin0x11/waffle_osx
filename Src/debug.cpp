/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  debug.cpp
    デバッグルーチン
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"


#ifdef DEBUGLOG
#ifdef LOGFILE
static FILE* debugfile;
#endif
#endif

bool IsLogging = false;
//bool IsLogging = true;


void DebugStart(void)
{
#ifdef DEBUGLOG
#ifdef LOGFILE
	debugfile = fopen("_debug.txt", "a");
#endif
#endif
}


void DebugEnd(void)
{
#ifdef DEBUGLOG
#ifdef LOGFILE
	if (debugfile) fclose(debugfile);
#endif
#endif
}


void dprintf(char* s)
{
#ifdef DEBUGLOG
	if ( !IsLogging ) return;
#ifdef LOGFILE
	if (debugfile) fprintf(debugfile, s);
#ifdef EVERYCLOSE
	fclose(debugfile);
	WAITMOUSE
	debugfile = fopen("_debug.txt", "a");
#endif
#else
	printf(s);
#endif
#endif
};

void dprintf(char* s, int arg1)
{
#ifdef DEBUGLOG
	if ( !IsLogging ) return;
#ifdef LOGFILE
	if (debugfile) fprintf(debugfile, s, arg1);
#ifdef EVERYCLOSE
	fclose(debugfile);
	WAITMOUSE
	debugfile = fopen("_debug.txt", "a");
#endif
#else
	printf(s, arg1);
#endif
#endif
};

void dprintf(char* s, int arg1, int arg2)
{
#ifdef DEBUGLOG
	if ( !IsLogging ) return;
#ifdef LOGFILE
	if (debugfile) fprintf(debugfile, s, arg1, arg2);
#ifdef EVERYCLOSE
	fclose(debugfile);
	WAITMOUSE
	debugfile = fopen("_debug.txt", "a");
#endif
#else
	printf(s, arg1, arg2);
#endif
#endif
};

void dprintf(char* s, int arg1, int arg2, int arg3)
{
#ifdef DEBUGLOG
	if ( !IsLogging ) return;
#ifdef LOGFILE
	if (debugfile) fprintf(debugfile, s, arg1, arg2, arg3);
#ifdef EVERYCLOSE
	fclose(debugfile);
	WAITMOUSE
	debugfile = fopen("_debug.txt", "a");
#endif
#else
	printf(s, arg1, arg2, arg3);
#endif
#endif
};

void dprintf(char* s, int arg1, int arg2, int arg3, int arg4)
{
#ifdef DEBUGLOG
	if ( !IsLogging ) return;
#ifdef LOGFILE
	if (debugfile) fprintf(debugfile, s, arg1, arg2, arg3, arg4);
#ifdef EVERYCLOSE
	fclose(debugfile);
	WAITMOUSE
	debugfile = fopen("_debug.txt", "a");
#endif
#else
	printf(s, arg1, arg2, arg3, arg4);
#endif
#endif
};

void dprintf(char* s, int arg1, int arg2, int arg3, int arg4, int arg5)
{
#ifdef DEBUGLOG
	if ( !IsLogging ) return;
#ifdef LOGFILE
	if (debugfile) fprintf(debugfile, s, arg1, arg2, arg3, arg4, arg5);
#ifdef EVERYCLOSE
	fclose(debugfile);
	WAITMOUSE
	debugfile = fopen("_debug.txt", "a");
#endif
#else
	printf(s, arg1, arg2, arg3, arg4, arg5);
#endif
#endif
};
