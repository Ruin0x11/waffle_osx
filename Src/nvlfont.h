/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  nvlfont.h
    NVL_SYSTEM=1 時のフォントデータのクラス
=======================================================================*/

#ifndef _nvlfont_h
#define _nvlfont_h

class SYSTEM;

class NOVELFONT
{
private:
	SYSTEM* sys;
	unsigned char* buf;
	int SJIS2JIS(int n);

public:
	NOVELFONT(SYSTEM* s);
	~NOVELFONT(void);
	unsigned char* GetFont(int num);
};

#endif
