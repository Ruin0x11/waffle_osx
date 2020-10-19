/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtbuf.cpp
    PDTバッファ
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdtbuf.h"
#include "debug.h"

/************************************************************************
  class PDTBUF
    画像バッファ
************************************************************************/

PDTBUFFER::PDTBUFFER(int x, int y, int p, int l, bool flag)
{
	xsize = x;
	ysize = y;
	bpp = p;			// Bytes per Pixel
	bpl = l;			// Bytes per Line
	mask = NULL;
	buffer = NULL;
	if ( flag ) {
		buffer = new unsigned char[x*y*p];
		bufdel = true;	// デストラクタでバッファを消す
		memset(buffer, 0, x*y*p);
	} else {
		buffer = 0;
		bufdel = false;	// バッファを別確保してる時は消さない（表示PDT用）
	}
	mask = new unsigned char[x*y];
	memset(mask, 255, x*y);
};

PDTBUFFER::‾PDTBUFFER(void)
{
	if (bufdel) delete[] buffer;
	delete[] mask;
};
