/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtbuf.cpp
    PDT�o�b�t�@
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdtbuf.h"
#include "debug.h"

/************************************************************************
  class PDTBUF
    �摜�o�b�t�@
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
		bufdel = true;	// �f�X�g���N�^�Ńo�b�t�@������
		memset(buffer, 0, x*y*p);
	} else {
		buffer = 0;
		bufdel = false;	// �o�b�t�@��ʊm�ۂ��Ă鎞�͏����Ȃ��i�\��PDT�p�j
	}
	mask = new unsigned char[x*y];
	memset(mask, 255, x*y);
};

PDTBUFFER::~PDTBUFFER(void)
{
	if (bufdel) delete[] buffer;
	delete[] mask;
};
