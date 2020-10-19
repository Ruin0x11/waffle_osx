/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtmacro.h
    PDT操作セーブ用マクロのクラス
=======================================================================*/

#ifndef _pdtmacro_h
#define _pdtmacro_h

struct MACROITEM {
	int cmd;
	int filenum;
	char file[512];
	int arg[90];
};

class SYSTEM;
class PDTMGR;

class PDTMACRO
{
private:
	MACROITEM m[32];
	unsigned int num;
	unsigned int start;
	PDTMGR* mgr;

public:
	PDTMACRO(PDTMGR* m);
	~PDTMACRO(void);
	void Reset(void);
	void RedrawPDT(void);
	void StackMacro(MACROITEM* item);
	void DeleteMacro(int n);
	void ClearMacro(void);
	int GetMacroNum(void);
	MACROITEM* GetMacro(int n);
};

#endif
