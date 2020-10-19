/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  flags.cpp
    フラグ管理
=======================================================================*/

#include <stdio.h>
#include <string.h>

#include "flags.h"
#include "common.h"
#include "debug.h"

/************************************************************************
  class FLAGS
    フラグ管理
************************************************************************/

FLAGS::FLAGS(void)
{
        int i;
        memset(val, 0, sizeof(int)*2000);
        memset(bit, 0, 250);
        for (i=0; i<100; i++) memset(str[i], 0, 64);
        stackindex = 0;
        savedstackindex = 0;
};

FLAGS::‾FLAGS(void)
{
};


int FLAGS::GetBit(int i)
{
        if ( (i>=0)&&(i<2000) ) {
                if ( (bit[i>>3]&(0x80>>(i&7))) ) return 1;
        } else {
#ifdef FLAG_ERROR
                Error("GetBit: out of range.");
#endif
        }
        return 0;
}


int FLAGS::GetVal(int i)
{
        if ( (i>=0)&&(i<2000) ) {
                return val[i];
        } else {
#ifdef FLAG_ERROR
                Error("GetVal: out of range.");
#endif
        }
        return 0;
}


char* FLAGS::GetStr(int i)
{
        if ( (i>=0)&&(i<100) ) {
                return str[i];
        } else {
#ifdef FLAG_ERROR
                Error("GetStr: out of range.");
#endif
        }
        return 0;
}


void FLAGS::SetBit(int i, int n)
{
        if ( (i>=0)&&(i<2000) ) {
                if ( n )
                        bit[i>>3] |= (0x80>>(i&7));
                else
                        bit[i>>3] &= ‾(0x80>>(i&7));
        } else {
#ifdef FLAG_ERROR
                Error("SetBit: out of range.");
#endif
        }
}



void FLAGS::SetVal(int i, int n)
{
        if ( (i>=0)&&(i<2000) ) {
                val[i] = n;
        } else {
#ifdef FLAG_ERROR
                Error("SetVal: out of range.");
#endif
        }
}


void FLAGS::SetStr(int i, char* s)
{
        if ( (i>=0)&&(i<100) ) {
        	if ( s ) {
                strncpy(str[i], s, 64);
                str[i][63] = 0;
			} else {
                str[i][0] = 0;
			}
        } else {
#ifdef FLAG_ERROR
                Error("GetStr: out of range.");
#endif
        }
}


void FLAGS::PushStack(int seen, int pos)
{
        if ( stackindex<MAX_STACK ) {
                stackseen[stackindex] = seen;
                stackpos[stackindex] = pos;
                stackindex++;
        } else {
                Error("PushStack: Stack overflow.");
        }
}


void FLAGS::PopStack(int* seen, int* pos)
{
        if ( stackindex>0 ) {
                stackindex--;
                *seen = stackseen[stackindex];
                *pos = stackpos[stackindex];
        } else {
                Error("PopStack: Stack underflow.");
        }
}


void FLAGS::ClearStack(void)
{
        stackindex = 0;
}


int FLAGS::GetSavedStackNum(void)
{
        return savedstackindex;
}


void FLAGS::GetSavedStackItem(int n, int* seen, int* pos)
{
        if ( n<MAX_STACK ) {
                *seen = savedstackseen[n];
                *pos = savedstackpos[n];
        } else {
                Error("GetStackItem: out of range.");
        }
}


void FLAGS::SaveStack(void)
{
        savedstackindex = stackindex;
        memcpy(savedstackseen, stackseen, sizeof(stackseen));
        memcpy(savedstackpos, stackpos, sizeof(stackpos));
}
