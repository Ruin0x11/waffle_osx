/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtfile.cpp
    PDTファイルへのアクセス
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdtfile.h"
#include "pdtbuf.h"
#include "common.h"
#include "debug.h"
#include "system.h"

/************************************************************************
  マクロみたいなの
************************************************************************/
inline int ReadInt(unsigned char* b, int pos)
{
	int ret;
	ret  =  b[pos];
	ret += (b[pos+1]<<8);
	ret += (b[pos+2]<<16);
	ret += (b[pos+3]<<24);
	return ret;
};

inline bool CheckHeader(unsigned char* b, char* header)
{
	int i = 0;
	while(header[i]) {
		if ( (char)(b[i]) != header[i] ) return false;
		i++;
	}
	return true;
};


/************************************************************************
  class PDTFILE
    画像ファイル. 圧縮されてるよ
************************************************************************/

PDTFILE::PDTFILE(char* fname, SYSTEM* sys)
{
	int num, srccount, i, bit, count, maskptr, n;
	unsigned char *repeat, *buf, *bufend, *src;
	unsigned char *srcbuf;
	unsigned char flag;
//	FILE* fp;
	int index[16];
//	unsigned char tmp[32];
	char f[256];

	for (i=0; fname[i]; i++) {
		if ( (fname[i]>='a')&&(fname[i]<='z') ) fname[i]-=0x20;
	}
	mask = NULL;
	buffer = NULL;
	srcbuf = 0;
/*
	fp = fopen(fname, "rb");
	if ( fp ) {
		fseek(fp, 0, 0);
		fread(tmp, 32, 1, fp);
		if ( CheckHeader(tmp, "PDT1\0") ) {
			filesize = ReadInt(tmp, 8);
			xsize    = ReadInt(tmp, 12);
			ysize    = ReadInt(tmp, 16);
			maskptr  = ReadInt(tmp, 28);
			size     = xsize * ysize;
			if ( filesize>32 ) {
				srcbuf = new unsigned char[filesize];
//			srcbuf = (unsigned char*)malloc(filesize);
				if ( srcbuf ) {
					fread(srcbuf+32, filesize-32, 1, fp);
					memcpy(srcbuf, tmp, 32);
				}
			}
		}
		fclose(fp);
	}
*/
	sprintf(f, "%s.PDT", fname);
	srcbuf = sys->ReadFile(f, "PDT", &filesize);
	if ( srcbuf ) {
/*if ( (!strcmp(f, "D_DITH.PDT")) ) {
FILE* o = fopen(f, "wb");
fwrite(srcbuf, 1, filesize, o);
fclose(o);
}*/
		if ( CheckHeader(srcbuf, "PDT1\0") ) {
			xsize   = ReadInt(srcbuf, 12);
			ysize   = ReadInt(srcbuf, 16);
			size    = xsize * ysize;
			maskptr = ReadInt(srcbuf, 28);
/*{
FILE* fp = fopen("_file.txt", "a");
fprintf(fp, "File:%s  Buf:$%08X\n", fname, (int)srcbuf);
for(i=0; i<32; i++) {
fprintf(fp, " %02X", srcbuf[i]);
}
fprintf(fp, "\nX=%d Y=%d Mask:$%08X\n", xsize, ysize, maskptr);
fclose(fp);
}*/
		} else {
			delete[] srcbuf;
			srcbuf = 0;
		}
	}

	if ( srcbuf ) {
		if ( maskptr ) {								// Maskがある場合
			mask = new unsigned char[size];
			memset(mask, 0, size);
			src = srcbuf+maskptr;
			bit = 0;
			srccount = filesize-maskptr;
			bufend = mask+size;
			buf = mask;
			while ( (buf<bufend)&&(srccount>0) ) {
				if (!bit) {
					bit = 8;
					flag = *src++;
					srccount--;
				}
				if ( flag&0x80 ) {
					*buf++ = *src++;
					srccount--;
				} else {
					count = (*src++)+2;
					repeat = buf-((*src++)+1);
					srccount -= 2;
					for (i=0; (i<count)&&(buf<bufend); i++) *buf++ = *repeat++;
				}
				bit--;
				flag <<= 1;
			}
		}

		size *= 3;	// RGB各1バイト
		buffer = new unsigned char[size];

		if ( !strcmp((char*)srcbuf, "PDT10") ) {
		// PDT10形式
			src = srcbuf+32;
			bit = 0;
			if ( maskptr )
				srccount = maskptr-32;
			else
				srccount = filesize-32;
			bufend = buffer+size;
			buf = buffer;
			while ( (buf<bufend)&&(srccount>0) ) {
				if (!bit) {
					bit = 8;
					flag = *src++;
					srccount--;
				}
				if ( flag&0x80 ) {
					*buf++ = *src++;
					*buf++ = *src++;
					*buf++ = *src++;
					srccount -= 3;
				} else {
					num  = *src++;
					num += ((*src++)<<8);			// 引数はリトルエンディアンのWORD
					srccount -= 2;
					count = ((num&15)+1)*3;			// 下位4bitがコピー数. 2回が最小単位？
					repeat = buf-((num>>4)+1)*3;	// bufは次の書き込み位置を指してるので、1バイト多めに戻る
					for (i=0; (i<count)&&(buf<bufend); i++) *buf++ = *repeat++;
				}
				bit--;
				flag <<= 1;
			}

		} else {
		// PDT11
			for (i=0; i<16; i++) {					// リピート数のIndexTable
				index[i]  = (srcbuf[i*4+0x420]);
				index[i] |= (srcbuf[i*4+0x421]<<8);
				index[i] |= (srcbuf[i*4+0x422]<<16);
				index[i] |= (srcbuf[i*4+0x423]<<24);
			}
			src = srcbuf+0x460;
			bit = 0;
			if ( maskptr )
				srccount = maskptr-32;
			else
				srccount = filesize-32;
			bufend = buffer+size;
			buf = buffer;
			while ( (buf<bufend)&&(srccount>0) ) {
				if (!bit) {
					bit = 8;
					flag = *src++;
					srccount--;
				}
				if ( flag&0x80 ) {
					n = (*src++)*4+0x20;
					*buf++ = srcbuf[n];
					*buf++ = srcbuf[n+1];
					*buf++ = srcbuf[n+2];
					srccount--;
				} else {
					num  = *src++;					// 引数はBYTE
					srccount--;
					count = (((num>>4)&15)+2)*3;	// 上位4bitがコピー数. 2回が最小単位？
					if ( (buf+count)>=bufend ) count = bufend-buf;
					repeat = buf-(index[num&15])*3;	// Indexで戻り位置をきめる？
					for (i=0; (i<count)&&(buf<bufend); i++) *buf++ = *repeat++;
				}
				bit--;
				flag <<= 1;
			}
		}

		delete[] srcbuf;
//		free(srcbuf);
	}
};

PDTFILE::~PDTFILE(void)
{
	delete[] buffer;
	delete[] mask;
};

void PDTFILE::CopyBuffer(PDTBUFFER* pdt)
{
	unsigned char *dst, *src, *srcbuf, *dstbuf, *msk1, *msk2;
	int dstx, dsty, bpl, bpp, x, y;
	int xx, yy;

	if ( !buffer ) return;

	dstx = pdt->GetSizeX();
	dsty = pdt->GetSizeY();
	bpl = pdt->GetBPL();
	bpp = pdt->GetBPP();
	dstbuf = pdt->GetBuffer();
	if ( bpp==4 ) dstbuf++;
	srcbuf = buffer;
	msk1 = pdt->GetMaskBuffer();
	msk2 = mask;

	xx = ((xsize<dstx)?xsize:dstx);
	yy = ((ysize<dsty)?ysize:dsty);

	for(y=0; y<yy; y++) {
		src = srcbuf;
		dst = dstbuf;
		for (x=0; x<xx; x++) {
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst += bpp;
			src += 3;
		}
		srcbuf += xsize*3;
		dstbuf += bpl;
	}
	if ( xx>0 ) {
		if (mask) {
			for(y=0; y<yy; y++) {
				memcpy(msk1, msk2, xx);
				msk1 += dstx;
				msk2 += xsize;
			}
		} else {
			for(y=0; y<yy; y++) {
				memset(msk1, 255, xx);
				msk1 += dstx;
			}
		}
	}
};
