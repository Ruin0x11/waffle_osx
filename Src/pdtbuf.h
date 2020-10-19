/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtbuf.h
    PDTバッファのクラス
=======================================================================*/

#ifndef _pdtbuf_h
#define _pdtbuf_h

class PDTBUFFER
{
private:
	unsigned char* buffer;
	unsigned char* mask;
	int xsize, ysize, bpp, bpl;
	bool bufdel;

public:
	PDTBUFFER(int x, int y, int p, int l, bool flag);
	~PDTBUFFER(void);
	int GetSizeX(void) {return xsize;}
	int GetSizeY(void) {return ysize;}
	int GetBPP(void) {return bpp;}
	int GetBPL(void) {return bpl;}
	void SetMaskBuffer(unsigned char* buf) {mask = buf;}
	void SetBuffer(unsigned char* buf) {buffer = buf;}
	unsigned char* GetMaskBuffer(void) {return mask;}
	unsigned char* GetBuffer(void) {return buffer;}
};

#endif
