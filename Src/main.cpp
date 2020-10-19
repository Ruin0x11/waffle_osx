/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  main.cpp
    ���C���֐��E�C�x���g�����Ȃ�
=======================================================================*/

#include "system.h"
#include "debug.h"
#include "common.h"

static WindowPtr hWnd;
static SYSTEM* sys;

//--------------------------------------------------------------------
// Initialize()
//   �ь炨�񑩂̏�����
//--------------------------------------------------------------------

static void Initialize(void)
{
	int i;

	MaxApplZone();	

	for (i=0; i<16; i++) {		// ���񂭂炢���΂���������i�΁j
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
//   �܁A���񑩂���
//--------------------------------------------------------------------

void main()
{
	hWnd = NULL;
	Initialize();

	DebugStart();

	// Window�쐬
	hWnd = GetNewCWindow(128, NULL, (WindowPtr)-1);

	// �V�X�e���I�u�W�F�N�g�쐬�Ǝ��s
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
		// �E�B���h�E�ƃI�u�W�F�N�g���폜���ďI��
		DisposeWindow(hWnd);
	} else {
		Error("Could not make the window.");
	}

	DebugEnd();
}
