/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  scenario.cpp
    シナリオデコーダ
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "scenario.h"
#include "flags.h"
#include "pdtbuf.h"
#include "pdtfile.h"
#include "soundmgr.h"
#include "pdtmgr.h"
#include "mousectl.h"

/************************************************************************
  class SCENARIO
    実行中のシナリオデータのクラス
************************************************************************/

// 最悪〜
bool SCENARIO::Decode(void)
{
	switch(cmd) {
		case 0x01: return d00();
		case 0x02: return d01();
		case 0x03: return d02();
		case 0x04: return d03();
		case 0x05: return d04();
		case 0x06: return d05();
		case 0x08: return d06();
		case 0x0b: return d07();
		case 0x0c: return d08();
		case 0x0e: return d09();
		case 0x10: return d0a();
		case 0x13: return d0b();
		case 0x15: return d0c();
		case 0x16: return d0d();
		case 0x17: return d0e();
		case 0x18: return d0f();
		case 0x19: return d10();
		case 0x1a: return d11();
		case 0x1b: return d12();
		case 0x1c: return d13();
		case 0x1d: case 0x1e: return d14();
		case 0x20: return d15();
		case 0x22: case 0x23: case 0x24: case 0x25:
		case 0x26: case 0x27: case 0x28: case 0x29: return d16();
		case 0x2c: return d17();
		case 0x2d: return d18();
		case 0x2e: return d19();
		case 0x2f: return d1a();
		case 0x30: return d1b();
		case 0x31: return d1c();
		case 0x37: case 0x39: case 0x3b: case 0x3c:
		case 0x3d: case 0x3e: case 0x3f: case 0x40:
		case 0x41: case 0x42: case 0x43: case 0x49:
		case 0x4a: case 0x4b: case 0x4c: case 0x4d:
		case 0x4e: case 0x4f: case 0x50: case 0x51:
		case 0x56: case 0x57: return d1d();
		case 0x58: return d1e();
		case 0x59: return d1f();
		case 0x5b: return d20();
		case 0x5c: return d21();
		case 0x5d: return d22();
		case 0x5e: return d23();
		case 0x5f: return d24();
		case 0x60: return d25();
		case 0x61: return d26();
		case 0x63: return d27();
		case 0x64: case 0x65: case 0x67: case 0x68:
		case 0x69: case 0x6a: return d28();
		case 0x66: return d29();
		case 0x6c: return d2a();
		case 0x6d: return d2b();
		case 0x6e: return d2c();
		case 0x6f: return d2d();
		case 0x70: return d2e();
		case 0x72: return d2f();
		case 0x73: return d30();
		case 0x74: return d31();
		case 0x75: return d32();
		case 0x76: return d33();
		case 0x7f: return d34();
		case 0xfe: return d35();
		case 0xff: return d36();
	}
	return d37();
}


/* -------------------------------------------------------------------
  デコードルーチン集
  3216系は結構洗い出したと思うんだけど、3217系での洗い出しがまだ〜
------------------------------------------------------------------- */

#define ERROR dprintf("¥n************************************¥nError!!!!!!¥n************************************¥n")

// -------------------------------------------------------------------
// 00 - Cmd:$01 Text:マウスクリック待ち（クリック待ちアイコン付き）
// -------------------------------------------------------------------

bool SCENARIO::d00(void)
{
	// NVL_SYSTEM = 0 時はクリック後にテキストを消去する模様。
	// NVL_SYSTEM = 1 時はクリック後に改行。
	if ( sys->CheckSkip() ) {
		cmd = 0;
		sys->MesWin_HideIcon();
		mouse->TextIconEnd();
		mesflag = sys->MesWin_LineFeed();
		SavePoint();
		sys->PlaySE(3);			// SE.003 が鳴るらしい
	} else {
		sys->MesWin_DrawIcon(0);
		mouse->TextIconStart();
		if ( mouse->GetButton() ) {
			cmd = 0;
			sys->MesWin_HideIcon();
			mouse->TextIconEnd();
			mesflag = sys->MesWin_LineFeed();
			SavePoint();
			sys->PlaySE(3);		// SE.003 が鳴るらしい
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 01 - Cmd:$02 Text:改行
// -------------------------------------------------------------------
bool SCENARIO::d01(void)
{
	unsigned char subcmd;
	char buf[2];

	subcmd = databuf[curpos++];

	switch (subcmd) {
		case 1:					// 改行後、インデントは0に戻る
			buf[0] = 0x0d;
			buf[1] = 0x00;
			sys->MesWin_SetMes(buf);
			cmd = 0;
			break;
		case 2:					// 改行後も改行前のインデントを使う
			buf[0] = 0x0d;
			buf[1] = 0x00;
			sys->MesWin_SetMes(buf);
			cmd = 0;
			break;
		case 3:					// 謎（すわっぷAふぉ〜B）
			buf[0] = 0x0d;
			buf[1] = 0x00;
			sys->MesWin_SetMes(buf);
			cmd = 0;
			break;
	}
	return false;
};


// -------------------------------------------------------------------
// 02 - Cmd:$03 Text:クリック待ち。クリック後はメッセージをクリアせずに続けて書く
// -------------------------------------------------------------------
bool SCENARIO::d02(void)
{
	if ( sys->CheckSkip() ) {
		cmd = 0;
		sys->MesWin_HideIcon();
		mouse->TextIconEnd();
		SavePoint();
	} else {
		mouse->TextIconStart();
		sys->MesWin_DrawIcon(0);
		if ( mouse->GetButton() ) {
			cmd = 0;
			sys->MesWin_HideIcon();
			mouse->TextIconEnd();
			SavePoint();
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 03 - Cmd:$04 Text:ウィンドウ消去系
// -------------------------------------------------------------------
bool SCENARIO::d03(void)
{
	unsigned char subcmd;

	subcmd = databuf[curpos];
	switch (subcmd) {
		case 0x01:			// ウィンドウ消去。メッセージバッファもクリア
			dprintf("Text - Hide Window (subcmd $%02X)¥n", subcmd);
			sys->MesWin_ClearMes();
			sys->MesWin_Hide();
			curpos++;
			cmd = 0;
			break;
		case 0x02:			// ウィンドウをうにょんと消去（？）メッセージバッファもクリア
			// まず縦方向が中心に向かって縮んだ後（この段階で横に細長くなっている）、
			// 横方向が中心に向かって縮んでいって消える。
			dprintf("Text - Hide Window w/ effect (subcmd $%02X)¥n", subcmd);
			sys->MesWin_ClearMes();
			sys->MesWin_Hide();
			curpos++;
			cmd = 0;
			break;
		case 0x03:			// ウィンドウがうにょんと上書き、メッセージバッファもクリア
			// 0x02と逆手順で表示。この時、今表示されているウィンドウは消さない。
			dprintf("Text - Mes buffer clear w/ redraw window (subcmd $%02X)¥n", subcmd);
			sys->MesWin_ClearMes();
			curpos++;
			cmd = 0;
			break;
		case 0x04:			// クリックを待ってメッセージバッファクリア
			if ( sys->CheckSkip() ) {
				cmd = 0;
				curpos++;
				mouse->TextIconEnd();
				sys->MesWin_ClearMes();
				SavePoint();
				sys->PlaySE(3);			// SE.003 が鳴るらしい
				dprintf("Text - Mouse Click Wait / Hide Window (subcmd $%02X)¥n", subcmd);
			} else {
				mouse->TextIconStart();
				sys->MesWin_DrawIcon(1);
				if ( mouse->GetButton() ) {
					cmd = 0;
					curpos++;
					mouse->TextIconEnd();
					sys->MesWin_ClearMes();
					SavePoint();
					sys->PlaySE(3);		// SE.003 が鳴るらしい
					dprintf("Text - Mouse Click Wait / Hide Window (subcmd $%02X)¥n", subcmd);
				}
			}
			break;
		case 0x05:			// テキストクリア。ウィンドウは消さない（絶望, Ribbon2 NovelMode時）
			dprintf("Clear Text??? - (subcmd $%02X)¥n", subcmd);
			sys->MesWin_ClearMes();
			curpos++;
			cmd = 0;
			break;
		default:		// ここには来ないはず
			dprintf("Text??? - (subcmd $%02X)¥n", subcmd);
			ERROR;
			curpos++;
			cmd = 0;
			break;
	}
	return true;
};


// -------------------------------------------------------------------
// 04 - Cmd:$05 Text:テキストフォント修飾
// -------------------------------------------------------------------
bool SCENARIO::d04(void)
{
	int subcmd;

	subcmd = ReadValue();		// 内部的には可変長数値でデータを取ってるみたい
	switch (subcmd) {
		case 0x01:
			dprintf("Font Hx1 Wx1 (Normal Size) SubCmd:$%02X¥n", subcmd);
			sys->MesWin_DoubleText(0);
			break;
		case 0x02:
			dprintf("Font Hx2 Wx2 (Double Size) SubCmd:$%02X¥n", subcmd);
			sys->MesWin_DoubleText(1);
			break;
		default:
			dprintf("Font???  SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 05 - Cmd:$06 ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d05(void)
{
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 06 - Cmd:$08 ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d06(void)
{
	unsigned char subcmd;
	int data;

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:
			data = 0;
			break;
		case 0x10:
		case 0x11:
			data = ReadValue();
			break;
		default:
			data = 0;
			ERROR;
			break;
	}
	dprintf("???????  SubCmd:$%02X Arg:%d¥n", subcmd, data);
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 07 - Cmd:$0B グラフィック読み込み
// -------------------------------------------------------------------
bool SCENARIO::d07(void)
{
	char buf[128], buf2[128];
	int subcmd, idx, i;
	int srcx1, srcy1, srcx2, srcy2;
	int dstx1, dsty1, count, num;
	int arg1;
	EFFECT e;

	sys->MesWin_Hide();

	subcmd = databuf[curpos++];

	dprintf("Grp Load SubCmd $%02X  ", subcmd);

	switch (subcmd) {

	case 0x01:			// 違いがふめー
	case 0x03:
	case 0x05:
		ReadText(buf);
		idx = ReadValue();
		sys->CopySel(idx, &effect);
		dprintf("Grp '");
		dprintf(buf);
		dprintf(".PDT' Load and Displayed  w/ effect  Cmd:%d Step:%d Time:%d¥n", effect.cmd, effect.step, effect.steptime);
		sys->SnrPDT_LoadEffect(buf, &effect);
		break;

	case 0x02:			// 違いがふめー
	case 0x04:
	case 0x06:
		ReadText(buf);
		effect.sx1 = ReadValue(); effect.sy1 = ReadValue();
		effect.sx2 = ReadValue(); effect.sy2 = ReadValue();
		effect.dx = ReadValue(); effect.dy = ReadValue();
		effect.steptime = ReadValue();
		effect.cmd = ReadValue();
		effect.mask = ReadValue();
		effect.arg1 = ReadValue(); effect.arg2 = ReadValue();
		effect.arg3 = ReadValue(); effect.step = ReadValue();
		effect.arg5 = ReadValue(); effect.arg6 = ReadValue();
		effect.prevtime = sys->GetCurrentTimer();
		effect.srcpdt = 1;
		effect.dstpdt = 0;
		effect.curcount = 0;
		dprintf("Graphics - Graphic Load w/ effect, File:");
		dprintf(buf);
		dprintf(".PDT  Cmd:%d Step:%d Time:%d", effect.cmd, effect.step, effect.steptime);
		dprintf(".%d:(%d,%d)-(%d,%d)¥n", 1, effect.sx1, effect.sy1, effect.sx2, effect.sy2);
		sys->SnrPDT_LoadEffect(buf, &effect);
		break;

	case 0x09:
		ReadText(buf);
		idx = ReadValue();
		dprintf("??? Grp '");
		dprintf(buf);
		dprintf("' Load to Buffer#%d¥n", idx);
		sys->SnrPDT_LoadFile(buf, idx);
		break;

	case 0x10:
	case 0x54:
		ReadText(buf);
		idx = ReadValue();
		dprintf("Grp '");
		dprintf(buf);
		dprintf("' Load to Buffer#%d¥n", idx);
		sys->SnrPDT_LoadFile(buf, idx);
		break;

	case 0x11:		// Graphic Load Caching?
		ReadText(buf);
		dprintf("Graphic '");
		dprintf(buf);
		dprintf("' Cached???¥n");
		break;

	case 0x13:
		dprintf("?????????????¥n");
		break;

	case 0x22:			// 複数のPDTの重ね合わせ？（ベースはファイル）
		count = databuf[curpos++];		// 上に重ねる枚数
		ReadText(buf);					// ベースとなるファイル（ファイル名）
		sys->SnrPDT_MultiLoadFile(buf);		// PDT#1を使うらしい
		idx = ReadValue();				// 表示エフェクト番号（from GAMEEXE.INI）
		sys->CopySel(idx, &effect);
		dprintf("%d Grps are copies on '", count);
		dprintf(buf);
		dprintf("' w/ effect#%d (EfctCmd:%d)¥n", idx, effect.cmd);
		for (i=0; i<count; i++) {
			idx = databuf[curpos++];	// 重ね方？
			ReadText(buf2);				// 重ねるファイル（ファイル名）
			if ( buf2[0] ) strcpy(buf, buf2);
			dprintf(buf);
			dprintf("¥n");
			switch (idx) {
				case 0x01:
					sys->SnrPDT_LoadCopy(buf, 0, 0, 639, 479, 0, 0, 1, 0, idx);
					dprintf("SubSubCmd $01 : (0,0)-(639.479)->(0,0)¥n");
					break;

				case 0x02:
					num = ReadValue();
					sys->CopySel(num, &e);
					sys->SnrPDT_LoadCopy(buf, e.sx1, e.sy1, e.sx2, e.sy2, e.dx, e.dy, 1, 0, idx);
					dprintf("SubSubCmd $02 : Effect:%d (%d,%d)-(%d,%d)->", num, e.sx1, e.sy1, e.sx2, e.sy2);
					dprintf("(%d,%d)¥n", e.dx, e.dy);
					break;

				case 0x03:
					srcx1 = ReadValue(); srcy1 = ReadValue();
					srcx2 = ReadValue(); srcy2 = ReadValue();
					dstx1 = ReadValue(); dsty1 = ReadValue();
					sys->SnrPDT_LoadCopy(buf, srcx1, srcy1, srcx2, srcy2, dstx1, dsty1, 1, 0, idx);
					dprintf("SubSubCmd $02 : (%d,%d)-(%d,%d)->", srcx1, srcy1, srcx2, srcy2);
					dprintf("(%d,%d)¥n", dstx1, dsty1);
					break;

				case 0x04:
					srcx1 = ReadValue(); srcy1 = ReadValue();
					srcx2 = ReadValue(); srcy2 = ReadValue();
					dstx1 = ReadValue(); dsty1 = ReadValue();
					arg1 = ReadValue();
					sys->SnrPDT_LoadCopy(buf, srcx1, srcy1, srcx2, srcy2, dstx1, dsty1, 1, arg1, idx);
					dprintf("SubSubCmd $02 : (%d,%d)-(%d,%d)->", srcx1, srcy1, srcx2, srcy2);
					dprintf("(%d,%d) Arg:%d¥n", dstx1, dsty1, arg1);
					break;
			}
		}
		sys->SnrPDT_Effect(&effect);
		break;

	case 0x24:			// 複数のPDTの重ね合わせ？（ベースは既存のPDT）
		count = databuf[curpos++];		// 上に重ねる枚数
		i = ReadValue();			// ベースとなるファイル（PDT No.）
		sys->SnrPDT_MultiLoadPDT(i);
		idx = ReadValue();				// 表示エフェクト番号（from GAMEEXE.INI）
		sys->CopySel(idx, &effect);
		dprintf("%d Grps are copies on PDT#%d¥n", count, i);
		dprintf("  w/ Effect(Index:%d)  Cmd:%d Time:%d Step:%d¥n", idx, effect.cmd, effect.steptime, effect.step);
		for (i=0; i<count; i++) {
			idx = databuf[curpos++];	// 重ね方？
			ReadText(buf2);				// 重ねるファイル（ファイル名）
			if ( buf2[0] ) strcpy(buf, buf2);
			dprintf(buf);
			dprintf("¥n");
			switch (idx) {
				case 0x01:
					sys->SnrPDT_LoadCopy(buf, 0, 0, 639, 479, 0, 0, 1, 0, idx);
					break;

				case 0x02:
					sys->CopySel(ReadValue(), &e);
					sys->SnrPDT_LoadCopy(buf, e.sx1, e.sy1, e.sx2, e.sy2, e.dx, e.dy, 1, 0, idx);
					break;

				case 0x03:
					srcx1 = ReadValue(); srcy1 = ReadValue();
					srcx2 = ReadValue(); srcy2 = ReadValue();
					dstx1 = ReadValue(); dsty1 = ReadValue();
					sys->SnrPDT_LoadCopy(buf, srcx1, srcy1, srcx2, srcy2, dstx1, dsty1, 1, 0, idx);
					break;

				case 0x04:
					srcx1 = ReadValue(); srcy1 = ReadValue();
					srcx2 = ReadValue(); srcy2 = ReadValue();
					dstx1 = ReadValue(); dsty1 = ReadValue();
					arg1 = ReadValue();
					sys->SnrPDT_LoadCopy(buf, srcx1, srcy1, srcx2, srcy2, dstx1, dsty1, 1, arg1, idx);
					break;
			}
		}
		sys->SnrPDT_Effect(&effect);
		break;

	case 0x30:
		dprintf("Macro Buffer Clear¥n");
		sys->ClearMacro();
		break;

	case 0x31:
		idx = ReadValue();
		dprintf("Delete %d Macros¥n", idx);
		sys->DeleteMacro(idx);
		break;

	case 0x32:
		idx = ReadValue();
		dprintf("Macro Buffer??? Arg:%d¥n", idx);
		break;

	case 0x33:
		idx = ReadValue();
		flags->SetVal(idx, sys->GetMacroNum());
		dprintf("Set Macro Num to Val[%d] <- %d¥n", idx, flags->GetVal(idx));
		break;

	case 0x50:			// スクリーンバッファ退避？
		dprintf("Backup Screen Buffer???  No Arg¥n");
		sys->SnrPDT_Copy(0, 0, 639, 479, 0, 0, 0, HIDEPDT, 0);
		break;

	case 0x52:			// $50で退避した物を、idx番のエフェクトで表示？
		idx = ReadValue();
		sys->CopySel(idx, &effect);
		effect.srcpdt = HIDEPDT;
		dprintf("Display backuped Screen Buffer w/ Effect#%d ???¥n", idx);
		sys->SnrPDT_Effect(&effect);
		break;

	}

	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 08 - Cmd:$0C アニメーション
// -------------------------------------------------------------------
bool SCENARIO::d08(void)
{
	unsigned char subcmd;
	char buf[1024];
	int idx, arg1, arg2;

	if ( anmflag ) {
		if ( sys->AnimationExec() ) {
			anmflag = false;
			cmd = 0;
		}
	} else {
		subcmd = databuf[curpos++];
		dprintf("Animation - SubCmd:$%02X¥n", subcmd);

		switch( subcmd ) {
			case 0x10:
				ReadText(buf);
				idx = ReadValue();
				dprintf("   File - ");
				dprintf(buf);
				dprintf("  Index:%d¥n", idx);
				if ( sys->AnimationSetup(buf, idx) ) {
					anmflag = true;
				} else {
					dprintf("***** Animation Initialize Fault.¥n");
					cmd = 0;
				}
				break;

			case 0x18:
				ReadText(buf);
				arg1 = ReadValue();
				arg2 = ReadValue();
				dprintf("   File - ");
				dprintf(buf);
				dprintf("  Arg1=%d, Arg2=%d¥n", arg1, arg2);
				if ( sys->AnimationSetup(buf, 0) ) {
					anmflag = true;
				} else {
					dprintf("***** Animation Initialize Fault.¥n");
					cmd = 0;
				}
				break;

			case 0x12:			// Multi Animation Caching
			case 0x30:			// .KOEに同期のアニメ（取敢えずMultiAnime扱いにしとく）
				ReadText(buf);
				dprintf("  File:");
				dprintf(buf);
//				idx = ReadValue();
				while (databuf[curpos]) {
					idx = ReadValue();
					dprintf("  Data:%d¥n", idx);
					sys->MultiAnimationSetup(buf, idx);
				}
				dprintf("  Data:%d¥n", idx);
				curpos++;
				if ( subcmd==0x30 )		// .KOE同期
					multianmflag = -1;
				else
					multianmflag = 1;
				cmd = 0;
				break;
				
			case 0x13:
				dprintf("***** Animation Timer?¥n");
				cmd = 0;
				break;
			case 0x16:
				ReadText(buf);
				idx = ReadValue();
				arg1 = ReadValue();
				arg2 = ReadValue();
				dprintf("   File - ");
				dprintf(buf);
				dprintf("  Index:%d Arg1:%d Arg2:%d¥n", idx, arg1, arg2);
				if ( sys->AnimationSetup(buf, idx) ) {
					anmflag = true;
				} else {
					dprintf("***** Animation Initialize Fault.");
					cmd = 0;
				}
				break;
			case 0x20:				// Delete Multi Animation Item
			case 0x24:				// ?
				ReadText(buf);
				dprintf("  File:");
				dprintf(buf);
				while (databuf[curpos]) {
					idx = ReadValue();
					dprintf("  Data:%d¥n", idx);
					sys->MultiAnimationStop(buf, idx);
				}
				curpos++;
				cmd = 0;
				break;
			case 0x21:		// $12でキャッシュしたのを消去し、MultiAnimation停止
			case 0x25:
				sys->MultiAnimationClear();
				multianmflag = 0;
				cmd = 0;
				break;
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 09 - Cmd:$0E サウンド関連
// -------------------------------------------------------------------
bool SCENARIO::d09(void)
{
	unsigned char subcmd;
	char buf[1024];
	char temp[32];
	int data, idx, num, arg;
	int x1, y1, x2, y2;

	if ( soundwait ) {			// 再生終了まで停止指定の時の処理
		switch (soundwait) {
//			case 0x02:			// BGM
//			case 0x06:
//				break;
			case 0x20:			// KOE
				data = sound->KOE_IsPlaying();
				break;
			case 0x34:			// WAV
			case 0x35:
				data = sound->Sound_IsPlaying();
				break;
			default:
				data = 0;
				break;
		}
		if ( !data ) {
			cmd = 0;
			soundwait = 0;
		}
	} else {

	subcmd = databuf[curpos++];
	dprintf("Music - SubCmd:$%02X¥n", subcmd);

	switch( subcmd ) {
		// BGM (CD-DA/DSound) 関連
		// $00,04,08〜0F,13〜 のコマンドは無い模様
		case 0x01:			// BGMループ再生
			ReadText(buf);
			dprintf("Play CD w/ loop : Track ");
			dprintf(buf);
			dprintf("¥n");
			sound->CD_Play(buf, 1);
			cmd = 0;
			break;
		case 0x02:			// BGM単発再生 再生終了までここで停止
			ReadText(buf);
			dprintf("Play CD w/ wait : Track ");
			dprintf(buf);
			dprintf("¥n");
			sound->CD_Play(buf, 0);
			soundwait = subcmd;
//			cmd = 0;
			break;
		case 0x03:			// BGM単発再生
			ReadText(buf);
			dprintf("Play CD once : Track ");
			dprintf(buf);
			dprintf("¥n");
			sound->CD_Play(buf, 0);
			cmd = 0;
			break;
		case 0x05:			// $01 w/ FadeIn
			ReadText(buf);
			data = ReadValue();
			dprintf("Fade In CD w/ loop  Arg:%d  Track ", data);
			dprintf(buf);
			dprintf("¥n");
			sound->CD_FadeIn(buf, 1, data);
			cmd = 0;
			break;
		case 0x06:			// $02 w/ FadeIn ???
			ReadText(buf);
			data = ReadValue();
			dprintf("Fade In CD w/ wait Arg:%d  Track ", data);
			dprintf(buf);
			dprintf("¥n");
			sound->CD_FadeIn(buf, 0, data);
			soundwait = subcmd;
//			cmd = 0;
			break;
		case 0x07:			// $03 w/ FadeIn
			ReadText(buf);
			data = ReadValue();
			dprintf("Fade In CD and play once Arg:%d  Track ", data);
			dprintf(buf);
			dprintf("¥n");
			sound->CD_FadeIn(buf, 0, data);
			cmd = 0;
			break;
		case 0x10:			// BGM FadeOut / Stop
			data = ReadValue();
			dprintf("FadeOut CD ($10)  Arg:%d¥n", data);
			sound->CD_FadeOut(data);
			cmd = 0;
			break;
		case 0x11:			// BGM Stop
			dprintf("Stop CD ?? ($11)¥n");
			sound->CD_Stop();
			cmd = 0;
			break;
		case 0x12:			// BGM Rewind
			dprintf("Rewind CD ? ($12)¥n");
			sound->CD_Stop();
			cmd = 0;
			break;

		// --- KOE 関連 ---
		case 0x20:			// KOEファイル再生（再生終了までシナリオ停止）
			data = ReadValue();
			if ( sys->Version()>=1714 ) {
				idx = data/100000;
				num = data%100000;
			} else {
				idx = data/10000;
				num = data%10000;
			}
			dprintf("Play Voice w/ wait Idx:%d (Z%03d / %04d)¥n", data, idx, num);
			sound->KOE_Play(data);
/*			if ( koetextskip ) {
				if ( (databuf[curpos]==0xff)||(databuf[curpos]==0xfe) ) {
					curpos++;
					if ( sys->Version()>=1714 ) curpos += 4;
					ReadText(buf);
					if ( databuf[curpos]==0x01 ) curpos++;
				}
			}
*/
			soundwait = subcmd;
//			cmd = 0;
			break;
		case 0x21:			// KOEファイル再生（シナリオは継続）
			data = ReadValue();
			if ( sys->Version()>=1714 ) {
				idx = data/100000;
				num = data%100000;
			} else {
				idx = data/10000;
				num = data%10000;
			}
			dprintf("Play Voice Idx:%d (Z%03d / %04d)¥n", data, idx, num);
			sound->KOE_Play(data);
/*			if ( koetextskip ) {
				if ( (databuf[curpos]==0xff)||(databuf[curpos]==0xfe) ) {
					curpos++;
					if ( sys->Version()>=1714 ) curpos += 4;
					ReadText(buf);
					if ( databuf[curpos]==0x01 ) curpos++;
				}
			}
*/
			cmd = 0;
			break;
		case 0x22:			// KOEファイル再生2（シナリオは継続）
			data = ReadValue();
			arg = ReadValue();		// 用途不明
			if ( sys->Version()>=1714 ) {
				idx = data/100000;
				num = data%100000;
			} else {
				idx = data/10000;
				num = data%10000;
			}
			dprintf("Play Voice Idx:%d (Z%03d / %04d), Arg:%d¥n", data, idx, num, arg);
			sound->KOE_Play(data);
/*			if ( koetextskip ) {
				if ( (databuf[curpos]==0xff)||(databuf[curpos]==0xfe) ) {
					curpos++;
					if ( sys->Version()>=1714 ) curpos += 4;
					ReadText(buf);
					if ( databuf[curpos]==0x01 ) curpos++;
				}
			}
*/
			cmd = 0;
			break;

		// --- WAV 関連 ---
		case 0x30:			// WAV単発再生
			ReadText(buf);
			dprintf("[$30] Play WAV file :");
			dprintf(buf);
			dprintf("¥n");
			sound->Sound_Play(buf, 0);
			cmd = 0;
			break;
		case 0x31:			// WAV単発再生2（Arg付き）
			ReadText(buf);
			idx = ReadValue();		// 用途不明
			dprintf("[$31] Play WAV file :");
			dprintf(buf);
			dprintf(" Arg:%d($%08X)", idx, idx);
			dprintf("¥n");
			sound->Sound_Play(buf, 0);
			cmd = 0;
			break;
		case 0x32:			// WAVループ再生 ($30 w/ loop)
			ReadText(buf);
			dprintf("[$32] Play WAV file w/ loop :");
			dprintf(buf);
			dprintf("¥n");
			sound->Sound_Play(buf, 1);
			cmd = 0;
			break;
		case 0x33:			// WAVループ再生2 ($31 w/ loop)
			ReadText(buf);
			idx = ReadValue();		// 用途不明
			dprintf("[$33] Play WAV file  w/ loop :");
			dprintf(buf);
			dprintf(" Arg:%d($%08X)", idx, idx);
			dprintf("¥n");
			sound->Sound_Play(buf, 1);
			cmd = 0;
			break;
		case 0x34:			// WAV単発再生、終了までここで停止 ($30 w/ wait)
			ReadText(buf);
			dprintf("[$34] Play WAV file w/ wait :");
			dprintf(buf);
			dprintf("¥n");
			sound->Sound_Play(buf, 0);
			soundwait = subcmd;
//			cmd = 0;
			break;
		case 0x35:			// WAV単発再生2、終了までここで停止 ($31 w/ wait)
			ReadText(buf);
			idx = ReadValue();		// 用途不明
			dprintf("[$35] Play WAV file w/ wait :");
			dprintf(buf);
			dprintf(" Arg:%d($%08X)", idx, idx);
			dprintf("¥n");
			sound->Sound_Play(buf, 0);
			soundwait = subcmd;
//			cmd = 0;
			break;
		case 0x36:			// WAV停止？
			dprintf("[$36] WAV stop?¥n");
			sound->Sound_Stop();
			cmd = 0;
			break;
		case 0x37:			// WAV停止2？
			data = ReadValue();		// 用途不明
			dprintf("[$37] WAV stop?  Arg:%d¥n", data);
			sound->Sound_Stop();
			cmd = 0;
			break;
		case 0x38:			// WAV停止3？
			dprintf("[$38] WAV stop?¥n");
			sound->Sound_Stop();
			cmd = 0;
			break;
		case 0x39:			// ?????
			data = ReadValue();
			dprintf("[$39] WAV ??????  Arg:%d¥n", data);
			cmd = 0;
			break;

		case 0x44:			// SE (GAMEEXE.INI中で指定したWAVを鳴らす)
			data = ReadValue();
			dprintf("[$44] WAV Play 'SE'  Arg:%d¥n", data);
			sys->PlaySE(data);
			cmd = 0;
			break;

		// --- Movie 関連、サイズに合わせて拡大縮小もします ---
		case 0x50:			// Movie(AVI)再生しつつシナリオ続行
		case 0x51:			// Movie(AVI)再生しつつシナリオ続行 ループ再生
			ReadText(buf);
			x1 = ReadValue(); y1 = ReadValue();
			x2 = ReadValue(); y2 = ReadValue();
			dprintf("[$50/$51] Play Movie(2)   File - ");
			dprintf(buf);
			dprintf("¥n");
			dprintf("   Pos : (%d,%d)-(%d,%d)¥n", x1, y1, x2, y2);
			cmd = 0;
			break;
		case 0x52:			// Movie(AVI)再生 終了までシナリオ停止 マウスキャンセルなし
		case 0x53:			// Movie(AVI)再生 終了までシナリオ停止 マウスキャンセルあり
			ReadText(buf);
			x1 = ReadValue(); y1 = ReadValue();
			x2 = ReadValue(); y2 = ReadValue();
			dprintf("[$52/$53] Play Movie(2)   File - ");
			dprintf(buf);
			dprintf("¥n");
			dprintf("   Pos : (%d,%d)-(%d,%d)¥n", x1, y1, x2, y2);
			cmd = 0;
			break;
		case 0x54:			// Movie(AVI)再生2 終了までシナリオ停止 マウスキャンセルなし
		case 0x55:			// Movie(AVI)再生2 終了までシナリオ停止 マウスキャンセルあり
			ReadText(buf);
			ReadText(temp);		// ここのテキストの用途不明（複数AVI連続再生ってわけぢゃないみたい）
			x1 = ReadValue(); y1 = ReadValue();
			x2 = ReadValue(); y2 = ReadValue();
			dprintf("[$54/$55] Play Movie(2)   File - ");
			dprintf(buf);
			dprintf("¥n");
			dprintf("   Pos : (%d,%d)-(%d,%d)¥n", x1, y1, x2, y2);
			cmd = 0;
			break;

		default:			// SubCmd $23 とかが出てくることもあるみたいなので（恋ごころとか）
			cmd = 0;
			break;
	}

	}
	return true;
};


// -------------------------------------------------------------------
// 0A - Cmd:$10 文字列を画像バッファにダイレクトに表示
// -------------------------------------------------------------------
bool SCENARIO::d0a(void)
{
	unsigned char subcmd;
	int idx, idx2;
	char buf[256], tmp[256];

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:
			idx = ReadValue();
			sprintf(&buf[1], "%d", flags->GetVal(idx));
			Han2Zen(&buf[1]);
			buf[0] = 0xff;
			sys->MesWin_SetMes(buf);
			dprintf("Draw Val[%d] as Text - (subcmd $%02X) : ", idx, subcmd);
			dprintf(&buf[1]);
			dprintf("¥n", subcmd, idx);
			break;
		case 0x02:
			idx = ReadValue();
			idx2 = ReadValue();
			sprintf(tmp, "%%0%dd", idx2);
			sprintf(&buf[1], tmp, flags->GetVal(idx));
			Han2Zen(&buf[1]);
			buf[0] = 0xff;
		sys->MesWin_SetMes(buf);
			dprintf("Draw Val[%d] as Text w/ %d digit - (subcmd $%02X) : ", idx, idx2, subcmd);
			dprintf(&buf[1]);
			dprintf("¥n", subcmd, idx);
			break;
		case 0x03:
			idx = ReadValue();
			sprintf(&buf[1], "%s", flags->GetStr(idx));
//			Han2Zen(&buf[1]);
			buf[0] = 0xff;
			sys->MesWin_SetMes(buf);
			dprintf("Draw Str[%d] as Text - (subcmd $%02X) : ", idx, subcmd);
			dprintf(&buf[1]);
			dprintf("¥n", subcmd, idx);
			break;
	}
	meswaitbase = sys->GetCurrentTimer();
	mesflag = 1;
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 0B - Cmd:$13 フェードイン／アウト
// -------------------------------------------------------------------
bool SCENARIO::d0b(void)
{
	unsigned char subcmd;
	int idx;

	if ( fadecmd ) {
		if ( ((sys->GetCurrentTimer()-fadeprevtime)>=fadestep)||(sys->CheckSkip()) ) {
			fadeprevtime = sys->GetCurrentTimer();
			sys->SnrPDT_ScreenFade(fadecmd, fadecount, fader, fadeg, fadeb);
			fadecount++;
			if ( fadecount==16 ) {
				sys->SnrPDT_FillRect(0, 0, 639, 479, 3, fader, fadeg, fadeb);	// エフェクトと同じで#3を埋めとく必要あり？（flowers）
				mouse->FinishPDTDraw();
				fadecmd = 0;
				cmd = 0;
			}
		}
	} else {
		sys->MesWin_Hide();
		fadecount = 0;
		subcmd = fadecmd = databuf[curpos++];
		fadeprevtime = sys->GetCurrentTimer();
		mouse->StartPDTDraw();
		switch (subcmd) {
			case 0x01:
				idx = ReadValue();
				fadestep = sys->GetFadeTime();
				sys->GetFadeColor(idx, &fader, &fadeg, &fadeb);
				sys->SnrPDT_FillRect(0, 0, 639, 479, 1, fader, fadeg, fadeb);
				dprintf("Fade In/Out  SubCmd:$%02X  FadePattern:%d¥n", subcmd, idx);
				break;
			case 0x02:
				idx = ReadValue();
				fadestep = ReadValue();
				sys->GetFadeColor(idx, &fader, &fadeg, &fadeb);
				sys->SnrPDT_FillRect(0, 0, 639, 479, 1, fader, fadeg, fadeb);
				dprintf("Fade In/Out w/ Time  SubCmd:$%02X  FadePattern:%d  FadeTime:%d¥n", subcmd, idx, fadestep);
				break;
			case 0x03:
				fader = ReadValue(); fadeg = ReadValue(); fadeb = ReadValue();
				fadestep = sys->GetFadeTime();
				sys->SnrPDT_FillRect(0, 0, 639, 479, 1, fader, fadeg, fadeb);
				dprintf("Fade In/Out  SubCmd:$%02X  R:%d G:%d B:%d¥n", fadecmd, fader, fadeg, fadeb);
				break;
			case 0x04:
				fader = ReadValue(); fadeg = ReadValue(); fadeb = ReadValue();
				fadestep = ReadValue();
				sys->SnrPDT_FillRect(0, 0, 639, 479, 1, fader, fadeg, fadeb);
				dprintf("Fade In/Out w/ Time  SubCmd:$%02X  R:%d G:%d B:%d  FadeTime:%d¥n", fadecmd, fader, fadeg, fadeb, fadestep);
				break;
			case 0x10:
				idx = ReadValue();
				fadestep = 0;
				sys->GetFadeColor(idx, &fader, &fadeg, &fadeb);
//				sys->SnrPDT_FillRect(0, 0, 639, 479, 1, fader, fadeg, fadeb);
				dprintf("Fade In/Out (Fill Screen?)  SubCmd:$%02X  FadePattern:%d¥n", subcmd, idx);
				fadecount = 15;
//				cmd = 0;
				break;
			case 0x11:
				fader = ReadValue(); fadeg = ReadValue(); fadeb = ReadValue();
				fadestep = 0;
//				sys->SnrPDT_FillRect(0, 0, 639, 479, 1, fader, fadeg, fadeb);
				dprintf("Fade In/Out (Fill Screen?)  SubCmd:$%02X  R:%d G:%d B:%d¥n", fadecmd, fader, fadeg, fadeb);
				fadecount = 15;
//				cmd = 0;
				break;
			default:
				dprintf("?????? Fade In/Out?  SubCmd:$%02X¥n", subcmd);
				break;
		}
		multianmflag = 0;
	}
	return false;
};


// -------------------------------------------------------------------
// 0C - Cmd:$15 条件分岐
// -------------------------------------------------------------------
bool SCENARIO::d0c(void)
{
	int ptr, condition;

	dprintf("IF ");
	condition = DecodeConditions();
	ptr = ReadInt();
	if ( condition ) {
		dprintf(" - true; Decode next command¥n");
	} else {
		dprintf(" - false; Jump to $%08X¥n", ptr);
		curpos = ptr+jumpbase;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 0D - Cmd:$16 他シーンへのジャンプ／コール
// -------------------------------------------------------------------
bool SCENARIO::d0d(void)
{
	unsigned char subcmd;
	int ptr;

	subcmd = databuf[curpos++];
	ptr = ReadValue();
	if ( subcmd!=1 ) {					// 他シーンコール
		flags->PushStack(seennum, curpos);
		dprintf("Call", seennum);
	} else {
		dprintf("Jump", seennum);
	}
	if ( !ChangeSeen(ptr) ) sys->Terminate();
	SavePoint();
	dprintf(" to other seen #%d¥n", seennum);
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 0E - Cmd:$17 画面揺れ
// -------------------------------------------------------------------
bool SCENARIO::d0e(void)
{
	int idx;
	unsigned char subcmd;

	if ( shakeflag ) {
		if ( sys->ScreenShake() ) {
			cmd = 0;
			shakeflag = false;
		}
	} else {
		subcmd = databuf[curpos++];
		switch (subcmd) {
			case 0x01:
				idx = ReadValue();
				sys->ScreenShakeSetup(idx);
				shakeflag = true;
				dprintf("Screen Shake : Shake Pattern:%d¥n", idx);
				break;
			default:
				dprintf("???????????  Screen Shake???(SubCmd:$%02X) Unknown¥n", subcmd);
				break;
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 0F - Cmd:$18 Text:フォント色変更
// -------------------------------------------------------------------
bool SCENARIO::d0f(void)
{
	unsigned char subcmd;
	int idx;

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:
			idx = ReadValue();
			sys->ChangeFontColor(idx);
			dprintf("Text Color Change: Color#%d¥n", idx);
			break;
		default:
			dprintf("?????????  Unimpremented Text Color???(SubCmd:$%02X), Unknown¥n", subcmd);
			break;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 10 - Cmd:$19 ウェイト
// -------------------------------------------------------------------
bool SCENARIO::d10(void)
{
	int idx;

	if ( waitcmd ) {					// Waitコマンド実行中
			switch( waitcmd ) {
			case 0x01:
				if ( sys->CheckSkip() ) {
					cmd = 0;
					waitcmd = 0;
				}
				if ( ((sys->GetCurrentTimer())-waitbase)>=waittime ) {
					cmd = 0;
					waitcmd = 0;
				}
				break;
			case 0x02:
				if ( sys->CheckSkip() ) {
					cmd = 0;
					waitcmd = 0;
				}
				if ( ((sys->GetCurrentTimer())-waitbase)>=waittime ) {
					flags->SetVal(waitcancelindex, 0);
					cmd = 0;
					waitcmd = 0;
				} else if ( mouse->GetButton() ) {
					flags->SetVal(waitcancelindex, 1);
					cmd = 0;
					waitcmd = 0;
				}
				break;
			case 0x04:
				if ( (sys->GetTimer())>=waittime ) {
					dprintf("  Timer done. Acctual:%d us¥n", sys->GetTimer());
					cmd = 0;
					waitcmd = 0;
				}
				break;
			case 0x05:
				if ( (sys->GetTimer())>=waittime ) {
					flags->SetVal(waitcancelindex, 0);
					cmd = 0;
					waitcmd = 0;
				} else if ( mouse->GetButton() ) {
					flags->SetVal(waitcancelindex, 1);
					cmd = 0;
					waitcmd = 0;
				}
				break;
		}
		if ( !waitcmd ) mouse->FinishPDTDraw();
	} else {							// 新規のWaitコマンド
		waitcmd = databuf[curpos++];
		switch( waitcmd ) {
			case 0x01:
				mouse->StartPDTDraw();
				waittime = (unsigned int)ReadValue();
				waitbase = sys->GetCurrentTimer();
				dprintf("Wait %d us¥n", waittime);
				break;
			case 0x02:
				mouse->StartPDTDraw();
				waittime = (unsigned int)ReadValue();
				waitcancelindex = ReadValue();
				waitbase = sys->GetCurrentTimer();
				dprintf("Wait %d us / Break by mouse flag:Val[%d]¥n", waittime, waitcancelindex);
				break;
			case 0x03:
				sys->ResetTimer();
				dprintf("Set Current time to Base Time¥n");
				waitcmd = 0;
				cmd = 0;		// すぐに帰る（次コマンド）
				break;
			case 0x04:
				mouse->StartPDTDraw();
				waittime = (unsigned int)ReadValue();
				dprintf("Wait %d us from Base Time¥n", waittime);
				break;
			case 0x05:
				mouse->StartPDTDraw();
				waittime = (unsigned int)ReadValue();
				waitcancelindex = ReadValue();
				dprintf("Wait %d us from Base Time / Break by mouse flag:Val[%d]¥n", waittime, waitcancelindex);
				break;
			case 0x06:
				idx = ReadValue();
				flags->SetVal(idx, sys->GetTimer());
				dprintf("Set times from Base Time to Val[%d]  <- %d¥n", idx, flags->GetVal(idx));
				waitcmd = 0;
				cmd = 0;
				break;
			case 0x10:
				dprintf("($10) ??????????? SkipModeFlag = 1 ???¥n");
//				mouse->EnablePopup();
				waitcmd = 0;
				cmd = 0;
				break;
			case 0x11:
				dprintf("($11) ??????????? SkipModeFlag = 0 ???¥n");
//				mouse->DisablePopup();
				waitcmd = 0;
				cmd = 0;
				break;
			case 0x12:
				dprintf("($12) ??????????? Wait??? Flag = 1 ???¥n");
				waitcmd = 0;
				cmd = 0;
				break;
			case 0x13:
				dprintf("($13) ??????????? Wait??? Flag = 0 ???¥n");
				waitcmd = 0;
				cmd = 0;
				break;
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 11 - Cmd:$1A ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d11(void)
{
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 12 - Cmd:$1B 同一シーン内サブルーチンコール
// -------------------------------------------------------------------
bool SCENARIO::d12(void)
{
	int ptr;

	ptr = ReadInt();
	dprintf("Call (in this seen) to $%08X¥n", ptr);
	flags->PushStack(seennum, curpos);
	curpos = ptr+jumpbase;
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 13 - Cmd:$1C 無条件Jump
// -------------------------------------------------------------------
bool SCENARIO::d13(void)
{
	int ptr;

	ptr = ReadInt();
	dprintf("Jump to $%08X¥n", ptr);
	curpos = ptr+jumpbase;
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 14 - Cmd:$1D/1E テーブルサブルーチンコール/テーブルジャンプ
// -------------------------------------------------------------------
bool SCENARIO::d14(void)
{
	int ptr, idx, n, i;

	ptr = -1;
	n = databuf[curpos++];
	idx = ReadValue();
	for (i=1; i<=n; i++) {
		if ( i==flags->GetVal(idx) )
			ptr = ReadInt();
		else
			curpos += 4;
	}
	dprintf("In Seen#%d ... Table Call/Jump Num:Val[%d](%d) in %d choices ... to $%08X¥n", seennum, idx, flags->GetVal(idx), n, ptr);
	if ( ptr!=(-1) ) {
		if ( cmd==0x1d ) flags->PushStack(seennum, curpos);
		curpos = ptr+jumpbase;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 15 - Cmd:$20 サブルーチンリターン
// -------------------------------------------------------------------
bool SCENARIO::d15(void)
{
	unsigned char subcmd;
	int n, ptr, dmy1, dmy2;

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:						// 同一シーン内リターン
			dprintf(" Return in the same seen¥n");
			flags->PopStack(&seennum, &curpos);
			break;
		case 0x02:						// 他シーンからのリターン
			dprintf(" Return to the other seen¥n");
			flags->PopStack(&n, &ptr);
			if ( !ChangeSeen(n) ) sys->Terminate();
			curpos = ptr;
			SavePoint();
			break;
		case 0x03:						// スタックの内容を一個消す？（すわっぷAふぉ〜B）
			flags->PopStack(&dmy1, &dmy2);
			break;
		case 0x06:						// スタック初期化？
			dprintf(" Stack clear?¥n");
			flags->ClearStack();
			break;
		default:
			dprintf(" Return Unknown. - subcmd:$%02X¥n", subcmd);
	}
	dprintf(" Return to seen #%d¥n", seennum);
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 16 - Cmd:$22-29 ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d16(void)
{
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 17 - Cmd:$2C ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d17(void)
{
	unsigned char subcmd;
	int idx;

	subcmd = databuf[curpos++];

	switch( subcmd ) {
		case 0x01:
			idx = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg:%d¥n", subcmd, idx);
			break;
		default:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 18 - Cmd:$2D ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d18(void)
{
	unsigned char subcmd;
	int idx, data;

	subcmd = databuf[curpos++];

	switch( subcmd ) {
		case 0x01:
			idx = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg:%d¥n", subcmd, idx);
			break;
		case 0x02:
			idx = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg:%d¥n", subcmd, idx);
			break;
		case 0x03:
			idx = ReadValue();
			data = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg1:%d Arg2:%d¥n", subcmd, idx, data);
			break;
		case 0x04:
			idx = ReadValue();
			data = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg1:%d Arg2:%d¥n", subcmd, idx, data);
			break;
		default:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 19 - Cmd:$2E ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d19(void)
{
	// シナリオメニューの選択肢の変更をするみたいだけど・・・
	// 詳細不明
	unsigned char subcmd;
	int idx, data;

	subcmd = databuf[curpos++];

	switch( subcmd ) {
		case 0x01:
			// 指定ビットに変更、かなぁ・・・
			data = ReadValue();
			smenu.bitcount = data-1;
			smenu.bit = (1<<smenu.bitcount);
			dprintf("???????????????? - SubCmd:$%02X Arg:%d¥n", subcmd, data);
			break;  
		case 0x02:
			// これもだと思うけど。こっちはサブメニュー用？
			idx = ReadValue()-1;
			data = ReadValue();
//			smenu.bit = (1<<(data-1));
			smenu.subbitcount = data-1;
			smenu.subbit = (1<<smenu.subbitcount);
//			smenu.subbitcount[idx] = (data-1);
//			smenu.subbit[idx] = (1<<smenu.subbitcount[idx]);
			dprintf("???????????????? - SubCmd:$%02X Arg1:%d Arg2:%d¥n", subcmd, idx, data);
			break;
		default:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 1A - Cmd:$2F ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d1a(void)
{
	unsigned char subcmd;
	int idx, idx2, idx3;

	subcmd = databuf[curpos++];

	switch( subcmd ) {
		case 0x01:
			idx = ReadValue();
			idx2 = ReadValue();
			idx3 = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg1:%d Arg2:%d Arg3:%d¥n", subcmd, idx, idx2, idx3);
			break;
		default:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 1B - Cmd:$30 ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d1b(void)
{
	unsigned char subcmd;

	subcmd = databuf[curpos++];

	switch( subcmd ) {
		case 0x01:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			break;
		default:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 1C - Cmd:$31 ******************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d1c(void)
{
	unsigned char subcmd;
	int idx;

	subcmd = databuf[curpos++];

	switch( subcmd ) {
		case 0x01:
			idx = ReadValue();
			dprintf("???????????????? - SubCmd:$%02X Arg:%d¥n", subcmd, idx);
			break;
		case 0x02:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			break;
		default:
			dprintf("???????????????? - SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 1D - Cmd:$37/39/3B-43/49-51/56/57 変数・フラグ操作
// -------------------------------------------------------------------
bool SCENARIO::d1d(void)
{
	int idx, data, rnd;

	idx = ReadValue();
	if ( cmd!=0x56 ) data = ReadValue();

	switch( cmd ) {

	case 0x37:
		dprintf("Bit[%d] = %d¥n", idx, data);
		flags->SetBit(idx, data);
		break;
	case 0x39:
		dprintf("Bit[%d] = Bit[%d]¥n", idx, data);
		flags->SetBit(idx, flags->GetBit(data));
		break;
	case 0x3b:
		dprintf("Val[%d] = %d¥n", idx, data);
		flags->SetVal(idx, data);
		break;
	case 0x3c:
		dprintf("Val[%d] += %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)+data);
		break;
	case 0x3d:
		dprintf("Val[%d] -= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)-data);
		break;
	case 0x3e:
		dprintf("Val[%d] *= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)*data);
		break;
	case 0x3f:
		dprintf("Val[%d] /= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)/data);
		break;
	case 0x40:
		dprintf("Val[%d] %%= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)%data);
		break;
	case 0x41:
		dprintf("Val[%d] &= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)&data);
		break;
	case 0x42:
		dprintf("Val[%d] |= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)|data);
		break;
	case 0x43:
		dprintf("Val[%d] ^= %d¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)^data);
		break;
	case 0x49:
		dprintf("Val[%d] = Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(data));
		break;
	case 0x4a:
		dprintf("Val[%d](%d) += Val[%d](%d)¥n", idx, flags->GetVal(idx), data, flags->GetVal(data));
		flags->SetVal(idx, flags->GetVal(idx)+flags->GetVal(data));
		break;
	case 0x4b:
		dprintf("Val[%d] -= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)-flags->GetVal(data));
		break;
	case 0x4c:
		dprintf("Val[%d] *= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)*flags->GetVal(data));
		break;
	case 0x4d:
		dprintf("Val[%d] /= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)/flags->GetVal(data));
		break;
	case 0x4e:
		dprintf("Val[%d] %%= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)%flags->GetVal(data));
		break;
	case 0x4f:
		dprintf("Val[%d] &= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)&flags->GetVal(data));
		break;
	case 0x50:
		dprintf("Val[%d] |= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)|flags->GetVal(data));
		break;
	case 0x51:
		dprintf("Val[%d] ^= Val[%d]¥n", idx, data);
		flags->SetVal(idx, flags->GetVal(idx)^flags->GetVal(data));
		break;
	case 0x56:		// ランダムに bit[idx] をセット／リセットする
		dprintf("Bit[%d] = Rand(0,1)¥n", idx);
		flags->SetBit(idx, sys->GetRandom(0, 1));
		break;
	case 0x57:		// data〜rnd の間の乱数を val[idx] にセットする
		rnd = ReadValue();
		dprintf("Val[%d] = Rand(%d,%d)¥n", idx, data, rnd);
		flags->SetVal(idx, sys->GetRandom(data, rnd));
		break;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 1E - Cmd:$58 選択肢
// -------------------------------------------------------------------
bool SCENARIO::d1e(void)
{
	int i, flag, attr, idx;
	char buf[256];

	if ( selectflag ) {
		switch (selectflag) {
			case 0x01:
			case 0x02:
				i = sys->Select();
				if ( i!=(-1) ) {
					flags->SetVal(selectindex, i);
					cmd = 0;
					dprintf("### Selected - %d¥n", i);
					sys->MesWin_ClearMes();
//					sys->MesWin_Draw();
					if ( selectflag==0x01 ) sys->Select_SubWinClose();
					selectflag = 0;
					mouse->TextIconEnd();
				}
				break;
			default:
				selectflag = 0;
				cmd = 0;
				break;
		}
	} else {
		selectflag = databuf[curpos++];
		i = 0;
		switch (selectflag) {
			case 0x01:		// 別窓で選択
			case 0x02:
				curpos-=2; SavePoint(); curpos+=2;
				selectindex = ReadValue();
				dprintf("Select SubCmd:$%02X  Result:Val[%d]¥n", selectflag, selectindex);
				dprintf("Dump: ");
				for (i=0; i<128; i++) {
					dprintf("%02X ", databuf[curpos+i]);
					if ( (i%16)==15 ) dprintf("¥n");
				}
				if ( databuf[curpos++]==0x22 ) {
					if ( !databuf[curpos] ) curpos++;
					do {
						flag = ReadFormattedText(buf, &attr);
						if ( flag ) {
							switch ( attr&0xff ) {
								case 0x00:		// のーまる
									sys->Select_AddItem(buf, 1, 0);
									break;
								case 0x20:		// 色付き、選択可
									sys->Select_AddItem(buf, 1, (attr>>8));
									break;
								case 0x21:		// 選択肢として出さない？
									sys->Select_AddItem(buf, -1, 0);	// ダミーを追加
									break;
								case 0x22:		// 色付き、選択は不可
									sys->Select_AddItem(buf, 0, (attr>>8));
									break;
							}
						} else {
							sys->Select_AddItem(buf, -1, 0);	// ダミーを追加
						}
						dprintf("  Item - ");
						dprintf(buf);
						dprintf(" (Flag:%d)¥n", flag);
					} while (databuf[curpos]!=0x23);
					curpos++;
				}
				sys->MesWin_ClearMes();
				if ( selectflag==0x01 ) sys->Select_SubWinSetup();
				mouse->TextIconStart();
				break;
			case 0x04:
				idx = ReadValue();
				dprintf("Select - OpenLoadMenu SubCmd:$%02X¥n", selectflag);
				flags->SetVal(idx, sys->PopupLoadMenu());
				selectflag = 0;
				cmd = 0;
				break;
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 1F - Cmd:$59 文字列操作
// -------------------------------------------------------------------
bool SCENARIO::d1f(void)
{
	char buf[1024];
	int idx, idx2, idx3;
	unsigned char subcmd;

	subcmd = databuf[curpos++];
	dprintf("String Handle - SubCmd:$%02X¥n", subcmd);

	idx = ReadValue();

	switch( subcmd ) {
		case 0x01:
			ReadText(buf);
			flags->SetStr(idx, buf);
//			for (i=0; (i<63)&&(buf[i]); i++) flags->str[idx][i]=buf[i];
//			flags->str[idx][i] = 0;
			dprintf("strcpy  Str[%d] = ", idx);
			dprintf(buf);
			dprintf("¥n");
			break;
		case 0x02:
			idx2 = ReadValue();
			dprintf("Val[%d] = strlen(Str[%d])  (", idx, idx2);
			dprintf(flags->GetStr(idx2));
			dprintf(")¥n");
			flags->SetVal(idx, strlen(flags->GetStr(idx2)));
			break;
		case 0x03:
			idx2 = ReadValue();
			idx3 = ReadValue();
			dprintf("Val[%d] = strcmp(Str[%d], Str[%d])  (", idx, idx2, idx3);
			dprintf(flags->GetStr(idx2));
			dprintf(", ");
			dprintf(flags->GetStr(idx3));
			dprintf(")¥n");
			flags->SetVal(idx, strcmp(flags->GetStr(idx2), flags->GetStr(idx3)));
//			flags->val[idx] = strcmp(flags->str[idx2], flags->str[idx3]);
			break;
		case 0x04:
			idx2 = ReadValue();
			dprintf("strcat(Str[%d], Str[%d])  (", idx, idx2);
			dprintf(flags->GetStr(idx));
			dprintf(", ");
			dprintf(flags->GetStr(idx2));
			dprintf(")¥n");
			strcat(flags->GetStr(idx), flags->GetStr(idx2));
			break;
		case 0x05:
			idx2 = ReadValue();
			flags->SetStr(idx, flags->GetStr(idx2));
			dprintf("strcpy(Str[%d], Str[%d])  (", idx, idx2);
			dprintf(flags->GetStr(idx));
			dprintf(")¥n");
			break;
		case 0x06:
			idx2 = ReadValue();
			idx3 = ReadValue();
			if ( idx3==10 ) {			// 10進
				sprintf(flags->GetStr(idx2), "%d", flags->GetVal(idx));
			} else if ( idx3==16 ) {	// 16進
				sprintf(flags->GetStr(idx2), "%X", flags->GetVal(idx));
			}
			dprintf("Str[%d] (", idx2);
			dprintf(flags->GetStr(idx2));
			dprintf(") = itoa(val[%d]) (%d)¥n", idx, flags->GetVal(idx));
			dprintf("%d進文字列変換¥n", idx3);
			break;
		case 0x07:
			dprintf("Hankaku to Zenkaku¥n");
			dprintf("Str[%d] ", idx);
			dprintf(flags->GetStr(idx));
			dprintf(" -> ");
			Han2Zen(flags->GetStr(idx));
			dprintf(flags->GetStr(idx));
			dprintf("¥n");
			break;
		case 0x08:
			idx2 = ReadValue();
			flags->SetVal(idx2, atoi(flags->GetStr(idx)));
			dprintf("Val[%d](%d) = atoi(str[%d])  (", flags->GetVal(idx), idx, idx2);
			dprintf(flags->GetStr(idx2));
			dprintf(")¥n");
			break;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 20 - Cmd:$5B 複数の変数に順番にデータをセット
// -------------------------------------------------------------------
bool SCENARIO::d20(void)
{
	unsigned char subcmd;
	int idx, data;

	dprintf("Multiple Value Set¥n");
	subcmd = databuf[curpos++];
	idx = ReadValue();
	if ( subcmd==1 ) {
		while (databuf[curpos]) {
			data =  ReadValue();
			dprintf("  Val[%d] = %d¥n", idx, data);
			flags->SetVal(idx++, data);
		}
	} else if ( subcmd==2 ) {
		while (databuf[curpos]) {
			data =  ReadValue();
			dprintf("  Bit[%d] = %d¥n", idx, data);
			flags->SetBit(idx++, data);
		}
	}
	curpos++;
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 21 - Cmd:$5C 複数の変数に同一データをセット
// -------------------------------------------------------------------
bool SCENARIO::d21(void)
{
	unsigned char subcmd;
	int idx, idx2, data, i;

	subcmd = databuf[curpos++];
	idx = ReadValue();
	idx2 = ReadValue();
	data = ReadValue();
	if (idx2 >= 2000) idx2 = 1999;
	if ( subcmd==1 ) {
		for (i=idx; i<=idx2; i++) flags->SetVal(i, data);
		dprintf("Val[%d〜%d] = %d¥n", idx, idx2, data);
	} else if ( subcmd==2 ) {
		for (i=idx; i<=idx2; i++) flags->SetBit(i, data);
		dprintf("Bit[%d〜%d] = %d¥n", idx, idx2, data);
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 22 - Cmd:$5D 複数の変数をまとめて他へコピー
// -------------------------------------------------------------------
bool SCENARIO::d22(void)
{
	unsigned char subcmd;
	int idx, idx2, i;

	subcmd = databuf[curpos++];
	idx = ReadValue();
	idx2 = ReadValue();
	i = ReadValue();
	if ( subcmd==1 ) {
		dprintf("Val[%d〜%d] = Val[%d〜%d]¥n", idx2, idx2+i-1, idx, idx+i-1);
		for ( ; i>0; i--) {
			dprintf("  Val[%d] = %d¥n", idx, flags->GetVal(idx));
			flags->SetVal(idx2++, flags->GetVal(idx++));
		}
	} else if ( cmd==2 ) {
		dprintf("Bit[%d〜%d] = Bit[%d〜%d]¥n", idx2, idx2+i-1, idx, idx+i-1);
		for ( ; i>0; i--) flags->SetBit(idx2++, flags->GetBit(idx++));
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 23 - Cmd:$5E 日時やシーン番号を取得
// -------------------------------------------------------------------
bool SCENARIO::d23(void)
{
	unsigned char subcmd;
	int idx, data;

	subcmd = databuf[curpos++];
	idx = ReadValue();
	data = 0;
	switch (subcmd) {
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
			data = sys->GetDateTime(subcmd);
			dprintf("SubCmd:$%02X - Get Date/Time Idx:%d Data:%d¥n", subcmd, idx, data);
			break;
		case 0x10:
			data = seennum;
			dprintf("SubCmd:$%02X - Get Seen# Idx:%d Data:%d¥n", subcmd, idx, data);
			break;
		default:
			dprintf("SubCmd:$%02X - ?????????? Unimplemented¥n", subcmd);
			break;
	}
	flags->SetVal(idx, data);
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 24 - Cmd:$5F 複数数値の加算など
// -------------------------------------------------------------------
bool SCENARIO::d24(void)
{
	unsigned char subcmd;
	int idx, idx2, data, i;

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:
			idx = ReadValue();
			dprintf("Mutli Add   Val[%d] = ", idx);
			data = 0;
			subcmd = databuf[curpos++];
			do {
				switch (subcmd) {
				case 0x01:
					idx2 = ReadValue();
					data += flags->GetVal(idx2);
					dprintf("Val[%d] ", idx2);
					break;
				case 0x02:
					i = ReadValue();
					idx2 = ReadValue();
					dprintf("Val[%d〜%d] ", i, idx2);
					for ( ; i<=idx2; i++) data += flags->GetVal(i);
					break;
				case 0x11:
					idx2 = ReadValue();
					data += flags->GetBit(idx2);
					dprintf("Bit[%d] ", idx2);
					break;
				case 0x12:
					i = ReadValue();
					idx2 = ReadValue();
					dprintf("Bit[%d〜%d] ", i, idx2);
					for ( ; i<=idx2; i++) data += flags->GetBit(i);
					break;
				}
				subcmd = databuf[curpos++];
			} while (subcmd);
			dprintf("(%d)¥n", data);
			flags->SetVal(idx, data);
			break;
		case 0x10:
			idx = ReadValue();
			idx2 = ReadValue();
			data = ReadValue();
			dprintf("Value Percentage  Val[%d] = %d / %d %%¥n", idx, idx2, data);
			i = (idx2*100)/data;
			if ( i>100 ) i = 100;		// 100%オーバしたら100？(継母調教)
			if ( i<  0 ) i = 0;			// これも一応
			flags->SetVal(idx, i);
			break;
		case 0x20:
			i = databuf[curpos++];
			idx = ReadValue();
			idx2 = ReadValue();
			dprintf("Multiple Value Copy w/ index  Val[%d〜%d] = Val[%d + n]¥n", idx, idx+i-1, idx2);
			for ( ; i>0; i--) {
				flags->SetVal(idx++, flags->GetVal(idx2+ReadValue()));
			}
			break;
		default:
			dprintf("???? Multiple Value Copy? (SubCmd:$%02X)¥n", subcmd);
			break;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 25 - Cmd:$60 システム制御／セーブロード他
// -------------------------------------------------------------------
bool SCENARIO::d25(void)
{
	unsigned char subcmd;
	int idx, idx2, idx3, attr, data;
	char buf[256];

	if ( loading ) {
		if ( sys->LoadingProc() ) {
			cmd = 0;
			loading = false;
		}
	} else {
		subcmd = databuf[curpos++];
		switch (subcmd) {

		case 0x02:
			idx = ReadValue();
			dprintf("SaveData Loading - LoadFile:%d¥n", idx);
			if ( sys->Load(flags, idx-1, &idx2, &idx3) ) {
				sys->Reset();
				Reset(idx2, idx3);
				cmd = 0x60;
				loading = true;
			} else {
				cmd = 0;
				loading = false;
			}
			break;

		case 0x03:
			idx = ReadValue();
			dprintf("SaveData Saving - #%d¥n", idx);
			sys->Save(flags, idx-1, seennum, curpos);
			cmd = 0;
			break;

		case 0x04:
			idx = ReadFormattedText(buf, &attr);
			sys->SetWindowTitle(buf, idx);
			dprintf("SetTitle - ");
			dprintf(buf);
			dprintf("¥n");
			cmd = 0;
			break;

		case 0x05:
			dprintf("Make Popup???¥n");
			sys->PopupContextMenu();
			cmd = 0;
			break;

		case 0x20:
			dprintf("Game End.($20)¥n");
			sys->Terminate();
			cmd = 0;
			break;

		case 0x30:
			data = ReadValue();
			idx = ReadValue();
			dprintf("Get SaveData Title - Idx:%d Data:%d¥n", idx, data);
			sys->SetSaveDataTitle(data-1, flags->GetStr(idx));
			dprintf("  Str[%d] = ", idx);
			dprintf(flags->GetStr(idx));
			dprintf("¥n");
			cmd = 0;
			break;

		case 0x31:
			data = ReadValue();
			idx = ReadValue();
			dprintf("Check SaveData - Idx:%d Data:%d¥n", idx, data);
			flags->SetVal(idx, sys->CheckSaveData(data-1));
			cmd = 0;
			break;

		case 0x35:
			data = ReadValue();
			idx = ReadValue();
			dprintf("($35) ????????? - Idx:%d Data:%d¥n", idx, data);
			flags->SetStr(idx, sys->CheckSaveDataTitle(data-1));
			cmd = 0;
			break;

		case 0x36:
			data = ReadValue();
			idx = ReadValue();
			dprintf("($36) ????????? - Idx:%d Data:%d¥n", idx, data);
			flags->SetVal(idx, sys->CheckSaveDataDate(data-1));
			dprintf("  Val[%d] = %d¥n", idx, flags->GetVal(idx));
			cmd = 0;
			break;

		case 0x37:
			data = ReadValue();
			idx = ReadValue();
			dprintf("($37) ????????? - Idx:%d Data:%d¥n", idx, data);
			flags->SetVal(idx, sys->CheckSaveDataTime(data-1));
			dprintf("  Val[%d] = %d¥n", idx, flags->GetVal(idx));
			cmd = 0;
			break;

		default:
			dprintf("Unknown System Command:$%02X¥n", subcmd);
			cmd = 0;
			break;
		}
	}
	return true;
};


// -------------------------------------------------------------------
// 26 - Cmd:$61 名前変数の操作とか
// -------------------------------------------------------------------
bool SCENARIO::d26(void)
{
	unsigned char subcmd;
	int x, y, ex, ey;
	int r, g, b, br, bg, bb;
	int idx, data, i, attr;
	char buf[256];
	char nametitle[2][256];
	int nameindex[2];

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:		// メインスクリーン内でそのまま名前入力？（好き好き）
			x = ReadValue(); y = ReadValue();		// テキストボックス始点？
			ex = ReadValue(); ey = ReadValue();		// テキストボックス終点？
			r = ReadValue(); g = ReadValue(); b = ReadValue();		// テキスト色
			br = ReadValue(); bg = ReadValue(); bb = ReadValue();	// 背景色
			dprintf("SubCmd:$%02X - Name Input text box?  (%d,%d)-(%d,%d) ", subcmd, x, y, ex, ey);
			dprintf("TextColor(%d,%d,%d) ", r, g, b);
			dprintf("BG Color(%d,%d,%d)¥n", br, bg, bb);
			break;
		case 0x02:		// $01で作ったテキストボックスの値の代入先？（入力完了？）
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Name Input text box?  Str[%d] = Input? (Finish?)¥n", subcmd, idx);
			break;
		case 0x03:		// $01で作ったテキストボックスの値の代入先？（実際の入力開始？）
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Name Input text box?  Str[%d] = Input? (Start?)¥n", subcmd, idx);
			break;
		case 0x04:		// $01で作ったテキストボックスのクローズ？
			dprintf("SubCmd:$%02X - Name Input text box close?¥n", subcmd);
			break;
		case 0x11:
			idx = ReadValue();
			data = ReadValue();
			dprintf("SubCmd:$%02X - Change Name String Value NAME.%c = Str[%d]¥n", subcmd, idx+0x41, data);
			sys->SetNameString(idx, flags->GetStr(data));
			break;
		case 0x10:
		case 0x12:
			idx = ReadValue();
			data = ReadValue();
			dprintf("SubCmd:$%02X - Get Name String? Str[%d] = NAME.%c ???¥n", subcmd, data, idx+0x41);
			sys->GetNameString(idx, flags->GetStr(data));
			break;
		case 0x20:		// 名前入力（1項目だけ、項目名固定）（魔薬）
			nameindex[0] = ReadValue()+1;
			sprintf(nametitle[0], "名前");
			nametitle[1][0] = 0;
			nameindex[1] = 0;
			dprintf("SubCmd:$%02X - Name Change (NAME.%c)¥n", subcmd, 0x40+nameindex[0]);
			sys->NameInputDlg(nametitle[0], nametitle[1], nameindex[0], nameindex[1]);
			break;
		case 0x21:		// ？？？ 文字一覧から選択式の入力？（絶望）
			idx = ReadValue();
			ReadText(buf);
			ReadValue(); ReadValue(); ReadValue();
			ReadValue(); ReadValue(); ReadValue();
			ReadValue(); ReadValue(); ReadValue();
			break;
		case 0x24:		// 名前入力（最大2項目、項目名指定可）
			data = databuf[curpos++];
			dprintf("SubCmd:$%02X - Name Change (%d item(s))¥n", subcmd, data);
			nametitle[0][0] = 0;
			nametitle[1][0] = 0;
			nameindex[0] = 0;
			nameindex[1] = 0;
			for (i=0; i<data; i++) {
				idx = ReadValue();
				ReadFormattedText(buf, &attr);
				if ( i<2 ) {
					sprintf(nametitle[i], "%s", buf);
					nameindex[i] = idx;
				}
				dprintf("   Idx:%d Str:", idx);
				dprintf(buf);
				dprintf("¥n");
			}
			sys->NameInputDlg(nametitle[0], nametitle[1], nameindex[0], nameindex[1]);
			break;
		case 0x30:
		case 0x31:
			dprintf("SubCmd:$%02X - ????? No Arg Cmd.¥n", subcmd);
			break;
		default:
			dprintf("SubCmd:$%02X - Unimplemented¥n", subcmd);
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 27 - Cmd:$63 Graphics:Get/Put系
// -------------------------------------------------------------------
bool SCENARIO::d27(void)
{
	unsigned char subcmd;
	int srcx1, srcy1, srcx2, srcy2, dstx1, dsty1;
	int srcpdt, dstpdt;
	int data, n;

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dprintf("Graphics - Get/Put Get (subcmd 1)  %d:(%d,%d)-(%d,%d)¥n", srcpdt, srcx1, srcy1, srcx2, srcy2);
			sys->SnrPDT_Get(srcx1, srcy1, srcx2, srcy2, srcpdt);
			break;
		case 0x02:
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstpdt = ReadValue();
			dprintf("Graphics - Get/Put Put (subcmd 2) X:%d Y:%d PDT:%d¥n", dstx1, dsty1, dstpdt);
			sys->SnrPDT_Put(dstx1, dsty1, dstpdt);
			break;
		case 0x20:
			n = databuf[curpos++];
			data = ReadValue();
			dprintf("Graphics - ????????? (SubCmd:$20)  Arg1:%d  Arg2:%d¥n", n, data);
			break;
		default:
			dprintf("Graphics - ????????? (Cmd:$%02X SubCmd:$%02X)¥n", cmd, subcmd);
			break;
	}
	cmd = 0;
	return false;
};


// -------------------------------------------------------------------
// 28 - Cmd:$64/65/67-6A Graphics:バッファ描画関連
// -------------------------------------------------------------------
bool SCENARIO::d28(void)
{
	unsigned char subcmd;
	int srcx1, srcy1, srcx2, srcy2, srcdx, srcdy;
	int dstx1, dsty1, dstx2, dsty2;
	int srcpdt, dstpdt;
	int r, g, b, flag, count, data, zero, n;

	sys->MesWin_Hide();

	switch (cmd)
	{
	case 0x64:				// 領域に対する操作関連（クリアとか色マスクとか）
		subcmd = databuf[curpos++];
		switch (subcmd)
		{
		case 0x02:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			r = ReadValue(); g = ReadValue(); b = ReadValue();
			dprintf("Graphics - Clear Rect (subcmd $02)  %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" Col(%d,%d,%d)¥n", r, g, b);
			sys->SnrPDT_ClearRect(srcx1, srcy1, srcx2, srcy2, srcpdt, r, g, b);
			break;

		case 0x04:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			r = ReadValue(); g = ReadValue(); b = ReadValue();
			dprintf("Graphics - Draw Rect Line (subcmd $04)  %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" Col(%d,%d,%d)¥n", r, g, b);
			sys->SnrPDT_DrawRectLine(srcx1, srcy1, srcx2, srcy2, srcpdt, r, g, b);
			break;

		case 0x07:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dprintf("Graphics - Invert Color (subcmd $07)¥n");
			sys->SnrPDT_MakeInvert(srcx1, srcy1, srcx2, srcy2, srcpdt);
			break;

		case 0x10:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			r = ReadValue(); g = ReadValue(); b = ReadValue();
			dprintf("Graphics - ColorMask (subcmd $10)  %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf("  RGB:%d,%d,%d¥n", r, g, b);
			sys->SnrPDT_MakeColorMask(srcx1, srcy1, srcx2, srcy2, srcpdt, r, g, b);
			break;

		case 0x11:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dprintf("Graphics - Fade Out Color (subcmd $11)  %d:(%d,%d)-(%d,%d)¥n", srcpdt, srcx1, srcy1, srcx2, srcy2);
			sys->SnrPDT_FadeColor(srcx1, srcy1, srcx2, srcy2, srcpdt, 0, 0, 0, 0x80);
			break;

		case 0x12:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dprintf("Graphics - Fade Out Color (subcmd $12)  %d:(%d,%d)-(%d,%d)¥n", srcpdt, srcx1, srcy1, srcx2, srcy2);
			sys->SnrPDT_FadeColor(srcx1, srcy1, srcx2, srcy2, srcpdt, 0, 0, 0, 0xc0);
			break;

		case 0x15:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			r = ReadValue();
			g = ReadValue();
			b = ReadValue();
			count = ReadValue();
			dprintf("Graphics - FadeOutColor($15), %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" -> Col:%d,%d,%d Cnt:%d¥n", r, g, b, count);
			sys->SnrPDT_FadeColor(srcx1, srcy1, srcx2, srcy2, srcpdt, r, g, b, count);
			break;

		case 0x20:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dprintf("Graphics - Make Mono Image (subcmd $20)¥n");
			sys->SnrPDT_MakeMonochrome(srcx1, srcy1, srcx2, srcy2, srcpdt);
			break;

		case 0x30:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstx2 = ReadValue(); dsty2 = ReadValue();
			dstpdt = ReadValue();
			dprintf("Graphics - Stretch Blt (subcmd $30)¥n");
			dprintf("  %d:(%d,%d)-(%d,%d)->", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf("%d:(%d,%d)-(%d,%d)¥n", dstpdt, dstx1, dsty1, dstx2, dsty2);
			sys->SnrPDT_StretchCopy(srcx1, srcy1, srcx2, srcy2, srcpdt, dstx1, dsty1, dstx2, dsty2, dstpdt);
			break;

		case 0x32:
			effect.sx1 = ReadValue(); effect.sy1 = ReadValue();
			effect.sx2 = ReadValue(); effect.sy2 = ReadValue();
			effect.arg3 = ReadValue(); effect.arg4 = ReadValue();		// ex1, ey1
			effect.arg5 = ReadValue(); effect.arg6 = ReadValue();		// ex2, ey2
			effect.srcpdt = ReadValue();
			effect.dx = ReadValue(); effect.dy = ReadValue();
			effect.arg1 = ReadValue(); effect.arg2 = ReadValue();		// dx2, dy2
			effect.dstpdt = ReadValue();
			effect.step = ReadValue();
			effect.steptime = ReadValue();
			effect.cmd = 9999;
			effect.curcount = 0;
			effect.prevtime = sys->GetCurrentTimer();
			if (effect.sx1>639) effect.sy1 = 639;		// 恋愛CHU!デモ
			if (effect.sx2>639) effect.sy2 = 639;
			if (effect.sx1<0) effect.sy1 = 0;
			if (effect.sx2<0) effect.sy2 = 0;
			if (effect.sy1>479) effect.sy1 = 479;
			if (effect.sy2>479) effect.sy2 = 479;
			if (effect.sy1<0) effect.sy1 = 0;
			if (effect.sy2<0) effect.sy2 = 0;
			dprintf("Graphics - Stretch Blt (subcmd $32)¥n");
			dprintf("%d:(%d,%d)-(%d,%d)->", effect.srcpdt, effect.sx1, effect.sy1, effect.sx2, effect.sy2);
			dprintf("(%d,%d)-(%d,%d)  Dst:%d:", effect.arg3, effect.arg4, effect.arg5, effect.arg6, effect.dstpdt);
			dprintf("(%d,%d)-(%d,%d)¥n", effect.dx, effect.dy, effect.arg1, effect.arg2);
			dprintf("Wait:%d  TotalStep:%d¥n", effect.steptime, effect.step);
			sys->SnrPDT_Effect(&effect);
			break;

		default:
			dprintf("Graphics - ??? (subcmd $%02X)¥n", subcmd);
			break;
		}
		cmd = 0;
		break;

	case 0x65:				// ******************* Unimpremented
		cmd = 0;
		break;

	case 0x67:				// バッファコピーと表示関連
		subcmd = databuf[curpos++];
		switch (subcmd)
		{
		case 0x00:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			flag = ReadValue();
			dprintf("Graphics - Copy to Display PDT(#0), Same Pos (subcmd $00)¥n");
			dprintf("    %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" -> 0:(%d,%d)  UpdateFlag:$%02X¥n", srcx1, srcy1, flag);
			sys->SnrPDT_CopyBackBuffer(srcx1, srcy1, srcx2, srcy2, srcpdt, 1/*flag*/);
			break;

		case 0x01:		// マスクもコピーされる模様？ (Ribbon2放課後の選択時の数字)
						// （Kanon CGモードのCGがない部分をみると、マスクはコピーされない方が正しいっぽい・・・）
						// 見た目の部分ではRibbon2の方が汚く見えるので、そっちを優先
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstpdt = ReadValue();
			flag = 0;
			if ( sys->Version()>=1704 ) flag = ReadValue();		// AVG32 New Version (>17D) Only
			dprintf("Graphics - Copy($01), %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" -> %d:(%d,%d)  Flag:$%02X¥n", dstpdt, dstx1, dsty1, flag);
			sys->SnrPDT_Copy(srcx1, srcy1, srcx2, srcy2, srcpdt, dstx1, dsty1, dstpdt, flag);
			break;

		case 0x02:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstpdt = ReadValue();
			if ( sys->Version()>=1613 ) flag = ReadValue();			// AVG32 New Version (>16M) Only??
			dprintf("Graphics - Mask Copy($02), %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" -> %d:(%d,%d) Flag:$%02X¥n", dstpdt, dstx1, dsty1, flag);
			sys->SnrPDT_MaskCopy(srcx1, srcy1, srcx2, srcy2, srcpdt, dstx1, dsty1, dstpdt, flag);
			break;

		case 0x03:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstpdt = ReadValue();
			r = ReadValue(); g = ReadValue(); b = ReadValue();
			dprintf("Graphics - Copy w/o Color ($03), %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" -> %d:(%d,%d)", dstpdt, dstx1, dsty1);
			dprintf(" RGB:$%02X%02X%02X¥n", r, g, b);
			sys->SnrPDT_MonoCopy(srcx1, srcy1, srcx2, srcy2, srcpdt, dstx1, dsty1, dstpdt, r, g, b);
			break;

		case 0x05:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstpdt = ReadValue();
			dprintf("Graphics - Swap (subcmd $05)¥n");
			sys->SnrPDT_Swap(srcx1, srcy1, srcx2, srcy2, srcpdt, dstx1, dsty1, dstpdt);
			break;

		case 0x08:
			srcx1 = ReadValue(); srcy1 = ReadValue();
			srcx2 = ReadValue(); srcy2 = ReadValue();
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();
			dstpdt = ReadValue();
			flag = ReadValue();
			sys->SnrPDT_CopyWithMask(srcx1, srcy1, srcx2, srcy2, srcpdt, dstx1, dsty1, dstpdt, flag);
			dprintf("Graphics - Copy w/ Mask (subcmd $08), %d:(%d,%d)-(%d,%d)", srcpdt, srcx1, srcy1, srcx2, srcy2);
			dprintf(" -> %d:(%d,%d)  Flag:$%02X¥n", dstpdt, dstx1, dsty1, flag);
			dprintf("************************* Unimplemented!!!¥n");
			break;

		case 0x11:		// 全画面コピー。マスクバッファの内容もコピーされるみたい。
			srcpdt = ReadValue();
			dstpdt = ReadValue();
			flag = 0;
			if ( sys->Version()>=1704 ) flag = ReadValue();		// AVG32 New Version (>17D) Only
			sys->SnrPDT_AllCopy(srcpdt, dstpdt, flag);
//			sys->SnrPDT_MaskCopy(0, 0, 639, 479, srcpdt, 0, 0, dstpdt, flag);
			dprintf("Graphics - Copy all screen(buffer) (subcmd $11) %d->%d Flag:%d¥n", srcpdt, dstpdt, flag);
			break;

		case 0x12:		// 全画面マスクコピー
			srcpdt = ReadValue();
			dstpdt = ReadValue();
			flag = 0;
			if ( sys->Version()>=1613 ) flag = ReadValue();		// AVG32 New Version (>16M) Only
//			flag = ReadValue();
			sys->SnrPDT_MaskCopy(0, 0, 639, 479, srcpdt, 0, 0, dstpdt, flag);
			dprintf("Graphics - Copy all screen? w/ fade (subcmd $12) %d->%d  Flag:$%02X¥n", srcpdt, dstpdt, flag);
			break;

		case 0x20:		// PureHeartとか 数字表示？
			n = ReadValue();							// 表示する数値の入った変数番号
			data = flags->GetVal(n);
			srcx1 = ReadValue(); srcy1 = ReadValue();	// 元データの開始座標
			srcx2 = ReadValue(); srcy2 = ReadValue();	// 元データの1文字当たりの幅／高さ
			srcdx = ReadValue(); srcdy = ReadValue();	// 元データの次の文字位置までの移動量
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();	// 表示開始位置
			dstx2 = ReadValue(); dsty2 = ReadValue();	// 次の桁までの移動量
			count = ReadValue();						// 表示桁数
			zero = ReadValue();							// 桁数分まで先頭を0で埋めるかどうか
			dstpdt = ReadValue();

			dprintf("Graphics - Disp Strings??? (subcmd $20)¥n");
			dprintf("  Disp Num:%d¥n", data);
			for (count-- ; count>=0; count--) {
				n = data%10;
				dprintf("  Disp %d on Pos:%d¥n", n, count);
				sys->SnrPDT_Copy(srcx1+(srcdx*n), srcy1+(srcdy*n),
							  srcx1+(srcdx*n)+srcx2-1, srcy1+(srcdy*n)+srcy2-1,
							  srcpdt,
							  dstx1+dstx2*count, dsty1+dsty2*count,
							  dstpdt, 0);
				data /= 10;
				if ( (!data)&&(!zero) ) break;
			}
			break;

		case 0x21:		// #$20のマスクコピー版
			n = ReadValue();							// 表示する数値の入った変数番号
			data = flags->GetVal(n);
			srcx1 = ReadValue(); srcy1 = ReadValue();	// 元データの開始座標
			srcx2 = ReadValue(); srcy2 = ReadValue();	// 元データの1文字当たりの幅／高さ
			srcdx = ReadValue(); srcdy = ReadValue();	// 元データの次の文字位置までの移動量
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();	// 表示開始位置
			dstx2 = ReadValue(); dsty2 = ReadValue();	// 次の桁までの移動量
			count = ReadValue();						// 表示桁数
			zero = ReadValue();							// 桁数分まで先頭を0で埋めるかどうか
			dstpdt = ReadValue();
			flag = ReadValue();

			for (count-- ; count>=0; count--) {
				n = data%10;
				sys->SnrPDT_MaskCopy(srcx1+(srcdx*n), srcy1+(srcdy*n),
								  srcx1+(srcdx*n)+srcx2-1, srcy1+(srcdy*n)+srcy2-1,
								  srcpdt,
								  dstx1+dstx2*count, dsty1+dsty2*count,
								  dstpdt, flag);
				data /= 10;
				if ( (!data)&&(!zero) ) break;
			}

			dprintf("Graphics - Disp Strings? (subcmd $21)¥n");
			break;

		case 0x22:		// #$20のモノクロコピー版
			n = ReadValue();							// 表示する数値の入った変数番号
			data = flags->GetVal(n);
			srcx1 = ReadValue(); srcy1 = ReadValue();	// 元データの開始座標
			srcx2 = ReadValue(); srcy2 = ReadValue();	// 元データの1文字当たりの幅／高さ
			srcdx = ReadValue(); srcdy = ReadValue();	// 元データの次の文字位置までの移動量
			srcpdt = ReadValue();
			dstx1 = ReadValue(); dsty1 = ReadValue();	// 表示開始位置
			dstx2 = ReadValue(); dsty2 = ReadValue();	// 次の桁までの移動量
			count = ReadValue();						// 表示桁数
			zero = ReadValue();							// 桁数分まで先頭を0で埋めるかどうか
			dstpdt = ReadValue();
			r = ReadValue(); g = ReadValue(); b = ReadValue();

			for (count-- ; count>=0; count--) {
				n = data%10;
				sys->SnrPDT_MonoCopy(srcx1+(srcdx*n), srcy1+(srcdy*n),
								  srcx1+(srcdx*n)+srcx2-1, srcy1+(srcdy*n)+srcy2-1,
								  srcpdt,
								  dstx1+dstx2*count, dsty1+dsty2*count,
								  dstpdt, r, g, b);
				data /= 10;
				if ( (!data)&&(!zero) ) break;
			}

			dprintf("Graphics - Disp Strings? (subcmd $22)¥n");
			break;

		default:
			dprintf("Graphics - ??? (subcmd $%02X)¥n", subcmd);
			break;
		}
		cmd = 0;
		break;

	case 0x68:
		if ( flashcount ) {			// 画面フラッシュちう〜
			if ( flashcount&1 ) {
				sys->SnrPDT_Copy(0, 0, 639, 479, HIDEPDT, 0, 0, 0, 0);
				flashbase = sys->GetCurrentTimer();
				flashcount--;
				if ( !flashcount ) {		// おわり〜
					cmd = 0;
				}
			} else {
				if ( (sys->GetCurrentTimer()-flashbase)<flashtime ) break;
				sys->SnrPDT_FillRect(0, 0, 639, 479, 0, flashr, flashg, flashb);
				flashcount--;
			}
		} else {
			subcmd = databuf[curpos++];
			switch (subcmd) {
				case 0x01:				// 色でうめる？
					dstpdt = ReadValue();
					r = ReadValue(); g= ReadValue(); b = ReadValue();
					sys->SnrPDT_FillRect(0, 0, 639, 479, dstpdt, r, g, b);
					dprintf("Graphics - FillScreen w/ Color? (subcmd $01)  PDT:%d %d,%d,%d¥n", dstpdt, r, g, b);
					cmd = 0;
					break;
				case 0x10:				// 画面フラッシュ効果みたいなの？
					flashr = ReadValue();
					flashg = ReadValue();
					flashb = ReadValue();				// フラッシュ色
					flashtime = ReadValue();			// 時間間隔
					flashcount = ReadValue()*2;			// 回数
					dprintf("Graphics - Flash Screen? (subcmd $10)¥n");
					if ( flashcount ) {
						sys->SnrPDT_Copy(0, 0, 639, 479, 0, 0, 0, HIDEPDT, 0);
						sys->SnrPDT_FillRect(0, 0, 639, 479, 0, flashr, flashg, flashb);
						flashcount--;
					} else {							// 念の為
						cmd = 0;
					}
					break;
				default:
					dprintf("Graphics - ??? (subcmd $%02X)¥n", subcmd);
					cmd = 0;
					break;
			}
		}
		break;

	case 0x69:				// 2画面（PDT0/1）を使ったスクロール切り替え？
		// ホントはスクロールするんだけど ^^;
		// あと、PDT0からスクロールアウトした分は、PDT1にいなきゃなんない？のかな？
		//    subcmd役割不明（魔薬では常に2？） 別数値で左右とかになるのかも
		//    dataは方向みたい。0で上へ、1で下へのスクロールらしい
		//    countがスクロール量
		//    nは用途不明（魔薬では400が指定されている）。切り替えスピードとか？
#if 0
		subcmd = databuf[curpos++];
		data = databuf[curpos++];
		srcx1 = ReadValue(); srcy1 = ReadValue();
		srcx2 = ReadValue(); srcy2 = ReadValue();
		count = ReadValue();
		n = ReadValue();
		if ( data ) {
			// 逆方向。未実装
		} else {
			sys->SnrPDT_Copy(srcx1, srcy1+count, srcx2, srcy2, 0, srcx1, 0, 0, 0);
			sys->SnrPDT_Copy(srcx1, 0, srcx2, count-1, 1, srcx1, srcy2-srcy1-count+1, 0, 0);
			sys->SnrPDT_Copy(srcx1, count, srcx2, srcy2, 1, srcx1, 0, 1, 0);
		}
		dprintf("Graphics - 2 Screens camera panning???¥n");
		dprintf("  SubCmd?:$%02X Direction:$%02X  (%d,%d)-(%d,", subcmd, data, srcx1, srcy1, srcx2);
		dprintf("%d) Line:%d Arg8:%d¥n", srcy2, count, n);
#else
		subcmd = databuf[curpos++];
		effect.arg4 = databuf[curpos++];		// Direction
		effect.sx1 = ReadValue(); effect.sy1 = ReadValue();
		effect.sx2 = ReadValue(); effect.sy2 = ReadValue();
		effect.arg6 = ReadValue();				// Line nums
		effect.arg5 = ReadValue();				// ???
		effect.step = 8;
		effect.srcpdt = 1;
		effect.dstpdt = 0;
		effect.steptime = 1;
		effect.cmd = 1000;
		effect.curcount = 0;
		effect.prevtime = sys->GetCurrentTimer();
#endif
		cmd = 0;
		break;

	case 0x6a:		// Multiple PDT Handle
		DecodeEndingHandling();
		break;
	}
	return false;
};


// -------------------------------------------------------------------
// 29 - Cmd:$66 PDTバッファへの直接文字列描画
// -------------------------------------------------------------------
bool SCENARIO::d29(void)
{
	unsigned char subcmd;
	int x, y, r, g, b, idx, attr;
	char buf[256];

	subcmd = databuf[curpos++];		// SubCmd 意味無し？
	x = ReadValue(); y = ReadValue();
	idx = ReadValue();						// 描画するPDT
	r = ReadValue(); g = ReadValue(); b = ReadValue();
	ReadFormattedText(buf, &attr);
	dprintf("Draw string to PDT¥n");
	dprintf("  %d:(%d,%d) ", idx, x, y);
	dprintf("Col:(%d,%d,%d) Str:", r, g, b);
	dprintf(buf);
	dprintf("¥n");
	sys->SnrPDT_DrawString(x, y, idx, r, g, b, buf);

	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 2A - Cmd:$6C エリア情報関連
// -------------------------------------------------------------------
bool SCENARIO::d2a(void)
{
	unsigned char subcmd;
	int idx, idx2, idx3, x, y, flag;
	char buf[256];

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x02:
			dprintf("SubCmd:$%02X - Read Area Data from ARD/CUR(?) file¥n  CUR:", subcmd);
			ReadText(buf);
			dprintf(buf);
			dprintf("¥n  ARD:");
			ReadText(buf);
			dprintf(buf);
			dprintf("¥n");
			Area_Read(buf);
			break;
		case 0x03:
			dprintf("SubCmd:$%02X -  Init Area Buffer¥n", subcmd);
			Area_Clear();
			break;
		case 0x04:
			dprintf("SubCmd:$%02X -  Return clicked Area Number w/ mouse button data?¥n", subcmd);
			idx = ReadValue();
			idx2 = ReadValue();
			mouse->GetState(&x, &y, &flag);
			flags->SetVal(idx2, (flag<=0)?0:1);		// 右クリック以外は0が返る？（flowers）
			if ( flag==0 ) {		// クリックがあった時だけエリア#を返すみたい (flowers)
				flags->SetVal(idx, Area_Find(x, y));
			} else {
				flags->SetVal(idx, 0);
			}
			dprintf("X:%d Y:%d Area:%d Mouse:%d¥n", x, y, flags->GetVal(idx), flags->GetVal(idx2));
			break;
		case 0x05:
			dprintf("SubCmd:$%02X -  Return clicked Area Number w/ mouse button data(2)¥n", subcmd);
			idx = ReadValue();
			idx2 = ReadValue();
			mouse->GetState(&x, &y, &flag);
			flags->SetVal(idx2, flag);
			flags->SetVal(idx, Area_Find(x, y));
			dprintf("X:%d Y:%d Area:%d Mouse:%d¥n", x, y, flags->GetVal(idx), flags->GetVal(idx2));
			break;
		case 0x10:
			idx = ReadValue();
			dprintf("SubCmd:$%02X -  Disable Area Num#%d¥n", subcmd, idx);
			Area_Disable(idx);
			break;
		case 0x11:
			idx = ReadValue();
			dprintf("SubCmd:$%02X -  Enable Area Num#%d¥n", subcmd, idx);
			Area_Enable(idx);
			break;
		case 0x15:
			idx2 = ReadValue();
			idx3 = ReadValue();
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Get Area Num from Area Buffer  Idx:%d X:%d Y:%d¥n", subcmd, idx, idx2, idx3);
			flags->SetVal(idx, Area_Find(idx2, idx3));
			break;
		case 0x20:
			idx = ReadValue();
			idx2 = ReadValue();
			dprintf("SubCmd:$%02X - Area# assign change? Num:%d to #%d ???¥n", subcmd, idx, idx2);
			break;
		default:
			dprintf("SubCmd:$%02X - Unimplemented¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 2B - Cmd:$6D マウス制御
// -------------------------------------------------------------------
bool SCENARIO::d2b(void)
{
	unsigned char subcmd;
	int idx, idx2, idx3, flag, x, y;

	subcmd = databuf[curpos++];
	dprintf("Mouse Control  SubCmd:$%02X¥n", subcmd);
	switch (subcmd) {
		case 0x01:
			mouse->GetState(&x, &y, &flag);
			if ( flag!=-1 ) {
				idx = ReadValue();
				idx2 = ReadValue();
				idx3 = ReadValue();
				flags->SetVal(idx , x);
				flags->SetVal(idx2, y);
				flags->SetVal(idx3, flag);
				cmd = 0;
			} else {		// マウスクリックまで、この命令で待機
				curpos--;
			}
			break;
		case 0x02:
			idx = ReadValue();
			idx2 = ReadValue();
			idx3 = ReadValue();
			mouse->GetState(&x, &y, &flag);
			flags->SetVal(idx , x);
			flags->SetVal(idx2, y);
			flags->SetVal(idx3, flag);
			dprintf("   V[%d,%d,%d]=", idx, idx2, idx3);
			dprintf("(X:%d,Y:%d,Btn:%d)¥n", x, y, flag);
			cmd = 0;
			break;
		case 0x03:
			dprintf("   Click Data Flush?¥n");
			mouse->ClickFlush();
			cmd = 0;
			break;
		case 0x20:
			dprintf("   Cursor OFF???¥n");
			mouse->Hide();
			cmd = 0;
			break;
		case 0x21:
			dprintf("   Cursor ON???¥n");
			mouse->Show();
			cmd = 0;
			break;
		default:
			ERROR;
			cmd = 0;
			break;
	}
	return true;
};


// -------------------------------------------------------------------
// 2C - Cmd:$6E CGモード制御（MODE.CGMハンドル）関連
// -------------------------------------------------------------------
bool SCENARIO::d2c(void)
{
	unsigned char subcmd;
	int idx, idx2, idx3;

	subcmd = databuf[curpos++];
	switch(subcmd) {
		case 0x01:		// CGモード対象の総CG数を返す
			idx = ReadValue();
			dprintf("CG Mode GetTotal CG Num  SubCmd:$%02X  Val[%d]=TotalCGNum¥n", subcmd, idx);
			flags->SetVal(idx, sys->GetCGAllNum());
			break;
		case 0x02:		// 見たCG枚数を返す
			idx = ReadValue();
			dprintf("CG Mode Get Displayed CG Num  SubCmd:$%02X  Val[%d]=DispayedNum¥n", subcmd, idx);
			flags->SetVal(idx, sys->GetCGNum());
			break;
		case 0x03:		// CG達成率を返すみたい
			idx = ReadValue();
			dprintf("CG Mode Percentage  SubCmd:$%02X  Val[%d]=Percentage¥n", subcmd, idx);
			flags->SetVal(idx, sys->GetCGPercentage());
			break;
		case 0x04:		// スライドショー
			if ( DecodeAutoCGMode() ) {
				idx = ReadValue();
				dprintf("CG Mode AutoMode  SubCmd:$%02X Arg:%d¥n", subcmd, idx);
			} else {
				curpos--;
				return true;
			}
			break;
		case 0x05:		// Val[idx]番目のCGのCGファイル名をStr[idx2]に、フ ラ グ 番 号 をVal[idx3]に返す
			idx = ReadValue();
			idx2 = ReadValue();
			idx3 = ReadValue();
			dprintf("CG Mode ?????  SubCmd:$%02X  Arg1:%d Arg2:%d Arg3:%d¥n", subcmd, idx, idx2, idx3);
			dprintf("V[Arg1] = %d  V[Arg3] = %d  Arg3 = %d¥n", flags->GetVal(idx), flags->GetVal(idx3), idx3);
//			flags->SetBit(flags->GetVal(idx3), sys->GetCGFlag(flags->GetVal(idx)));
			flags->SetStr(idx2, sys->GetCGName(flags->GetVal(idx)));
			flags->SetVal(idx3, sys->GetCGFlagNum(flags->GetVal(idx)));
			break;
		default:	// ここに来たらおかしい
			dprintf("CG Mode ?????  SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 2D - Cmd:$6F ??????????????????
// -------------------------------------------------------------------
bool SCENARIO::d2d(void)
{
	int data, idx, idx2, idx3;

	data = ReadValue();
	idx  = ReadValue();
	idx2 = ReadValue();
	idx3 = ReadValue();
	// データは読んでるけど、何かをしてる気配はない

	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 2E - Cmd:$70 システム値取得・設定（メッセージウィンドゥ？）
// -------------------------------------------------------------------
bool SCENARIO::d2e(void)
{
	unsigned char subcmd;
	int idx, idx2, idx3, data, attr, r, g, b;

	subcmd = databuf[curpos++];
	switch (subcmd) {
		case 0x01:				// 絶望、MesWin背景色と半透明ON(0)/OFF(1)のフラグを得る
			data = ReadValue();
			idx  = ReadValue();
			idx2 = ReadValue();
			idx3 = ReadValue();
			sys->GetMesWinColor(&attr, &r, &g, &b);
			dprintf("SubCmd:$%02X - Get MesWin BG Flag/Color TransFlag->val[%d] R->val[%d] ", subcmd, attr, r);
			dprintf("G->val[%d] b->val[%d]¥n", g, b);
			flags->SetVal(data, attr);
			flags->SetVal(idx , r);
			flags->SetVal(idx2, g);
			flags->SetVal(idx3, b);
			break;
		case 0x02:				// MesWin背景色と半透明ON(0)/OFF(1)のフラグをセット
			attr = ReadValue();
			r = ReadValue();
			g = ReadValue();
			b = ReadValue();
			sys->SetMesWinColor(attr, r, g, b);
			dprintf("SubCmd:$%02X - Set MesWin BG Flag/Color TransFlag=%d  RGB=(%d,%d,%d)¥n", subcmd, attr, r, g, b);
			break;
		case 0x03:
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Get #WINDOW_MOVE to Val[%d]¥n", subcmd, idx);
			break;
		case 0x04:
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Set %d for #WINDOW_MOVE¥n", subcmd, idx);
			break;
		case 0x05:
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Get #WINDOW_CLEAR_BOX to Val[%d]¥n", subcmd, idx);
			break;
		case 0x06:
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Set %d for #WINDOW_CLEAR_BOX¥n", subcmd, idx);
			break;
		case 0x10:
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Get Window(Waku) Type to Val[%d]¥n", subcmd, idx);
			break;
		case 0x11:
			idx = ReadValue();
			dprintf("SubCmd:$%02X - Set Window(Waku) Type  #%d¥n", subcmd, idx+1);
			sys->ChangeMesWinStyle(idx);
			break;
		default:	// ここに来たらおかしい
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 2F - Cmd:$72 メッセージウィンドゥ関連システム値 取得・設定
// -------------------------------------------------------------------
bool SCENARIO::d2f(void)
{
	unsigned char subcmd;
	int idx, idx2, x, y;

	subcmd = databuf[curpos++];
	cmd = 0;
	dprintf("MesWin Control???  SubCmd:$%02X¥n", subcmd);
	switch (subcmd) {
		case 0x01:		// Get #WINDOW_MSG_POS
			// 現在のメッセージウィンドウの位置を(Val[idx],Val[idx2])にセット
			idx  = ReadValue();
			idx2 = ReadValue();
			dprintf("Set Cur MesWin Pos to Val[%d],Val[%d]¥n", idx, idx2);
			sys->MesWin_GetPos(&x, &y);
			flags->SetVal(idx , x);
			flags->SetVal(idx2, y);
			break;
		case 0x02:		// Get #WINDOW_COM_POS
			// 現在のサブウィンドウの位置を(Val[idx],Val[idx2])にセット
			idx  = ReadValue();
			idx2 = ReadValue();
			dprintf("Set Cur SubWin Pos to Val[%d],Val[%d]¥n", idx, idx2);
			sys->MesWin_GetSubPos(&x, &y);
			flags->SetVal(idx , x);
			flags->SetVal(idx2, y);
			break;
		case 0x03:		// Get #WINDOW_SYS_POS
			idx  = ReadValue();
			idx2 = ReadValue();
			dprintf("Get #WINDOW_SYS_POS to Val[%d],Val[%d]¥n", idx, idx2);
			break;
		case 0x04:		// Get #WINDOW_SUB_POS
			idx  = ReadValue();
			idx2 = ReadValue();
			dprintf("Get #WINDOW_SUB_POS to Val[%d],Val[%d]¥n", idx, idx2);
			break;
		case 0x05:		// Get #WINDOW_GRP_POS
			idx  = ReadValue();
			idx2 = ReadValue();
			dprintf("Get #WINDOW_GRP_POS to Val[%d],Val[%d]¥n", idx, idx2);
			break;
		case 0x11:		// Set #WINDOW_MSG_POS
			// メッセージウィンドウの位置を変更
			x = ReadValue();
			y = ReadValue();
			dprintf("Set (%d,%d) for MesWin Pos¥n", x, y);
			sys->MesWin_SetPos(x, y);
			break;
		case 0x12:		// Set #WINDOW_COM_POS
			// サブウィンドウの位置を変更
			x = ReadValue();
			y = ReadValue();
			dprintf("Set (%d,%d) for SubWin Pos¥n", x, y);
			sys->MesWin_SetSubPos(x, y);
			break;
		case 0x13:		// Set #WINDOW_SYS_POS
			x = ReadValue();
			y = ReadValue();
			dprintf("Set (%d,%d) for #WINDOW_SYS_POS¥n", x, y);
			break;
		case 0x14:		// Set #WINDOW_SUB_POS
			x = ReadValue();
			y = ReadValue();
			dprintf("Set (%d,%d) for #WINDOW_SUB_POS¥n", x, y);
			break;
		case 0x15:		// Set #WINDOW_GRP_POS
			x = ReadValue();
			y = ReadValue();
			dprintf("Set (%d,%d) for #WINDOW_GRP_POS¥n", x, y);
			break;
		default:	// ここに来たらおかしい
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 30 - Cmd:$73 システム値取得・設定
// -------------------------------------------------------------------
bool SCENARIO::d30(void)
{
	unsigned char subcmd;
	int idx, idx2, data, x, y;

	subcmd = databuf[curpos++];
	dprintf("Get/Set System Nums(in GAMEEXE.INI)  SubCmd:$%02X¥n", subcmd);
	switch (subcmd) {
		case 0x01:		// Get #MESSAGE_SIZE
			// 現在のメッセージウィンドウのサイズ（文字数）を得る
			idx  = ReadValue();
			idx2 = ReadValue();
			dprintf("Set Current Meswin W/H(MOJI_X/Y) to Val[%d],Val[%d]¥n", idx, idx2);
			sys->MesWin_GetSize(&x, &y);
			flags->SetVal(idx , x);
			flags->SetVal(idx2, y);
			break;
		case 0x02:		// Set #MESSAGE_SIZE
			// メッセージウィンドウのサイズ（文字数）を変更
			x = ReadValue();
			y = ReadValue();
			dprintf("Set %d,%d for Current MesWin W/H(MOJI_X/Y)¥n", x, y);
			sys->MesWin_SetSize(x, y);
			break;
		case 0x05:		// Get #MSG_MOJI_SIZE
			// メッセージウィンドウのフォントサイズを得る？？？
			idx = ReadValue();
			idx2 = ReadValue();
			dprintf("GetFontSize to Val[%d], Val[%d]¥n", idx, idx2);
			sys->GetFontSize(&x, &y);
			flags->SetVal(idx , x);
			flags->SetVal(idx2, y);
			break;
		case 0x06:		// Set #MSG_MOJI_SIZE
			// メッセージウィンドウのフォントサイズを変更
			x = ReadValue();
			y = ReadValue();
			dprintf("Set X:%d,Y:%d for FontSize¥n", x, y);
			sys->SetFontSize(x, y);
			break;
		case 0x10:		// Get #MOJI_COLOR
			idx = ReadValue();
			dprintf("Get #MOJI_COLOR to Val[%d]¥n", idx);
			break;
		case 0x11:		// Set #MOJI_COLOR
			data = ReadValue();
			dprintf("Set %d for #MOJI_COLOR¥n", data);
			break;
		case 0x12:		// Get #MSG_CANCEL
			idx = ReadValue();
			dprintf("Get #MSG_CANCEL to Val[%d]¥n", idx);
			break;
		case 0x13:		// Set #MSG_CANCEL
			data = ReadValue();
			dprintf("Set %d for #MSG_CANCEL¥n", data);
			break;
		case 0x16:		// Get #MOJI_KAGE
			idx = ReadValue();
			dprintf("Get #MOJI_KAGE to Val[%d]¥n", idx);
			break;
		case 0x17:		// Set #MOJI_KAGE
			data = ReadValue();
			dprintf("Set %d for #MOJI_KAGE¥n", data);
			break;
		case 0x18:		// Get #KAGE_COLOR
			idx = ReadValue();
			dprintf("Get #KAGE_COLOR to Val[%d]¥n", idx);
			break;
		case 0x19:		// Set #KAGE_COLOR
			data = ReadValue();
			dprintf("Set %d for #KAGE_COLOR¥n", data);
			break;
		case 0x1a:		// Get #SEL_CANCEL
			idx = ReadValue();
			dprintf("Get #SEL_CANCEL to Val[%d]¥n", idx);
			break;
		case 0x1b:		// Set #SEL_CANCEL
			data = ReadValue();
			dprintf("Set %d for #SEL_CANCEL¥n", data);
			break;
		case 0x1c:		// Get #CTRL_KEY
			idx = ReadValue();
			dprintf("Get #CTRL_KEY to Val[%d]¥n", idx);
			break;
		case 0x1d:		// Set #CTRL_KEY
			// Ctrlキーによるスキップの有効化／無効化
			data = ReadValue();
			dprintf("CtrlKey Ignore SW Value %d¥n", data);
			sys->SetSkipEnable((data)?false:true);
			break;
		case 0x1e:		// Get #SAVE_START
			idx = ReadValue();
			dprintf("Get #SAVE_START to Val[%d]¥n", idx);
			break;
		case 0x1f:		// Set #SAVE_START
			data = ReadValue();
			dprintf("Set %d for #SAVE_START¥n", data);
			break;
		case 0x20:		// Get DisableNvlText flag (Nvl時テキスト描写OFFフラグ)
			idx = ReadValue();
			dprintf("Get DisableNvlText flag to Val[%d]¥n", idx);
			break;
		case 0x21:		// Set DisableNvlText flag
			data = ReadValue();
			dprintf("Set %d for DisableNvlText flag¥n", data);
			break;
		case 0x22:		// Get #FADE_TIME
			idx = ReadValue();
			dprintf("Get #FADE_TIME to Val[%d]¥n", idx);
			break;
		case 0x23:		// Set #FADE_TIME
			data = ReadValue();
			dprintf("Set %d for #FADE_TIME¥n", data);
			break;
		case 0x24:		// Get #CURSOR_MONO
			idx = ReadValue();
			dprintf("Get #CURSOR_MONO to Val[%d]¥n", idx);
			break;
		case 0x25:		// Set #CURSOR_MONO
			data = ReadValue();
			dprintf("Set %d for #CURSOR_MONO¥n", data);
			break;
		case 0x26:		// Get #COPY_WIND_SW
			idx = ReadValue();
			dprintf("Get #COPY_WIND_SW to Val[%d]¥n", idx);
			break;
		case 0x27:		// Set #COPY_WIND_SW
			data = ReadValue();
			dprintf("Set %d for #COPY_WIND_SW¥n", data);
			break;
		case 0x28:		// Get #MSG SPEED
		case 0x2a:
			// 現在のメッセージ速度を得る
			data = ReadValue();
			dprintf("GetMsgSpeed to Val[%d]¥n", data);
			flags->SetVal(data, sys->GetMsgSpeed());
			break;
		case 0x29:		// Set #MSG SPEED
		case 0x2b:
			// メッセージ速度を変更
			data = ReadValue();
			dprintf("Set %d for Msg Speed¥n", data);
			sys->SetMsgSpeed(data);
			break;
		case 0x2c:		// Get #RETURN_KEY_WAIT
			idx = ReadValue();
			dprintf("Get #RETURN_KEY_WAIT to Val[%d]¥n", idx);
			break;
		case 0x2d:		// Set #RETURN_KEY_WAIT
			data = ReadValue();
			dprintf("Set %d for #RETURN_KEY_WAIT¥n", data);
			break;
		case 0x2e:		// Get #KOE_TEXT_TYPE
			idx = ReadValue();
			dprintf("Get #KOE_TEXT_TYPE to Val[%d]¥n", idx);
			break;
		case 0x2f:		// Set #KOE_TEXT_TYPE
			// KOE再生後のメッセージを表示しなくする
			data = ReadValue();
			dprintf("Make KOE Text disable Arg:%d¥n", data);
			koetextskip = data;
			break;
		case 0x30:		// Get #GAME_SPECK_INIT
		case 0x33:
			idx = ReadValue();
			dprintf("Get #GAME_SPECK_INIT to Val[%d]¥n", idx);
			break;
		case 0x31:		// Set Cursor position
			x = ReadValue();
			y = ReadValue();
			dprintf("Set cursor pos : (%d,%d)¥n", x, y);
			break;
		case 0x32:		// Set DisableKeyMouse flag（キーによるカーソル移動禁止フラグ）
			idx = ReadValue();
			dprintf("Set DisableKeyMouse flag : %d¥n", idx);
			break;
		case 0x34:		// Set #GAME_SPECK_INIT
			idx = ReadValue();
			dprintf("Set %d for #GAME_SPECK_INIT¥n", idx);
			break;
		default:	// ここに来たらおかしい
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 31 - Cmd:$74 ポップアップメニュー制御
// -------------------------------------------------------------------
bool SCENARIO::d31(void)
{
	unsigned char subcmd;
	int idx, data;

	subcmd = databuf[curpos++];
	dprintf("PopupMenu   SubCmd:$%02X¥n", subcmd);
	switch (subcmd) {
		case 0x01:
			idx = ReadValue();
			dprintf("    Get PopupMenu Disable SW to Val[%d]¥n", idx);
			flags->SetVal(idx, mouse->GetPopupFlag());
			break;
		case 0x02:
			idx = ReadValue();
			dprintf("     PopupMenu Disable SW???  Arg:%d¥n", idx);
			if ( idx )
				mouse->DisablePopup();
			else
				mouse->EnablePopup();
			break;
		case 0x03:
			idx = ReadValue();
			data = ReadValue();
			flags->SetVal(data, sys->GetMenuEnable(idx));
			dprintf("     Get PopupMenu Item Enable/Disable Menu#:%d to Val[%d]¥n", idx, data);
			break;
		case 0x04:
			idx = ReadValue();
			data = ReadValue();
			sys->MenuEnable(idx, data);
			dprintf("     PopupMenu Item Enable/Disable Menu#:%d - SW:%d¥n", idx, data);
			break;
		default:	// ここに来たらおかしい
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 32 - Cmd:$75 ************************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d32(void)
{
	unsigned char subcmd;
	int data;

	subcmd = databuf[curpos++];
	data = ReadValue();
	switch (subcmd) {
		case 0x01:			// Get BGM Volume
			dprintf("SubCmd:$%02X  Val[%d] <- BGM Volume¥n", subcmd, data);
			break;
		case 0x02:			// Get WAV Volume
			dprintf("SubCmd:$%02X  Val[%d] <- WAV Volume¥n", subcmd, data);
			break;
		case 0x03:			// Get KOE Volume
			dprintf("SubCmd:$%02X  Val[%d] <- KOE Volume¥n", subcmd, data);
			break;
		case 0x04:			// Get SE Volume
			dprintf("SubCmd:$%02X  Val[%d] <- SE Volume¥n", subcmd, data);
			break;
		case 0x11:			// Set BGM Volume
			dprintf("SubCmd:$%02X  Set BGM Volume : %d¥n", subcmd, data);
			break;
		case 0x12:			// Set BGM Volume
			dprintf("SubCmd:$%02X  Set WAV Volume : %d¥n", subcmd, data);
			break;
		case 0x13:			// Set BGM Volume
			dprintf("SubCmd:$%02X  Set KOE Volume : %d¥n", subcmd, data);
			break;
		case 0x14:			// Set BGM Volume
			dprintf("SubCmd:$%02X  Set SE Volume : %d¥n", subcmd, data);
			break;
		case 0x21:			// Mute BGM
			dprintf("SubCmd:$%02X  Mute BGM : %d¥n", subcmd, data);
			break;
		case 0x22:			// Mute WAV
			dprintf("SubCmd:$%02X  Mute WAV : %d¥n", subcmd, data);
			break;
		case 0x23:			// Mute KOE
			dprintf("SubCmd:$%02X  Mute KOE : %d¥n", subcmd, data);
			sound->KOE_Disable(data);
			break;
		case 0x24:			// Mute SE
			dprintf("SubCmd:$%02X  Mute SE : %d¥n", subcmd, data);
			break;
		default:	// ここに来たらおかしい
			dprintf("SubCmd:$%02X  ??????????? : %d¥n", subcmd, data);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 33 - Cmd:$76 ノベルモード制御
// -------------------------------------------------------------------
bool SCENARIO::d33(void)
{
	unsigned char subcmd;
	int data;

	subcmd = databuf[curpos++];
	// SubCmd=1 以外はエラー？
	switch (subcmd) {
		case 0x01:
			// NVL_SYSTEM フラグの値を変更する
			data = ReadValue();
			dprintf("SubCmd:$%02X  Set NvlMode Flag : %d¥n", subcmd, data);
			sys->SetNovelModeFlag(data);
			break;
		case 0x02:
			data = ReadValue();
			dprintf("SubCmd:$%02X  ???????? Set Somthing flag? : %d¥n", subcmd, data);
			break;
		case 0x04:
			dprintf("SubCmd:$%02X  ???????? No Arg¥n", subcmd);
			break;
		case 0x05:
			dprintf("SubCmd:$%02X  ???????? No Arg¥n", subcmd);
			break;
		default:	// ここに来たらおかしい
			dprintf("?????????  SubCmd:$%02X¥n", subcmd);
			ERROR;
			break;
	}
	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 34 - Cmd:$7F ************************* Unimpremented
// -------------------------------------------------------------------
bool SCENARIO::d34(void)
{
	int data;

	// SubCmd無し、Argは1個固定
	data = ReadValue();

	cmd = 0;
	return true;
};


// -------------------------------------------------------------------
// 35 - Cmd:$FE テキスト表示（半角）
// -------------------------------------------------------------------
bool SCENARIO::d35(void)
{
	return d36();
};


// -------------------------------------------------------------------
// 36 - Cmd:$FF テキスト表示（全角）
// -------------------------------------------------------------------
bool SCENARIO::d36(void)
{
	int pos;
	char buf[1024];

	curpos--; SavePoint(); curpos++;
	if ( sys->Version()>=1714 ) pos = ReadInt();		// AIR / Koigokoro
//	sys->MesWin_Draw();
	buf[0] = cmd;
	ReadText(buf+1);
	sys->MesWin_SetMes(buf);
	dprintf("Text:");
	dprintf(buf+1);
	dprintf("¥n");
	mesflag = 1;
	cmd = databuf[curpos];
	while ( (cmd==0xff)||(cmd==0xfe) ) {
		curpos++;
		if ( sys->Version()>=1714 ) pos = ReadInt();	// AIR / Koigokoro
		buf[0] = cmd;
		ReadText(buf+1);
		sys->MesWin_SetMes(buf);
		cmd = databuf[curpos];
	}
	meswaitbase = sys->GetCurrentTimer();
	cmd = 0;
	dprintf("Cmd $FE/FF End.¥n");
	return true;
};


// -------------------------------------------------------------------
// 37 - Error!!!
// -------------------------------------------------------------------
bool SCENARIO::d37(void)
{
	if ( !cmd ) {
		if ( smenu.num ) {						// ScenarioFileMenuがある場合
			if ( (curpos-1)==smenu.start ) {	// メニュー表示Posだったら
				curpos--;						// セレクト終わるまで、この場で待機
				ScnMenuSel();
			} else {							// それ以外の場所で00に遭遇したらエラー
				ERROR;
			}
		} else {
			dprintf("Scenario End.¥n");
			// ScenarioFile末端でCmd:00が現れたらゲーム終了扱い?（檸檬）
			if ( (databuf+curpos)>=(databuf_base+bufsize-1) ) {
				dprintf("Game End.($20)¥n");
				sys->Terminate();
			} else {
				ERROR;
			}
		}
	} else {
		ERROR;
	}
	cmd = 0;
	return true;
};

/* -------------------------------------------------------------------
  ScnMenuSel
------------------------------------------------------------------- */
void SCENARIO::ScnMenuSel(void)
{
	int i;

	if ( selectflag ) {
		if ( smenu.cur!=-1 ) {
			if ( smenu.subnum[smenu.cur]==1 ) {
				// サブアイテムが1つなら選択不要
				i = 1;
			} else 
				i = sys->Select();
		} else
			i = sys->Select();
		if ( i!=(-1) ) {
//			flags->SetVal(selectindex, i);
			dprintf("### Selected - %d¥n", i);
			sys->MesWin_ClearMes();
//			sys->MesWin_Draw();
			selectflag = 0;
			if ( smenu.cur==-1 ) {
				smenu.cur = i-1;
			} else {
				smenu.cursub = i-1;
				smenu.cursubsub = smenu.subcount[smenu.cur][smenu.cursub];
				if ( smenu.subcount[smenu.cur][smenu.cursub]<(smenu.subcountmax[smenu.cur][smenu.cursub]-1) ) {
					smenu.subcount[smenu.cur][smenu.cursub]++;
				}
				curpos = smenu.substart[smenu.cur][smenu.cursub][smenu.cursubsub];
				smenu.start = curpos;
				smenu.start += ReadInt(curpos-4)-1;
				if ( smenu.flag[smenu.cur][smenu.cursub][smenu.cursubsub] ) {
					if ( smenu.flag[smenu.cur][smenu.cursub][smenu.cursubsub]&0x80 ) {
						smenu.bitcount++;
						smenu.bit = (1<<smenu.bitcount);
					} else {
						smenu.subbitcount = smenu.flag[smenu.cur][smenu.cursub][smenu.cursubsub];
						smenu.subbit = (1<<smenu.subbitcount);
					}
				}
				dprintf("Selected. - %d,%d,%d  jump to $%08X¥n", smenu.cur, smenu.cursub, smenu.cursubsub, curpos+(int)(databuf-databuf_base));
				smenu.cur = -1;
				jumpbase = curpos;
			}
		}
	} else {
		sys->MesWin_ClearMes();
		selectflag = 2;
//		curpos-=2; SavePoint(); curpos+=2;
		if ( smenu.cur==-1 ) {
			if ( smenu.num==1 ) {
				// 親メニューが1つの場合は選択スキップ
				smenu.cur = 0;
				selectflag = 0;
			} else {
				for (i=0; i<smenu.num; i++) {
					if ( smenu.id[i]&smenu.bit )
						sys->Select_AddItem((char*)smenu.str[smenu.strid[i]], 1, 0);
					else
						sys->Select_AddItem((char*)smenu.str[smenu.strid[i]], -1, 0);
				}
			}
		} else {
			if ( (smenu.subnum[smenu.cur]>=2) ) {
				 // サブアイテムが1つの場合は選択スキップ
				for (i=0; i<smenu.subnum[smenu.cur]; i++) {
					if ( smenu.subid[smenu.cur][i]&smenu.subbit )
						sys->Select_AddItem((char*)smenu.str[smenu.substrid[smenu.cur][i]], 1, 0);
					else
						sys->Select_AddItem((char*)smenu.str[smenu.substrid[smenu.cur][i]], -1, 0);	// ダミーを追加
				}
			}
		}
	}
}






/* -------------------------------------------------------------------
  こんすとらくた
------------------------------------------------------------------- */
SCENARIO::SCENARIO(SYSTEM* s, FLAGS* f, SOUND* snd, AVG32MOUSE* m)
{
	sys = s;
	flags = f;
	sound = snd;
	mouse = m;
	databuf_base = 0;
	seennum = -1;

	if ( ChangeSeen(sys->GetStartSeen()) ) {
		dprintf("Base:$%08X  DataBuf:$%08X¥n", (int)databuf_base, (int)databuf);
	} else {
		sys->Abort("シナリオファイルを開けませんでした.");
	}

	curpos = 0;
	cmd = 0;

	memset(&ending, 0, sizeof(ending));

	waitcmd = 0;
	effect.cmd = 0;
	fadecmd = 0;
	mesflag = 0;
	selectflag = 0;
	shakeflag = false;
	anmflag = false;
	multianmflag = 0;
	loading = false;

	lastsaveseen = seennum;
	lastsavepos = 0;
	
	areabuf = 0;
	automodecount = -1;

	koetextskip = 0;
	jumpbase = 0;
	soundwait = 0;
	flashcount = 0;
};


/* -------------------------------------------------------------------
  ですとらくた
------------------------------------------------------------------- */
SCENARIO::‾SCENARIO(void)
{
	int i;
	delete[] databuf_base;
	delete[] areabuf;
	for ( i=0; i<256; i++ )
		if ( ending.pdt[i] ) delete ending.pdt[i];
};


/* -------------------------------------------------------------------
  りせっと（「メニューに戻る」用）
------------------------------------------------------------------- */
void SCENARIO::Reset(int seen, int pos)
{
	if ( !ChangeSeen(seen) ) sys->Terminate();

	curpos = pos;
	cmd = 0;

	lastsaveseen = seennum;
	lastsavepos = curpos;

	waitcmd = 0;
	effect.cmd = 0;
	fadecmd = 0;
	mesflag = 0;
	selectflag = 0;
	shakeflag = false;
	anmflag = false;
	multianmflag = 0;
	loading = false;
	memset(&ending, 0, sizeof(ending));

	delete[] areabuf;
	areabuf = 0;
	automodecount = -1;
	soundwait = 0;
	flashcount = 0;
};


/* -------------------------------------------------------------------
  シナリオの現在位置から NullEnd の String を読む
------------------------------------------------------------------- */
void SCENARIO::ReadText(char* buf)
{
	int n;
	if (databuf[curpos] == '@') { // 文字列変数
		curpos++;
		n =  ReadValue();
		strcpy(buf, flags->GetStr(n));
		dprintf("flags->str[%d] : ", n);
		dprintf(buf);
		dprintf("¥n");
	} else {
		n = strlen((char*)&databuf[curpos]);
		strcpy(buf, (char*)&databuf[curpos]);
		curpos += n+1;
	}
};


/* -------------------------------------------------------------------
  シナリオの現在位置から NullEnd の String をフォーマット付きで読む
------------------------------------------------------------------- */
int SCENARIO::ReadFormattedText(char* b, int* attr)
{
	int ret = 1;			// True
	unsigned char c, cc;
	int len, idx;
	char* buf = b;

	*attr = 0;
	do {
		c = databuf[curpos++];
dprintf("RFT:c=$%02x¥n", (int)c);
		switch (c) {					// テキスト系コマンドがそのまま通るのかな？
			case 0xfe:
			case 0xff:
				strcpy(buf, (char*)&databuf[curpos]);
				len = strlen(buf);
dprintf("RFT:%s (%dchar)¥n", (int)buf, len);
				buf += len;
				curpos += len+1;
				break;
			case 0xfd:
				idx = ReadValue();
				strcpy(buf, flags->GetStr(idx));
				buf += strlen(buf);
				break;
			case 0x28:
				curpos--;
				ret = DecodeConditions();
				if ( (ret!=0)&&(ret!=1) ) {				// 0/1以外の数値だったらアトリビュート
					*attr = ret;
					ret = 1;
				}
				break;
			case 0x10:
				cc = databuf[curpos++];
				switch (cc) {
					case 0x03:
						idx = ReadValue();
						strcpy(buf, flags->GetStr(idx));
						buf += strlen(buf);
						break;
				}
				break;
		}
	} while (c);
	*buf = 0;

	return ret;
};


/* -------------------------------------------------------------------
  シナリオの現在位置から、可変長数値を読む
------------------------------------------------------------------- */
int SCENARIO::ReadValue(void)
{
	int num, ret, i, l;

	num = (int)databuf[curpos];
	l = (num>>4)&7;
	ret = 0;

	for (i=curpos+l-1 ; i>curpos; i--) {
		ret <<= 8; 
		ret |= (int)databuf[i];
	}
	ret <<= 4;
	ret |= num&15;
	curpos += l;

	if ( num&0x80 ) {
		return flags->GetVal(ret);
	} else {
		return ret;
	}
};


/* -------------------------------------------------------------------
  シナリオの現在位置から、Int型の数値（リトルエンディアン）を読む
------------------------------------------------------------------- */
int SCENARIO::ReadInt(void)
{
	int ret;
	ret  =  (int)databuf[curpos++];
	ret |= ((int)databuf[curpos++])<<8;
	ret |= ((int)databuf[curpos++])<<16;
	ret |= ((int)databuf[curpos++])<<24;
	return ret;
};


/* -------------------------------------------------------------------
  シナリオファイルのオフセットposから、Int型の数値（リトルエンディアン）を読む
------------------------------------------------------------------- */
int SCENARIO::ReadInt(int pos)
{
	int ret;
	ret  =  (int)databuf[pos++];
	ret |= ((int)databuf[pos++])<<8;
	ret |= ((int)databuf[pos++])<<16;
	ret |= ((int)databuf[pos++])<<24;
	return ret;
};

	

/* -------------------------------------------------------------------
  条件構文のデコード
------------------------------------------------------------------- */
int SCENARIO::DecodeConditions(void)
{
	unsigned char chr;
	unsigned char buf[1024];
	unsigned char* ptr;
	int i, val1, val2, len, depth = 0;
	int attr = 0;
	int attrarray[8];
	int attrptr;

	ptr = buf;

	attrptr = 0;
	while(1) {
		chr = databuf[curpos++];

		if ( chr=='(' ) {
			dprintf("( ");
			*ptr++ = chr;
			depth++;
		} else if ( chr==')' ) {
			dprintf(") ");
			*ptr++ = chr;
			depth--;
			if ( depth<=0 ) break;
		} else if ( chr==0x26 || chr==0x27 ) {	// 0x26:AND  0x27:OR
			if ( chr==0x26 ) dprintf("&& ");
			else dprintf("|| ");
			*ptr++ = chr;
		} else if ( chr==0x58 ) {				// 選択肢の属性？
			// ( (条件A)$58属性A (条件B)$58属性B ) なんて記述もある模様
			attrarray[attrptr] = databuf[curpos++];
			dprintf(" ??? SetRetValue?($%02X) ", attrarray[attrptr]);
			switch (attrarray[attrptr]) {
				case 0x20:						// 色をつける（Arg=Color）
					attrarray[attrptr] += (ReadValue()<<8);
					break;
				case 0x21:						// 選択肢を追加しない（Argなし）
					break;
				case 0x22:						// 色を付けて選択不可にする（Arg=Color）
					attrarray[attrptr] += (ReadValue()<<8);
					break;
			}
			attrptr++;
			*ptr++ = chr;
		} else if ( chr>=0x36 && chr<=0x55) {	// 条件判定
			val1 = ReadValue();
			val2 = ReadValue();

			switch (chr) {
			case 0x36:
				dprintf("B[%d]!=%d ", val1, val2);
				if ( flags->GetBit(val1)!=val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x37:
				dprintf("B[%d]==%d ", val1, val2);
				if ( flags->GetBit(val1)==val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x38:
				dprintf("B[%d]!=B[%d] ", val1);
				if ( flags->GetBit(val1)!=flags->GetBit(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x39:
				dprintf("B[%d]==B[%d] ", val1, val2);
				if ( flags->GetBit(val1)==flags->GetBit(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x3a:
				dprintf("V[%d](%d)!=%d ", val1, flags->GetVal(val1), val2);
				if ( flags->GetVal(val1)!=val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x3b:
				dprintf("V[%d](%d)==%d ", val1, flags->GetVal(val1), val2);
				if ( flags->GetVal(val1)==val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x41:
			case 0x42:
				dprintf("V[%d]&%d ", val1, val2);
				if ( flags->GetVal(val1)&val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x43:
				dprintf("V[%d]^%d ", val1, val2);
				if ( flags->GetVal(val1)^val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x44:
				dprintf("V[%d]>%d ", val1, val2);
				if ( flags->GetVal(val1)>val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x45:
				dprintf("V[%d]<%d ", val1, val2);
				if ( flags->GetVal(val1)<val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x46:
				dprintf("V[%d]>=%d ", val1, val2);
				if ( flags->GetVal(val1)>=val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x47:
				dprintf("V[%d]<=%d ", val1, val2);
				if ( flags->GetVal(val1)<=val2 ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x48:
				dprintf("V[%d]!=V[%d] ", val1, val2);
				if ( flags->GetVal(val1)!=flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x49:
				dprintf("V[%d]==V[%d] ", val1, val2);
				if ( flags->GetVal(val1)==flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x4f:
			case 0x50:
				dprintf("V[%d]&V[%d] ", val1, val2);
				if ( flags->GetVal(val1)&flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x51:
				dprintf("V[%d]^V[%d] ", val1, val2);
				if ( flags->GetVal(val1)^flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x52:
				dprintf("V[%d]>V[%d] ", val1, val2);
				if ( flags->GetVal(val1)>flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x53:
				dprintf("V[%d]<V[%d] ", val1, val2);
				if ( flags->GetVal(val1)<flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x54:
				dprintf("V[%d]>=V[%d] ", val1, val2);
				if ( flags->GetVal(val1)>=flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			case 0x55:
				dprintf("V[%d]<=V[%d] ", val1, val2);
				if ( flags->GetVal(val1)<=flags->GetVal(val2) ) *ptr++ = 1; else  *ptr++ = 0;
				break;
			default:
				dprintf("???(False) ", val1, val2);
				*ptr++ = 0;
				break;
			}
		}
	}

	len = ptr-buf;
	attrptr = 0;
	while (len>1)
	{
		ptr = buf;
		while (ptr<buf+len)
		{
			if ( *ptr=='(' ) {
				if ( (ptr[2]==')')&&((ptr[1]==0)||(ptr[1]==1)) ) {
					ptr[0] = ptr[1];
					ptr++;
					len -= 2;
					for (i=ptr-buf; i<len; i++) buf[i] = buf[i+2];
					// 0x58 の処理
					if ( *ptr==0x58 ) {
						if ( *(ptr-1)==1 ) {
							attr = attrarray[attrptr];
						}
						attrptr++;
						if ( len>2 ) {
							// 後ろが条件式ならorを追加
							if ( (*(ptr+1)==0)||(*(ptr+1)==1)||(*(ptr+1)=='(') ) {
								*ptr = 0x27;
							} else {
								len--;
								for (i=ptr-buf; i<len; i++) buf[i] = buf[i+1];
							}
						}
					}
					continue;
				}
			} else if (*ptr==0x26) {
				if ( (*(ptr-1)<2) && (*(ptr+1)<2) ) {
					*(ptr-1) &= *(ptr+1);
					for (i=ptr-buf; i<len-2; i++) buf[i] = buf[i+2];
					len -= 2;
					continue;
				}
			} else if (*ptr==0x27) {
				if ( (*(ptr-1)<2) && (*(ptr+1)<2) ) {
					*(ptr-1) |= *(ptr+1);
					for (i=ptr-buf; i<len-2; i++) buf[i] = buf[i+2];
					len -= 2;
					continue;
				}
			}
			ptr++;
		}
	}

	if (*buf) {
		if ( attr )
			return attr;
		else
			return 1;
	} else {
			return 0;
	}
}


/* -------------------------------------------------------------------
  ジャンプ・コール系のサブルーチン
------------------------------------------------------------------- */
void SCENARIO::SavePoint(void)
{
	if ( sys->CheckNovelSave() ) {
		lastsaveseen = seennum;
		lastsavepos = curpos;
		flags->SaveStack();
	}
//	flags->savedstackindex = flags->stackindex;
//	memcpy(flags->savedstackseen, flags->stackseen, sizeof(flags->stackseen));
//	memcpy(flags->savedstackpos, flags->stackpos, sizeof(flags->stackpos));
//	dprintf(" SavePoint. StackIndex=%d¥n", flags->GetStackNum());
};



void SCENARIO::ReadHeader(void)
{
	int index, pad, pad2, str, i, j, k, l;
	int dummy, count;

	memset(&smenu, 0, sizeof(smenu));
	index = ReadInt(0x18);
	pad = index*4+0x50;
	pad2 = ((ReadInt(pad)==5)?4:0);		// 白濁姫（無理矢理つじつま合わせた・・・）
	smenu.num = ReadInt(pad-0x24);
	str = 0;
	count = ReadInt(pad-0x28)-1;
	dprintf("¥n¥nHeader RootMenu Num = %d¥n", smenu.num);
	dprintf("¥n¥nCounter Start = %d¥n", count);
	for (i=0; i<smenu.num; i++) {
		dprintf("Header $%02x¥n", databuf[pad]);
		smenu.id[i] = databuf[pad];
		smenu.subnum[i] = databuf[pad+1];
		smenu.strid[i] = str;
		pad += 2;
		str++;
		for (j=0; j<smenu.subnum[i]; j++) {
			dprintf("  %d : $%02x¥n", j, (int)databuf[pad]);
			smenu.subid[i][j] = databuf[pad];
			smenu.subcountmax[i][j] = databuf[pad+1];
			smenu.substrid[i][j] = str;			// 何番目の文字列が対応するか
			pad += 2;
			str++;
			for (k=0; k<smenu.subcountmax[i][j]; k++) {
				dummy = databuf[pad];
				pad++;
				dprintf("    %d : ", k);
				for (l=0; l<dummy; l++) {
					smenu.flag[i][j][k] = databuf[pad];
					dprintf("(%d,%d,%d), ", databuf[pad], databuf[pad+1], databuf[pad+2]);
					pad += 3;
				}
				dprintf("¥n");
			}
		}
//		smenu.subbit[i] = 1;
//		smenu.subbitcount[i] = 0;
	}
	for (i=0; i<str; i++) {
		dprintf("head string[%2d] = ", i);
		dprintf((char*)(databuf+pad+1));
		dprintf("¥n");
		smenu.str[i] = databuf+pad+1;
		pad += databuf[pad] + 1;
	}
	smenu.start = ReadInt(pad+pad2+0x0f)-1;
	databuf += (pad+pad2+0x13);
	dummy = smenu.start+1;
	if ( count>0 ) {
		smenu.loopback = dummy+4;
		for (i=0; i<count; i++) {
			dummy += ReadInt(dummy)+4;
		}
	}
	dprintf("BaseStartPoint = $%08X¥n", smenu.start);
	for (i=0; i<smenu.num; i++) {
		for (j=0; j<smenu.subnum[i]; j++ ) {
			for (k=0; k<smenu.subcountmax[i][j]; k++ ) {
				smenu.substart[i][j][k] = dummy+4;
				dummy += ReadInt(dummy)+4;
				dprintf("StartPoint[%d][%d][%d] = $%08X¥n", i, j, k, smenu.substart[i][j][k]);
			}
		}
	}
	smenu.cur = -1;		// どの選択にも入っていない
	smenu.bit = 1;
	smenu.bitcount = 0;
	smenu.subbit = 1;
	smenu.subbitcount = 0;
	jumpbase = 0;		// シナリオメニューがある時のローカルオフセット
}


int SCENARIO::ChangeSeen(int s)
{
	unsigned char* newbuf;
	int newsize;
	char buf[256];

	if ( (seennum==s)&&(databuf_base) ) {
		if ( smenu.num ) {
			curpos = smenu.loopback;
smenu.loopback = smenu.loopback+ReadInt(smenu.loopback-4)+4;
			smenu.start = curpos;
			smenu.start += ReadInt(curpos-4)-1;
			jumpbase = curpos;
		} else {
			curpos = 0;
		}
mouse->DisablePopup();
   		return 1;
	}

	sprintf(buf, "SEEN%03d.TXT¥0", s);
	newbuf = sys->ReadFile(buf, "TXT", &newsize);

	if ( newbuf ) {
		delete[] databuf_base;
		databuf_base = newbuf;
		bufsize = newsize;
		seennum = s;
		databuf = databuf_base;
		ReadHeader();
//		index = ReadInt(0x18);
//		pad = ReadInt(index*4+0x50);
//		databuf += index*4+0x63;
//		if ( pad==5 ) databuf += 4;	// 白濁姫
		curpos = 0;
		dprintf("****** Change Seen to #%d¥n", seennum);
mouse->DisablePopup();
		return 1;
	}

	dprintf("****** Fail to change seen to #%d¥n", seennum);
	return 0;
};


static const unsigned char H2ZTable[0x60] = {
	0x40, 0x49, 0x68, 0x94, 0x90, 0x93, 0x95, 0x66, 0x69, 0x6a, 0x96, 0x7b, 0x43, 0x7c, 0x44, 0x5e,
	0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x46, 0x47, 0x83, 0x81, 0x84, 0x48,
	0x97, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
	0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x6d, 0x8f, 0x6e, 0x4f, 0x51,
	0x65, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x6f, 0x62, 0x70, 0x60, 0x45
};

void SCENARIO::Han2Zen(char *s)
{
	char buf[256], *ptr;
	int i;
	unsigned char c;

	strcpy(buf, s);
	i = 0;
	ptr = buf;
	while (*ptr) {
		c = *ptr++;
		if ( (c>=' ')&&(c<=0x80) ) {
			if ( ((c>='0')&&(c<='9')) || ((c>='A')&&(c<='Z')) || ((c>='a')&&(c<='z')) )
				s[i++] = 0x82;
			else
				s[i++] = 0x81;
			s[i++] = H2ZTable[c-0x20];
			if ( i>61 ) break;
		}
	}
	s[i] = 0;
}



/* -------------------------------------------------------------------
  マルチPDTコマンドの処理
------------------------------------------------------------------- */
void SCENARIO::DecodeEndingHandling(void)
{
	int data, i, line, x, y;
	PDTFILE* pdt;

	if ( !ending.flag ) {		// 初期化
		ending.cmd = databuf[curpos++];			// 0x10:Cannot cancel 0x30:w/ mouse cancel
		switch ( ending.cmd ) {
			case 0x10:				//  エンディング系（複数枚PDTのスクロール）
			case 0x20:
			case 0x30:
				ending.poscmd = databuf[curpos++];		// To base pos, 1:centering / 3:right align / Others:left align
				ending.num = databuf[curpos++];			// Picture nums
				ending.pos = ReadValue();				// Base pos
				ending.wait = ReadValue();				// Wait
				ending.pixel = ReadValue();				// Step pixel
				if ( ending.cmd==0x30 ) {
					waitcancelindex = ReadValue();		// Set 1 to this Val, if Canceled
				}
				dprintf("MultiPDT (for Ending?)  Cmd:$%02X PosCmd:$%02X Num:%d ", ending.cmd, ending.poscmd, ending.num);
				dprintf("PosBase:%d Wait:%d Pixel:%d¥n", ending.pos, ending.wait, ending.pixel);
				ending.totalline = 0;
				for (i=0; i<ending.num; i++) {
					ReadText(ending.file[i]);	// CG Name
					data = ReadValue();			// Spacing to next picture
					dprintf("#%d : ", i);
					dprintf(ending.file[i]);
					dprintf(" - Pad:%d ", data);
					pdt = sys->OpenPDT(ending.file[i]);
					if ( pdt ) {
						ending.pdtx[i] = pdt->GetSizeX();
						ending.pdty[i] = pdt->GetSizeY();
						ending.pdtpos[i] = ending.totalline;
						ending.totalline += ending.pdty[i]+data;
						dprintf(" SizeX:%d SizeY:%d Pos:%d  Total:%d¥n", ending.pdtx[i], ending.pdty[i], ending.totalline);
						delete pdt;
						pdt = 0;
					}
					ending.pdt[i] = 0;
				}
				curpos++;		// ???
				sys->SnrPDT_Copy(0, 0, 639, 479, 0, 0, 0, 1, 0);		// Backup...
				ending.line = 0;
				ending.flag = 1;
				ending.curtime = sys->GetCurrentTimer();
				break;

			case 0x03:					// 複数枚のPDTを順番に表示していく（flowers）
				ending.num = databuf[curpos++];			// Picture nums
				ending.pos = ReadValue();				// ?????
				ending.wait = ReadValue();				// ?????
				dprintf("MultiPDT (flowers OP)  Cmd:$%02X Num:%d ", ending.cmd, ending.num);
				dprintf("Arg1:%d Arg2:%d¥n", ending.pos, ending.wait);
				for (i=0; i<ending.num; i++) {
					ReadText(ending.file[i]);			// CG Name
					data = ReadValue();					// Wait to next picture ???
					ending.pdtpos[i] = data;
					dprintf(ending.file[i]);
					dprintf(" - Wait?:%d¥n", data);
				}
				curpos++;								// ???
				effect.cmd = 2;							// まず1枚目を表示しておく
				effect.steptime = 0;					// 即時表示のエフェクトを使おう
				sys->SnrPDT_LoadEffect(ending.file[0], &effect);
				ending.curtime = sys->GetCurrentTimer();
				ending.line = 0;						// いくつ目のPDTかを指すのに使おう
				ending.flag = 1;
				ending.wait = ending.pdtpos[0];
				break;

			case 0x04:					// 複数枚のPDTを順番に表示、ループする（BabyFace）
				ending.num = databuf[curpos++];			// Picture nums
				ending.pos = ReadValue();				// ?????
				ending.wait = ReadValue();				// ?????
				dprintf("MultiPDT (BabyFace ED)  Cmd:$%02X Num:%d ", ending.cmd, ending.num);
				dprintf("Arg1:%d Arg2:%d¥n", ending.pos, ending.wait);
				for (i=0; i<ending.num; i++) {
					ReadText(ending.file[i]);			// CG Name
					data = ReadValue();					// Wait to next picture ???
					ending.pdtpos[i] = data;
					dprintf(ending.file[i]);
					dprintf(" - Wait?:%d¥n", data);
				}
				curpos++;								// ??? 終了マーク？
				effect.cmd = 2;							// まず1枚目を表示しておく
				effect.steptime = 0;					// 即時表示のエフェクトを使おう
				sys->SnrPDT_LoadEffect(ending.file[0], &effect);
				ending.curtime = sys->GetCurrentTimer();
				ending.line = 0;						// いくつ目のPDTかを指すのに使おう
				ending.flag = 1;
				ending.wait = ending.pdtpos[0];
				break;

			case 0x05:					// $04の動作を止める？（BabyFace）
				dprintf("MultiPDT Stop? (BabyFace ED)¥n");
				cmd = 0;
				ending.cmd = 0;
				ending.flag = 0;
				break;

			default:
				dprintf("MultiPDT  Unknown Style  Cmd:$%02X¥n", ending.cmd);
				break;
		}
	} else {					// 実行中
		switch ( ending.cmd ) {
			case 0x10:				//  エンディング系（複数枚PDTのスクロール）
			case 0x20:
			case 0x30:
				if ( ending.cmd==0x30 ) {
					if ( mouse->GetButton() ) {
						flags->SetVal(waitcancelindex, 1);
						cmd = 0;
						ending.cmd = 0;
						ending.flag = 0;
						dprintf("   ... Canceled.¥n");
						for (i=0; i<ending.num; i++) {
							if ( ending.pdt[i] ) delete ending.pdt[i];
							ending.pdt[i] = 0;
						}
					}
				}
				if ( (sys->GetCurrentTimer()-ending.curtime)<ending.wait ) return;
				ending.curtime = sys->GetCurrentTimer();
				dprintf("   ... Line:%d¥n", ending.line);
				for (i=0; i<ending.num; i++) {
					line = ending.line-ending.pdtpos[i];
					if ( (line>=0)&&((line-ending.pdty[i])<480) ) {
						if ( ending.poscmd==1 )
							x = ending.pos-(ending.pdtx[i]/2);
						else if ( ending.poscmd==3 )
							x = ending.pos-ending.pdtx[i];
						else
							x = ending.pos;
							y = 479-line;
						if ( !ending.pdt[i] ) {
							dprintf("   Make PDT Buffer #%d for ", i);
							dprintf(ending.file[i]);
							dprintf("¥n");
							ending.pdt[i] = sys->MakePDT(ending.file[i]);
						}
						if ( ending.pdt[i] ) {
							sys->SnrPDT_Copy(x, y, x+ending.pdtx[i]-1, y+ending.pdty[i]+ending.pixel-1, 1, x, y, 2, 0);
							sys->SnrPDT_MaskCopy(0, 0, ending.pdtx[i]-1, ending.pdty[i]-1, ending.pdt[i], x, y, 2, 0);
							sys->SnrPDT_Copy(x, y, x+ending.pdtx[i]-1, y+ending.pdty[i]+ending.pixel-1, 2, x, y, 0, 0);
						}
					} else {
						if ( ending.pdt[i] ) {
							dprintf("   Delete ending PDT #%d¥n", i);
							delete ending.pdt[i];
							ending.pdt[i] = 0;
						}
					}
				}
				ending.line += ending.pixel;
				if ( ending.line>=ending.totalline ) {
					flags->SetVal(waitcancelindex, 0);
					cmd = 0;
					ending.cmd = 0;
					ending.flag = 0;
					dprintf("   ... Finished.¥n");
					for (i=0; i<ending.num; i++) {
						if ( ending.pdt[i] ) delete ending.pdt[i];
						ending.pdt[i] = 0;
					}
				}
				break;

			case 0x03:					// 複数枚のPDTを順番に表示していく（flowers）
				if ( (sys->GetCurrentTimer()-ending.curtime)<ending.wait ) return;
				ending.curtime = sys->GetCurrentTimer();
				ending.line++;
				if ( ending.line==ending.num ) {	// 全部表示したら終わり
					cmd = 0;
					ending.cmd = 0;
					ending.flag = 0;
					dprintf("   ... Finished.¥n");
				} else {
					effect.cmd = 2;
					effect.steptime = 0;
					sys->SnrPDT_LoadEffect(ending.file[ending.line], &effect);
					ending.wait = ending.pdtpos[ending.line];
				}
				break;

			case 0x04:					// 複数枚のPDTを順番に表示、ループする（BabyFace）
				if ( (sys->GetCurrentTimer()-ending.curtime)<ending.wait ) return;
				ending.curtime = sys->GetCurrentTimer();
				ending.line++;
				if ( ending.line==ending.num ) {	// 全部表示したらループ・・・なんだけど
					cmd = 0;
					ending.cmd = 0;
					ending.flag = 0;
					dprintf("   ... Finished.¥n");
				} else {
					effect.cmd = 2;
					effect.steptime = 0;
					sys->SnrPDT_LoadEffect(ending.file[ending.line], &effect);
					ending.wait = ending.pdtpos[ending.line];
				}
				break;
		}
	}
};


/* -------------------------------------------------------------------
  エリア情報検索用ルーチン
------------------------------------------------------------------- */
void SCENARIO::Area_Read(char* f)
{
	int dummy;
	char buf[64];

	if ( areabuf ) {
		delete[] areabuf;
		areabuf = 0;
	}

	memset(areaflag, 1, 256);

	sprintf(buf, "%s.ARD", f);
	areabuf = sys->ReadFile(buf, "ARD", &dummy);
	areax = (int)areabuf[0x08]+((int)areabuf[0x09]<<8)+((int)areabuf[0x0a]<<16)+((int)areabuf[0x0b]<<24);
	areay = (int)areabuf[0x0c]+((int)areabuf[0x0d]<<8)+((int)areabuf[0x0e]<<16)+((int)areabuf[0x0f]<<24);
};

int SCENARIO::Area_Find(int x, int y)
{
	int ret = 0;
	if ( areabuf ) {
		dprintf("   - Area Find:(%d,%d)", x, y);
		if ( (x>=0)&&(y>=0)&&(x<640)&&(y<480) ) {
			x = (x*areax)/640;
			y = (y*areay)/480;
			ret = areabuf[y*areax+x+0x120];
		}
		if ( !areaflag[ret] ) ret = 0;
		dprintf("=%d¥n", ret);
	}
	return ret;
};


void SCENARIO::Area_Enable(int n)
{
	if ( n<256 ) areaflag[n] = 1;
};


void SCENARIO::Area_Disable(int n)
{
	if ( n<256 ) areaflag[n] = 0;
};


void SCENARIO::Area_Clear(void)
{
	if ( areabuf ) {
		delete[] areabuf;
		areabuf = 0;
		memset(areaflag, 1, 256);
	}
};


/* -------------------------------------------------------------------
  CG Auto Mode
------------------------------------------------------------------- */
int SCENARIO::DecodeAutoCGMode(void)
{
	if ( (automodecount==-1)||(mouse->GetButton())||(sys->CheckSkip()) ) {
		while (1) {
			automodecount++;
			if ( automodecount>=sys->GetCGAllNum() ) {
				automodecount = -1;
				return 1;
			}
			if ( sys->GetCGFlag(automodecount) ) {
				sys->CopySel(0, &effect);
				sys->SnrPDT_LoadEffect(sys->GetCGName(automodecount), &effect);
				break;
			}
		}
	}
	return 0;
};


/* -------------------------------------------------------------------
  1タイマステップ分のシナリオをデコードして実行する
------------------------------------------------------------------- */
void SCENARIO::Run()
{
	bool endflag;
	do {
		if ( sys->MesWin_InEffect() ) {
		} else if ( effect.cmd ) {
			sys->Effect(&effect);
//			endflag = true;
			if ( !sys->CheckSkip() ) endflag = true;
		} else if ( mesflag ) {
			PrintMessage();
			if ( !sys->CheckSkip() ) endflag = true;
		} else {
			if ( (multianmflag)&&(cmd!=0x13) ) {
				if ( (multianmflag==1)||(sound->KOE_IsPlaying()) ) sys->MultiAnimationExec();
			}
			if ( !cmd ) {
				cmd = databuf[curpos++];
				dprintf("$%08X : Cmd $%02X - ", curpos-1, cmd);
			}
 			endflag = Decode();
		}
	} while (!endflag);
};



void SCENARIO::PrintMessage(void)
{
	if ( mesflag==-1 ) {
		if ( !(sys->CheckSkip()) ) {
			sys->MesWin_DrawIcon(1);
			if ( !mouse->GetButton() ) return;
			meswaitbase = sys->GetCurrentTimer();
		}
		mesflag = 1;
		sys->MesWin_ClearWindow();
	} else {
		if ( (sys->CheckSkip())||(mouse->GetButton()) ) {
			mesflag = sys->MesWin_PrintMesAll();
		} else {
			if ( (sys->GetCurrentTimer()-meswaitbase)<sys->GetMesWait() ) return;
			meswaitbase = sys->GetCurrentTimer();
			mesflag = sys->MesWin_PrintMes();
		}
	}
}
