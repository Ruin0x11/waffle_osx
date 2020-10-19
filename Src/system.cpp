/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  system.cpp
    Macシステム依存部分
=======================================================================*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "debug.h"
#include "system.h"
#include "cgmfile.h"
#include "flags.h"
#include "scenario.h"
#include "pdtmacro.h"
#include "nvlfont.h"
#include "pdtmgr.h"
#include "pdtbuf.h"
#include "pdtfile.h"
#include "soundmgr.h"
#include "mousectl.h"

#define TIMEDIV 1000		// ms?
// Window ID
#define WINDOW_MAIN			128
#define DIALOG_ABOUT		128

// Menu ID
#define MENU_APPLE			128
#define MENU_FILE			129
#define MENU_SYSTEM			130
#define MENU_POPUP			135
#define MENU_DEBUG			140

// Apple Menu
#define MENUITEM_ABOUT		1

// File Menu
#define MENUITEM_MENU		4
#define MENUITEM_QUIT		6

// Debug Menu
#define MENUITEM_STARTLOG	1
#define MENUITEM_STOPLOG	2
#define MENUITEM_OTHER0		3
#define MENUITEM_OTHER1		4


SYSTEM::SYSTEM(WindowPtr wnd, GWorldPtr os, PixMapHandle pm, PDTBUFFER* pdtb)
{
	hWnd = wnd;
	offscreen = os;
	pixmap = pm;
	Handle menu;

	AbortFlag = 0;

	memcpy(menuname[0], "\pSAVE", 5);
	memcpy(menuname[1], "\pLOAD", 5);
	menu = GetNewMBar(128);
	SetMenuBar(menu);
	applmenu = GetMenu(MENU_APPLE);
	filemenu = GetMenu(MENU_FILE);
	systmenu = NewMenu(MENU_SYSTEM, "\pシステム");
	popupmenu = NewMenu(MENU_POPUP, "\pContextMenu");
	InsertMenu(applmenu,  0);
	AppendResMenu(applmenu, 'DRVR');
	InsertMenu(filemenu,  0);
	InsertMenu(systmenu,  0);
#ifdef DEBUGLOG
	debugmenu = NewMenu(MENU_DEBUG, "\pでばっぐ");
	InsertMenuItem(debugmenu, "\pログ開始", 1);
	InsertMenuItem(debugmenu, "\pログ停止", 2);
	InsertMenuItem(debugmenu, "\pFlag All ON", 3);
	InsertMenuItem(debugmenu, "\pFlag All OFF", 3);
	InsertMenu(debugmenu,  0);
#endif

	memset(&ini, 0, sizeof(INIFILE));
	memset(menus, 0, sizeof(menus));
	memset(menushier, 0, sizeof(menushier));
	memset(selitemenable, 0, sizeof(selitemenable));
	ReadINIFile(":GAMEEXE.INI");

	flags = new FLAGS();
	nvlfont = new NOVELFONT(this);
	mouse = new AVG32MOUSE(this);
	mgr = new PDTMGR(this, pdtb, (unsigned char*)GetPixBaseAddr(pixmap));
	macro = new PDTMACRO(mgr);
	cgm = new CGMODE(this, flags);

	sound = new SOUND(this, &ini);
	scn = new SCENARIO(this, flags, sound, mouse);

	anmbuf = 0;
	multianmnum = 0;
	ActiveFlag = 1;
	RunningFlag = 0;
	loading = 0;
	hidewindow = 0;
	MesWinStyle = 1;
	MesWinStyleForce = 0;
	MouseClickFlag = 0;
	KeyDownFlag = 0;
	selitemflag = 0;
	MesWinFlag = 0;
	MesIconFlag = 0;
	mesicon = 0;
	SkipEnableFlag = 1;
	FullScrSW = 0;
	memset(&scnefct, 0, sizeof(EFFECT));
	GetFNum("\pOsaka", &fontid);
	curfont = 2;
	doubletext = 0;
	doubleline = 0;
}


SYSTEM::~SYSTEM(void)
{
	int i;

	SaveGlobalFlags(flags);

	if ( FullScrSW ) ChangeFullScreen(0);	// 解像度を元に戻しておく
	DisposeGWorld(offscreen);

	if ( scn     ) delete scn;
	if ( cgm     ) delete cgm;
	if ( macro   ) delete macro;
	if ( mgr     ) delete mgr;
	if ( nvlfont ) delete nvlfont;
	if ( mouse   ) delete mouse;
	if ( sound   ) delete sound;
	if ( flags   ) delete flags;

	if ( anmbuf  ) delete[] anmbuf;

	DisposeMenu(applmenu);
	DisposeMenu(filemenu);
	DisposeMenu(systmenu);
	DisposeMenu(popupmenu);
	for (i=0; i<=MAX_MENU; i++) DisposeMenu(menus[i]);
}


SYSTEM* SYSTEM::Create(WindowPtr wnd)
{
	SYSTEM* ret = NULL;
	PDTBUFFER* pb;
	GWorldPtr os;
	PixMapHandle pm;
	Rect rect;

	SetRect(&rect, 0, 0, 640, 480);
	if ( NewGWorld(&os, 32, &rect, 0, 0, 0)==noErr ) {
		pm = GetGWorldPixMap(os);
		pb = new PDTBUFFER(640, 480, 4, ((*pm)->rowBytes)&0x3fff, false);
		if ( pb ) {
			ret = new SYSTEM(wnd, os, pm, pb);
		} else {
			delete pb;
			DisposeGWorld(os);
		}
	}
	return ret;
}


void SYSTEM::Init(void)
{
	int i, n, m, pad;

	CheckVersion();
	SetWindowTitle("", 1);
	MesWin_Setup(MesWinStyle);
	AddSaveLoadMenu();
	LoadGlobalFlags(flags);
	ClearScreen();
	memset(menuid, -1, sizeof(menuid));
	memset(menusysid, -1, sizeof(menusysid));
	memset(menuflag, 1, sizeof(menuflag));
	pad = 0;
	if ( menus[25] ) {
		InsertMenuItem(popupmenu, "\p ", 0);
		SetMenuItemText(popupmenu, 1, menuname[25]);
		InsertMenuItem(popupmenu, "\p-", 1);
		InsertMenuItem(systmenu, "\p ", 0);
		SetMenuItemText(systmenu, 1, menuname[25]);
		InsertMenuItem(systmenu, "\p-", 1);
		menuid[1] = 25;
		menusysid[1] = 25;
		pad += 2;
	}
	if ( menus[31] ) {
		InsertMenuItem(popupmenu, "\p ", pad);
		SetMenuItemText(popupmenu, pad+1, menuname[31]);
		InsertMenuItem(popupmenu, "\p-", pad+1);
		InsertMenuItem(systmenu, "\p ", pad);
		SetMenuItemText(systmenu, pad+1, menuname[31]);
		InsertMenuItem(systmenu, "\p-", pad+1);
		menuid[pad+1] = 31;
		menusysid[pad+1] = 31;
		pad += 2;
	}
	for (i=0, n=pad, m=pad; i<MAX_MENU-1; i++) {
		if ( (menus[i])&&(i!=25)&&(i!=30) ) {
			n++;
			InsertMenuItem(popupmenu, menuname[i], i+pad);
			if ( (i!=0)&&(i!=1)&&(i!=28)&&(i!=29) ) {
				InsertMenuItem(systmenu, menuname[i], i+pad);
			}
			if ( (i!=0)&&(i!=1)&&(i!=28)&&(i!=29) ) {
				m++;
				menusysid[m] = i;
			}
			menuid[n] = i;
			if ( menushier[i] ) {
				SetMenuItemHierarchicalID(popupmenu, n, i+1);
				if ( (i!=0)&&(i!=1)&&(i!=28)&&(i!=29) ) {
					menusysid[m] = i;
					SetMenuItemHierarchicalID(systmenu, m, i+1);
				}
				InsertMenu(menus[i], -1);
			} else if ( i==9 ) {		// Font
				AppendResMenu(menus[9],  'FONT');
				SetMenuItemHierarchicalID(popupmenu, n, 10);
				SetMenuItemHierarchicalID(systmenu, m, 10);
				InsertMenu(menus[9], -1);
			}
		}
	}
	SetMenuItemText(filemenu, 1, menuname[0]);
	SetMenuItemText(filemenu, 2, menuname[1]);
	SetMenuItemText(filemenu, 4, menuname[28]);
	SetMenuItemHierarchicalID(filemenu, 1, 1);	// File-Save
	SetMenuItemHierarchicalID(filemenu, 2, 2);	// File-Load
	InsertMenu(popupmenu, -1);
	if ( menus[5] ) CheckItem(menus[5], 2, true);		// Window Mode
	if ( menus[9] ) CheckItem(menus[9], 2, true);		// Font
	if ( menus[12] ) CheckItem(menus[12], 1, true);		// Window Style
	PopupMenuEnable(1);
	DrawMenuBar();

	if ( ini.exfont[0] ) mgr->LoadFile(ini.exfont, EXFONTPDT);
}


void SYSTEM::Reset(void)
{
	multianmnum = 0;
	mesbuf[0] = 0;
	mesbufptr = mesbuf;
	curmesx = 0;
	curmesy = 0;
	indentflag = 0;
	mesindent = 0;
	MesIconFlag = 0;
	MesWinFlag = 0;
//	MesWinStyleForce = 0;
	MouseClickFlag = 0;
	KeyDownFlag = 0;
	mesredraw = 0;
	selitemnum = 0;
	selitemflag = 0;
	selfinish = 0;
	selcurid = 0;
	selnovelsetup = 0;
	mesicon = 0;
	SkipEnableFlag = 1;
	subwinflag = 0;
	doubletext = 0;
	doubleline = 0;
	MesWin_Setup(MesWinStyle);
	ResetTimer();
	sound->Sound_Stop();
	sound->KOE_Stop();
	memset(selitemenable, 0, sizeof(selitemenable));
}


void SYSTEM::Abort(char* mes)
{
	char buf[256];
	sprintf(buf, "%s 強制終了します.", mes);
	Error(buf);
	AbortFlag = 1;
}


void SYSTEM::EventLoop(void)
{
	EventRecord event;
	char code;
//sound->CD_Play("RB2SONG", 1);
	RunningFlag = 1;
	do {
		if ( AbortFlag ) {
			RunningFlag = 0;
			break;
		}
		if ( WaitNextEvent(everyEvent, &event, 0, nil) ) {
			switch(event.what) {

				case keyDown:
					code = (char)(event.message&charCodeMask);
					if ( event.modifiers&cmdKey )
						MenuFunc(MenuKey(code));
					else
						KeyDownFunc(code);
					break;

				case autoKey:
					code = (char)(event.message&charCodeMask);
					if ( event.modifiers&cmdKey )
						MenuFunc(MenuKey(code));
					break;

				case updateEvt:
					Update((WindowPtr)event.message);
					break;

				case mouseDown:
					MouseDownFunc(&event);
					DrawMenuBar();
					break;

				case mouseUp:
					MouseUpFunc(&event);
					break;

				case osEvt:
					OSEventFunc(event.message);
					break;

				default:
					break;
			}
		} else {
			IdleFunc();
		}
	} while(RunningFlag);
}


//--------------------------------------------------------------------
//   各イベント処理
//--------------------------------------------------------------------
// フルスクリーン切り替え
/*	Flags
　	1	カーソルを消す
	2	他のアプリにも制御を移す 解像度は変わらない
	4	メニューバーを消さない
	8	解像度を変えない
*/
void SYSTEM::ChangeFullScreen(int sw)
{
	short w = 640;
	short h = 480;
	RGBColor col = {0, 0, 0};
	Rect rect;
	PDTBUFFER *newpdt0 = 0, *oldpdt0, *pdt1bk;

	// 古い画面を保存するためのPDT用意
	oldpdt0 = new PDTBUFFER(640, 480, 3, 640*3, true);
	if ( oldpdt0 ) {
		// 一時的にPDT1と差し換えてバックアップを取る
		pdt1bk = mgr->GetPDT(1);
		mgr->SetPDT(1, oldpdt0);
		mgr->AllCopy(0, 1, 0);
		// 古いGWorldを削除
		if ( mgr->GetPDT(0) ) delete (mgr->GetPDT(0));
		DisposeGWorld(offscreen);

		// 新しいオフスクリーンを作る
		SetRect(&rect, 0, 0, 640, 480);
		if ( NewGWorld(&offscreen, 32, &rect, 0, 0, 0)==noErr ) {

		// 画面切り替え
			if ( sw ) {
#if 0
				FullScrMenu = 1;
				MenuOFF();
				DisposeWindow(hWnd);
				BeginFullScreen(&bkfullscreen, 0, &w, &h, &hWnd, &col, 4/*Flags*/);
			} else {
				EndFullScreen(bkfullscreen, 0);
				hWnd = GetNewCWindow(128, NULL, (WindowPtr)-1);
				SizeWindow(hWnd, 640, 480, true);
				FullScrMenu = 0;
				MenuON();
				DrawMenuBar();
#else
				DisposeWindow(hWnd);
				BeginFullScreen(&bkfullscreen, 0, &w, &h, &hWnd, &col, 0/*Flags*/);
			} else {
				EndFullScreen(bkfullscreen, 0);
				hWnd = GetNewCWindow(128, NULL, (WindowPtr)-1);
				SizeWindow(hWnd, 640, 480, true);
#endif
			}
			pixmap = GetGWorldPixMap(offscreen);
			newpdt0 = new PDTBUFFER(640, 480, 4, ((*pixmap)->rowBytes)&0x3fff, false);
			mgr->SetPDT(0, newpdt0);
			mgr->AllCopy(1, 0, 0);
			ShowWindow(hWnd);
		}

		// PDT1を元に戻して、バックアップ用のPDT0削除
		mgr->SetPDT(1, pdt1bk);
		delete oldpdt0;
	}

	// エラーが起こっていたら終了する
	if ( !newpdt0 ) Abort("画面切り替えに失敗しました.");
}


// メニュー
void SYSTEM::MenuFunc(long m)
{
//	short fontid;
	Str255 buf;
	int seen, pos, i, menu;

	menu = HiWord(m);
	if ( menu<128) menu--;
	switch (menu) {

		case MENU_APPLE:
			switch(LoWord(m)) {
				case MENUITEM_ABOUT:
					AboutDlg();
					break;
				default:
					GetMenuItemText(GetMenuHandle(MENU_APPLE), LoWord(m), buf);
					OpenDeskAcc(buf);
                    break;
			}
			break;

		case MENU_FILE:
			switch(LoWord(m)) {
				case MENUITEM_QUIT:
					if ( QuitDlg() ) RunningFlag = 0;
					break;

				case MENUITEM_MENU:
					if ( MenuDlg() ) {
						Reset();
						scn->Reset(ini.menuseen, 0);
					}
					break;
			}
			break;

		case 0:		// Save
			Save(flags, LoWord(m)-1, scn->GetSaveSeen(), scn->GetSavePos());
			break;

		case 1:		// Load
			Load(flags, LoWord(m)-1, &seen, &pos);
			Reset();
			scn->Reset(seen, pos);
			loading = 1;
			break;

		case 4:		// VolumeCntrol
			break;

		case 5:		// ScrMode
			if ( LoWord(m)==1 ) {
				if ( !FullScrSW ) {
					FullScrSW = 1;
					ChangeFullScreen(FullScrSW);
					if ( menus[5] ) {
						CheckItem(menus[5], 1, true);
						CheckItem(menus[5], 2, false);
					}
				}
			} else {
				if ( FullScrSW ) {
					FullScrSW = 0;
					ChangeFullScreen(FullScrSW);
					if ( menus[5] ) {
						CheckItem(menus[5], 1, false);
						CheckItem(menus[5], 2, true);
					}
				}
			}
			break;

		case 9:		// Font
			if ( menus[9] ) CheckItem(menus[9], curfont, false);
			curfont = LoWord(m);
			if ( menus[9] ) GetMenuItemText(menus[9], curfont, buf);
			GetFNum(buf, &fontid);
			CheckItem(menus[9], curfont, true);
			MesWin_Setup(MesWinStyle);
			break;

		case 12:	// Waku Type
			if ( menus[12] ) CheckItem(menus[12], MesWinStyle, false);
			MesWinStyle = LoWord(m);
			if ( menus[12] ) CheckItem(menus[12], MesWinStyle, true);
			MesWin_Setup(MesWinStyle);
			break;

		case 28:	// Return to Menu
			if ( LoWord(m)==2 ) {
				Reset();
				scn->Reset(ini.menuseen, 0);
			}
			break;

		case 29:	// Game End
			if ( LoWord(m)==2 ) {
				RunningFlag = 0;
			}
			break;

		case MENU_SYSTEM:
			switch(menusysid[LoWord(m)]) {
				case 2:		// Message Speed
					MsgSpeedDlg();
					break;
				case 3:		// Message window BG color
					ColorCfgDlg();
					MesWin_Setup(MesWinStyle);
					break;
				case 31:	// Hide message window
					if ( (!hidewindow)&&(MesWinFlag) ) {
						hidewindow = 1;
						MesWin_HideTemp();
					}
					break;
			}
			break;

		case MENU_POPUP:
			switch(menuid[LoWord(m)]) {
				case 2:		// Message Speed
					MsgSpeedDlg();
					break;
				case 3:		// Message window BG color
					ColorCfgDlg();
					MesWin_Setup(MesWinStyle);
					break;
				case 31:	// Hide message window
					if ( (!hidewindow)&&(MesWinFlag) ) {
						hidewindow = 1;
						MesWin_HideTemp();
					}
					break;
			}
			break;

		case MENU_DEBUG:
			if ( LoWord(m)==MENUITEM_STARTLOG ) {
				IsLogging = true;
			} else if ( LoWord(m)==MENUITEM_STOPLOG ) {
				IsLogging = false;
			} else if ( LoWord(m)==MENUITEM_OTHER0 ) {
				for (i=1000; i<2000; i++) flags->SetBit(i, 1);
				for (i=1000; i<2000; i++) flags->SetVal(i, 1);
			} else if ( LoWord(m)==MENUITEM_OTHER1 ) {
				for (i=1000; i<2000; i++) flags->SetBit(i, 0);
			}
			break;
	}
	HiliteMenu(0);
}


// マウスクリック
void SYSTEM::MouseDownFunc(EventRecord* ev)
{
	WindowPtr win;
	Rect rect;

	switch(FindWindow(ev->where, &win)) {

		case inContent:			// ウィンドウ内
			mouse->ButtonDown();
			break;

		case inMenuBar:			// メニュー
			MenuFunc(MenuSelect(ev->where));
			break;

		case inDrag:			// タイトルバー
			rect = qd.screenBits.bounds;
			InsetRect(&rect, 5, 5);
			DragWindow(win, ev->where, &rect);
			break;

		case inGoAway:			// クローズボタン
			if ( TrackGoAway(win, ev->where) ) {
				if ( QuitDlg() ) RunningFlag = 0;
			}
			break;
	}
}


// マウスリリース
void SYSTEM::MouseUpFunc(EventRecord* ev)
{
	WindowPtr win;
	switch(FindWindow(ev->where, &win)) {

		case inContent:			// ウィンドウ内
			if ( hidewindow ) {
				hidewindow = 0;
				MesWin_Setup(MesWinStyle);
			} else {
				mouse->ButtonUp();
			}
			break;

		case inMenuBar:			// メニュー
			break;

		case inDrag:			// タイトルバー
			break;

		case inGoAway:			// 他ウィンドウ
			break;
	}
}


// キー入力
void SYSTEM::KeyDownFunc(char code)
{
/*
char buf[256];
sprintf(buf, "KeyDown  Code:$%02X", code);
Error(buf);
*/
	switch(code) {
		case 0x03:				// Return (10key) (マウスクリックに相当)
		case 0x0d:				// Return
			if ( hidewindow ) {
				hidewindow = 0;
				MesWin_Setup(MesWinStyle);
			} else {
				mouse->ButtonUp();
			}
			KeyDownFlag = code;
			KeyDownTime = GetCurrentTimer();
			break;

		case 0x0b:				// Scroll Up (BackLog用?)
		case 0x0c:				// Scroll Down (BackLog用?)
		case 0x1b:				// ESC (右クリック相当？ 将来的に使うかも)
		case 0x1e:				// Cursor Up (選択肢用)
		case 0x1f:				// Cursor Down (選択肢用)
			KeyDownFlag = code;
			KeyDownTime = GetCurrentTimer();
			break;

		case 0x20:				// Space (「ウィンドウを消す」に相当)
			if ( hidewindow ) {
				hidewindow = 0;
				MesWin_Setup(MesWinStyle);
			} else if (MesWinFlag) {
				hidewindow = 1;
				MesWin_HideTemp();
			}
			break;
	}
}


// 表示の更新
void SYSTEM::Update(WindowPtr wnd)
{
	SetPort(wnd);
	BeginUpdate(wnd);
	UpdateScreen();
	EndUpdate(wnd);
}


// OSイベント
void SYSTEM::OSEventFunc(unsigned long mes)
{
	Point pt;
	switch(((mes&osEvtMessageMask)>>24)&0xff)
	{
		case suspendResumeMessage:
			if(mes&resumeFlag) {
				ActiveFlag = 1;
			} else {
				ActiveFlag = 0;
			}
			break;
		
		case mouseMovedMessage:
			GetMouse(&pt);
			mouse->Move(pt.h, pt.v);
			break;
	}
}


// 暇～
void SYSTEM::IdleFunc(void)
{
	sound->CD_CheckPlay();
	sound->Sound_CheckPlay();
	if ( loading ) {
		if ( LoadingProc() ) loading = 0;
	} else if ( !hidewindow ) {
		scn->Run();
	}
}


//--------------------------------------------------------------------
//   各ダイアログ処理
//--------------------------------------------------------------------
// About Dialog
void SYSTEM::AboutDlg(void)
{
	DialogPtr hDlg = NULL;
	int flag = 1;
	short item;
	DialogItemType type;
	Handle it;
	Rect rect;
	unsigned char str[256];

	hDlg = GetNewDialog(129, NULL, (WindowPtr)-1);
	if ( hDlg ) {
		SetDialogDefaultItem(hDlg, 2);
		sprintf((char*)&str[1], "Running Mode : AVG32%d%c", version/100, (version%100)+0x40);
		str[0] = strlen((char*)&str[1]);
		GetDialogItem(hDlg, 4, &type, &it, &rect);
		SetDialogItemText(it, str);
		while ( flag ) {
			ModalDialog(NULL, &item);
			if ( item == 2 ) flag = 0;
		}
		DisposeDialog(hDlg);
	}
}


// Quit Confirm
int SYSTEM::QuitDlg(void)
{
	DialogPtr hDlg = NULL;
	int ret = 0;
	int flag = 1;
	short item;

	hDlg = GetNewDialog(128, NULL, (WindowPtr)-1);
	if ( hDlg ) {
		SetDialogDefaultItem(hDlg, 3);
		SetDialogCancelItem(hDlg, 4);
		while (1) {
			ModalDialog(NULL, &item);
			if ( item == 3 ) { ret = 1; break; }
			if ( item == 4 ) break;
		}
		DisposeDialog(hDlg);
	}
	return ret;
}


// Return to Menu Confirm
int SYSTEM::MenuDlg(void)
{
	DialogPtr hDlg = NULL;
	int ret = 0;
	int flag = 1;
	short item;

	hDlg = GetNewDialog(130, NULL, (WindowPtr)-1);
	if ( hDlg ) {
		SetDialogDefaultItem(hDlg, 3);
		SetDialogCancelItem(hDlg, 4);
		while (1) {
			ModalDialog(NULL, &item);
			if ( item == 3 ) { ret = 1; break; }
			if ( item == 4 ) break;
		}
		DisposeDialog(hDlg);
	}
	return ret;
}


// Message Window Color Config Dialog
void SYSTEM::ColorCfgDlg(void)
{
	DialogPtr hDlg = NULL;
	ControlHandle rbar, gbar, bbar, flagbox;
	short num, r, g, b, f;
	Str255 buf;
	DialogItemType type;
	Handle item;
	Rect rect;

	hDlg = GetNewDialog(131, NULL, (WindowPtr)-1);

	r = ini.wincolor[0];
	g = ini.wincolor[1];
	b = ini.wincolor[2];
	f = ini.wincolorflag;

	if ( hDlg ) {

		GetDialogItemAsControl(hDlg, 2, &rbar);
		GetDialogItemAsControl(hDlg, 3, &gbar);
		GetDialogItemAsControl(hDlg, 4, &bbar);
		GetDialogItemAsControl(hDlg, 11, &flagbox);

		SetControlValue(rbar, r);
		NumToString(r, buf);
		GetDialogItem(hDlg, 8, &type, &item, &rect);
		SetDialogItemText(item, buf);
		SetControlValue(gbar, g);
		NumToString(g, buf);
		GetDialogItem(hDlg, 9, &type, &item, &rect);
		SetDialogItemText(item, buf);
		SetControlValue(bbar, b);
		NumToString(b, buf);
		GetDialogItem(hDlg,10, &type, &item, &rect);
		SetDialogItemText(item, buf);

		SetControlValue(flagbox, f);

		SetDialogDefaultItem(hDlg, 1);

		while (1) {
			ModalDialog(NULL, &num);
			if ( num == 1 ) {
				ini.wincolor[0] = r;
				ini.wincolor[1] = g;
				ini.wincolor[2] = b;
				ini.wincolorflag = f;
				break;
			} else if ( num == 11 ) {
				if ( f ) f = 0; else f = 1;
				SetControlValue(flagbox, f);
			} else if ( num == 2 ) {
				r = GetControlValue(rbar);
				NumToString(r, buf);
				GetDialogItem(hDlg, 8, &type, &item, &rect);
				SetDialogItemText(item, buf);
			} else if ( num == 3 ) {
				g = GetControlValue(gbar);
				NumToString(g, buf);
				GetDialogItem(hDlg, 9, &type, &item, &rect);
				SetDialogItemText(item, buf);
			} else if ( num == 4 ) {
				b = GetControlValue(bbar);
				NumToString(b, buf);
				GetDialogItem(hDlg, 10, &type, &item, &rect);
				SetDialogItemText(item, buf);
			}
		}
		DisposeDialog(hDlg);
	}
};


// Inline Check routine for Name Input Dlg
inline int CheckNameString(unsigned char* buf)
{
	int i, flag;

	flag = 1;
	for (i=1; i<=buf[0]; i++) {
		if ( buf[i]<0x80 ) { flag = 0; break; }						// 半角
		if ( (buf[i]>0xa0)&&(buf[i]<0xe0) ) { flag = 0; break; }	// 半角カナ
		i++;
		if ( buf[i-1]==0x81 ) {		// 【】「」チェック
			if ( (buf[i]==0x75)||(buf[i]==0x76)||(buf[i]==0x79)||(buf[i]==0x7a) ) {
				flag = 0;
				break;
			}
		}
	}
	if ( (!buf[0])||(buf[0]>12) ) flag = 0;
	if ( (buf[0]%2)==1 ) flag = 0;

	return flag;
}


// Name Change Dialog
void SYSTEM::NameInputDlg(char* title1, char* title2, int index1, int index2)
{
	DialogPtr hDlg = NULL;
	unsigned char buf[256];
	short num;
	Handle item;
	DialogItemType type;
	Rect rect;
	int i;

	hDlg = GetNewDialog(132, NULL, (WindowPtr)-1);

	if ( hDlg ) {
		if ( index1 ) {
			sprintf((char*)&buf[1], "%s", title1);
			buf[0] = strlen(title1);
			GetDialogItem(hDlg, 8, &type, &item, &rect);
			SetDialogItemText(item, buf);
			sprintf((char*)&buf[1], "%s", ini.name[index1-1]);
			buf[0] = strlen(ini.name[index1-1]);
			GetDialogItem(hDlg, 1, &type, &item, &rect);
			SetDialogItemText(item, buf);
			GetDialogItem(hDlg, 11, &type, &item, &rect);
			SetDialogItemText(item, buf);
		} else {
			HideDialogItem(hDlg, 1);
			HideDialogItem(hDlg, 6);
			HideDialogItem(hDlg, 8);
			HideDialogItem(hDlg, 11);
		}
		if ( index2 ) {
			sprintf((char*)&buf[1], "%s", title2);
			buf[0] = strlen(title2);
			GetDialogItem(hDlg, 9, &type, &item, &rect);
			SetDialogItemText(item, buf);
			sprintf((char*)&buf[1], "%s", ini.name[index2-1]);
			buf[0] = strlen(ini.name[index2-1]);
			GetDialogItem(hDlg, 2, &type, &item, &rect);
			SetDialogItemText(item, buf);
			GetDialogItem(hDlg, 12, &type, &item, &rect);
			SetDialogItemText(item, buf);
		} else {
			HideDialogItem(hDlg, 2);
			HideDialogItem(hDlg, 7);
			HideDialogItem(hDlg, 9);
			HideDialogItem(hDlg, 12);
		}

		SetDialogDefaultItem(hDlg, 10);
		while (1) {
			ModalDialog(NULL, &num);
			if ( num == 10 ) {				// 終了
				break;
			}
		}

		if ( index1 ) {
			GetDialogItem(hDlg, 1, &type, &item, &rect);
			GetDialogItemText(item, buf);
			if ( CheckNameString(buf) ) {
				for (i=0; i<buf[0]; i++) ini.name[index1-1][i] = buf[i+1];
				ini.name[index1-1][i] = 0;
			}
		}
		if ( index2 ) {
			GetDialogItem(hDlg, 2, &type, &item, &rect);
			GetDialogItemText(item, buf);
			if ( CheckNameString(buf) ) {
				for (i=0; i<buf[0]; i++) ini.name[index2-1][i] = buf[i+1];
				ini.name[index2-1][i] = 0;
			}
		}

		DisposeDialog(hDlg);
	}
};


unsigned int	SYSTEM::MsgDlgTimebase;
int				SYSTEM::MsgDlgCount;
unsigned char	SYSTEM::MsgDlgBuf[256];
short			SYSTEM::MsgDlgSpeed;
Handle			SYSTEM::MsgDlgItem;

// Callback routine for Message speed change dialog
pascal Boolean SYSTEM::MsgDlgFilter(DialogPtr dlg, EventRecord *event, short *item)
{
	UnsignedWide current;
	ModalFilterUPP proc;
	Boolean ret;
	int i, c;
	static Boolean isDisabled = false;
	GetStdFilterProc( &proc ) ;
	ret = CallModalFilterProc(proc, dlg, event, item);
	if ( MsgDlgSpeed ) {
		Microseconds(&current);
		if ( !MsgDlgCount ) {
			if ( (((unsigned int)(current.lo/TIMEDIV))-MsgDlgTimebase)<200 ) return ret;
		}
		if ( (((unsigned int)(current.lo/TIMEDIV))-MsgDlgTimebase)>=(MsgDlgSpeed/2) ) {
			MsgDlgTimebase = (unsigned int)(current.lo/TIMEDIV);
			for (i=1; i<=32; i++) {
				MsgDlgBuf[i] = '_';
			}
			c = ((MsgDlgCount>15)?15:MsgDlgCount);
			for (i=0; i<=c; i++) {
				MsgDlgBuf[i*2+16-c] = 0x81;
				MsgDlgBuf[i*2+17-c] = 0xa1;
			}
			SetDialogItemText(MsgDlgItem, MsgDlgBuf);
			MsgDlgCount = ((MsgDlgCount+1)%16);
		}
	}
	return ret;
}


// Message speed change dialog
void SYSTEM::MsgSpeedDlg(void)
{
	DialogPtr hDlg = NULL;
	ControlHandle sbar;
	short num;
	DialogItemType type;
	Rect rect;
	int i;

	hDlg = GetNewDialog(133, NULL, (WindowPtr)-1);

	MsgDlgSpeed = ini.meswait;
	MsgDlgTimebase = GetCurrentTimer();
	MsgDlgCount = 0;

	if ( hDlg ) {
		for (i=0; i<16; i++) {
			MsgDlgBuf[i*2+1] = 0x81;
			MsgDlgBuf[i*2+2] = 0xa1;
		}
		MsgDlgBuf[0] = 32;
		GetDialogItem(hDlg, 5, &type, &MsgDlgItem, &rect);
		GetDialogItemAsControl(hDlg, 1, &sbar);
		if ( MsgDlgSpeed>8 )
			SetControlValue(sbar, MsgDlgSpeed-8);
		else
			SetControlValue(sbar, 0);
		SetDialogDefaultItem(hDlg, 4);
		while (1) {
			ModalDialog(NewModalFilterProc(MsgDlgFilter), &num);
			if ( num == 4 ) {
				ini.meswait = MsgDlgSpeed;
				break;
			} else if ( num == 1 ) {
				MsgDlgSpeed = GetControlValue(sbar);
				if ( !MsgDlgSpeed ) {
					for (i=0; i<16; i++) {
						MsgDlgBuf[i*2+1] = 0x81;
						MsgDlgBuf[i*2+2] = 0xa1;
					}
					SetDialogItemText(MsgDlgItem, MsgDlgBuf);
					MsgDlgCount = 0;
				} else {
					MsgDlgSpeed += 8;
				}
			}
		}
		DisposeDialog(hDlg);
	}
};


/* -------------------------------------------------------------------
  Window操作関連
------------------------------------------------------------------- */

// C文字列をPASCAL文字列に変換
void SYSTEM::ConvertString(unsigned char* out, char* in)
{
	int i;
	out[0] = 255;
	for (i=1; i<256; i++) {
		if (in[i-1]==0) {
			out[0] = i-1;
			return;
		} else {
			out[i] = in[i-1];
		}
	}
};

// ウィンドウタイトルの変更
void SYSTEM::SetWindowTitle(char* s, int flag)
{
	char temp[256];
	unsigned char str[256];
	strcpy(curtitle, s);
	sprintf(temp, "%s %s", ini.caption, s);
	ConvertString(str, temp);
	if ( flag ) SetWTitle(hWnd, str);
};


/* -------------------------------------------------------------------
  Mouse操作関連
------------------------------------------------------------------- */
int SYSTEM::CheckSkipKey(void)
{
	unsigned char state[16];
	GetKeys((unsigned long*)state);
	if ( (state[7]&0x11)&&(ActiveFlag) )		// bit4:RShift, bit0:LShift
		return 1;
	else
		return 0;
}


int SYSTEM::CheckSkip(void)
{
	if ( (CheckSkipKey())&&(SkipEnableFlag) )
		return 1;
	else
		return 0;
}


int SYSTEM::GetKeyInput(void)
{
	int ret = 0;
	int t = GetCurrentTimer()-KeyDownTime;
	if ( (t>=0)&&(t<200)&&(KeyDownFlag) ) ret = KeyDownFlag;
	KeyDownFlag = 0;
	return ret;
}


/* -------------------------------------------------------------------
  タイマー、乱数
------------------------------------------------------------------- */

void SYSTEM::ResetTimer(void)
{
    Microseconds(&TimerBase);
}

unsigned int SYSTEM::GetCurrentTimer(void)
{
	UnsignedWide current;
	Microseconds(&current);
	
	return (unsigned int)(current.lo/TIMEDIV);
}

int SYSTEM::GetTimer(void)
{
	UnsignedWide current;
	Microseconds(&current);
	WideSubtract((wide*)&current,(wide*)&TimerBase);
	return (current.lo/TIMEDIV);
}

int SYSTEM::GetRandom(int lo, int hi)
{
	int ret;
	ret = ((unsigned int)Random()%(hi-lo+1))+lo;
	return ret;
}

int SYSTEM::GetDateTime(int n)
{
	int ret = 0;
	time_t t;
	tm* lt;

	t = time(0);
	lt = localtime(&t);
	
	switch (n) {
		case 0x01: ret = (lt->tm_mon+1)*100 + lt->tm_mday; break;
		case 0x02: ret = lt->tm_hour*100 + lt->tm_min;  break;
		case 0x03: ret = lt->tm_year;                   break;
		case 0x04: ret = lt->tm_wday;                   break;
	}
	return ret;
}


/* -------------------------------------------------------------------
  GAMEEXE.INIファイル読み込み
------------------------------------------------------------------- */
// GAMEEXE.INI 及び SETUP.INI から必要情報を得る。
// あまりにも酷すぎるルーチン（汗  なんとかしよう・・・

bool SYSTEM::CmpStr(char* src, char* dst)
{
	while( *dst ) {
		if ( (*src++)!=(*dst++) ) return false;
	}
	return true;
}

int SYSTEM::ReadNum(char* &buf)
{
	int ret = 0, i = 0;

	if ( buf[0]=='-' ) i = 1;
	while ((buf[i]>='0')&&(buf[i]<='9')) {
		ret *= 10;
		ret += (buf[i]-0x30);
		i++;
	}
	if ( buf[0]=='-' ) ret = -ret;

	buf += i;
	return ret;
}


bool SYSTEM::SearchChar(char* &buf, char c)
{
	char n;
	do {
		n = *buf++;
		if ( (!n)||(n==0x0a)||(n==0x0d) ) { buf--; return false; }
	} while ( n!=c );
	return true;
};


void SYSTEM::ReadString(char* &buf, char *out)
{
	char *st, *ed;
	if ( SearchChar(buf, '\"') ) {
		st = buf;
		if ( SearchChar(buf, '\"') ) {
			ed = buf-1;
			while (st<ed) *out++ = *st++;
		}
	}
	*out = 0;
};


static int selnum = 0;
void SYSTEM::CheckINIItem(char* buf)
{
	int i, n;
	unsigned char tmp [256];
	if ( CmpStr(buf, "#CDTRACK") ) {
		SearchChar(buf, '=');
		i = ((buf[0])-0x30)*10+(buf[1])-0x30;
		if ( (i<100)&&(i>=0) ) {
			SearchChar(buf, '=');
			ReadString(buf, ini.cd[i]);
		}
	} else if ( CmpStr(buf, "#DSTRACK") ) {
		if ( ini.dsnum<100 ) {
			SearchChar(buf, '=');
			SearchChar(buf, '-');
			SearchChar(buf, '-');
			ini.dsound[ini.dsnum].cutsize = ReadNum(buf);
			SearchChar(buf, '=');
			ReadString(buf, ini.dsound[ini.dsnum].file);
			if ( SearchChar(buf, '=') ) {
				ReadString(buf, ini.dsound[ini.dsnum].name);
			} else {
				strcpy(ini.dsound[ini.dsnum].name, ini.dsound[ini.dsnum].file);
			}
			ini.dsnum++;
		}
	} else if ( CmpStr(buf, "#DIRC.PDT") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.pdtdir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.pdtpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.pdtpack);
	} else if ( CmpStr(buf, "#DIRC.ANM") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.anmdir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.anmpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.anmpack);
	} else if ( CmpStr(buf, "#DIRC.TXT") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.txtdir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.txtpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.txtpack);
	} else if ( CmpStr(buf, "#DIRC.WAV") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.wavdir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.wavpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.wavpack);
	} else if ( CmpStr(buf, "#DIRC.ARD") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.arddir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.ardpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.ardpack);
	} else if ( CmpStr(buf, "#DIRC.CUR") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.curdir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.curpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.curpack);
	} else if ( CmpStr(buf, "#DIRC.***") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.othdir);
		SearchChar(buf, '=');
		while(*buf==' ') buf++;
		ini.othpackflag = *buf;
		SearchChar(buf, ':');
		ReadString(buf, ini.othpack);
	} else if ( CmpStr(buf, "#SE.") ) {
		SearchChar(buf, '.');
		i = ReadNum(buf);
		if ( (i<MAX_SE)&&(i>=0) ) {
			SearchChar(buf, '=');
			ReadString(buf, ini.se[i]);
		}
	} else if ( CmpStr(buf, "#SEL.")||CmpStr(buf, "#SEL=") ) {
		if ( CmpStr(buf, "#SEL.") ) {
			SearchChar(buf, '.');
			i = ReadNum(buf);
		} else {
			i = selnum++;
		}
		if ( (i<MAX_SEL)&&(i>=0) ) {
			SearchChar(buf, '='); ini.sel[i].sx1 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].sy1 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].sx2 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].sy2 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].dx  = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].dy  = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].steptime = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].cmd  = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].mask = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].arg2 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].arg3 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].arg4 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].step = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].arg5 = ReadNum(buf);
			SearchChar(buf, ','); ini.sel[i].arg6 = ReadNum(buf);
			dprintf("Sel[%d] = (%d,%d)-(%d,%d)", i, ini.sel[i].sx1, ini.sel[i].sy1, ini.sel[i].sx2, ini.sel[i].sy2);
			dprintf("->(%d,%d) Efct:%d Time:%d Step:%d\n", ini.sel[i].dx, ini.sel[i].dy, ini.sel[i].cmd, ini.sel[i].steptime, ini.sel[i].step);
		}
	} else if ( CmpStr(buf, "#WAKUPDT") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.wakufile);
	} else if ( CmpStr(buf, "#EXFONT_N_NAME") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.exfont);
	} else if ( CmpStr(buf, "#EXFONT_N_XSIZE") ) {
		SearchChar(buf, '='); ini.exfontx = ReadNum(buf);
	} else if ( CmpStr(buf, "#EXFONT_N_YSIZE") ) {
		SearchChar(buf, '='); ini.exfonty = ReadNum(buf);
	} else if ( CmpStr(buf, "#EXFONT_N_XCONT") ) {
		SearchChar(buf, '='); ini.exfontmaxx = ReadNum(buf);
	} else if ( CmpStr(buf, "#EXFONT_N_YCONT") ) {
		SearchChar(buf, '='); ini.exfontmaxy = ReadNum(buf);
	} else if ( CmpStr(buf, "#CAPTION") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.caption);
	} else if ( CmpStr(buf, "#SAVENOTITLE") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.savenotitle);
	} else if ( CmpStr(buf, "#NAME.") ) {
		SearchChar(buf, '.');
		n = (*buf++)-'A';
		if ( (n<26)&&(n>=0)&&((*buf<'A')||(*buf>'Z')) ) {
			SearchChar(buf, '=');
			ReadString(buf, ini.name[n]);
dprintf("NAME.%c = %s\n", n+0x41, (int)ini.name[n]);
		}
	} else if ( CmpStr(buf, "#COLOR_TABLE") ) {
		SearchChar(buf, '.');
		n = ReadNum(buf);
		if ( (n<16)&&(n>=0) ) {
			SearchChar(buf, '='); ini.colortable[n][0] = ReadNum(buf);
			SearchChar(buf, ','); ini.colortable[n][1] = ReadNum(buf);
			SearchChar(buf, ','); ini.colortable[n][2] = ReadNum(buf);
		}
	} else if ( CmpStr(buf, "#FADE_TABLE") ) {
		SearchChar(buf, '.');
		n = ReadNum(buf);
		if ( (n<16)&&(n>=0) ) {
			SearchChar(buf, '='); ini.fadetable[n][0] = ReadNum(buf);
			SearchChar(buf, ','); ini.fadetable[n][1] = ReadNum(buf);
			SearchChar(buf, ','); ini.fadetable[n][2] = ReadNum(buf);
		}
	} else if ( CmpStr(buf, "#WINDOW_ATTR=") ) {
		SearchChar(buf, '='); ini.wincolor[0] = ReadNum(buf);
		SearchChar(buf, ','); ini.wincolor[1] = ReadNum(buf);
		SearchChar(buf, ','); ini.wincolor[2] = ReadNum(buf);
	} else if ( CmpStr(buf, "#WINDOW_ATTR_AREA") ) {
		SearchChar(buf, '='); ini.winatrx1 = ReadNum(buf);
		SearchChar(buf, ','); ini.winatry1 = ReadNum(buf);
		SearchChar(buf, ','); ini.winatrx2 = ReadNum(buf);
		SearchChar(buf, ','); ini.winatry2 = ReadNum(buf);
	} else if ( CmpStr(buf, "#WINDOW_ATTR_TYPE") ) {
		SearchChar(buf, '='); ini.wincolorflag = ReadNum(buf);
	} else if ( CmpStr(buf, "#WINDOW_MSG_POS") ) {
		SearchChar(buf, '='); ini.winx = ReadNum(buf);
		SearchChar(buf, ','); ini.winy = ReadNum(buf);
	} else if ( CmpStr(buf, "#WINDOW_COM_POS") ) {
		SearchChar(buf, '='); ini.subwinx = ReadNum(buf);
		SearchChar(buf, ','); ini.subwiny = ReadNum(buf);
	} else if ( CmpStr(buf, "#COM_WIND_MIN_SIZE") ) {
		SearchChar(buf, '='); ini.subwinmin = ReadNum(buf);
	} else if ( CmpStr(buf, "#MESSAGE_SIZE") ) {
		SearchChar(buf, '='); ini.mesx = ReadNum(buf);
		SearchChar(buf, ','); ini.mesy = ReadNum(buf);
	} else if ( CmpStr(buf, "#MSG_MOJI_SIZE") ) {
		SearchChar(buf, '='); ini.fontx = ReadNum(buf);
		SearchChar(buf, ','); ini.fonty = ReadNum(buf);
	} else if ( CmpStr(buf, "#FONT_SIZE") ) {
		SearchChar(buf, '='); ini.fontsize = ReadNum(buf);
	} else if ( CmpStr(buf, "#MOJI_COLOR") ) {
		SearchChar(buf, '='); ini.fontcolor = ReadNum(buf);
	} else if ( CmpStr(buf, "#MSG_SPEED") ) {
		SearchChar(buf, '='); n = ReadNum(buf);
		if ( n<0 ) n = 0;
		if ( n ) n += 8;
		ini.meswait = n;
	} else if ( CmpStr(buf, "#NAME_AFTER_SPACE") ) {
		SearchChar(buf, '='); ini.namesp = ReadNum(buf);
	} else if ( CmpStr(buf, "#NAME_INDENT_SPACE") ) {
		SearchChar(buf, '='); ini.nameindent = ReadNum(buf);
	} else if ( CmpStr(buf, "#SEL_BLINK_COUNT") ) {
		SearchChar(buf, '='); ini.selblinkcount = ReadNum(buf);
	} else if ( CmpStr(buf, "#SEL_BLINK_SPEED") ) {
		SearchChar(buf, '='); ini.selblinktime = ReadNum(buf);
	} else if ( CmpStr(buf, "#FADE_TIME") ) {
		SearchChar(buf, '='); ini.defaultfadetime = ReadNum(buf);
	} else if ( CmpStr(buf, "#MUSIC_TYPE") ) {
		SearchChar(buf, '='); ini.musictype = ReadNum(buf);
	} else if ( CmpStr(buf, "#MUSIC_LINEAR_PAC") ) {
		SearchChar(buf, '='); ini.musiclinear = ReadNum(buf);
	} else if ( CmpStr(buf, "#WAVE_LINEAR_PAC") ) {
		SearchChar(buf, '='); ini.wavlinear = ReadNum(buf);
	} else if ( CmpStr(buf, "#KOE_LINEAR_PAC") ) {
		SearchChar(buf, '='); ini.koelinear = ReadNum(buf);
	} else if ( CmpStr(buf, "#RETN_SPEED") ) {
		SearchChar(buf, '='); ini.mesiconwait = ReadNum(buf);
	} else if ( CmpStr(buf, "#RETN_CONT") ) {
		SearchChar(buf, '='); ini.mesiconnum = ReadNum(buf);
	} else if ( CmpStr(buf, "#RETN_XSIZE") ) {
		SearchChar(buf, '='); ini.mesiconx = ReadNum(buf);
	} else if ( CmpStr(buf, "#RETN_YSIZE") ) {
		SearchChar(buf, '='); ini.mesicony = ReadNum(buf);
	} else if ( CmpStr(buf, "#SHAKE.") ) {
		SearchChar(buf, '.');
		i = ReadNum(buf);
		if ( (i>=0)&&(i<16) ) {
			SearchChar(buf, '=');
			n = 0;
			while ( (*buf=='(')&&(n<16) ) {
				buf++;
				ini.shake[i][n][0] = ReadNum(buf);
				SearchChar(buf, ','); ini.shake[i][n][1] = ReadNum(buf);
				SearchChar(buf, ','); ini.shake[i][n][2] = ReadNum(buf);
				SearchChar(buf, ')');
				n++;
			}
			ini.shakecount[i] = n;
		}
	} else if ( (CmpStr(buf, "#SEEN_START"))||(CmpStr(buf, "#SEEN_SRT")) ) {
		SearchChar(buf, '='); ini.startseen = ReadNum(buf);
	} else if ( CmpStr(buf, "#SEEN_MENU") ) {
		SearchChar(buf, '='); ini.menuseen = ReadNum(buf);
	} else if ( CmpStr(buf, "#SAVENAME") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.savefile);
	} else if ( CmpStr(buf, "#REGNAME") ) {
		SearchChar(buf, '=');
		ReadString(buf, ini.regname);
	} else if ( CmpStr(buf, "#SAVETITLE") ) {
		memset(ini.saveheader, 0, 128);
		SearchChar(buf, '=');
		ReadString(buf, ini.saveheader);
	} else if ( CmpStr(buf, "#SAVEFILETIME") ) {
		SearchChar(buf, '=');
		i = (buf[0]-0x30)*10+buf[1]-0x30;
		if ( (i<1)||(i>30) ) i=30;
		ini.savefilenum = i;
	} else if ( CmpStr(buf, "#NVL_SYSTEM") ) {
		SearchChar(buf, '='); ini.novelmode = ReadNum(buf);
	} else if ( CmpStr(buf, "#SYSCOM.") ) {
		SearchChar(buf, '.'); i = ReadNum(buf);
		if ( *buf=='.' ) {
			buf++;
			n = ReadNum(buf);
			if ( menus[i] ) {
				SearchChar(buf, '=');
				ReadString(buf, (char*)&tmp[1]);
				tmp[0] = strlen((char*)&tmp[1]);
				if ( i>=2 ) {
					InsertMenuItem(menus[i], "\p ", n);
					SetMenuItemText(menus[i], n+1, tmp);
				}
				menushier[i] = 1;
			}
		} else if ( *buf=='=' ) {
			buf++;
			if ( (i>=0)&&(i<MAX_MENU)&&(*buf=='U') ) {
				buf += 2;
				ReadString(buf, (char*)&menuname[i][1]);
				menuname[i][0] = strlen((char*)&menuname[i][1]);
				menus[i] = NewMenu(i+1, menuname[i]);
			}
		}
		if ( (i>=0)&&(i<32) ) ini.sysmenusw[i]++;
	} else if ( CmpStr(buf, "#KOE_DOUBLE_PAC") ) {
		SearchChar(buf, '='); ini.koetype = ReadNum(buf);
	}
}


void SYSTEM::CheckSetupINIItem(char* buf)
{
	int i, j;
	char c;
	if ( CmpStr(buf, "BGM_FOLDER") ) {
		SearchChar(buf, '=');
		for (i=0, j=0; j<32; i++) {
			c = buf[i];
			if ( !c ) { ini.bgmdir[j] = 0; break; }
			if ( c!=0x20 ) ini.bgmdir[j++] = c;
		}
	} else if ( CmpStr(buf, "KOE_FOLDER") ) {
		SearchChar(buf, '=');
		for (i=0, j=0; j<32; i++) {
			c = buf[i];
			if ( !c ) { ini.koedir[j] = 0; break; }
			if ( c!=0x20 ) ini.koedir[j++] = c;
		}
	} else if ( CmpStr(buf, "MOV_FOLDER") ) {
		SearchChar(buf, '=');
		for (i=0, j=0; j<32; i++) {
			c = buf[i];
			if ( !c ) { ini.movdir[j] = 0; break; }
			if ( c!=0x20 ) ini.movdir[j++] = c;
		}
	}
}


int SYSTEM::ReadINIFile(char* f)
{
	FILE* fp=NULL;
	char buf[1024];
	char* p = buf;
	char c;
	int size;

	// GAMEEXE.INI から取得する Dir
	strcpy(ini.pdtdir, "PDT");
	strcpy(ini.wavdir, "WAV");
	strcpy(ini.txtdir, "DAT");
	strcpy(ini.anmdir, "DAT");
	strcpy(ini.arddir, "DAT");
	strcpy(ini.curdir, "DAT");
	strcpy(ini.othdir, "DAT");

	// SETUP.INI から取得する Dir
	strcpy(ini.bgmdir, "BGM");
	strcpy(ini.koedir, "KOE");
	strcpy(ini.movdir, "MOV");

	ini.mesiconx = 16;
	ini.mesicony = 16;
	ini.mesiconwait = 100;
	ini.mesiconnum = 16;

	selnum = 0;

	fp = fopen(f, "rb");
	if ( !fp ) {
		Abort("GAMEEXE.INIファイルを開けません.");
		return 0;
	}
	dprintf("Open GAMEEXE.INI\n");
	p = buf;
	*p = 0;
	while ( !feof(fp) ) {
		c = fgetc(fp);
		if ( (c==0x0a)||(c==0x0d) ) {
			*p++ = 0;
			if ( buf[0]=='#' ) {
				dprintf("Checking ... ");
				dprintf(buf);
				dprintf("\n");
				CheckINIItem(buf);
				buf[0] = 0;
			}
			p = buf;
		} else {
			*p++ = c;
		}
	}
	*p++ = 0;		// ファイルの最後が改行で終わってない時用
	if ( buf[0]=='#' ) CheckINIItem(buf);
	fclose(fp);

	// ------ SETUP.INI も調べる -------
	fp = fopen("SETUP.INI", "rb");
	if ( fp ) {
		dprintf("Open SETUP.INI\n");
		p = buf;
		*p = 0;
		while ( !feof(fp) ) {
			c = fgetc(fp);
			if ( (c==0x0a)||(c==0x0d) ) {
				*p++ = 0;
				if ( buf[0] ) {
					dprintf("Checking ... ");
					dprintf(buf);
					dprintf("\n");
					CheckSetupINIItem(buf);
					buf[0] = 0;
				}
				p = buf;
			} else {
				*p++ = c;
			}
		}
	}
	*p++ = 0;		// ファイルの最後が改行で終わってない時用
	if ( buf[0] ) CheckSetupINIItem(buf);
	fclose(fp);

	if ( (ini.novelmode)&&(!ini.fontx) ) ini.fontx = 12;
	if ( !ini.fontsize ) {
		ini.fontsize = (((ini.fontx*2)<ini.fonty )?(ini.fontx*2):ini.fonty);
	} else {
		size = (((ini.fontx*2)<ini.fonty )?(ini.fontx*2):ini.fonty);
		if ( (size)&&(size<ini.fontsize) ) {
			ini.fontsize = size;
		}
		if ( !ini.fontx ) ini.fontx = ini.fontsize/2;
		if ( !ini.fonty ) ini.fonty = ini.fontsize+2;
	}
	if ( !ini.fontsize ) {
		ini.fontsize = 16;
		ini.fonty = 18;
		ini.fontx = 8;
	}

	ini.fontx &= 0xfffe;
	ini.fonty &= 0xfffe;

	if ( !ini.menuseen ) ini.menuseen = ini.startseen;

//	sprintf(buf, ":%s:FN.DAT", ini.othdir);
//	if ( ini.novelmode ) nvlfont = new NOVELFONT(buf);

	return 1;
}


/* -------------------------------------------------------------------
  AVG32***.EXEのバージョンチェック
------------------------------------------------------------------- */
// バージョンには大きく分けて3216系と3217系があり、またそれぞれに
// D版とM版が存在する模様。
// 実際にはファイル名は同じでも内部仕様が異なる物があるようで、実際の
// バージョンの総数は10近くありそう。
// D版とM版の違いは、M版はDSTRACKやKOEの再生ができない、といった点
// 以外は不明。

void SYSTEM::CheckVersion(void)
{
	FILE* fp = 0;
	version = 0;

	// GAMEEXE.INI からレジストリ情報を探して動作バージョンを決める（原始的・・・）
	// 外にデータベースファイルを用意した方がいいかもね
	if ( !strcmp(ini.regname, "BONBEE\\RIBBON2") )      version = 1704;
	if ( !strcmp(ini.regname, "Mebius\\絶望") )         version = 1704;
	if ( !strcmp(ini.regname, "KEY\\KANON_DEMO") )      version = 1613;
	if ( !strcmp(ini.regname, "KEY\\KANON") )           version = 1613;
	if ( !strcmp(ini.regname, "KEY\\KANON_ALL") )       version = 1713;
	if ( !strcmp(ini.regname, "KEY\\AIR") )             version = 1714;
	if ( !strcmp(ini.regname, "KEY\\DEMO\\AIR_01") )    version = 1713;
	if ( !strcmp(ini.regname, "RAM\\NEGAI") )           version = 1613;
	if ( !strcmp(ini.regname, "RAM\\KOIGOKORO") )       version = 1714;
	if ( !strcmp(ini.regname, "OTHERWISE\\SENSEOFF") )  version = 1713;
	if ( !strcmp(ini.regname, "OZ_PROJECT\\BABYFACE") ) version = 1713;
	if ( !strcmp(ini.regname, "SAGAPLANETS\\P_HEART") ) version = 1713;
	if ( !strcmp(ini.regname, "SAGAPLANETS\\DEMO\\REN"))version = 1714;
	if ( !strcmp(ini.regname, "13CM\\SUKI") )           version = 1604;
	if ( !strcmp(ini.regname, "13CM\\フロレアール") )   version = 1613;
	if ( !strcmp(ini.regname, "13CM\\DEVOTE") )         version = 1704;
	if ( !strcmp(ini.regname, "13CM\\LEMON") )          version = 1713;
	if ( !strcmp(ini.regname, "13CM\\SHIMAI") )         version = 1714;
	if ( !strcmp(ini.regname, "13CM\\NYUIN") )          version = 1613;
	if ( !strcmp(ini.regname, "ZERO\\IINARI\\") )       version = 1713;
	if ( !strcmp(ini.regname, "ZERO\\BALLET\\") )       version = 1613;
	if ( !strcmp(ini.regname, "ZERO\\RYO_DOU") )        version = 1714;
	if ( !strcmp(ini.regname, "MANBOU\\MIND") )         version = 1613;
	if ( !strcmp(ini.regname, "FLADY\\MAYAKU") )        version = 1604;
	if ( !strcmp(ini.regname, "G-PANDA\MAMAHAHA") )     version = 1714;
	if ( !strcmp(ini.regname, "CRAFT\\FLOWERS") )       version = 1604;
	if ( !strcmp(ini.regname, "SIRIUS\\HIME") )         version = 1714;
	if ( !strcmp(ini.regname, "REX\\HAKO") )            version = 1714;

	if ( version ) return;

	// レジストリから取得できなかった時は、ファイル名とサイズからてきとーに判断
	version = 1613;
	fp = fopen("AVG3217D.EXE","rb");
	if ( fp ) {
		fseek(fp, 330000, 0);
		if ( ftell(fp)==330000 )
			version = 1714;							// 17D' (Koigokoro) $FFコマンドにバックログ情報がある
		else
			version = 1704;							// 17D (Ribbon2)
		fclose(fp);
		return;
	}
	fp = fopen("AVG3217M.EXE","rb");
	if ( fp ) {
		fseek(fp, 310000, 0);
		if ( ftell(fp)==310000 )
			version = 1714;							// 17M' (AIR) $FFコマンドにバックログ情報（int）がある
		else
			version = 1713;							// 17M 最近のはほぼコレ
		fclose(fp);
		return;
	}
	fp = fopen("AVG3216D.EXE","rb");				// SukiSuki/Floreal
	if ( fp ) {
		fseek(fp, 280000, 0);
		if ( ftell(fp)==280000 )
			version = 1613;							// Floreal
		else
			version = 1604;							// Suki Suki Daisuki!
		fclose(fp);
		return;
	}
}


/* -------------------------------------------------------------------
  ファイル読み込み
------------------------------------------------------------------- */
// PAC形式かRAWファイルかをINIから判断し、目的ファイルを読み込む
// 汎用ルーチン

void SYSTEM::ConvertCapital(unsigned char* buf)
{
	int i;
	for (i=0; i<strlen((char*)buf); i++) {
		if ( (buf[i]>='a')&&(buf[i]<='z') ) buf[i] -= 0x20;
	}
}


unsigned char* SYSTEM::ReadFile(char* file, char* type, int* size)
{
	FILE* fp;
	char buf[256];
	unsigned char tmp[256];
	unsigned char* ret = 0, *ret2;
	int packed = 0;
	int filenum, i, pos;
	// PACかRAWかを調べてファイル名を作る
 	if ( !strcmp(type, "PDT") ) {
		if ( (ini.pdtpackflag=='P')||(ini.pdtpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.pdtdir, ini.pdtpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.pdtdir, file);
		}
	} else if ( !strcmp(type, "TXT") ) {
		if ( (ini.txtpackflag=='P')||(ini.txtpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.txtdir, ini.txtpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.txtdir, file);
		}
	} else if ( !strcmp(type, "ANM") ) {
		if ( (ini.anmpackflag=='P')||(ini.anmpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.anmdir, ini.anmpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.anmdir, file);
		}
	} else if ( !strcmp(type, "WAV") ) {
		if ( (ini.wavpackflag=='P')||(ini.wavpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.wavdir, ini.wavpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.wavdir, file);
		}
	} else if ( !strcmp(type, "ARD") ) {
		if ( (ini.ardpackflag=='P')||(ini.ardpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.arddir, ini.ardpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.arddir, file);
		}
	} else if ( !strcmp(type, "CUR") ) {
		if ( (ini.curpackflag=='P')||(ini.curpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.curdir, ini.curpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.curdir, file);
		}
	} else {
		if ( (ini.othpackflag=='P')||(ini.othpackflag=='p') ) {
			sprintf(buf, ":%s:%s", ini.othdir, ini.othpack);
			packed = 1;
		} else {
			sprintf(buf, ":%s:%s", ini.othdir, file);
		}
	}
	fp = fopen(buf, "rb");
	if ( fp ) {
		if ( packed ) {			// PACLファイル
			ConvertCapital((unsigned char*)file);
			fseek(fp, 0, 0);
			fread(tmp, 32, 1, fp);
			if ( !strcmp((char*)tmp, "PACL") ) {
				filenum = (tmp[16]|(tmp[17]<<8)|(tmp[18]<<16)|(tmp[19]<<24));
				for (i=0; i<filenum; i++) {
					fread(tmp, 32, 1, fp);
					ConvertCapital(tmp);
					if ( !strcmp((char*)tmp, file) ) {
						*size = (tmp[20]|(tmp[21]<<8)|(tmp[22]<<16)|(tmp[23]<<24));
						pos = (tmp[16]|(tmp[17]<<8)|(tmp[18]<<16)|(tmp[19]<<24));
						ret = new unsigned char[*size];
						if ( ret ) {
							fseek(fp, pos, 0);
							fread(ret, *size, 1, fp);
							ret2 = Unpack(ret, size);	// さらにPACK圧縮がかかってれば解凍
							if ( ret2 ) {
								delete[] ret;
								ret = ret2;
							}
						} else {
							Error("Could not allocate memory.");
						}
						break;
					}
				}
			}
			fclose(fp);
		} else {				// RAWファイル
			fseek(fp, 0, 2);
			*size = ftell(fp);
			ret = new unsigned char[*size];
			if ( ret ) {
				fseek(fp, 0, 0);
				fread(ret, *size, 1, fp);
			} else {
				Error("Could not allocate memory.");
			}
			fclose(fp);
		}
	} else {
//		sprintf((char*)tmp, "File %s could not be opened.", buf);
//		Error((char*)tmp);
		*size = 0;
	}
	return ret;
}


// PACK形式のファイルの解凍
unsigned char* SYSTEM::Unpack(unsigned char* src, int* size)
{
	int num, srccount;
	int i, bit, count, rawlen;
	unsigned char* repeat;
	unsigned char* bufend;
	unsigned char* dst;
	unsigned char* buf = 0;
	unsigned char flag;

	if ( !strcmp((char*)src, "PACK") ) {
		*size = (src[8]|(src[9]<<8)|(src[10]<<16)|(src[11]<<24));
		rawlen = (src[12]|(src[13]<<8)|(src[14]<<16)|(src[15]<<24));
		buf = new unsigned char[*size];
		dst = buf;
		bit = 0;
		srccount = rawlen-16;
		bufend = buf+(*size);
		src += 16;
		while ( (dst<bufend)&&(srccount>0) ) {
			if (!bit) {
				bit = 8;
				flag = *src++;
				srccount--;
			}
			if ( flag&0x80 ) {
				*dst++ = *src++;
				srccount--;
			} else {
				num  = *src++;
				num += ((*src++)<<8);		// 引数はリトルエンディアンのWORD
				srccount -= 2;
				count = (num&15)+2;			// 下位4bitがリピート回数. 2回が最小単位？
				num >>= 4;					// 上位24bitが戻り位置
				repeat = (dst-num)-1;		// bufは次の書き込み位置を指してるので、1バイト多めに戻る
				for (i=0; (i<count)&&(dst<bufend); i++) *dst++ = *repeat++;
			}
			bit--;
			flag <<= 1;
		}
	}
	return buf;
}


/* -------------------------------------------------------------------
  メニュー操作
------------------------------------------------------------------- */
// ポップアップメニューは、マウスカーソルがONで、且つ画面効果中でも
// テキスト表示中でも無い場合だけ現れるっぽい？ ちとまだ不明

void SYSTEM::AddSaveLoadMenu(void)
{
	unsigned char* buf;
	unsigned char temp[256];
	char title[256];
	FILE* fp;
	int i, j, flag;
	int month, day, hour, min;
	int header, size, pad;

	switch(Version()) {
		case 1604:			// Suki Suki Daisuki!
			header   = 0x01454;
			size     = 0x1f400;
			pad = 0;
			break;
		case 1613:
		case 1704:
			header   = 0x01468;
			size     = 0x20000;
			pad = 0;
			break;
		case 1714:
			header   = 0x048c0;
			size     = 0x253f8;
			pad = 4;
			break;
		default:
			header   = 0x01468;
			size     = 0x21fa0;
			pad = 4;
			break;
	}

	buf = new unsigned char[512];
	fp = fopen(ini.savefile, "rb");

	for (i=1; i<=ini.savefilenum; i++) {
		memset(buf, 0, 512);
		if ( fp ) {
			fseek(fp, header+size*(i-1), 0);
			fread(buf, 512, 1, fp);
		}

		flag = 1;
		if ( LoadInt(buf, 0)!=1 ) flag = 0;
		month = LoadInt(buf, pad+4);
		day   = LoadInt(buf, pad+8);
		hour  = LoadInt(buf, pad+12);
		min   = LoadInt(buf, pad+16);
		if ( (month>12)||(month<1) ) flag = 0;
		if ( (day>31)  ||(day<1)   ) flag = 0;
		if ( (hour>23) ||(hour<0)  ) flag = 0;
		if ( (min>59)  ||(min<0)  ) flag = 0;
		sprintf(title, " %02d/%02d(%02d:%02d) ", month, day, hour, min);
		LoadStr(buf, pad+20, title+14, 64);
		savedatadate[i-1] = month*100+day;
		savedatatime[i-1] = hour*100+min;
		strcpy(savedatatitle[i-1], title+14);

		InsertMenuItem(menus[0], "\p ", i-1);
		InsertMenuItem(menus[1], "\p ", i-1);
		EnableMenuItem(menus[0], i);
		savedataflag[i-1] = 0;
		if ( (flag)&&(title[13]) ) {
			savedataflag[i-1] = 1;
			EnableMenuItem(menus[1], i);
		} else {
			sprintf(title, "%s", ini.savenotitle);
			DisableMenuItem(menus[1], i);
		}
		ConvertString(temp, title);
		for (j=255; j>0; j--) {
			temp[j] = temp[j-1];
		}
		temp[1] = 0;
		temp[0]++;
		SetMenuItemText(menus[0], i, temp);
		SetMenuItemText(menus[1], i, temp);
	}

	if ( fp ) fclose(fp);
	delete[] buf;

	if ( ini.sysmenusw[12]<3 ) DisableMenuItem(menus[2], 8);
//	DisableMenuItem(menus[1], 1);		// 起動直後はSAVE/LOADはOFFにしておく
//	DisableMenuItem(menus[1], 2);
//	DisableMenuItem(menus[1], 4);
	savemenuflag = false;
};


int SYSTEM::PopupLoadMenu(void)
{
	int ret, n;
	Point p;
	int x, y, btn;

	mouse->GetState(&x, &y, &btn);
	p.h = x;
	p.v = y;
	LocalToGlobal(&p);
//	CalcMenuSize(menus[1]);
	PostEvent(mouseUp, 0);		// 画面効果後にポップアップすると、クリックホールド扱いになってしまうので
	n = LoWord(PopUpMenuSelect(menus[1], p.v, p.h, 0));
	ret = 0;
	if ( (n>0)&&(n<=ini.savefilenum) ) {
		if ( savedataflag[n-1] ) ret = n;
	}
	return ret;
};



void SYSTEM::MenuEnable(int num, int sw)
{
	int i, id = -1, sid = -1;
	menuflag[num] = sw;
	for (i=0; i<MAX_MENU; i++) {
		if ( menuid[i]==num ) id = i;
		if ( menusysid[i]==num ) sid = i;
	}
	if ( num==0 ) {
		if ( (sw)&&(popupflag) ) {
			if ( id!=-1 ) EnableMenuItem(popupmenu, id);
			EnableMenuItem(filemenu, 1);
		} else {
			if ( id!=-1 ) DisableMenuItem(popupmenu, id);
			DisableMenuItem(filemenu, 1);
		}
	} else if ( num==1 ) {
		if ( (sw)&&(popupflag) ) {
			if ( id!=-1 ) EnableMenuItem(popupmenu, id);
			EnableMenuItem(filemenu, 2);
		} else {
			if ( id!=-1 ) DisableMenuItem(popupmenu, id);
			DisableMenuItem(filemenu, 2);
		}
	} else if ( num==28 ) {
		if ( (sw)&&(popupflag) ) {
			if ( id!=-1 ) EnableMenuItem(popupmenu, id);
			EnableMenuItem(filemenu, 4);
		} else {
			if ( id!=-1 ) DisableMenuItem(popupmenu, id);
			DisableMenuItem(filemenu, 4);
		}
	} else {
		if ( (sw)&&(popupflag) ) {
			if ( id!=-1 ) EnableMenuItem(popupmenu, id);
			if ( sid!=-1 ) EnableMenuItem(systmenu, sid);
		} else {
			if ( id!=-1 ) DisableMenuItem(popupmenu, id);
			if ( sid!=-1 ) DisableMenuItem(systmenu, sid);
		}
	}
};


int SYSTEM::GetMenuEnable(int num)
{
	return menuflag[num];
}


void SYSTEM::PopupMenuEnable(int sw)
{
	if ( (sw)&&(menuflag[0]) )
		EnableMenuItem(filemenu, 1);
	else
		DisableMenuItem(filemenu, 1);

	if ( (sw)&&(menuflag[1]) )
		EnableMenuItem(filemenu, 2);
	else
		DisableMenuItem(filemenu, 2);

	if ( (sw)&&(menuflag[28]) )
		EnableMenuItem(filemenu, 4);
	else
		DisableMenuItem(filemenu, 4);

	if ( sw )
		EnableMenuItem(systmenu, 0);
	else
		DisableMenuItem(systmenu, 0);
//	DrawMenuBar();
};


void SYSTEM::PopupContextMenu(void)
{
//	unsigned char state[16];
	Point p;
	int x, y, btn, n;

	mouse->GetState(&x, &y, &btn);
	p.h = x+2;
	p.v = y+2;
	LocalToGlobal(&p);
	//CalcMenuSize(popupmenu);
	PostEvent(mouseUp, 0);		// 画面効果後にポップアップすると、クリックホールド扱いになってしまうので
	n = PopUpMenuSelect(popupmenu, p.v, p.h, 0);
	MenuFunc(n);
};


void SYSTEM::MenuON(void)
{
	if ( (!FullScrMenu)&&(FullScrSW) ) {
		FullScrMenu = 1;
		ShowMenuBar();
	}
}


void SYSTEM::MenuOFF(void)
{
	if ( (FullScrMenu)&&(FullScrSW) ) {
		FullScrMenu = 0;
		HideMenuBar();
	}
}


/* -------------------------------------------------------------------
  シナリオクラスからのPDT関連のリダイレクタ
------------------------------------------------------------------- */
void SYSTEM::SnrPDT_MultiLoadFile(char* f) {
	MacroItemNum = 1;
	mm.cmd = 0x96;
	strcpy(mm.file, f);
	mgr->LoadBaseFile(f, 1);
	cgm->SetFlag(f);
}
void SYSTEM::SnrPDT_MultiLoadPDT(int n) {
	MacroItemNum = 1;
	mm.cmd = 0x97;
	sprintf(mm.file, "%d\0", n);
	mgr->AllCopy(n, 1, 0);
}
void SYSTEM::SnrPDT_ScreenFade(unsigned int cmd, unsigned int count, int r, int g, int b) {
	MACROITEM m;
	if ( !count ) {
		m.cmd = 0x68;
		m.filenum = 0;
		m.arg[26] = 0; m.arg[27] = 0; m.arg[28] = 639; m.arg[29] = 479;
		m.arg[30] = 0;
		m.arg[24] = ((r<<16)|(g<<8)|b);
		macro->StackMacro(&m);
	}
	mgr->ScreenFade(cmd, count, r, g, b);
}
void SYSTEM::SnrPDT_LoadFile(char* f, int dst) {
	MACROITEM m;
	m.cmd = 0x02;
	m.filenum = 1;
	strcpy(m.file, f);
	m.arg[26] = dst;
	macro->StackMacro(&m);
	mgr->LoadFile(f, dst);
	cgm->SetFlag(f);
}
void SYSTEM::SnrPDT_LoadCopy(char* f, int sx1, int sy1, int sx2, int sy2, int dx, int dy, int dstpdt, int flag, int subcmd) {
	char* buf = mm.file;
	int i;
	for (i=0; i<MacroItemNum; i++) buf += (strlen(buf)+1);
	strcpy(buf, f);
	switch(subcmd) {
		case 1:
			mm.arg[26+(MacroItemNum-1)*8] = 0;
			break;
		case 4:
			mm.arg[33+(MacroItemNum-1)*8] = flag;
		case 2:
		case 3:
			mm.arg[26+(MacroItemNum-1)*8] = 2;
			mm.arg[27+(MacroItemNum-1)*8] = sx1; mm.arg[28+(MacroItemNum-1)*8] = sy1;
			mm.arg[29+(MacroItemNum-1)*8] = sx2; mm.arg[30+(MacroItemNum-1)*8] = sy2;
			mm.arg[31+(MacroItemNum-1)*8] = dx;  mm.arg[32+(MacroItemNum-1)*8] = dy;
			break;
	}
	MacroItemNum++;
	mgr->LoadCopy(f, sx1, sy1, sx2, sy2, dx, dy, dstpdt, flag);
	cgm->SetFlag(f);
}
void SYSTEM::SnrPDT_LoadEffect(char* f, EFFECT* e) {
	MACROITEM m;
	m.cmd = 0x00;
	m.filenum = 1;
	strcpy(m.file, f);
	m.arg[ 0] = e->sx1; m.arg[ 1] = e->sy1; m.arg[ 2] = e->sx2; m.arg[ 3] = e->sy2;
	m.arg[ 4] = e->dx;  m.arg[ 5] = e->dy;  m.arg[ 6] = e->steptime; m.arg[ 7] = e->cmd;
	m.arg[ 8] = e->mask;  m.arg[ 9] = e->arg1;  m.arg[10] = e->arg2; m.arg[11] = e->arg3;
	m.arg[12] = e->step;  m.arg[13] = e->arg5;  m.arg[14] = e->arg6;
	macro->StackMacro(&m);
	mgr->LoadFile(f, 1);
	cgm->SetFlag(f);
}
void SYSTEM::SnrPDT_Copy(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag) {
	MACROITEM m;
	m.cmd = 0x64;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = src; m.arg[31] = dx;  m.arg[32] = dy;  m.arg[33] = dst;
	m.arg[34] = flag;
	macro->StackMacro(&m);
	mgr->Copy(sx1, sy1, sx2, sy2, src, dx, dy, dst, flag);
}
void SYSTEM::SnrPDT_CopyBackBuffer(int sx1, int sy1, int sx2, int sy2, int src, int update) {
	mgr->CopyBackBuffer(sx1, sy1, sx2, sy2, src, update);
}
void SYSTEM::SnrPDT_CopyWithMask(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag) {
	mgr->CopyWithMask(sx1, sy1, sx2, sy2, src, dx, dy, dst, flag);
}
void SYSTEM::SnrPDT_MaskCopy(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int flag) {
	MACROITEM m;
	m.cmd = 0x66;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = src; m.arg[31] = dx;  m.arg[32] = dy;  m.arg[33] = dst;
	m.arg[34] = flag;
	macro->StackMacro(&m);
	mgr->MaskCopy(sx1, sy1, sx2, sy2, src, dx, dy, dst, flag);
}
void SYSTEM::SnrPDT_MaskCopy(int sx1, int sy1, int sx2, int sy2, PDTBUFFER* src, int dx, int dy, int dst, int flag) {
	mgr->MaskCopy(sx1, sy1, sx2, sy2, src, dx, dy, dst, flag);
}
void SYSTEM::SnrPDT_MonoCopy(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst, int r, int g, int b) {
	MACROITEM m;
	m.cmd = 0x68;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = src; m.arg[31] = dx;  m.arg[32] = dy;  m.arg[33] = dst;
	m.arg[34] = ((r<<16)|(g<<8)|b);
	macro->StackMacro(&m);
	mgr->MonoCopy(sx1, sy1, sx2, sy2, src, dx, dy, dst, r, g, b);
}
void SYSTEM::SnrPDT_FadeColor(int sx1, int sy1, int sx2, int sy2, int src, int c1, int c2, int c3, int count) {
	MACROITEM m;
	m.cmd = 0x66;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = src; m.arg[31] = c1;  m.arg[32] = c2;  m.arg[33] = c3;
	m.arg[34] = count;
	macro->StackMacro(&m);
	mgr->FadeColor(sx1, sy1, sx2, sy2, src, c1, c2, c3, count);
}
void SYSTEM::SnrPDT_MakeMonochrome(int sx1, int sy1, int sx2, int sy2, int src) {
	mgr->MakeMonochrome(sx1, sy1, sx2, sy2, src);
}
void SYSTEM::SnrPDT_MakeInvert(int sx1, int sy1, int sx2, int sy2, int src) {
	mgr->MakeInvert(sx1, sy1, sx2, sy2, src);
}
void SYSTEM::SnrPDT_MakeColorMask(int sx1, int sy1, int sx2, int sy2, int src, int r, int g, int b) {
	mgr->MakeColorMask(sx1, sy1, sx2, sy2, src, r, g, b);
}
void SYSTEM::SnrPDT_StretchCopy(int sx1, int sy1, int sx2, int sy2, int src, int dx1, int dy1, int dx2, int dy2, int dst) {
	mgr->StretchCopy(sx1, sy1, sx2, sy2, src, dx1, dy1, dx2, dy2, dst);
}
void SYSTEM::SnrPDT_Effect(EFFECT* /*effect*/) {
	if ( MacroItemNum ) {
		mm.filenum = MacroItemNum;
		macro->StackMacro(&mm);
	}
	MacroItemNum = 0;
}
void SYSTEM::SnrPDT_FillRect(int sx1, int sy1, int sx2, int sy2, int src, int r, int g, int b) {
	MACROITEM m;
	m.cmd = 0x68;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = src;
	m.arg[24] = ((r<<16)|(g<<8)|b);
	macro->StackMacro(&m);
	mgr->FillRect(sx1, sy1, sx2, sy2, src, r, g, b);
}
void SYSTEM::SnrPDT_ClearRect(int sx1, int sy1, int sx2, int sy2, int srcpdt, int r, int g, int b) {
	MACROITEM m;
	m.cmd = 0x68;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = srcpdt;
	m.arg[24] = ((r<<16)|(g<<8)|b);
	macro->StackMacro(&m);
	mgr->ClearRect(sx1, sy1, sx2, sy2, srcpdt, r, g, b);
}
void SYSTEM::SnrPDT_DrawRectLine(int sx1, int sy1, int sx2, int sy2, int srcpdt, int r, int g, int b) {
//	MACROITEM m;
//	m.cmd = 0x68;
//	m.filenum = 0;
//	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
//	m.arg[30] = srcpdt;
//	m.arg[24] = ((r<<16)|(g<<8)|b);
//	macro->StackMacro(&m);
	mgr->DrawRectLine(sx1, sy1, sx2, sy2, srcpdt, r, g, b);
}
void SYSTEM::SnrPDT_AllCopy(int src, int dst, int flag) {
	MACROITEM m;
	m.cmd = 0x64;
	m.filenum = 0;
	m.arg[26] = 0;   m.arg[27] = 0;   m.arg[28] = 639; m.arg[29] = 479;
	m.arg[30] = src; m.arg[31] = 0;   m.arg[32] = 0;   m.arg[33] = dst;
	m.arg[34] = flag;
	macro->StackMacro(&m);
	mgr->AllCopy(src, dst, flag);
}
void SYSTEM::SnrPDT_Swap(int sx1, int sy1, int sx2, int sy2, int src, int dx, int dy, int dst) {
	MACROITEM m;
	m.cmd = 0x6a;
	m.filenum = 0;
	m.arg[26] = sx1; m.arg[27] = sy1; m.arg[28] = sx2; m.arg[29] = sy2;
	m.arg[30] = src; m.arg[31] = dx;  m.arg[32] = dy;  m.arg[33] = dst;
	macro->StackMacro(&m);
	mgr->Swap(sx1, sy1, sx2, sy2, src, dx, dy, dst);
}
void SYSTEM::SnrPDT_Get(int sx1, int sy1, int sx2, int sy2, int srcpdt) {
	mgr->Get(sx1, sy1, sx2, sy2, srcpdt);
}
void SYSTEM::SnrPDT_Put(int dx, int dy, int dstpdt) {
	mgr->Put(dx, dy, dstpdt);
}
void SYSTEM::SnrPDT_DrawString(int dx, int dy, int dstpdt, int r, int g, int b, char* s) {
	mgr->DrawString(dx, dy, dstpdt, r, g, b, s);
}


void SYSTEM::CopySel(int n, EFFECT* e)
{
	if ( n<MAX_SEL ) {
		memcpy(e, &ini.sel[n], sizeof(EFFECT));
dprintf("\nSEL#%d Copied(%d bytes). CMD=%d\n", n, sizeof(EFFECT), e->cmd);
		e->curcount = 0;
		e->prevtime = GetCurrentTimer();
		e->srcpdt = 1;
		e->dstpdt = 0;
	} else {
		dprintf("********* SEL#%d is too large!\n");
		memset(e, 0, sizeof(EFFECT));
	}
}


void SYSTEM::Effect(EFFECT* effect) {
	if ( effect->cmd ) mouse->StartPDTDraw();
	mgr->Effect(effect);
	if ( !effect->cmd ) mouse->FinishPDTDraw();
}


/* -------------------------------------------------------------------
  画面更新関連
------------------------------------------------------------------- */
void SYSTEM::UpdateScreen(void)
{
	Rect rect;
	
	GetGWorld(&gw_backup, &gd_backup);
	LockPixels(pixmap);
	SetGWorld(offscreen, NULL);
	mgr->GetPDT(0)->SetBuffer((unsigned char*)GetPixBaseAddr(pixmap));
	SetRect(&rect, 0, 0, 640, 480);
	if ( (FullScrSW)&&(!FullScrMenu) )
		CopyBits((BitMap*)*pixmap, &(hWnd->portBits), &rect, &rect, srcCopy, 0);
	else
		CopyBits((BitMap*)*pixmap, &(hWnd->portBits), &rect, &rect, srcCopy, hWnd->visRgn);
	UnlockPixels(pixmap);
	SetGWorld(gw_backup, gd_backup);
}


void SYSTEM::LockPDT(int n)
{
	if (!n) {
		GetGWorld(&gw_backup, &gd_backup);
		LockPixels(pixmap);
		SetGWorld(offscreen, NULL);
	}
	// Lockが掛かる毎にPDT0のアドレスを再設定しておく（変動するので）
	mgr->GetPDT(0)->SetBuffer((unsigned char*)GetPixBaseAddr(pixmap));
}

void SYSTEM::UnlockPDT(int n, int x1, int y1, int x2, int y2, int update)
{
	Rect rect;
	if (!n) {
		if ( (update)&&(loadingcount!=16) ) {
			SetRect(&rect, x1, y1, x2+1, y2+1);
			CopyBits((BitMap*)*pixmap, &(hWnd->portBits), &rect, &rect, srcCopy, hWnd->visRgn);
		}
		UnlockPixels(pixmap);
		SetGWorld(gw_backup, gd_backup);
	}
}


void SYSTEM::ClearScreen(void)
{
	unsigned char *dst;
	int dstbpl, y;
	GetGWorld(&gw_backup, &gd_backup);
	LockPixels(pixmap);
	SetGWorld(offscreen, NULL);
	dstbpl = mgr->GetPDT(0)->GetBPL();
	dst = (unsigned char*)GetPixBaseAddr(pixmap);
	for(y=0; y<=479; y++) {
		memset(dst, 0, 640*4);
		dst += dstbpl;
	}
	UnlockPixels(pixmap);
	SetGWorld(gw_backup, gd_backup);
}


/* -------------------------------------------------------------------
  メッセージウィンドウ関連
------------------------------------------------------------------- */
// ウィンドウはシステムウィンドウとサブウィンドウの2種類が存在する。
// コマンド$FE/$FFでの表示、$58-$02での選択肢はシステムウィンドウで、
// $58-$01での選択はサブウィンドウで行われる模様。
// 
// システムウィンドウのサイズは#MESSAGE_SIZEと#MSG_MOJI_SIZEで決定される。
// 位置は#WINDOW_SYS_POSが左上隅を指す。
// 
// サブウィンドウのサイズは#COM_WIND_MIN_SIZEが最小値、実際の大きさは
// 選択肢の数と文字列長で決定される模様。
// 位置は#WINDOW_COM_POSが左上隅を指すが、文字列が長く、ウィンドウが右側に
// はみだすような場合は、はみだす分これより左に移動する（BabyFaceなど）。
// 
// 尚、これらサイズ／位置はコマンド$72/$73で変更可能。
// NVL_SYSTEM=001 時はまた少し違う・・・

bool SYSTEM::MesWin_Setup(int n)
{
	int x, y, i, maxx, maxy;
	PDTFILE* temp=0;
//	char str[256];
	unsigned char *buf, *base, *mask, *maskbase;
	int r, g, b;
	bool redraw = MesWinFlag;
	bool redrawicon = MesIconFlag;
	PDTBUFFER* pdt;

dprintf("Waku Style = %d\n", n);
	MesWinStyle = n;

	pdt = mgr->GetPDT(WAKUPDT);
	if ( (pdt)&&(!ini.novelmode) ) MesWin_Hide();

	// 枠データの入ったファイルを開く
	if ( !pdt ) {
//		sprintf(str, ":%s:%s.PDT", ini.pdtdir, ini.wakufile);
//		temp = new PDTFILE(str, this);
		temp = new PDTFILE(ini.wakufile, this);
		pdt = new PDTBUFFER(640, 480, 3, 640*3, true);
		mgr->SetPDT(WAKUPDT, pdt);
		if ( (!pdt)||(!temp) ) {
			delete temp;
			delete pdt;
			mgr->SetPDT(WAKUPDT, 0);
			Error("MesWin - Can't create WAKUPDT");
			return false;
		}
		temp->CopyBuffer(pdt);
		delete temp;
	}

	if ( ini.novelmode ) {
		if ( redraw ) {
			mgr->Copy(0, 0, 639, 479, MESWINPDT, 0, 0, 0, 0);
			if ( selitemflag ) {
				selitemflag = false;
			} else {
				MesWin_ResetMesPos();
				MesWin_PrintMesAll();
			}
		} else {
			MesWinFlag = false;
			MesIconFlag = false;
		}
		return true;		// ノベルモード時はWAKUPDTだけ読めばよい
	}

	// メッセージウィンドウ用のバッファを確保
	pdt = mgr->GetPDT(MESWINPDT);
	if ( pdt ) delete pdt;
	pdt = new PDTBUFFER(640, 480, 3, 640*3, true);
	mgr->SetPDT(MESWINPDT, pdt);
	if ( !pdt )  {
		delete (mgr->GetPDT(WAKUPDT));
		dprintf("MesWin - Can't create MESWINPDT\n");
		return false;
	}

	n--;

	// メニューからの枠タイプ変更と、スクリプトからの変更では、情報は
	// 別々に保存されてるみたい。
	// スクリプトからの変更はセーブファイル毎に保存されてる。で、スク
	// リプトからの設定が0以外ならそれを使い、0ならば標準情報の方を使
	// う、って感じ（AIRとか）
	if ( MesWinStyleForce ) n = MesWinStyleForce;

	// 枠を描く
	mgr->FillRect(0, 0, 639, 479, MESWINPDT, 255, 255, 255);


	// 窓サイズ計算
	// ここまでに、フォントサイズの最下位ビットはマスクしておくように
	if ( subwinflag ) {
		maxx = ini.winx+((ini.mesx*ini.fontx*2+31)&0xfff0);			// SubWindow時はちょっと違うの
	} else {
		maxx = ini.winx+(((ini.mesx+1)*ini.fontx*2+31)&0xfff0);		// こーゆー計算のハズ
	}
/*
	if ( ini.fontsize<20 )
		maxy = ini.winy+((ini.mesy*ini.fonty+36)&0xfff0);		// 36も同様
	else
		maxy = ini.winy+((ini.mesy*ini.fonty+26)&0xfff0);		// 26も同様
*/
	maxy = ini.winy+((ini.mesy*ini.fonty+31)&0xfff0);				// こんな感じ？

	MesWinX1 = ini.winx;
	MesWinY1 = ini.winy;
	if ( maxx>639 ) { MesWinX1 -= (maxx-640); maxx = 640; }
	if ( maxy>479 ) { MesWinY1 -= (maxy-480); maxy = 480; }

	MesWinX2 = maxx-1;
	MesWinY2 = maxy-1;
	x = MesWinX1;
	y = MesWinY1;

	mgr->Copy(8,  8+n*100, 39, 31+n*100, WAKUPDT, x, y, MESWINPDT, 0);
	mgr->Copy(8, 48+n*100, 39, 71+n*100, WAKUPDT, x, maxy-24, MESWINPDT, 0);
	x += 32;
	for (i=0; i<(maxx-MesWinX1-64); i++) {
		mgr->Copy(40,  8+n*100, 40, 31+n*100, WAKUPDT, x, y, MESWINPDT, 0);
		mgr->Copy(40, 48+n*100, 40, 71+n*100, WAKUPDT, x, maxy-24, MESWINPDT, 0);
		x++;
	}
	mgr->Copy(88,  8+n*100, 119, 31+n*100, WAKUPDT, x, y, MESWINPDT, 0);
	mgr->Copy(88, 48+n*100, 119, 71+n*100, WAKUPDT, x, maxy-24, MESWINPDT, 0);
	y += 24;
	for (i=0; i<(maxy-MesWinY1-48); i++) {
		mgr->Copy( 8, 32+n*100,  39, 32+n*100, WAKUPDT, MesWinX1, y, MESWINPDT, 0);
		mgr->Copy(88, 32+n*100, 119, 32+n*100, WAKUPDT, maxx-32, y, MESWINPDT, 0);
		y++;
	}

	buf = mgr->GetPDT(WAKUPDT)->GetBuffer();
	r = buf[0]; g = buf[1]; b = buf[2];
	mgr->FillRect(MesWinX1+8, MesWinY1+8, maxx-8, maxy-8, MESWINPDT, r, g, b);

	base = mgr->GetPDT(MESWINPDT)->GetBuffer();
	maskbase = mgr->GetPDT(MESWINPDT)->GetMaskBuffer();

	memset(maskbase, 0, 640*480);

	dprintf("MesWin MesWinX1:%d MesWinY1:%d MesWinX2:%d MesWinY2:%d\n", MesWinX1, MesWinY1, MesWinX2, MesWinY2);
	dprintf("       ini.mesx:%d mesy:%d\n", ini.mesx, ini.mesy);

	for (y=MesWinY1; y<maxy; y++) {
		if ( y<0 ) continue;
		buf = base+(y*640*3)+MesWinX1*3;
		mask = maskbase+(y*640)+MesWinX1;
		for (x=MesWinX1; x<maxx; x++) {
			if ( x>=0 ) {
			if ( (buf[0]==r)&&(buf[1]==g)&&(buf[2]==b) ) {
				if ( (x>=(MesWinX1+ini.winatrx1))&&(y>=(MesWinY1+ini.winatry1))&&
						(x<(maxx-ini.winatrx2))&&(y<(maxy-ini.winatry2)) ) {
					buf[0] = ini.wincolor[0];
					buf[1] = ini.wincolor[1];
					buf[2] = ini.wincolor[2];
					if ( ini.wincolorflag )
						*mask = 255;		// 「半透明にしない」にチェックされてる時
					else
						*mask = 128;		// 半透明エリア（メッセージの背景）
				}
			} else {
				*mask = 255;
			}
			}
			buf += 3;
			mask++;
		}
	}

	if ( redraw ) {
		MesWin_Draw();
		if ( selitemflag ) {
			selitemflag = false;
		} else {
			MesWin_ResetMesPos();
			MesWin_PrintMesAll();
			if ( redrawicon ) MesWin_DrawIcon(0);
		}
	} else {
		MesWinFlag = false;
		MesIconFlag = false;
	}

	return true;
};


void SYSTEM::MesWin_SetPos(int x, int y)
{
	if ( (x)||(y) ) {
		MesWin_Hide();
		ini.winx = x;
		ini.winy = y;
		MesWin_Setup(MesWinStyle);
	}
};


void SYSTEM::MesWin_GetPos(int* x, int* y)
{
	*x = ini.winx;
	*y = ini.winy;
};


void SYSTEM::MesWin_SetSize(int w, int h)
{
	MesWin_Hide();
	ini.mesx = w;
	ini.mesy = h;
	MesWin_Setup(MesWinStyle);
};


void SYSTEM::MesWin_GetSize(int* w, int* h)
{
	*w = ini.mesx;
	*h = ini.mesy;
};


void SYSTEM::MesWin_SetSubPos(int x, int y)
{
	ini.subwinx = x;
	ini.subwiny = y;
};


void SYSTEM::MesWin_GetSubPos(int* x, int* y)
{
	*x = ini.subwinx;
	*y = ini.subwiny;
};


void SYSTEM::MesWin_SetSubSize(int w)
{
	ini.subwinmin = w;
};


void SYSTEM::MesWin_GetSubSize(int* w)
{
	*w = ini.subwinmin;
};


void SYSTEM::MesWin_DrawWaku(void)
{
	unsigned char *dst, *src, *msk, *srcbuf, *dstbuf, *mskbuf, maskc;
	unsigned char *tmp, *tmpbuf;
	int dstbpl, x, y;

	LockPDT(0);
	dstbpl = mgr->GetPDT(0)->GetBPL();
	dstbuf = mgr->GetPDT(0)->GetBuffer();
	srcbuf = mgr->GetPDT(MESWINPDT)->GetBuffer();
	mskbuf = mgr->GetPDT(MESWINPDT)->GetMaskBuffer();
	tmpbuf = mgr->GetPDT(BACKUPPDT)->GetBuffer();
	for(y=MesWinY1; y<=MesWinY2; y++) {
		src = srcbuf+y*640*3+MesWinX1*3;
		tmp = tmpbuf+y*640*3+MesWinX1*3;
		dst = dstbuf+y*dstbpl+MesWinX1*4;
		msk = mskbuf+y*640+MesWinX1;
		for (x=MesWinX1; x<=MesWinX2; x++) {
			maskc = *msk++;
			if ( !maskc ) {
				dst[1] = tmp[0];
				dst[2] = tmp[1];
				dst[3] = tmp[2];
			} else if ( maskc==255 ) {
				dst[1] = src[0];
				dst[2] = src[1];
				dst[3] = src[2];
			} else {
				dst[1] = (src[0]*tmp[0])>>8;
				dst[2] = (src[1]*tmp[1])>>8;
				dst[3] = (src[2]*tmp[2])>>8;
			}
			src += 3;
			dst += 4;
			tmp += 3;
		}
	}
	UnlockPDT(0, MesWinX1, MesWinY1, MesWinX2, MesWinY2, 1);
};


void SYSTEM::MesWin_Draw(void)
{
	if ( !MesWinFlag ) {
		PlaySE(2);						// SE.002 が鳴るらしい
		MesWinFlag = true;
		MesIconFlag = false;
		if ( ini.novelmode ) {			// ノベルシステム時
			mgr->AllCopy(0, BACKUPPDT, 0);
			mgr->AllCopy(BACKUPPDT, MESWINPDT, 0);
			mgr->MakeColorMask(0, 0, 639, 479, MESWINPDT, ini.wincolor[0], ini.wincolor[1], ini.wincolor[2]);
			scnefct.sx1 = 0; scnefct.sy1 = 0;
			scnefct.sx2 = 639; scnefct.sy2 = 479;
			scnefct.dx = 0; scnefct.dy = 0;
			scnefct.cmd = 4;
			scnefct.curcount = 0;
			scnefct.srcpdt = MESWINPDT; scnefct.dstpdt = 0;
			scnefct.steptime = ini.defaultfadetime;
			scnefct.prevtime = GetCurrentTimer();
		} else {
			mgr->Copy(MesWinX1, MesWinY1, MesWinX2, MesWinY2, 0, MesWinX1, MesWinY1, BACKUPPDT, 0);
			MesWin_DrawWaku();
		}
	}
};


void SYSTEM::MesWin_Hide(void)
{
	if ( MesWinFlag ) {
		MesWinFlag = false;
		MesIconFlag = false;
		if ( ini.novelmode ) {
			scnefct.sx1 = 0; scnefct.sy1 = 0;
			scnefct.sx2 = 639; scnefct.sy2 = 479;
			scnefct.dx = 0; scnefct.dy = 0;
			scnefct.cmd = 4;
			scnefct.curcount = 0;
			scnefct.srcpdt = BACKUPPDT; scnefct.dstpdt = 0;
			scnefct.steptime = ini.defaultfadetime;
			scnefct.prevtime = GetCurrentTimer();
		} else {
			mgr->Copy(MesWinX1, MesWinY1, MesWinX2, MesWinY2, BACKUPPDT, MesWinX1, MesWinY1, 0, 0);
		}
	}
};


void SYSTEM::MesWin_HideTemp(void)
{
	if ( MesWinFlag ) {
		if ( ini.novelmode )
			mgr->AllCopy(BACKUPPDT, 0, 0);
		else
			mgr->Copy(MesWinX1, MesWinY1, MesWinX2, MesWinY2, BACKUPPDT, MesWinX1, MesWinY1, 0, 0);
	}
};


int SYSTEM::MesWin_InEffect(void)
{
	if ( scnefct.cmd ) {
		mouse->StartPDTDraw();
		mgr->Effect(&scnefct);
		if ( !scnefct.cmd ) mouse->FinishPDTDraw();
		return 1;
	} else {
		return 0;
	}
}


/* -------------------------------------------------------------------
  メッセージ表示
------------------------------------------------------------------- */

// 機種依存文字変換テーブル
static unsigned short SpCharTable[0x60] = {
/* 8740 */	0x8540, 0x8541, 0x8542, 0x8543, 0x8544, 0x8545, 0x8546, 0x8547,
/* 8748 */	0x8548, 0x8549, 0x854a, 0x854b, 0x854c, 0x854d, 0x854e, 0x854f,
/* 8750 */	0x8550, 0x8551, 0x8552, 0x8553, 0x859f, 0x85a0, 0x85a1, 0x85a2,
/* 8758 */	0x85a3, 0x85a4, 0x85a5, 0x85a6, 0x85a7, 0x85a8, 0x8140, 0x879f,
/* 8760 */	0x87a2, 0x87a0, 0x87a1, 0x87a9, 0x87ab, 0x87a7, 0x87a8, 0x87ac,
/* 8768 */	0x87af, 0x87b0, 0x87b3, 0x87b2, 0x87b5, 0x87ad, 0x87b4, 0x8640,
/* 8770 */	0x8642, 0x8648, 0x864a, 0x864c, 0x864d, 0x8646, 0x8140, 0x8140,
/* 8778 */	0x8140, 0x8140, 0x8140, 0x8140, 0x8140, 0x8140, 0x87e8, 0x8140,
/* 8780 */	0x8854, 0x8855, 0x869b, 0x869c, 0x869d, 0x8793, 0x8794, 0x8795,
/* 8788 */	0x8796, 0x8797, 0x874d, 0x8750, 0x874b, 0x87e5, 0x87e6, 0x87e7,
/* 8790 */	0x81e0, 0x81df, 0x81e7, 0x8840, 0x83b0, 0x81e3, 0x81db, 0x81da,
/* 8798 */	0x8841, 0x8842, 0x81e6, 0x81bf, 0x81be, 0x8140, 0x8140, 0x8140
};



// NVL_SYSTEM=1 専用文字表示
void SYSTEM::MesWin_PutNovelChar(int dx, int dy, int c)
{
	unsigned char *dst, *dstbuf, *src, *srcbuf;
	int dstbpl, x, y, n, nl, nh;
	int r, g, b;

	srcbuf = nvlfont->GetFont(c);			// FontDataを取ってくる。24x24pixel(4bit/pixel)ね
	if ( srcbuf ) {
		LockPDT(0);
		r = ini.colortable[ini.fontcolor][0]+1;
		g = ini.colortable[ini.fontcolor][1]+1;
		b = ini.colortable[ini.fontcolor][2]+1;
		dstbpl = mgr->GetPDT(0)->GetBPL();
		dstbuf = mgr->GetPDT(0)->GetBuffer();
		dstbuf += (dx+2)*4+(dy+2)*dstbpl;
		src = srcbuf;
		for (y=0; y<24; y++) {				// まず影を描く（2dotずれ？）
			dst = dstbuf;
			for (x=0; x<12; x++) {
				n = *src++;
				nl = (256-((n&0x0f)|((n&0x0f)<<4)));
				nh = (256-((n&0xf0)|((n&0xf0)>>4)));
				dst[1] -= ((dst[1]*nl)>>8);
				dst[2] -= ((dst[2]*nl)>>8);
				dst[3] -= ((dst[3]*nl)>>8);
				dst[5] -= ((dst[5]*nh)>>8);
				dst[6] -= ((dst[6]*nh)>>8);
				dst[7] -= ((dst[7]*nh)>>8);
				dst += 8;
			}
			dstbuf += dstbpl;
		}
		dstbuf = mgr->GetPDT(0)->GetBuffer();
		dstbuf += dx*4+dy*dstbpl;
		src = srcbuf;
		for (y=0; y<24; y++) {				// テキスト色で本体を描く
			dst = dstbuf;
			for (x=0; x<12; x++) {
				n = *src++;
				nl = (255-((n&0x0f)|((n&0x0f)<<4)));
				nh = (255-((n&0xf0)|((n&0xf0)>>4)));
				dst[1] += (((r - dst[1])*nl)>>8);
				dst[2] += (((g - dst[2])*nl)>>8);
				dst[3] += (((b - dst[3])*nl)>>8);
				dst[5] += (((r - dst[5])*nh)>>8);
				dst[6] += (((g - dst[6])*nh)>>8);
				dst[7] += (((b - dst[7])*nh)>>8);
				dst += 8;
			}
			dstbuf += dstbpl;
		}
		UnlockPDT(0, dx, dy, dx+ini.fontx*2-1, dy+ini.fonty-1, 1);
	}
}


void SYSTEM::MesWin_ClearMes(void)
{
	mesbuf[0] = 0;
	mesbufptr = mesbuf;
	curmesx = 0;
	curmesy = 0;
	indentflag = 0;
	mesindent = 0;
	MesIconFlag = 0;
	doubleline = 0;
	if ( MesWinFlag ) {
		if ( ini.novelmode ) {
			mgr->Copy(0, 0, 639, 479, MESWINPDT, 0, 0, 0, 0);
		} else {
			MesWin_DrawWaku();
		}
	}
}


// 文章途中で改ページになった時用
void SYSTEM::MesWin_ClearWindow(void)
{
	unsigned char* s = mesbufptr;
	unsigned char* d = mesbuf;

	// バッファが溢れないように、前ページ部分は切り詰める
	while (*s) {
		*d++ = *s++;
	}
	*d = 0;
	mesbufptr = mesbuf;

	curmesx = mesindent;
	curmesy = 0;
	MesIconFlag = 0;
	doubleline = 0;
	if ( MesWinFlag ) {
		if ( ini.novelmode ) {
			mgr->Copy(0, 0, 639, 479, MESWINPDT, 0, 0, 0, 0);
		} else {
			MesWin_DrawWaku();
		}
	}
}


void SYSTEM::MesWin_SetMes(char* buf)
{
	int i = 0, st = 0, ed = 0;
	char* tmp;
	unsigned char a, b;
	int firstflag = 0;

	mouse->StartPDTDraw();
//	dprintf("SetMes - CurX:%d CurY:%d Base:$%08X/Ptr:$%08X\n", curmesx, curmesy, (int)mesbuf, (int)mesbufptr);

	while ( mesbuf[i] ) i++;			// Add...

//	if ( mesbufptr==mesbuf ) firstflag=1;

	a = *buf++;
	mesbuf[i++] = a;

	if ( a==0xfe ) {					// Hankaku
		while (*buf) {
			a = *buf++;
			mesbuf[i++] = a;
		}
	} else if ( a==0xff ) {				// Zenkaku
		while (*buf) {
			a = *buf++;
			b = *buf++;
			if ( (a==0x81)&&(b==0x96) ) {
				buf++;
				if ( *buf==0x4f ) {		// ExFont (*00～*??)
					mesbuf[i++] = a;
					mesbuf[i++] = b;
					mesbuf[i++] = (*buf-0x4f)*10+((*(buf+2))-0x4f);
					buf += 3;
				} else {
					tmp = ini.name[(*buf++)-0x60];
					while(*tmp) { mesbuf[i++] = *tmp++; }
				}
			} else {
				mesbuf[i++] = a;
				mesbuf[i++] = b;
			}
		}
	} else if ( a==0x0d ) {
//		mesbuf[i++] = a;
	}

	mesbuf[i] = 0;
//	indentflag = 0;
//	mesindent = 0;
	curmestime = GetCurrentTimer();
}



void SYSTEM::MesWin_ResetMesPos(void)
{
	mesbufptr = mesbuf;
	curmesx = 0;
	curmesy = 0;
	indentflag = 0;
	mesindent = 0;
	doubletext = 0;
	doubleline = 0;
}


void SYSTEM::MesWin_PutChar_Novel(void)
{
	int flag, x, y, c;
	unsigned char a = 0, b = 0;

	if ( (curmesy>=ini.mesy) ) return;

	flag = 1;
	while ( flag ) {
		a = *mesbufptr;
		if ( a )
			b = *(mesbufptr+1);
		else
			return;

		if ( a==0x0d ) {
			mesbufptr++;
			curmesx = mesindent;
			curmesy++;
			return;
		}
		if ( a==0xff ) {
			hankaku = false;
			mesbufptr++;
		} else if ( (a==0x81)&&(b==0x79) ) {	// 【
			indentflag = 0;
			mesindent = 0;
			curmesx = 0;
			mesbufptr+=2;
		} else if ( (a==0x81)&&(b==0x7a) ) {	//　】
			indentflag = 1000;			// 名前が来たらインデント開始
			mesbufptr+=2;
		} else if ( (a==0x81)&&(b==0x75) ) {	// 「
			if ( indentflag==1000 ) mesindent = curmesx;
			indentflag++;
			flag = 0;
		} else if ( (a==0x81)&&(b==0x76) ) {	// 」
			indentflag--;
			if ( indentflag<=0 ) {
				mesindent = 0;
//				indentflag = -99;
//				indentflag = 0;
			} else if ( indentflag==1000 ) {
				mesindent = 0;
				indentflag = 0;
			}
			flag = 0;
		} else if ( (a==0x81)&&(b==0x77) ) {	// 『
			if ( indentflag==1000 ) mesindent = curmesx;
			indentflag++;
			flag = 0;
		} else if ( (a==0x81)&&(b==0x78) ) {	// 』
			indentflag--;
			if ( indentflag<=0 ) {
				mesindent = 0;
//				indentflag = -99;
//				indentflag = 0;
			} else if ( indentflag==1000 ) {
				mesindent = 0;
				indentflag = 0;
			}
			flag = 0;
		} else {
			flag = 0;
			if ( !indentflag ) indentflag = 99;		// 括弧以外の文字が出た後はインデントしない
		}
	}

	if ( curmesx>=(ini.mesx*2-2) ) {
		if ( curmesx>=(ini.mesx*2) ) {
			curmesx = mesindent;
			curmesy++;
			if ( curmesy>=ini.mesy ) return;
		} else if ( ((*mesbufptr)!=0x81)||((*(mesbufptr+2))==0x81) ) {
			curmesx = mesindent;
			curmesy++;
			if ( curmesy>=ini.mesy ) return;
		} else if ( ((*(mesbufptr+1))>=0x65)&&((*(mesbufptr+1))<0x7a)&&((*(mesbufptr+1))&1) ) {
			curmesx = mesindent;
			curmesy++;
			if ( curmesy>=ini.mesy ) return;
		}
	}

	x = curmesx*ini.fontx+ini.winx;
	y = curmesy*ini.fonty+ini.winy;
	c = (a<<8)|b;
	MesWin_PutNovelChar(x, y, c);
	mesbufptr += 2;
	curmesx += 2;

	if ( *mesbufptr==0x0d ) {		// 行末チェックが入る前に、もう一度改行チェックをしとく（Pureheart）
		curmesx = 0;
		curmesy++;
		mesbufptr++;
		indentflag = 0;
		mesindent = 0;
	}
}


void SYSTEM::MesWin_PutExFont(int dx, int dy, int c)
{
	unsigned char *dst, *dstbuf, *src, *srcbuf;
	int srcbpl, dstbpl, x, y;
	int r, g, b;

	x = c%ini.exfontmaxx;
	y = c/ini.exfontmaxx;
	srcbuf = mgr->GetPDT(EXFONTPDT)->GetBuffer();
	srcbuf += (y*ini.exfonty*mgr->GetPDT(EXFONTPDT)->GetBPL())+x*ini.exfontx*3;
	if ( srcbuf ) {
//		LockPDT(0);
		r = ini.colortable[ini.fontcolor][0]+1;
		g = ini.colortable[ini.fontcolor][1]+1;
		b = ini.colortable[ini.fontcolor][2]+1;
		dstbpl = mgr->GetPDT(0)->GetBPL();
		srcbpl = mgr->GetPDT(EXFONTPDT)->GetBPL();
		dstbuf = mgr->GetPDT(0)->GetBuffer();
		dstbuf += (dx+2)*4+(dy+2)*dstbpl;
		for (y=0; y<ini.fontsize; y++) {				// まず影を描く（2dotずれ？）
			src = srcbuf;
			dst = dstbuf;
			for (x=0; x<ini.fontsize; x++) {
				if ( src[2]||src[1]||src[0] ) {
					dst[1] -= ((dst[1]*src[2])>>8);
					dst[2] -= ((dst[2]*src[1])>>8);
					dst[3] -= ((dst[3]*src[0])>>8);
				}
				src += 3;
				dst += 4;
			}
			srcbuf += srcbpl;
			dstbuf += dstbpl;
		}
		dstbuf = mgr->GetPDT(0)->GetBuffer();
		dstbuf += (dx+1)*4+(dy+1)*dstbpl;
		x = c%ini.exfontmaxx;
		y = c/ini.exfontmaxx;
		srcbuf = mgr->GetPDT(EXFONTPDT)->GetBuffer();
		srcbuf += (y*ini.exfonty*mgr->GetPDT(EXFONTPDT)->GetBPL())+x*ini.exfontx*3;
		for (y=0; y<ini.exfonty; y++) {				// テキスト色で本体を描く
			src = srcbuf;
			dst = dstbuf;
			for (x=0; x<ini.exfontx; x++) {
				if ( src[2]||src[1]||src[0] ) {
					dst[1] += (((r - dst[1])*src[2])>>8);
					dst[2] += (((g - dst[2])*src[1])>>8);
					dst[3] += (((b - dst[3])*src[0])>>8);
				}
//dst[1]=dst[2]=dst[3]=0xff;
				src += 3;
				dst += 4;
			}
			srcbuf += srcbpl;
			dstbuf += dstbpl;
		}
//		UnlockPDT(0, dx, dy, dx+ini.exfontx-1, dy+ini.exfonty-1, 1);
	}
}


void SYSTEM::MesWin_PutChar(void)
{
	int i = 0, flag, x, y, spindent = 0;
	unsigned char a = 0, b = 0;
	RGBColor col;
	FontInfo info;

	if ( ini.novelmode ) { MesWin_PutChar_Novel(); return; }

	flag = 1;

	while ( flag ) {

		a = *mesbufptr;
		if ( a )
			b = *(mesbufptr+1);
		else
			return;

		if ( a==0x0d ) {
			mesbufptr++;
			curmesx = mesindent;
			curmesy++;
if ( doubleline ) {
	curmesy++;
	doubleline = 0;
}
			return;
		}
		if ( a==0xfe ) {
			hankaku = true;
			mesbufptr++;
		} else if ( a==0xff ) {
			hankaku = false;
			mesbufptr++;
		} else if ( (a==0x81)&&(b==0x79) ) {	// 【
			indentflag = 0;
			mesindent = 0;
			curmesx = 0;
			mesbufptr+=2;
		} else if ( (a==0x81)&&(b==0x7a) ) {	//　】
			if ( (*(mesbufptr+2)==0x81)&&(*(mesbufptr+3)==0x40) ) {
				mesindent = curmesx;
				indentflag = 1;
				while ( (*(mesbufptr+2)==0x81)&&(*(mesbufptr+3)==0x40) ) {
					mesbufptr += 2;
					mesindent += 2;
				}
			} else {
				indentflag = 0;			// 名前はインデント停止文字から除外
				mesbufptr+=2;
			}
		} else if ( (a==0x81)&&(b==0x75) ) {	// 「
			if ( !indentflag ) mesindent = curmesx+2;
			indentflag++;
			flag = 0;
		} else if ( (a==0x81)&&(b==0x76) ) {	// 」
			indentflag--;
			if ( indentflag<=0 ) {
				spindent = mesindent;
				mesindent = 0;
//				indentflag = -99;
//				indentflag = 0;
			}
			flag = 0;
		} else if ( (a==0x81)&&(b==0x77) ) {	// 『
			if ( !indentflag ) mesindent = curmesx+2;
			indentflag++;
			flag = 0;
		} else if ( (a==0x81)&&(b==0x78) ) {	// 』
			indentflag--;
			if ( indentflag<=0 ) {
				spindent = mesindent;
				mesindent = 0;
//				indentflag = -99;
//				indentflag = 0;
			}
			flag = 0;
		} else {
			flag = 0;
			if ( !indentflag ) indentflag = 99;		// 括弧以外の文字が出た後はインデントしない
		}
	}

	if ( curmesx>=(ini.mesx*2-2) ) {
		if ( curmesx>=(ini.mesx*2) ) {
			if (spindent)
				curmesx = spindent;
			else
				curmesx = mesindent;
			curmesy++;
if ( doubleline ) {
	curmesy++;
	doubleline = 0;
}
			if ( curmesy==ini.mesy ) return;
//		} else if ( ((*mesbufptr)!=0x81)||((*(mesbufptr+2))==0x81) ) {
		} else if ( (*mesbufptr)!=0x81 ) {
			if (spindent)
				curmesx = spindent;
			else
				curmesx = mesindent;
			curmesy++;
if ( doubleline ) {
	curmesy++;
	doubleline = 0;
}
			if ( curmesy==ini.mesy ) return;
		} else if ( ((*(mesbufptr+1))>=0x65)&&((*(mesbufptr+1))<0x7a)&&((*(mesbufptr+1))&1) ) {
			if (spindent)
				curmesx = spindent;
			else
				curmesx = mesindent;
			curmesy++;
if ( doubleline ) {
	curmesy++;
	doubleline = 0;
}
			if ( curmesy==ini.mesy ) return;
		}
	}

	GetFontInfo(&info);
	x = curmesx*ini.fontx+MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2+ini.fontx)/2;
	y = MesWinY1 + curmesy*ini.fonty + ini.fontsize+(info.leading)/3 + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2;
if ( doubletext ) {
	TextSize(ini.fontsize*2);
	y += ini.fonty;
	doubleline = 1;
}
	if ( hankaku ) {
		col.red   = 0;
		col.green = 0;
		col.blue  = 0;
		RGBForeColor(&col);
		MoveTo(x+1, y+1);
		DrawText(mesbufptr, 0, 1);
		col.red   = ini.colortable[ini.fontcolor][0]<<8;
		col.green = ini.colortable[ini.fontcolor][1]<<8;
		col.blue  = ini.colortable[ini.fontcolor][2]<<8;
		RGBForeColor(&col);
		MoveTo(x, y);
		DrawText(mesbufptr, 0, 1);
		mesbufptr += 1;
		curmesx += 1;
if ( doubletext ) curmesx++;
	} else {
		if ( (mesbufptr[0]==0x87) ) {		// 機種依存文字 (-_-;
			if ( (mesbufptr[1]>=0x40)&&(mesbufptr[1]<0xa0) ) {
				*((unsigned short*)mesbufptr) = SpCharTable[mesbufptr[1]-0x40];
			}
		}
		if ( (mesbufptr[0]==0x81)&&(mesbufptr[1]==0x96) ) {
			x = curmesx*ini.fontx+MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2+ini.fontx)/2;
			y = MesWinY1 + curmesy*ini.fonty + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2;
			MesWin_PutExFont(x, y, mesbufptr[2]);
			mesbufptr += 3;
			curmesx += 2;
		} else {
			col.red   = 0;
			col.green = 0;
			col.blue  = 0;
			RGBForeColor(&col);
			MoveTo(x+1, y+1);
			DrawText(mesbufptr, 0, 2);
			col.red   = ini.colortable[ini.fontcolor][0]<<8;
			col.green = ini.colortable[ini.fontcolor][1]<<8;
			col.blue  = ini.colortable[ini.fontcolor][2]<<8;
			RGBForeColor(&col);
			MoveTo(x, y);
			DrawText(mesbufptr, 0, 2);
			mesbufptr += 2;
			curmesx += 2;
			if ( doubletext ) curmesx += 2;
		}
	}
}


int SYSTEM::MesWin_PrintMesAll(void)
{
	RGBColor colbk;
	int ret;

	if ( !(*mesbufptr) ) return 0;

	if ( !MesWinFlag ) {
		MesWin_Draw();
		if ( ini.novelmode ) mesredraw = 1;
		return 1;
	}

	if ( (ini.novelmode)&&(mesredraw) ) {
		mesredraw = 0;
		MesWin_ResetMesPos();
	}

	if ( !ini.novelmode ) {
		LockPDT(0);
		GetForeColor(&colbk);
		TextFont(fontid);
		TextSize(ini.fontsize);
	}

	ret = 1;
	while ( *mesbufptr ) {

		MesWin_PutChar();

		if ( !(*mesbufptr) ) { ret = 0; break; }
		if ( curmesy>=ini.mesy ) { ret = -1; break; };
	}

	if ( !ini.novelmode ) {
		RGBForeColor(&colbk);
		UnlockPDT(0, MesWinX1, MesWinY1, MesWinX2, MesWinY2, 1);
	}

	mouse->FinishPDTDraw();
	return ret;
}


int SYSTEM::MesWin_PrintMes(void)
{
	unsigned char c;
	RGBColor colbk;

	if ( !ini.meswait ) return MesWin_PrintMesAll();
	if ( !(*mesbufptr) ) {
		mouse->FinishPDTDraw();
		return 0;
	}

	if ( (GetCurrentTimer()-curmestime)<ini.meswait ) return 1;
	curmestime = GetCurrentTimer();

	if ( !MesWinFlag ) {
		MesWin_Draw();
		if ( ini.novelmode ) mesredraw = 1;
		return 1;
	}

	if ( (ini.novelmode)&&(mesredraw) ) {
		mesredraw = 0;
		c = *mesbufptr;
		*mesbufptr = 0;
		MesWin_ResetMesPos();
		MesWin_PrintMesAll();
		*mesbufptr = c;
	}

	if ( !ini.novelmode ) {
		LockPDT(0);
		GetForeColor(&colbk);
		TextFont(fontid);
		TextSize(ini.fontsize);
	}

	MesWin_PutChar();

	if ( !ini.novelmode ) {
		RGBForeColor(&colbk);
		UnlockPDT(0, MesWinX1, MesWinY1, MesWinX2, MesWinY2, 1);
	}

	if ( !(*mesbufptr) ) {
		mouse->FinishPDTDraw();
		return 0;
	}
	if ( curmesy>=ini.mesy ) {
//		mouse->FinishPDTDraw();
		return -1;
	}

	return 1;
}


void SYSTEM::ChangeFontColor(int n)
{
	ini.fontcolor = n;
}


void SYSTEM::ChangeMesWinStyle(int n)
{
	MesWinStyleForce = n;
	MesWin_Setup(MesWinStyle);
}


/* -------------------------------------------------------------------
  クリック待ちアイコン
------------------------------------------------------------------- */

void SYSTEM::MesWin_DrawIcon_Novel(int n)
{
	int x = curmesx;
	int y = curmesy;
	if ( x>=(ini.mesx*2) ) {	// 行末に来た場合、アイコンは次の行の先頭に描く
//		x = mesindent;			// ただし、文章自体はこの時点でまだ改行していない
		x = 0;
		y++;
	}
	MesWin_DrawNovelIcon(x*ini.fontx+ini.winx, y*ini.fonty+ini.winy, n);
}


void SYSTEM::MesWin_DrawIcon(int n)
{
	unsigned char *dst, *src, *srcbuf, *dstbuf, *tmp;
	int dstbpl, x, y;
	unsigned char r, g, b;

	if ( !MesWinFlag ) return;			// メッセージウィンドウが無い時は帰る

	if ( ini.novelmode ) { MesWin_DrawIcon_Novel(n); return; }

	if ( !MesIconFlag ) {
 		mgr->Copy(MesWinX2-ini.mesiconx-7, MesWinY2-ini.mesicony-7, MesWinX2-8, MesWinY2-8, 0, 640-ini.mesiconx, 480-ini.mesicony, WAKUPDT, 0);
		MesIconFlag = true;
		mesiconbase = GetCurrentTimer();

		mouse->GetButton();
	}

	if ( GetCurrentTimer()<(mesiconbase+ini.mesiconwait) ) return;
	mesiconbase = GetCurrentTimer();

	LockPDT(0);

	dstbpl = mgr->GetPDT(0)->GetBPL();
	dstbuf = mgr->GetPDT(0)->GetBuffer();
	srcbuf = mgr->GetPDT(WAKUPDT)->GetBuffer();

	r = srcbuf[0]; g = srcbuf[1]; b = srcbuf[2];

	for(y=0; y<ini.mesicony; y++) {
		src = srcbuf+(y+8)*640*3+(160+mesicon*ini.mesiconx)*3;
		tmp = srcbuf+(y+480-ini.mesicony)*640*3+(640-ini.mesiconx)*3;
		dst = dstbuf+(y+MesWinY2-ini.mesicony-7)*dstbpl+(MesWinX2-ini.mesiconx-7)*4;
		for (x=0; x<ini.mesiconx; x++) {
			if ( (src[0]==r)&&(src[1]==g)&&(src[2]==b) ) {
				dst[1] = tmp[0];
				dst[2] = tmp[1];
				dst[3] = tmp[2];
			} else {
				dst[1] = src[0];
				dst[2] = src[1];
				dst[3] = src[2];
			}
			src += 3;
			dst += 4;
			tmp += 3;
		}
	}

	UnlockPDT(0, MesWinX2-ini.mesiconx-7, MesWinY2-ini.mesicony-7, MesWinX2-8, MesWinY2-8, 1);
	mesicon = (mesicon+1)%ini.mesiconnum;
};


void SYSTEM::MesWin_DrawNovelIcon(int dx, int dy, int chr)
{
	unsigned char *dst, *src, *srcbuf, *dstbuf, *tmp, *tmpbuf;
	int dstbpl, x, y;
	unsigned char r, g, b;

	if ( !MesWinFlag ) return;			// メッセージウィンドウが無い時は帰る

	if ( !MesIconFlag ) {
		MesIconFlag = 1;
		mesiconbase = GetCurrentTimer()-ini.mesiconwait;
	}

	if ( GetCurrentTimer()<(mesiconbase+ini.mesiconwait) ) return;
	mesiconbase = GetCurrentTimer();

	LockPDT(0);

	dstbpl = mgr->GetPDT(0)->GetBPL();
	dstbuf = mgr->GetPDT(0)->GetBuffer();
	srcbuf = mgr->GetPDT(WAKUPDT)->GetBuffer();
	tmpbuf = mgr->GetPDT(MESWINPDT)->GetBuffer();

	r = srcbuf[0]; g = srcbuf[1]; b = srcbuf[2];

	for(y=0; y<24; y++) {
		src = srcbuf+(chr*48+48+y)*640*3+(160+mesicon*32)*3;
		tmp = tmpbuf+(y+dy)*640*3+dx*3;
		dst = dstbuf+(y+dy)*dstbpl+dx*4;
		for (x=0; x<32; x++) {
			if ( (src[0]==r)&&(src[1]==g)&&(src[2]==b) ) {
				dst[1] = tmp[0];
				dst[2] = tmp[1];
				dst[3] = tmp[2];
			} else {
				dst[1] = src[0];
				dst[2] = src[1];
				dst[3] = src[2];
			}
			src += 3;
			dst += 4;
			tmp += 3;
		}
	}

	UnlockPDT(0, dx, dy, dx+31, dy+23, 1);
	mesicon = (mesicon+1)%8;
};


void SYSTEM::MesWin_HideIcon(void)
{
	int x = curmesx;
	int y = curmesy;
	if ( (MesIconFlag)&&(MesWinFlag) ) {
		MesIconFlag = 0;
		if ( ini.novelmode ) {
			if ( x>=(ini.mesx*2) ) {
				x = 0;//mesindent;
				y++;
			}
			mgr->Copy(x*ini.fontx+ini.winx, y*ini.fonty+ini.winy, x*ini.fontx+ini.winx+31, y*ini.fonty+ini.winy+23, MESWINPDT, x*ini.fontx+ini.winx, y*ini.fonty+ini.winy, 0, 0);
		} else {
	 		mgr->Copy(640-ini.mesiconx, 480-ini.mesicony, 639, 479, WAKUPDT, MesWinX2-ini.mesiconx-7, MesWinY2-ini.mesicony-7, 0, 0);
		}
	}
}


int SYSTEM::MesWin_LineFeed(void)
{
	if ( MesWinFlag ) {
		if ( ini.novelmode ) {
			curmesx = 0;
			curmesy++;
			*mesbufptr++ = 0x0d;	// CG描画等で一回テキストが消えた後に、
			*mesbufptr = 0;			// 再度ここまでのテキストを書き直す時のため
			indentflag = 0;
			mesindent = 0;
			// 画面外にはみだした時は「次ページ」のアイコンを出して待つため
			if ( (curmesy>=ini.mesy) ) return -1;
		} else {
			mesbuf[0] = 0;
			mesbufptr = mesbuf;
			curmesx = 0;
			curmesy = 0;
			indentflag = 0;
			mesindent = 0;
			doubletext = 0;
			MesWin_DrawWaku();
		}
	}
	return 0;
}


/* -------------------------------------------------------------------
  こまごまとしたの色々
------------------------------------------------------------------- */
PDTBUFFER* SYSTEM::MakePDT(char* f)
{
	PDTFILE* pdtfile = 0;
	PDTBUFFER* pdtbuf = 0;
//	char file[256];

//	sprintf(file, ":%s:%s.PDT", ini.pdtdir, f);

//	pdtfile = new PDTFILE(file, this);
	pdtfile = new PDTFILE(f, this);
	if ( pdtfile ) {
		dprintf("      PDT_MakePDT : ");
		dprintf(f);
		dprintf(" X:%d Y:%d\n", pdtfile->GetSizeX(), pdtfile->GetSizeY());
		pdtbuf = new PDTBUFFER(pdtfile->GetSizeX(), pdtfile->GetSizeY(), 3, pdtfile->GetSizeX()*3, true);
		if ( pdtbuf ) pdtfile->CopyBuffer(pdtbuf);
		delete pdtfile;
	}
	return pdtbuf;
};


PDTFILE* SYSTEM::OpenPDT(char* f)
{
	PDTFILE* pdtfile = 0;
//	char file[256];

//	sprintf(file, ":%s:%s.PDT", ini.pdtdir, f);

//	pdtfile = new PDTFILE(file, this);
	pdtfile = new PDTFILE(f, this);
	return pdtfile;
};


void SYSTEM::SetMsgSpeed(int n)
{
	if ( n ) n += 8;
	ini.meswait = n;
}


int SYSTEM::GetMsgSpeed(void)
{
	int i = ini.meswait;
	if ( i ) i -= 8;
	return i;
}


void SYSTEM::SetFontSize(int x, int y)
{
	ini.fontx = x;
	ini.fonty = y;
	MesWin_Setup(MesWinStyle);
}


void SYSTEM::GetFontSize(int* x, int* y)
{
	*x = ini.fontx;
	*y = ini.fonty;
}


// CG達成率をパーセントで返す
int SYSTEM::GetCGPercentage(void)
{
	return cgm->GetPercentage();
}


// （CGモードファイル内に於ける）n番目のCGを見たかどうかチェックする
int SYSTEM::GetCGFlag(int n)
{
	return cgm->GetFlag(n);
}


// CGモードファイルにに登録されたCGの総数を返す
int SYSTEM::GetCGAllNum(void)
{
	return cgm->GetCGAllNum();
}


// 今までに見たCGの数を返す
int SYSTEM::GetCGNum(void)
{
	return cgm->GetCGNum();
}


// （CGモードファイル内に於ける）n番目のCGのファイル名
char* SYSTEM::GetCGName(int n)
{
	return cgm->GetCGName(n);
}


int SYSTEM::GetCGFlagNum(int n)
{
	return cgm->GetCGFlagNum(n);
}


void SYSTEM::GetNameString(int num, char* buf)
{
	if ( (num>=0)&&(num<26) )
		sprintf(buf, "%s", ini.name[num]);
}


void SYSTEM::SetNameString(int num, char* buf)
{
	if ( (num>=0)&&(num<26) )
		sprintf(ini.name[num], "%s", buf);
}


void SYSTEM::SetNovelModeFlag(int sw)
{
	ini.novelmode = sw;
	MesWin_Setup(MesWinStyle);
}


void SYSTEM::GetMesWinColor(int* flag, int* r, int* g, int* b)
{
	*flag = ini.wincolorflag;
	*r = ini.wincolor[0];
	*g = ini.wincolor[1];
	*b = ini.wincolor[2];
}


void SYSTEM::SetMesWinColor(int flag, int r, int g, int b)
{
	ini.wincolorflag = flag;
	ini.wincolor[0] = r;
	ini.wincolor[1] = g;
	ini.wincolor[2] = b;
	MesWin_Setup(MesWinStyle);
}


/* -------------------------------------------------------------------
  選択肢処理
------------------------------------------------------------------- */
void SYSTEM::Select_AddItem(char* buf, int flag, int col)
{
	int i;
	char* tmp;
	unsigned char a;

	selcurid++;
	if ( flag==-1 ) return;

	if ( selitemnum<MAXSELECT ) {
		for (i=0; *buf; i++) {
			a = *buf++;
			if ( (a==0x81)&&((unsigned char)(*buf)==0x96) ) {
				tmp = ini.name[(unsigned char)buf[2]-0x60];
				while(*tmp) { selitem[selitemnum][i++] = *tmp++; }
				buf += 3;
				i--;
			} else {
				selitem[selitemnum][i] = a;
			}
		}
		selitem[selitemnum][i] = 0;
//		strcpy(selitem[selitemnum], buf);
		selitemenable[selitemnum] = flag;
		selitemcolor[selitemnum] = col;
		selitemid[selitemnum] = selcurid;
		selitemnum++;
	}
	selitemflag = false;
};


void SYSTEM::Select_SubWinSetup(void)
{
	int len, i;

	if ( !ini.novelmode ) {
		MesWin_Hide();
	}
	oldmesx = ini.mesx;
	oldmesy = ini.mesy;
	oldwinx = ini.winx;
	oldwiny = ini.winy;
	len = ini.subwinmin;
	for ( i=0; i<selitemnum; i++ ) if ( len<strlen(selitem[i]) ) len = strlen(selitem[i]);
	ini.mesx = (len+1)/2;
	ini.mesy = selitemnum;
	ini.winx = ini.subwinx;
	ini.winy = ini.subwiny;
	subwinflag = 1;			// SubWindow時はサイズ計算方法が違う
	MesWin_Setup(MesWinStyle);
}


void SYSTEM::Select_SubWinClose(void)
{
	if ( !ini.novelmode ) {
		MesWin_Hide();
	}
	ini.mesx = oldmesx;
	ini.mesy = oldmesy;
	ini.winx = oldwinx;
	ini.winy = oldwiny;
	subwinflag = 0;
	MesWin_Setup(MesWinStyle);
}


int SYSTEM::Select_Novel(void)
{
	int i, j, sel, x, y, btn;
	int x1, y1;
	int chr, key;

	if ( selfinish ) { return Select_Finish(); }
	if ( !selitemnum ) return 0;
	if ( !selitemflag ) {
//		if ( !selnovelsetup ) {
//			selnovelsetup = 1;
			MesWin_Draw();
//		}
		if ( scnefct.cmd ) return -1;		// メッセージウィンドウの描画が終わるのを待つ
		MesWin_ClearMes();
		selitemflag = true;
		selitemold = 0;
		for (i=0; i<selitemnum; i++) {
			x = ini.winx;
			y = ini.winy + i*ini.fonty;
			for (j=0; j<strlen(selitem[i]); j+=2) {
				chr = (((unsigned char)selitem[i][j])<<8)|((unsigned char)(selitem[i][j+1]));
				MesWin_PutNovelChar(x, y, chr);
				x += ini.fontx*2;
			}
		}
		mouse->GetButton();

		mesicon = 0;
		mesiconbase = GetCurrentTimer()-ini.mesiconwait;
		MesWin_DrawNovelIcon(ini.winx-32, ini.winy, 2);
		KeySelect = 0;
	}

	sel = selitemold;
	x1 = ini.winx-32;
	y1 = ini.winy;

	mouse->GetState(&x, &y, &btn);
	if ( (LastMouseX!=x)||(LastMouseY!=y) ) {
		LastMouseX = x;
		LastMouseY = y;
		KeySelect = 0;
	}
	key = GetKeyInput();
	switch (key) {
		case 0x1e:
			sel = selitemold-1;
			while ( sel>=0 ) {
				if ( selitemenable[sel] ) break;
				sel--;
			}
			if ( sel==(-1) ) sel = selitemold;
			KeySelect = 1;
			break;
		case 0x1f:
			do {
				sel++;
				if ( sel>=selitemnum ) { sel = selitemold; break; }
				if ( selitemenable[sel] ) break;
			} while	( sel<selitemnum );
			KeySelect = 1;
			break;
	}
	if ( !KeySelect ) {
		if ( (y>=ini.winy)&&(y<(ini.fonty*selitemnum+ini.winy)) ) {
			sel = (y-ini.winy)/ini.fonty;
			if ( !selitemenable[sel] ) sel = selitemold;
		}
	}
	if ( sel!=selitemold ) {
		mgr->Copy(x1, selitemold*ini.fonty+y1, x1+31, selitemold*ini.fonty+y1+23, MESWINPDT, x1, selitemold*ini.fonty+y1, 0, 0);
		selitemold = sel;
		mesicon = 0;
		mesiconbase = GetCurrentTimer()-ini.mesiconwait;
		PlaySE(0);						// セレクト時、SE.000 が鳴るらしい
	}
	if ( !btn ) {
		selfinish = (sel+1);
		selfinishcount = 0;
		selfinishbase = GetCurrentTimer();
		selitemnum = 0;
		selitemold =0;
		selitemflag = false;
		PlaySE(1);						// 決定時、SE.001 が鳴るらしい
	}
	MesWin_DrawNovelIcon(x1, selitemold*ini.fonty+y1, 2);
	return -1;
}


int SYSTEM::Select(void)
{
	int i, sel, x, y, btn;
	int x1, x2, ypad;
	RGBColor col, colbk;
	Rect r;
	FontInfo info;
	int key;

	if ( ini.novelmode ) return Select_Novel();

	if ( selfinish ) { return Select_Finish(); }
	if ( !selitemnum ) return 0;
	if ( !selitemflag ) {
		MesWin_Draw();
		selitemflag = true;
		selitemold = -1;
		dprintf("MesWin MesWinX1:%d MesWinY1:%d MesWinX2:%d MesWinY2:%d\n", MesWinX1, MesWinY1, MesWinX2, MesWinY2);
		LockPDT(0);

		TextFont(fontid);
		TextSize(ini.fontsize);
		GetForeColor(&colbk);
		GetFontInfo(&info);
		for (i=0; i<selitemnum; i++) {
			if ( i>=ini.mesy ) {
				x = MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2+ini.fontx)/2;
				x += ((MesWinX2-MesWinX1)>>1);
				y = MesWinY1 + (i%ini.mesy)*ini.fonty + ini.fontsize+(info.leading)/3 + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2;
			} else {
				x = MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2+ini.fontx)/2;
//				y = (i+1)*ini.fonty+MesWinY1+(ini.fontsize-ini.fonty)/2 + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2 -2;
				y = MesWinY1 + i*ini.fonty + ini.fontsize+(info.leading)/3 + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2;
			}
			col.red   = 0;
			col.green = 0;
			col.blue  = 0;
			RGBForeColor(&col);
			MoveTo(x+1, y+1);
			DrawText(selitem[i], 0, strlen(selitem[i]));
			if ( selitemenable[i] ) {
				col.red   = ini.colortable[ini.fontcolor][0]<<8;
				col.green = ini.colortable[ini.fontcolor][1]<<8;
				col.blue  = ini.colortable[ini.fontcolor][2]<<8;
			} else {
				col.red   = ini.colortable[selitemcolor[i]][0]<<8;
				col.green = ini.colortable[selitemcolor[i]][1]<<8;
				col.blue  = ini.colortable[selitemcolor[i]][2]<<8;
			}
			RGBForeColor(&col);
			MoveTo(x, y);
			DrawText(selitem[i], 0, strlen(selitem[i]));
		}
		RGBForeColor(&colbk);
		UnlockPDT(0, MesWinX1, MesWinY1, MesWinX2, MesWinY2, 1);

		mouse->GetButton();
		KeySelect = 0;
	}

	mouse->GetState(&x, &y, &btn);
	if ( (LastMouseX!=x)||(LastMouseY!=y) ) {
		LastMouseX = x;
		LastMouseY = y;
		KeySelect = 0;
	}

	x1 = MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2)/2;
	x2 = x1 + ini.mesx*2*ini.fontx - (ini.fontsize-ini.fontx*2)/2;
	ypad = MesWinY1+(ini.fontsize-ini.fonty)/2 + 4 + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2;

	sel = -1;

	if ( KeySelect ) {
		sel = selitemold;
	}
	key = GetKeyInput();
	switch (key) {
		case 0x1e:
			sel = selitemold;
			if ( selitemold ) {
				if ( selitemold==(-1) )
					sel = selitemnum-1;
				else
					sel = selitemold-1;
				while ( sel>=0 ) {
					if ( selitemenable[sel] ) break;
					sel--;
				}
			}
			KeySelect = 1;
			break;
		case 0x1f:
			sel = selitemold;
			if ( (sel+1)<selitemnum ) {
				do {
					sel++;
					if ( sel>=selitemnum ) { sel = -1; break; }
					if ( selitemenable[sel] ) break;
				} while	( sel<selitemnum );
			}
			KeySelect = 1;
			break;
	}
	if ( !KeySelect ) {
		if ( (x>=x1)&&(x<x2)&&(y>=ypad)&&(y<(ini.fonty*selitemnum+ypad))&&(y<(ini.fonty*ini.mesy+ypad)) ) {
			sel = (y-ypad)/ini.fonty;
			if ( (selitemnum>ini.mesy)&&(x>=(((MesWinX2-MesWinX1)>>1)+x1)) ) {	// 二列且つ右側
				sel += ini.mesy;
			}
			if ( !selitemenable[sel] ) sel = -1;
		}
	}

	if ( sel!=selitemold ) {
		LockPDT(0);
		if ( selitemold!=(-1) ) {
			if ( selitemnum>ini.mesy ) {	// 二列
				i = selitemold%ini.mesy;
				x1 = MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2)/2;
				x2 = x1 + ini.mesx*2*ini.fontx - (ini.fontsize-ini.fontx*2)/2;
				if ( selitemold >=ini.mesy) {
					x1 += ((MesWinX2-MesWinX1)>>1);
				} else {
					x2 -= ((MesWinX2-MesWinX1)>>1)-1;
				}
				SetRect(&r, x1, i*ini.fonty+ypad, x2, (i+1)*ini.fonty+ypad);
				InvertRect(&r);
			} else {
				SetRect(&r, x1, selitemold*ini.fonty+ypad, x2, (selitemold+1)*ini.fonty+ypad);
				InvertRect(&r);
			}
		}
		if ( sel!=(-1) ) {
			if ( selitemnum>ini.mesy ) {	// 二列
				i = sel%ini.mesy;
				x1 = MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2)/2;
				x2 = x1 + ini.mesx*2*ini.fontx - (ini.fontsize-ini.fontx*2)/2;
				if ( sel >=ini.mesy) {
					x1 += ((MesWinX2-MesWinX1)>>1);
				} else {
					x2 -= ((MesWinX2-MesWinX1)>>1)-1;
				}
				SetRect(&r, x1, i*ini.fonty+ypad, x2, (i+1)*ini.fonty+ypad);
				InvertRect(&r);
			} else {
				SetRect(&r, x1, sel*ini.fonty+ypad, x2, (sel+1)*ini.fonty+ypad);
				InvertRect(&r);
			}
			PlaySE(0);					// セレクト時、SE.000 が鳴るらしい
		}
		UnlockPDT(0, MesWinX1, MesWinY1, MesWinX2, MesWinY2, 1);
		selitemold = sel;
	}
	if ( (!btn)&&(selitemold!=(-1)) ) {
		selfinish = (selitemold+1);
		selfinishcount = 0;
		selfinishbase = GetCurrentTimer();
		selitemold =0;
		selitemflag = false;
		PlaySE(1);						// 決定時、SE.001 が鳴るらしい
	}

	return -1;
};


int SYSTEM::Select_Finish(void)
{
	Rect r;
	int x1, x2, ret=-1;
	int ypad, i;
	
	selcurid = 0;
	if ( (GetCurrentTimer()-selfinishbase)>=ini.selblinktime ) {
		selfinishbase = GetCurrentTimer();
		if ( ini.novelmode ) {
			x1 = ini.winx-32;
			x2 = (selfinish-1)*ini.fonty+ini.winy;
			if ( selfinishcount&1 ) {
				mesicon = 0;
				mesiconbase = GetCurrentTimer()-ini.mesiconwait;
				MesWin_DrawNovelIcon(x1, x2, 2);
			} else {
				mgr->Copy(x1, x2, x1+31, x2+23, MESWINPDT, x1, x2, 0, 0);
			}
		} else {
			LockPDT(0);
			ypad = MesWinY1+(ini.fontsize-ini.fonty)/2 + 4 + (MesWinY2-MesWinY1-ini.fonty*ini.mesy)/2;
			x1 = MesWinX1+(ini.fontsize-ini.fontx*2)/2 + (MesWinX2-MesWinX1-ini.fontx*ini.mesx*2)/2;
			x2 = x1 + ini.mesx*2*ini.fontx - (ini.fontsize-ini.fontx*2)/2;
			if ( selitemnum>ini.mesy ) {	// 二列
				i = (selfinish-1)%ini.mesy;
				if ( selfinish>ini.mesy ) {
					x1 += ((MesWinX2-MesWinX1)>>1);
				} else {
					x2 -= ((MesWinX2-MesWinX1)>>1)-1;
				}
			} else {
				i = selfinish-1;
			}
			SetRect(&r, x1, i*ini.fonty+ypad, x2, (i+1)*ini.fonty+ypad);
			InvertRect(&r);
			UnlockPDT(0, MesWinX1, MesWinY1, MesWinX2, MesWinY2, 1);
		}
		selfinishcount++;
		if ( selfinishcount>(ini.selblinkcount*2) ) {
			ret = selfinish;
			selfinish = 0;
			selitemnum = 0;
			memset(selitemenable, 0, sizeof(selitemenable));
		}
	}

	if ( ret>0 )
		return selitemid[ret-1];
	else
		return ret;
};


/* -------------------------------------------------------------------
  画面揺れ処理
------------------------------------------------------------------- */
void SYSTEM::ScreenShakeSetup(int n)
{
	shakepattern = n;
	shakecount = 0;
	shakebase = GetCurrentTimer();
	shaketime = 0;
};


bool SYSTEM::ScreenShake(void)
{
	int sx1, sy1, sx2, sy2;
	int dx1, dy1, dx2, dy2;
	int x,y;
	Rect src, dst;
	if ( (GetCurrentTimer()-shakebase)>shaketime ) {
		if ( shakecount>=ini.shakecount[shakepattern] ) {
			UpdateScreen();
			return true;
		}
		shakebase = GetCurrentTimer();
		shaketime = ini.shake[shakepattern][shakecount][2];
		sx1 = sy1 = dx1 = dy1 = 0;
		sx2 = dx2 = 640;
		sy2 = dy2 = 480;
		x = ini.shake[shakepattern][shakecount][0];
		y = ini.shake[shakepattern][shakecount][1];
		if ( x<0 ) {
			sx1 -= x;
			dx2 += x;
		} else {
			sx2 -= x;
			dx1 += x;
		}
		if ( y<0 ) {
			sy1 -= y;
			dy2 += y;
		} else {
			sy2 -= y;
			dy1 += y;
		}
		SetRect(&src, sx1, sy1, sx2, sy2);
		SetRect(&dst, dx1, dy1, dx2, dy2);
		GetGWorld(&gw_backup, &gd_backup);
		LockPixels(pixmap);
		SetGWorld(offscreen, NULL);
//		pdt[0]->SetBuffer((unsigned char*)GetPixBaseAddr(pixmap));
		CopyBits((BitMap*)*pixmap, &(hWnd->portBits), &src, &dst, srcCopy, hWnd->visRgn);
		UnlockPixels(pixmap);
		SetGWorld(gw_backup, gd_backup);
		shakecount++;
	}
	return false;
};


/* -------------------------------------------------------------------
  アニメーション処理
------------------------------------------------------------------- */
inline int GetInt(unsigned char* buf)
{
	int ret;
	ret  =  (int)buf[0];
	ret |= ((int)buf[1])<<8;
	ret |= ((int)buf[2])<<16;
	ret |= ((int)buf[3])<<24;
	return ret;
};


int SYSTEM::GetAnmCell(int seen, int stream, int frame) {
	int pos;
	pos = GetInt( anmbuf + seen*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + stream*4 +8 );
	return GetInt( anmbuf + maxanmframe*0x60 + pos*0x68 + 0xb8 + frame*4 + 8);
};


bool SYSTEM::AnimationSetup(char* f, int n)
{
	char* head = "ANM32";
	int size, i;
	char file[256];

	if ( anmbuf ) {
		delete[] anmbuf;
		anmbuf = 0;
	}

	sprintf(file, "%s.ANM", f);
	anmbuf = ReadFile(file, "ANM", &size);

	if ( !anmbuf ) {
		maxanmstream = 0;
		maxanmframe = 0;
		return false;
	}

	i = 0;
	while ( i<5 ) {
		if ( anmbuf[i]!=head[i] ) {
			delete[] anmbuf;
			anmbuf = 0;
			return false;
		}
		i++;
	}
	if ( GetInt(anmbuf+0x06)!=0x1000000 ) {
		delete[] anmbuf;
		anmbuf = 0;
		return false;
	}
	maxanmframe = GetInt(anmbuf+0x8c);
	maxanmstream = GetInt(anmbuf+0x90);
	maxanmseen = GetInt(anmbuf+0x94);
	for (i=0; i<maxanmframe; i++) {
		anmcell[i].sx1  = GetInt(anmbuf+0xb8+(i*0x60));			// 0xb8:Base 0x60:1cellあたりのサイズ
		anmcell[i].sy1  = GetInt(anmbuf+0xb8+(i*0x60)+0x04);
		anmcell[i].sx2  = GetInt(anmbuf+0xb8+(i*0x60)+0x08);
		anmcell[i].sy2  = GetInt(anmbuf+0xb8+(i*0x60)+0x0c);
		anmcell[i].dx   = GetInt(anmbuf+0xb8+(i*0x60)+0x10);
		anmcell[i].dy   = GetInt(anmbuf+0xb8+(i*0x60)+0x14);
		anmcell[i].wait = GetInt(anmbuf+0xb8+(i*0x60)+0x38);
	}

	curanmseen = n;
	curanmstream = 0;
	curanmframe = 0;
	anmwait = 0;
	prevanmtime = GetCurrentTimer();
	endanmstream = GetInt( anmbuf + curanmseen*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + 4 );
	i = GetInt( anmbuf + curanmseen*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + 8 );
	endanmframe = GetInt( anmbuf + maxanmframe*0x60 + i*0x68 + 0xb8 + 4);


	mgr->LoadFile((char*)(anmbuf+0x1c), ANMPDT);
	return true;
}


bool SYSTEM::AnimationExec(void)
{
	int anm, i;

	if ( !anmbuf ) return true;

	if ( (GetCurrentTimer()-prevanmtime)<anmwait ) return false;
	prevanmtime += anmwait;

	if ( curanmframe>=endanmframe ) {
		curanmframe = 0;
		curanmstream++;
		if ( curanmstream>=endanmstream ) {
			delete[] anmbuf;
			anmbuf = 0;
			return true;			// Animation Finished.
		} else {
			i = GetInt( anmbuf + curanmseen*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + curanmstream*4 + 8 );
			endanmframe = GetInt( anmbuf + maxanmframe*0x60 + i*0x68 + 0xb8 + 4);
		}
	}

	anm = GetAnmCell(curanmseen, curanmstream, curanmframe);

	if ( anm<maxanmframe ) {
		anmwait = anmcell[anm].wait;
		mgr->Copy(anmcell[anm].sx1, anmcell[anm].sy1, anmcell[anm].sx2, anmcell[anm].sy2, ANMPDT, anmcell[anm].dx, anmcell[anm].dy, 0, 0);
		curanmframe++;
	}
	return false;
};


void SYSTEM::MultiAnimationSetup(char* f, int n)
{
	char* head = "ANM32";
	int size, i;
	char file[256];

	for (i=0; i<multianmnum; i++) {
		if ( multianmseen[i]==n ) return;
	}

	if ( strcmp(curanmname, f) ) {
		if ( anmbuf ) {
			delete[] anmbuf;
			anmbuf = 0;
		}
	}

	if ( !anmbuf ) {
		sprintf(file, "%s.ANM", f);
		anmbuf = ReadFile(file, "ANM", &size);

		if ( anmbuf ) {
			maxanmframe = GetInt(anmbuf+0x8c);
			maxanmstream = GetInt(anmbuf+0x90);
			maxanmseen = GetInt(anmbuf+0x94);
			for (i=0; i<maxanmframe; i++) {
				anmcell[i].sx1  = GetInt(anmbuf+0xb8+(i*0x60));			// 0xb8:Base 0x60:1cellあたりのサイズ
				anmcell[i].sy1  = GetInt(anmbuf+0xb8+(i*0x60)+0x04);
				anmcell[i].sx2  = GetInt(anmbuf+0xb8+(i*0x60)+0x08);
				anmcell[i].sy2  = GetInt(anmbuf+0xb8+(i*0x60)+0x0c);
				anmcell[i].dx   = GetInt(anmbuf+0xb8+(i*0x60)+0x10);
				anmcell[i].dy   = GetInt(anmbuf+0xb8+(i*0x60)+0x14);
				anmcell[i].wait = GetInt(anmbuf+0xb8+(i*0x60)+0x38);
			}
			prevanmtime = GetCurrentTimer();
			mgr->LoadFile((char*)(anmbuf+0x1c), ANMPDT);
			multianmnum = 0;
			strcpy(curanmname, f);
		}
	}

	if ( (multianmnum<64)&&(anmbuf) ) {
		multianmseen[multianmnum] = n;
		multianmstream[multianmnum] = 0;
		multianmframe[multianmnum] = 0;
		multiendanmstream[multianmnum] = GetInt( anmbuf + multianmseen[multianmnum]*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + 4 );
		i = GetInt( anmbuf + multianmseen[multianmnum]*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + 8 );
		multiendanmframe[multianmnum] = GetInt( anmbuf + maxanmframe*0x60 + i*0x68 + 0xb8 + 4);
		multianmprevtime[multianmnum] = GetCurrentTimer();
		multianmwait[multianmnum] = 0;
		multianmnum++;

		anmwait = 0;
	}
};


void SYSTEM::MultiAnimationExec(void)
{
	int anm, i, j;

	if ( !anmbuf ) return;
	if ( !multianmnum ) return;

	for (j=0; j<multianmnum; j++) {
		if ( (GetCurrentTimer()-multianmprevtime[j])<multianmwait[j] ) return;
		multianmprevtime[j] = GetCurrentTimer();
		if ( multianmframe[j]>=multiendanmframe[j] ) {
			multianmframe[j] = 0;
			multianmstream[j]++;
			if ( multianmstream[j]>=multiendanmstream[j] ) {
				multianmframe[j] = 0;
				multianmstream[j] = 0;
			}
			i = GetInt( anmbuf + multianmseen[j]*0x78 + maxanmframe*0x60 + maxanmstream*0x68 + 0xb8 + multianmstream[j]*4 + 8 );
			multiendanmframe[j] = GetInt( anmbuf + maxanmframe*0x60 + i*0x68 + 0xb8 + 4);
		}
		anm = GetAnmCell(multianmseen[j], multianmstream[j], multianmframe[j]);
		if ( anm<maxanmframe ) {
			multianmwait[j] = anmcell[anm].wait;
			mgr->Copy(anmcell[anm].sx1, anmcell[anm].sy1, anmcell[anm].sx2, anmcell[anm].sy2, ANMPDT, anmcell[anm].dx, anmcell[anm].dy, 0, 0);
			multianmframe[j]++;
		}
	}
};


void SYSTEM::MultiAnimationClear(void)
{
	if ( anmbuf ) {
		delete[] anmbuf;
		curanmname[0] = 0;
	}
	anmbuf = 0;
	multianmnum = 0;
};


void SYSTEM::MultiAnimationStop(char* /*f*/, int n)
{
	int i, j;
	for (j=0; j<multianmnum; j++) {
		if ( multianmseen[j]==n ) {
			for (i=j+1; i<multianmnum; i++) {
				multianmseen[i-1] = multianmseen[i];
				multianmstream[i-1] = multianmstream[i];
				multianmframe[i-1] = multianmframe[i];
				multiendanmstream[i-1] = multiendanmstream[i];
				multiendanmframe[i-1] = multiendanmframe[i];
			}
			multianmnum--;
		}
	}
};


/* -------------------------------------------------------------------
  SAVE.INIロード
------------------------------------------------------------------- */

int SYSTEM::LoadInt(unsigned char* buf, int pos) {
	int ret;
	ret  = (buf[pos  ]);
	ret += (buf[pos+1]<<8);
	ret += (buf[pos+2]<<16);
	ret += (buf[pos+3]<<24);
	return ret;
}


void SYSTEM::LoadStr(unsigned char* src, int pos, char* dst, int len) {
	memcpy(dst, &src[pos], len);
	dst[len-1] = 0;
}


void SYSTEM::LoadGlobalFlags(FLAGS* f)
{
	FILE* fp;
	long i, j, header;
	unsigned char c, bit, *buf;
	char temp[256];

	if ( version>1713 )
		header   = 0x048c0;
	else
		header   = 0x01468;

	fp = fopen(ini.savefile, "rb");
	if ( !fp ) return;

	buf = new unsigned char[header];
	memset(buf, 0, header);

	fseek(fp, 0, 0);
	fread(buf, header, 1, fp);
	fclose(fp);
	memcpy(temp, buf, strlen(ini.saveheader));
	temp[strlen(ini.saveheader)] = 0;
	if ( strcmp(temp, ini.saveheader) ) {
		delete[] buf;
		return;
	}

	for ( i=0; i<1000; i++ ) {
		f->SetVal(i+1000, LoadInt(buf, 0x80+(i*4)));
	}
	for ( i=0; i<125; i++ ) {
		c = buf[0x1020+i];
		for (j=0, bit=0x80; j<8; j++, bit>>=1) {
			if ( c&bit )
				f->SetBit(i*8+j+1000, 1);
			else
				f->SetBit(i*8+j+1000, 0);
		}
	}
	MesWinStyle = LoadInt(buf, 0x11d0)+1;

	j = 0;
	for ( i=0; i<26; i++) {				// これまでのini.nameがセーブされていない
		if ( buf[i*16+0x11e4] ) j = 1;	// データ時の対処 ^^;
	}
	if ( j ) {
		for ( i=0; i<26; i++ ) {
			memcpy(ini.name[i], buf+i*16+0x11e4, 16);
		}
	}

	fclose(fp);
	delete[] buf;
};


int SYSTEM::Load(FLAGS* f, int n, int* seenptr, int* posptr)
{
	FILE* fp;
	long i, j, k;
	unsigned char c, bit;
	unsigned char *buf;
	MACROITEM m;
	int header, size, cdpos, stackpos, macropos, pad, ver;

	switch( version ) {
		case 1604:			// Suki Suki Daisuki!
			header   = 0x01454;
			size     = 0x1f400;
			cdpos    = 0x14426;
			stackpos = 0x1447a;		// ???
			macropos = 0x1585e;
			pad = 0;
			break;
		case 1613:			// Kanon
			header   = 0x01468;
			size     = 0x20000;
			cdpos    = 0x14426;
			stackpos = 0x1447a;		// ???
			macropos = 0x1587e;
			pad = 0;
			break;
		case 1704:			// Ribbon2
			header   = 0x01468;
			size     = 0x20000;
			cdpos    = 0x14426;
			stackpos = 0x1448a;		// ???
			macropos = 0x1587e;
			pad = 0;
			break;
		case 1714:			// AIR / Koigokoro
			header   = 0x048c0;
			size     = 0x253f8;
			cdpos    = 0x1444e;
			stackpos = 0x164be;
			macropos = 0x178c2;
			pad = 4;
			break;
		default:			// Kanon ALL / BabyFace / SenseOff
			header   = 0x01468;
			size     = 0x21fa0;
			cdpos    = 0x1444e;
			stackpos = 0x164be;
			macropos = 0x178c2;
			pad = 4;
			break;
	}

	SaveGlobalFlags(f);

	if ( (n<0)||(n>31) ) return 0;
	if ( !savedataflag[n] ) return 0;
	fp = fopen(ini.savefile, "rb");
	if ( !fp ) return 0;
	fseek(fp, header+size*n, 0);
	if ( ftell(fp)!=(header+size*n) ) {
		fclose(fp);
		return 0;
	}
	buf = new unsigned char[size];
	memset(buf, 0, size);
	fread(buf, size, 1, fp);
	fclose(fp);

	// タイトル
	LoadStr(buf, pad+0x14, loadingtitle, 32);

	ver = LoadInt(buf, size-4);		// Save Data Version (Mac original)

	// 変数とフラグ
	for ( i=0; i<1000; i++ ) {
		f->SetVal(i, LoadInt(buf, pad+0x10a38+(i*4)));
	}
	for ( i=0; i<125; i++ ) {
		c = buf[pad+0x12978+i];
		for (j=0, bit=0x80; j<8; j++, bit>>=1) {
			if ( c&bit )
				f->SetBit(i*8+j, 1);
			else
				f->SetBit(i*8+j, 0);
		}
	}
	for ( i=125; i<250; i++ ) {
		c = buf[pad+0x12978+i];
		for (j=0, bit=0x80; j<8; j++, bit>>=1) {
			if ( c&bit )
				f->SetBit(i*8+j, 1);
		}
	}
	for ( i=0; i<100; i++ ) {
		LoadStr(buf, pad+0x12a72+i*64, f->GetStr(i), 64);
	}

	// シーン番号とシーン内ポインタ
	*seenptr = LoadInt(buf, pad+0x14372);
	*posptr = LoadInt(buf, pad+0x14382);

	// 現在のBGM
	LoadStr(buf, cdpos, loadingbgm, 64);

	// スタックの内容
	f->ClearStack();
	j = LoadInt(buf, stackpos);
	dprintf("Load Stack - Num:%d\n", i);
	for (i=0 ; i<j; i++) {
		f->PushStack(LoadInt(buf, stackpos+ 4+i*20), LoadInt(buf, stackpos+20+i*20));
		dprintf("Load Stack - Seen:%d Pos:%d\n", LoadInt(buf, stackpos+ 4+i*20), LoadInt(buf, stackpos+20+i*20));
	}

	// 画面復旧用マクロ
	macro->ClearMacro();
	i = LoadInt(buf, macropos);
	for (j=0; j<i; j++) {
		m.cmd = LoadInt(buf, macropos+ 4+j*0x470);
		m.filenum = LoadInt(buf, macropos+ 8+j*0x470);
		LoadStr(buf, macropos+12+j*0x470, m.file, 512);
		for (k=0; k<90; k++) m.arg[k] = LoadInt(buf, macropos+524+j*0x470+k*4);
		macro->StackMacro(&m);
	}

	MesWinStyleForce = LoadInt(buf, 0x143aa);

/*
	if ( ver==1 ) {
		ini.wincolor[0]  = LoadInt(buf, 0x1438a);
		ini.wincolor[1]  = LoadInt(buf, 0x1438e);
		ini.wincolor[2]  = LoadInt(buf, 0x14392);
		ini.wincolorflag = LoadInt(buf, 0x14396);
		ini.winx         = LoadInt(buf, 0x143ae);
		ini.winy         = LoadInt(buf, 0x143b2);
		ini.subwinx      = LoadInt(buf, 0x143b6);
		ini.subwiny      = LoadInt(buf, 0x143ba);
		ini.novelmode    = 0;//LoadInt(buf, 0x14482);
		ini.mesx         = LoadInt(buf, 0x1448a);
		ini.mesy         = LoadInt(buf, 0x1448e);
		ini.fontx        = LoadInt(buf, 0x1449a);
		ini.fonty        = LoadInt(buf, 0x1449e);
		MesWin_Setup(MesWinStyle);
	}
*/

	delete[] buf;

	loadingcount = 0;
	loadingtime = GetCurrentTimer();

	fseek(fp, 0, 2);
	fclose(fp);

	return 1;
};


/* -------------------------------------------------------------------
  ロード直後の画面エフェクト
    最初は黒にフェードアウト（Effect#4）、
    画面が黒い間にマクロから画像を生成、
    最後に生成した画像をEffect#4でフェードインしながらスクリーンに表示
------------------------------------------------------------------- */

bool SYSTEM::LoadingProc(void)
{
	if ( !loadingcount ) mouse->StartPDTDraw();
	if ( loadingcount<16 ) {
		if ( (GetCurrentTimer()-loadingtime)>=50 ) {
			loadingtime = GetCurrentTimer();
			mgr->ScreenFade(1, loadingcount, 0, 0, 0);
			loadingcount++;
		}
	} else if ( loadingcount==16 ) {
		macro->RedrawPDT();
		mgr->AllCopy(0, BACKUPPDT, 0);
		mgr->FillRect(0, 0, 639, 479, 0, 0, 0, 0);
		SetWindowTitle(loadingtitle, 1);
		loadingcount++;
		loadefct.sx1 = 0; loadefct.sy1 = 0;
		loadefct.sx2 = 639; loadefct.sy2 = 479;
		loadefct.dx = 0; loadefct.dy = 0;
		loadefct.cmd = 4;
		loadefct.steptime = 50;
		loadefct.prevtime = GetCurrentTimer();
		loadefct.srcpdt = BACKUPPDT;
		loadefct.dstpdt = 0;
		loadefct.mask = 0;
		loadefct.step = 1;
		loadefct.curcount = 0;
	} else {
		mgr->Effect( &loadefct );
		if ( !loadefct.cmd ) {
			sound->CD_Play(loadingbgm, 1);
			mouse->FinishPDTDraw();
			return true;
		}
	}
	return false;
}


/* -------------------------------------------------------------------
  SAVE.INIセーブ
------------------------------------------------------------------- */

void SYSTEM::SaveInt(unsigned char* buf, int pos, int n) {
	buf[pos  ] = (unsigned char)((n    )&0xff);
	buf[pos+1] = (unsigned char)((n>>8 )&0xff);
	buf[pos+2] = (unsigned char)((n>>16)&0xff);
	buf[pos+3] = (unsigned char)((n>>24)&0xff);
}


void SYSTEM::SaveStr(unsigned char* dst, int pos, char* src, int len) {
	memcpy(&dst[pos], src, len);
	dst[pos+len-1] = 0;
}


void SYSTEM::SaveGlobalFlags(FLAGS* f)
{
	FILE* fp;
	long i, j, header;
	unsigned char c, bit, *buf;

	if ( version>1713 )
		header   = 0x048c0;
	else
		header   = 0x01468;

	buf = new unsigned char[header];
	memset(buf, 0, header);

	fp = fopen(ini.savefile, "rb");
	if ( fp ) {
		fseek(fp, header-1, 0);
		if ( ftell(fp)==(header-1) ) {
			fseek(fp, 0, 0);
			fread(buf, header, 1, fp);
		}
		fclose(fp);
	}

	SaveStr(buf, 0, ini.saveheader, 128);

	// 変数とフラグ
	for ( i=0; i<1000; i++ ) {
		SaveInt(buf, 0x80+(i*4), f->GetVal(i+1000));
	}
	for ( i=0; i<125; i++ ) {
		c = 0;
		for (j=0, bit=0x80; j<8; j++, bit>>=1) {
			if ( f->GetBit(i*8+j+1000) ) c |= bit;
		}
		buf[0x1020+i] = c;
	}
	for ( i=0; i<26; i++ ) {
		memcpy(buf+i*16+0x11e4, ini.name[i], 16);
	}

	SaveInt(buf, 0x11d0, MesWinStyle-1);

	fp = fopen(ini.savefile, "r+b");
	if ( !fp ) fp = fopen(ini.savefile, "wb");
	if ( fp ) {
		fseek(fp, 0, 0);
		fwrite(buf, header, 1, fp);
		fclose(fp);
	}

	delete[] buf;
};


void SYSTEM::Save(FLAGS* f, int n, int seen, int curpos)
{
	FILE* fp;
	long i, j, k;
	unsigned char c, bit, temp2[256];
	unsigned char *buf;
	char temp[256];
	int month, day, hour, min;
	MACROITEM* m;
	int header, size, cdpos, stackpos, macropos, pad;
	int sseen, spos;

	switch( version ) {
		case 1604:			// Suki Suki Daisuki!
			header   = 0x01454;
			size     = 0x1f400;
			cdpos    = 0x14426;
			stackpos = 0x1447a;		// ???
			macropos = 0x1585e;
			pad = 0;
			break;
		case 1613:			// Kanon
			header   = 0x01468;
			size     = 0x20000;
			cdpos    = 0x14426;
			stackpos = 0x1447a;
			macropos = 0x1587e;
			pad = 0;
			break;
		case 1704:			// Ribbon2
			header   = 0x01468;
			size     = 0x20000;
			cdpos    = 0x14426;
			stackpos = 0x1448a;		// ???
			macropos = 0x1587e;
			pad = 0;
			break;
		case 1714:			// AIR / Koigokoro
			header   = 0x048c0;
			size     = 0x253f8;
			cdpos    = 0x1444e;
			stackpos = 0x164be;
			macropos = 0x178c2;
			pad = 4;
			break;
		default:			// Kanon ALL / BabyFace / SenseOff
			header   = 0x01468;
			size     = 0x21fa0;
			cdpos    = 0x1444e;
			stackpos = 0x164be;
			macropos = 0x178c2;
			pad = 4;
			break;
	}

	SaveGlobalFlags(f);

	fp = fopen(ini.savefile, "r+b");
	if ( !fp ) {
		fp = fopen(ini.savefile, "wb");
		if ( !fp ) return;
	}

	buf = new unsigned char[size];
	memset(buf, 0, size);

	fseek(fp, header-1, 0);
	if ( ftell(fp)!=(header-1) ) {		// ヘッダすら無い時
		fseek(fp, 0, 0);
		SaveStr(buf, 0, ini.saveheader, 128);
		fwrite(buf, header, 1, fp);
	}
	for (i=1; i<=n; i++) {
		fseek(fp, size*i+header-1, 0);
		if ( ftell(fp)!=(size*i+header-1) ) {
			fseek(fp, size*(i-1)+header, 0);
			fwrite(buf, size, 1, fp);
		}
	}

	// 日付とタイトル
	savedatadate[n] = GetDateTime(1);
	savedatatime[n] = GetDateTime(2);
	strcpy(savedatatitle[n], curtitle);
	month = GetDateTime(1)/100;
	day   = GetDateTime(1)%100;
	hour  = GetDateTime(2)/100;
	min   = GetDateTime(2)%100;
	SaveInt(buf, 0x00, 1);	// Always 1 ?
	SaveInt(buf, 0x04, GetDateTime(3));		// Year
	SaveInt(buf, pad+0x04, month);	// Month
	SaveInt(buf, pad+0x08, day  );	// Day
	SaveInt(buf, pad+0x0c, hour );	// Hour
	SaveInt(buf, pad+0x10, min  );	// Minute
	if ( curtitle[0] )
		SaveStr(buf, pad+0x14, curtitle, 32);
	else
		SaveStr(buf, pad+0x14, "No Title", 32);

	// 変数とフラグ
	for ( i=0; i<2000; i++ ) {
		SaveInt(buf, pad+0x10a38+(i*4), f->GetVal(i));
	}
	for ( i=0; i<250; i++ ) {
		c = 0;
		for (j=0, bit=0x80; j<8; j++, bit>>=1) {
			if ( f->GetBit(i*8+j) ) c |= bit;
		}
		buf[pad+0x12978+i] = c;
	}
	for ( i=0; i<100; i++ ) {
		SaveStr(buf, pad+0x12a72+i*64, f->GetStr(i), 64);
	}

	// シーン番号とシーン内ポインタ
	SaveInt(buf, pad+0x14372, seen);
	SaveInt(buf, pad+0x14382, curpos);

	SaveStr(buf, cdpos, sound->GetCurrentBGM(), 64);

	// スタックの内容
	SaveInt(buf, stackpos, f->GetSavedStackNum());
	dprintf("Save Stack - Num:%d\n", f->GetSavedStackNum());
	for (i=0; i<f->GetSavedStackNum(); i++) {
		f->GetSavedStackItem(i, &sseen, &spos);
		SaveInt(buf, stackpos+ 4+i*20, sseen);
		SaveInt(buf, stackpos+20+i*20, spos);
		dprintf("Save Stack - Seen:%d Pos:%d\n", sseen, spos);
	}

	// 画面復旧用マクロ
	i = macro->GetMacroNum();
	SaveInt(buf, macropos, i);
	for (j=0; j<i; j++) {
		m = macro->GetMacro(j);
		SaveInt(buf, macropos+ 4+j*0x470, m->cmd);
		SaveInt(buf, macropos+ 8+j*0x470, m->filenum);
		SaveStr(buf, macropos+12+j*0x470, m->file, 512);
		for (k=0; k<90; k++) SaveInt(buf, macropos+524+j*0x470+k*4, m->arg[k]);
	}

	SaveInt(buf, 0x143aa, MesWinStyleForce);

/*
	// Save Data Version (Mac original)
	SaveInt(buf, size-4, 1);
	// Version 1 additional data
	if ( version>=1713 ) {
		SaveInt(buf, 0x1438a, ini.wincolor[0]);
		SaveInt(buf, 0x1438e, ini.wincolor[1]);
		SaveInt(buf, 0x14392, ini.wincolor[2]);
		SaveInt(buf, 0x14396, ini.wincolorflag);
		SaveInt(buf, 0x143ae, ini.winx);
		SaveInt(buf, 0x143b2, ini.winy);
		SaveInt(buf, 0x143b6, ini.subwinx);
		SaveInt(buf, 0x143ba, ini.subwiny);
		SaveInt(buf, 0x14482, ini.novelmode);
		SaveInt(buf, 0x1448a, ini.mesx);
		SaveInt(buf, 0x1448e, ini.mesy);
		SaveInt(buf, 0x1449a, ini.fontx);
		SaveInt(buf, 0x1449e, ini.fonty);
	}
*/

	fseek(fp, size*n+header, 0);		// 序盤でファイルサイズを調整しておいたから、シークできるはず
	fwrite(buf, size, 1, fp);
	fclose(fp);

	delete[] buf;

	if ( curtitle[0] )
		sprintf(temp, " %02d/%02d(%02d:%02d) %s", month, day, hour, min, curtitle);
	else
		sprintf(temp, " %02d/%02d(%02d:%02d) No Title", month, day, hour, min);
	ConvertString(temp2, temp);
	for (j=255; j>0; j--) {
		temp2[j] = temp2[j-1];
	}
	temp2[1] = 0;
	temp2[0]++;
	SetMenuItemText(menus[0], n+1, temp2);
	SetMenuItemText(menus[1], n+1, temp2);
	EnableMenuItem(menus[1], n+1);
	savedataflag[n] = 1;
};


int SYSTEM::CheckSaveData(int num) {
	if ( (num<0)||(num>31) ) return 0;
	return savedataflag[num];
};


int SYSTEM::CheckSaveDataDate(int num) {
	if ( (num<0)||(num>31) ) return 0;
	return savedatadate[num];
};


int SYSTEM::CheckSaveDataTime(int num) {
	if ( (num<0)||(num>31) ) return 0;
	return savedatatime[num];
};


char* SYSTEM::CheckSaveDataTitle(int num) {
	if ( (num<0)||(num>31) ) return 0;
	return savedatatitle[num];
};


void SYSTEM::SetSaveDataTitle(int num, char* buf) {
	unsigned char temp[256];
	GetMenuItemText(menus[0], num+1, temp);
	memcpy(buf, temp+2, 63);
	if ( temp[0]<64 )
		buf[temp[0]-1] = 0;
	else
		buf[63] = 0;
};


int SYSTEM::CheckNovelSave(void)
{
	int ret = 0;
	if ( (!ini.novelmode)||(mesbufptr==mesbuf) ) ret = 1;
	return ret;
}


/* -------------------------------------------------------------------
  SE関連
------------------------------------------------------------------- */
// 基本的にはサウンドコマンドの$44で鳴らすのだけど、000～003番のSEは
// それ以外でも自動で鳴る模様。
//   0 : 選択肢セレクト変更時
//   1 : 選択肢決定時
//   2 : メッセージウィンドウが開いた時？
//   3 : メッセージウィンドウの改ページが発生した時？

void SYSTEM::PlaySE(int n)
{
	if ( (n>=0)&&(n<MAX_SE)&&(sound) ) {
		if ( ini.se[n][0] ) sound->Sound_Play(ini.se[n], 0);
	}
};

