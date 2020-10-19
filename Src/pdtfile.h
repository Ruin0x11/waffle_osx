/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtfile.h
    PDTファイルへのアクセスクラス
=======================================================================*/

#ifndef _fileio_h
#define _fileio_h

class PDTBUFFER;
class SYSTEM;

class PDTFILE
{
private:
	unsigned char *buffer, *mask;
	int filesize, size, xsize, ysize;
//	unsigned char srcbuf[640*480*3];

public:
	PDTFILE(char* fname, SYSTEM* sys);
	~PDTFILE(void);
	void CopyBuffer(PDTBUFFER* pdt);
	unsigned char* GetBuffer(void) { return buffer; }
	unsigned char* GetMaskBuffer(void) { return mask; }
	int GetSizeX(void) { return xsize; }
	int GetSizeY(void) { return ysize; }
};

#endif
