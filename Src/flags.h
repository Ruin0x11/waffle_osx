/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  flags.h
    フラグ管理クラス
=======================================================================*/

#ifndef _flags_h
#define _flags_h

#define MAX_STACK       256

class FLAGS
{
private:
        int val[2000];
        unsigned char bit[250];
        char str[100][64];

        int stackindex;
        int stackseen[MAX_STACK];
        int stackpos[MAX_STACK];
        int savedstackindex;
        int savedstackseen[MAX_STACK];
        int savedstackpos[MAX_STACK];
public:
        FLAGS(void);
        ~FLAGS(void);

        int GetBit(int i);
        int GetVal(int i);
        char* GetStr(int i);

        void SetBit(int i, int n);
        void SetVal(int i, int n);
        void SetStr(int i, char* s);

        void PushStack(int seen, int pos);
        void PopStack(int* seen, int* pos);
        void ClearStack(void);
        int GetSavedStackNum(void);
        void GetSavedStackItem(int n, int* seen, int* pos);
        void SaveStack(void);
};

#endif
