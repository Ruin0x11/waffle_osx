/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  cgmfile.h
    CGモードデータ（MODE.CGM）管理用のクラス
=======================================================================*/

#ifndef _cgmfile_h
#define _cgmfile_h

#define MAXCG 512

class FLAGS;
class SYSTEM;

class CGMODE
{
private:
	unsigned int cgnum;
	unsigned char file[MAXCG][32];
	unsigned int flagbit[MAXCG];
	FLAGS* flags;
	SYSTEM* sys;
public:
	CGMODE(SYSTEM* s, FLAGS* flg);
	‾CGMODE(void);
	void SetFlag(char* f);
	int GetPercentage(void);
	int GetFlag(int n);
	char* GetCGName(int n);
	int GetCGAllNum(void);
	int GetCGNum(void);
	int GetCGFlagNum(int n);
};

#endif
