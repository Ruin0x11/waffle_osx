/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtmgr.h
     PDT描画関連クラス
=======================================================================*/

#ifndef _pdtmgr_h
#define _pdtmgr_h

#define MAXPDT 32		// PDTバッファの最大数
#define WAKUPDT MAXPDT-1
#define MESWINPDT MAXPDT-2
#define BACKUPPDT MAXPDT-3
#define ANMPDT MAXPDT-4
#define HIDEPDT MAXPDT-5
#define EXFONTPDT MAXPDT-6

class SYSTEM;
class PDTBUFFER;
class PDTFILE;
class EFFECT;

class PDTMGR
{
private:
	SYSTEM* sys;
	PDTBUFFER* pdt[MAXPDT];
	PDTBUFFER* temppdt;
	PDTBUFFER* getputpdt;
	int* lines;

public:
	PDTMGR(SYSTEM* sys, PDTBUFFER* pdtb, unsigned char* b);
	~PDTMGR(void);

	void ScreenFade(unsigned int cmd, unsigned int count, int r, int g, int b);
	void ClearScreen(void);
	void LoadFile(char* f, int dst);
	void LoadBaseFile(char* f, int dst);
	void LoadCopy(char* f, int sx1, int sy1, int sx2, int sy2, int dx, int dy, int dstpdt, int flag);
	void Copy(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag);
	void CopyReverse(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag);
	void CopyBackBuffer(int sx1, int sy1, int sx2, int sy2, int src, int update);
	void MaskCopy(int sx1, int sy1, int sx2, int sy2, PDTBUFFER* src, int dx, int dy, int dst, int flag);
	void MaskCopy(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag);
	void CopyWithMask(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag);
	void MonoCopy(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int r, int g, int b);
	void FadeColor(int sx1, int sy1, int sx2, int sy2, int src, int c1, int c2, int c3, int count);
	void MakeMonochrome(int sx1, int sy1, int sx2, int sy2, int src);
	void MakeInvert(int sx1, int sy1, int sx2, int sy2, int src);
	void MakeColorMask(int sx1, int sy1, int sx2, int sy2, int src, int r, int g, int b);
	void StretchCopy(int sx1, int sy1, int sx2, int sy2, int src, int dx1, int dy1, int dx2, int dy2, int dst);
	void StretchCopy(int sx1, int sy1, int sx2, int sy2, PDTBUFFER* src, int dx1, int dy1, int dx2, int dy2, int dst);
	void Effect(EFFECT* effect);
	void FillRect(int sx1, int sy1, int sx2, int sy2, int srcpdt, int r, int g, int b);
	void ClearRect(int sx1, int sy1, int sx2, int sy2, int srcpdt, int r, int g, int b);
	void DrawRectLine(int sx1, int sy1, int sx2, int sy2, int srcpdt, int r, int g, int b);
	void AllCopy(int srcpdt, int dstpdt, int flag);
	void Swap(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst);
	void Get(int sx1, int sy1, int sx2, int sy2, int srcpdt);
	void Put(int dx, int dy, int dstpdt);
	void DrawString(int dx, int dy, int dstpdt, int r, int g, int b, char* s);

	PDTBUFFER* GetPDT(int i) { return pdt[i]; }
	void SetPDT(int i, PDTBUFFER* p) { pdt[i] = p; }
};

#endif
