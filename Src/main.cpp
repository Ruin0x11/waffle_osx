/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  main.cpp
    メイン関数・イベント処理など
=======================================================================*/

#include "system.h"
#include "debug.h"
#include "common.h"

static WindowPtr hWnd;
static SYSTEM* sys;

//--------------------------------------------------------------------
// Initialize()
//   林檎お約束の初期化
//--------------------------------------------------------------------

static void Initialize(void)
{
	int i;

	MaxApplZone();	

	for (i=0; i<16; i++) {		// こんくらいやればいいっしょ（笑）
		MoreMasters();
	}

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	FlushEvents(everyEvent, 0);
	InitCursor();
}


//--------------------------------------------------------------------
// main()
//   ま、お約束っす
//--------------------------------------------------------------------

void main()
{
	hWnd = NULL;
	Initialize();

	DebugStart();

	// Window作成
	hWnd = GetNewCWindow(128, NULL, (WindowPtr)-1);

	// システムオブジェクト作成と実行
	if (hWnd) {
		SizeWindow(hWnd, 640, 480, true);
		ShowWindow(hWnd);
		sys = SYSTEM::Create(hWnd);
		if ( sys ) {
			sys->Init();
			sys->Reset();
			sys->EventLoop();
			delete sys;
		} else {
			Error("Could not make SYSTEM object.");
		}
		// ウィンドウとオブジェクトを削除して終了
		DisposeWindow(hWnd);
	} else {
		Error("Could not make the window.");
	}

	DebugEnd();
}
