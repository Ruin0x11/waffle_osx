/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  pdtmacro.cpp
    PDT操作セーブ用マクロのクラス
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdtmacro.h"
#include "pdtbuf.h"
#include "pdtmgr.h"
#include "system.h"
#include "debug.h"

/************************************************************************
  class PDTMACRO
    画像描画マクロバッファ
    マクロバッファはリング構造にする（メモリアクセス量を減らすため）
************************************************************************/

// こんすとらくた
PDTMACRO::PDTMACRO(PDTMGR* m)
{
	mgr = m;
	Reset();
};


// ですとらくた
PDTMACRO::‾PDTMACRO(void)
{
};


// リセット
void PDTMACRO::Reset(void)
{
	memset(&m, 0, 32*sizeof(MACROITEM));
	num = 0;
	start = 0;
}


// バッファに積まれたマクロに従って、画面を再構成する
void PDTMACRO::RedrawPDT(void)
{
	int i, j;
	int r, g, b;
	char* buf;
/*{
FILE* fp;
fp = fopen("_macro.log","a");
fprintf(fp, "Start:%d End:%d¥n", start, num);
fclose(fp);
}*/
	for (i=start; i!=num; ) {
/*{
FILE* fp;
fp = fopen("_macro.log","a");
fprintf(fp, "Cmd:$%02X FileNum:%d¥n", m[i].cmd, m[i].filenum);
if ( (m[i].cmd<0x30)||(m[i].cmd>0x90) ) {
buf = m[i].file;
for (j=0; j<m[i].filenum; j++) {
fprintf(fp, "  File:%s¥n", buf);
buf += (strlen(buf)+1);
}
}
fprintf(fp, "  Arg:¥n    ");
for (j=0; j<90; j++) {
fprintf(fp, "$%08X ", m[i].arg[j]);
if ( (j%20)==19 ) fprintf(fp, "¥n    ");
}
fprintf(fp, "¥n");
fclose(fp);
}*/
		switch(m[i].cmd) {
			case 0x00:
				//sys->PDT_FillRect(0, 0, 639, 479, 1, 255, 0, 255);
				mgr->LoadFile(m[i].file, 1);
				mgr->MaskCopy(m[i].arg[0], m[i].arg[1], m[i].arg[2], m[i].arg[3], 1, m[i].arg[4], m[i].arg[5], 0, 0);
				break;

			case 0x02:
				mgr->LoadFile(m[i].file, m[i].arg[26]);
				break;

			case 0x32:
				r = ((m[i].arg[24]>>16)&255);
				g = ((m[i].arg[24]>>8 )&255);
				b = ( m[i].arg[24]     &255);
				mgr->FillRect(m[i].arg[26], m[i].arg[27], m[i].arg[28], m[i].arg[29], m[i].arg[30], r, g, b);
				break;

			case 0x4c:
				mgr->FadeColor(m[i].arg[26], m[i].arg[27], m[i].arg[28], m[i].arg[29], m[i].arg[30], m[i].arg[31], m[i].arg[32], m[i].arg[33], m[i].arg[34]);
				break;

			case 0x64:
				mgr->Copy(m[i].arg[26], m[i].arg[27], m[i].arg[28], m[i].arg[29], m[i].arg[30], m[i].arg[31], m[i].arg[32], m[i].arg[33], m[i].arg[34]);
				break;

			case 0x66:
				mgr->MaskCopy(m[i].arg[26], m[i].arg[27], m[i].arg[28], m[i].arg[29], m[i].arg[30], m[i].arg[31], m[i].arg[32], m[i].arg[33], m[i].arg[34]);
				break;

			case 0x68:
				r = ((m[i].arg[34]>>16)&255);
				g = ((m[i].arg[34]>>8 )&255);
				b = ( m[i].arg[34]     &255);
				mgr->MonoCopy(m[i].arg[26], m[i].arg[27], m[i].arg[28], m[i].arg[29], m[i].arg[30], m[i].arg[31], m[i].arg[32], m[i].arg[32], r, g, b);
				break;

			case 0x6a:
				mgr->Swap(m[i].arg[26], m[i].arg[27], m[i].arg[28], m[i].arg[29], m[i].arg[30], m[i].arg[31], m[i].arg[32], m[i].arg[33]);
				break;

			case 0x96:
				buf = m[i].file;
				mgr->LoadBaseFile(buf, 1);
				buf += (strlen(buf)+1);
				for (j=0; j<(m[i].filenum-1); j++) {
					switch(m[i].arg[26+j*8]) {
//						case 2:
//							mgr->LoadCopy(buf, m[i].arg[27+j*8], m[i].arg[28+j*8], m[i].arg[29+j*8], m[i].arg[30+j*8], m[i].arg[31+j*8], m[i].arg[32+j*8], 1, 0);
//							break;
						case 3:
							mgr->LoadCopy(buf, m[i].arg[27+j*8], m[i].arg[28+j*8], m[i].arg[29+j*8], m[i].arg[30+j*8], m[i].arg[31+j*8], m[i].arg[32+j*8], 1, m[i].arg[33+j*8]);
							break;
						default:
							mgr->LoadCopy(buf, 0, 0, 639, 479, 0, 0, 1, 0);
							break;
					}
					buf += (strlen(buf)+1);
				}
				mgr->MaskCopy(0, 0, 639, 479, 1, 0, 0, 0, 0);
				break;

			case 0x97:
				buf = m[i].file;
				j = atoi(m[i].file);
				mgr->AllCopy(j, 1, 0);
				buf += (strlen(buf)+1);
				for (j=0; (j<m[i].filenum-1); j++) {
					switch(m[i].arg[26+j*8]) {
//						case 2:
//							mgr->LoadCopy(buf, m[i].arg[27+j*8], m[i].arg[28+j*8], m[i].arg[29+j*8], m[i].arg[30+j*8], m[i].arg[31+j*8], m[i].arg[32+j*8], 1, 0);
//							break;
						case 3:
							mgr->LoadCopy(buf, m[i].arg[27+j*8], m[i].arg[28+j*8], m[i].arg[29+j*8], m[i].arg[30+j*8], m[i].arg[31+j*8], m[i].arg[32+j*8], 1, m[i].arg[33+j*8]);
							break;
						default:
							mgr->LoadCopy(buf, 0, 0, 639, 479, 0, 0, 1, 0);
							break;
					}
					while(*buf) buf++;
					buf++;
				}
				mgr->MaskCopy(0, 0, 639, 479, 1, 0, 0, 0, 0);
				break;

			default:
				break;
		}
		i = (i+1)&31;
	}
};


// マクロを1個バッファに積む
void PDTMACRO::StackMacro(MACROITEM* item)
{
/*{
FILE* fp;
fp = fopen("_macro.log","a");
fprintf(fp, "Macro Stacked to #%d. Cmd:$%02X¥n", num, item->cmd);
if ( (item->cmd==0)||(item->cmd==2) )
fprintf(fp, "    File:%s¥n", item->file);
fclose(fp);
}*/
	memcpy(&m[num], item, sizeof(MACROITEM));
	num = (num+1)&31;
	if ( start==num ) start = (start+1)&31;
};


// マクロを（後ろから）n個消し、バッファを空ける
void PDTMACRO::DeleteMacro(int /*n*/)
{
//	int i;
//	for (i=0; ((i<n)&&(num!=start)); i++) {
//		num = (num-1)&31;
//	}
/*{
FILE* fp;
fp = fopen("_macro.log","a");
fprintf(fp, "%d Macros Deleted.¥n", n);
fclose(fp);
}*/
};


// 総てのマクロを消去
void PDTMACRO::ClearMacro(void)
{
//	memset(m, 0, num*sizeof(MACROITEM));
//	num = start = 0;
/*{
FILE* fp;
fp = fopen("_macro.log","a");
fprintf(fp, "All Macro Deleted.¥n");
fclose(fp);
}*/
};


// 現在ストックしているマクロの総数をかえす
int PDTMACRO::GetMacroNum(void)
{
	int ret;
	ret = ((num-start)&31);
	return ret;
};


// n番のマクロへのポインタをかえす
MACROITEM* PDTMACRO::GetMacro(int n)
{
	int ret;
	ret = ((start+n)&31);
	return &m[ret];
};
