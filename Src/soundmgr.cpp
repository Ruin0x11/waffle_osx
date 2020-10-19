/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  sound.cpp
    サウンド関連
=======================================================================*/

#include <stdio.h>
#include <string.h>

#include "soundmgr.h"
#include "soundwav.h"
#include "system.h"
#include "common.h"
#include "debug.h"

#define	CDCTRL_TRKADR	2
#define CDCTRL_STEREO	9
#define CDCTRL_READTOC	100
#define CDCTRL_READQ	101
#define	CDCTRL_PLAY		104
#define	CDCTRL_PAUSE	105
#define	CDCTRL_STOP		106
#define CDCTRL_STATUS	107
#define CDCTRL_SETVOL	109
#define CDCTRL_GETVOL	112

/************************************************************************
  class SOUND
    サウンド関連
************************************************************************/

SOUND::SOUND(SYSTEM* s, INIFILE* inifile)
{
	sys = s;
	ini = inifile;
	bgm = 0;
	wav = 0;
	koe = 0;

	KOE_SW = 1;

	if ( ini->musictype==2 ) {		// DSound
		bgm = new SOUNDWAV(ini->musiclinear);
		DS_GetVolume(&VolL, &VolR);
		CD_Stop();
	} else {						// CD-DA
		if ( OpenDriver("¥p.AppleCD", &cddev) ) {
			cddev = -1;
		} else {
//			VolL = CDDA_GetVolume();
			VolL = 127;
			CD_Stop();
		}
	}

	wav = new SOUNDWAV(ini->wavlinear);
	koe = new SOUNDKOE(ini->koetype, ini->koelinear);

	CD_LoopFlag = 0;
	WAV_LoopFlag = 0;
	CurTrack = -1;
	CD_FadeTime = 0;
	CD_FadeTimeBase = 0;
	WAV_FadeTime = 0;
	WAV_FadeTimeBase = 0;

	sys->ConvertCapital((unsigned char*)(ini->koedir));
};

SOUND::‾SOUND(void)
{
	CD_Stop();
	// 元の音量に復帰しておく
	if ( ini->musictype==2 ) {		// DSound
//		DS_SetVolume(VolL, VolR);
		if ( bgm ) delete bgm;
	} else {						// CD-DA
		CDDA_SetVolume(VolL);
	}
	if ( wav ) {
		wav->Stop();
		delete wav;
	}
	if ( koe ) {
		koe->Stop();
		delete koe;
	}
};


void SOUND::Reset(void)
{
	CD_Stop();
	Sound_Stop();
	KOE_Stop();
	CD_LoopFlag = 0;
	WAV_LoopFlag = 0;
	CurTrack = -1;
	CD_FadeTime = 0;
	CD_FadeTimeBase = 0;
	WAV_FadeTime = 0;
	WAV_FadeTimeBase = 0;
};


/* -------------------------------------------------------------------
  CD/DSound再生 公開I/F
------------------------------------------------------------------- */
void SOUND::CD_Play(char* trk, int loop)
{
dprintf("Play BGM   MusicType:%d  Loop:%d  Name:", ini->musictype, loop);
dprintf(trk);
dprintf("¥n");
	if ( ini->musictype==2 ) {		// DSound
		DS_Stop();
		DS_SetVolume(VolL, VolR);
		DS_Play(trk, loop);
	} else {						// CD-DA
		CDDA_Stop();
		CDDA_SetVolume(VolL);
		CDDA_Play(trk);
	}
	CD_LoopFlag = loop;
	CD_FadeTime = 0;
}


void SOUND::CD_Stop(void)
{
	if ( ini->musictype==2 ) {		// DSound
		DS_Stop();
	} else {						// CD-DA
		CDDA_Stop();
	}
	CD_LoopFlag = 0;
	CD_FadeTime = 0;
}


void SOUND::CD_FadeOut(int n)
{
	CD_FadeTime = n*40;
	CD_FadeTimeBase = sys->GetCurrentTimer();
}


void SOUND::CD_FadeIn(char* trk, int loop, int n)
{
	if ( ini->musictype==2 ) {		// DSound
		DS_Stop();
		DS_SetVolume(0, 0);
		DS_Play(trk, loop);
	} else {						// CD-DA
		CDDA_Stop();
		CDDA_SetVolume(0);
		CDDA_Play(trk);
	}
	CD_LoopFlag = loop;
	CD_FadeTime = (-n*40);
	CD_FadeTimeBase = sys->GetCurrentTimer();
}


void SOUND::CD_CheckPlay(void)
{
	if ( ini->musictype==2 ) {		// DSound
		DS_CheckPlay();
	} else {						// CD-DA
		CDDA_CheckPlay();
	}
}


void SOUND::CD_Rewind(void)
{
	char buf[64];
	strcpy(buf, CurBGMName);
	CD_Play(buf, CD_LoopFlag);
}


/* -------------------------------------------------------------------
  WAV再生
------------------------------------------------------------------- */
void SOUND::Sound_Play(char* f, int loop)
{
	char file[256];
	if ( wav ) {
		sprintf(file, ":%s:%s.WAV", ini->wavdir, f);
		sys->ConvertCapital((unsigned char*)file);
		wav->Play(file, loop, 0);
	}
}


void SOUND::Sound_Stop(void)
{
	if ( wav ) {
		wav->Stop();
	}
}


void SOUND::Sound_CheckPlay(void)
{
}


int SOUND::Sound_IsPlaying(void)
{
	int ret = 0;
	if ( wav ) ret = wav->IsPlaying();
	return ret;
}



/* -------------------------------------------------------------------
  CD-DA再生 (Internal routine)
------------------------------------------------------------------- */
void SOUND::CDDA_Play(char* trk)
{
	int i, n = -1;
	unsigned char params[10] = {0,CDCTRL_TRKADR,0,0,0,0/*TrackNo*/,0,0,0,CDCTRL_STEREO};

	strcpy(CurBGMName, trk);
	if ( cddev==-1 ) return;		// CDが無い

	for (i=0; i<100; i++) {			// INIファイルからトラックNoを得る
		if ( !strcmp(trk, ini->cd[i]) ) { n=i; break; }
	}
	CurTrack = n;
	if ( n == -1 ) return;

	params[5] = ((n/10)*16)+(n%10);	// BCD変換
	params[7] = 1;					// 終了トラック指定
	Control(cddev, CDCTRL_PLAY, &params);
	params[7] = 0;					// 開始トラック指定
	Control(cddev, CDCTRL_PLAY, &params);
}


void SOUND::CDDA_Stop(void)
{
	ParamBlockRec pb;

	CurBGMName[0] = 0;
	if ( cddev==-1 ) return;		// CDが無い

	memset(&pb, 0, sizeof(CntrlParam));
	pb.cntrlParam.ioCRefNum = cddev;
	pb.cntrlParam.csCode = CDCTRL_STOP;
	PBControl(&pb, false);
}


void SOUND::CDDA_CheckPlay(void)
{
	ParamBlockRec pb;
	unsigned char* st;
	int t, l;

	if ( cddev==-1 ) return;		// CDが無い

	if ( CD_FadeTime>0 ) {				// フェードアウト中
		t = 1000-(((sys->GetCurrentTimer()-CD_FadeTimeBase)*1000)/(CD_FadeTime*5));	// 気持ち長く取っておこう
		if ( (t<0)||(t>1000) ) {
			CD_Stop();
			CD_FadeTime = 0;
			return;
		}
		l = (VolL*t)/1000;
		CDDA_SetVolume(l);
	} else if ( CD_FadeTime<0 ) {		// 負の時はフェードイン
		t = (((sys->GetCurrentTimer()-CD_FadeTimeBase)*1000)/(-CD_FadeTime*5));
		if ( (t<0)||(t>1000) ) {
			t = 1000;
			CD_FadeTime = 0;
		}
		l = (VolL*t)/1000;
		CDDA_SetVolume(l);
	}

	if ( !CD_LoopFlag ) return;		// ループ演奏中ではない

	st = (unsigned char*)&pb.cntrlParam.csParam[0];
	memset(&pb, 0, sizeof(CntrlParam));
	pb.cntrlParam.ioCRefNum = cddev;
	pb.cntrlParam.csCode = CDCTRL_STATUS;
	if ( !PBControl(&pb, false) ) {
		if ( st[0]>2 ) {
			CDDA_Play(CurBGMName);
			return;
		}
	}
	memset(&pb, 0, sizeof(CntrlParam));
	pb.cntrlParam.ioCRefNum = cddev;
	pb.cntrlParam.csCode = CDCTRL_READQ;
	if ( !PBControl(&pb, false) ) {
		if ( st[1]!=((CurTrack/10)*16+(CurTrack%10)) ) {
			CDDA_Play(CurBGMName);
			return;
		}
	}
}


void SOUND::CDDA_SetVolume(int vol) {
	ParamBlockRec pb;
	unsigned char* st;
	st = (unsigned char*)&pb.cntrlParam.csParam[0];
	memset(&pb, 0, sizeof(CntrlParam));
	pb.cntrlParam.ioCRefNum = cddev;
	pb.cntrlParam.csCode = CDCTRL_SETVOL;
	st[0] = (unsigned char)(vol&0xff);
	st[1] = (unsigned char)(vol&0xff);
	PBControl(&pb, false);
}


int SOUND::CDDA_GetVolume(void) {
	ParamBlockRec pb;
	unsigned char* st;
	st = (unsigned char*)&pb.cntrlParam.csParam[0];
	memset(&pb, 0, sizeof(CntrlParam));
	pb.cntrlParam.ioCRefNum = cddev;
	pb.cntrlParam.csCode = CDCTRL_GETVOL;
	if ( !PBControl(&pb, false) )
		return ((int)st[0]);
	else
		return 0;
}


/* -------------------------------------------------------------------
  DSound再生 (Internal routine)
------------------------------------------------------------------- */
void SOUND::DS_Play(char* trk, int loop)
{
	char file[256];
	int i, num;
	int size = 0;

	strcpy(CurBGMName, trk);
	if ( !bgm ) return;

	num = -1;
	for (i=0; i<ini->dsnum; i++) {
		if ( !strcmp(trk, ini->dsound[i].name) ) {
			num = i;
			break;
		}
	}
	CurTrack = num;
	if ( num==-1 ) return;

	if ( (loop)&&(ini->dsound[i].cutsize>0) ) size = ini->dsound[i].cutsize;
	sprintf(file, ":%s:%s.WAV", ini->bgmdir, ini->dsound[num].file);
	sys->ConvertCapital((unsigned char*)file);
	bgm->Play(file, loop, size);

dprintf("  DS - File:");
dprintf(file);
dprintf("  Loop:%d  Size:%d¥n", loop, size);
}


void SOUND::DS_Stop(void)
{
	CurBGMName[0] = 0;
	if ( bgm ) {
		bgm->Stop();
	}
}


void SOUND::DS_CheckPlay(void)
{
	int t, l, r;
	if ( CD_FadeTime>0 ) {				// フェードアウト中
		t = 1000-(((sys->GetCurrentTimer()-CD_FadeTimeBase)*1000)/(CD_FadeTime));
		if ( (t<0)||(t>1000) ) {
			CD_Stop();
			CD_FadeTime = 0;
			return;
		}
		l = (VolL*t)/1000;
		r = (VolR*t)/1000;
		DS_SetVolume(l, r);
	} else if ( CD_FadeTime<0 ) {		// 負の時はフェードイン
		t = (((sys->GetCurrentTimer()-CD_FadeTimeBase)*1000)/(-CD_FadeTime));
		if ( (t<0)||(t>1000) ) {
			t = 1000;
			CD_FadeTime = 0;
		}
		l = (VolL*t)/1000;
		r = (VolR*t)/1000;
		DS_SetVolume(l, r);
	}
	// DSはループチェックはいらない
}


void SOUND::DS_SetVolume(int l, int r) {
	if ( bgm ) {
		bgm->SetVolume(l, r);
	}
}


void SOUND::DS_GetVolume(int* l, int* r) {
	if ( bgm ) {
		bgm->GetVolume(l, r);
	}
}



/* -------------------------------------------------------------------
  KOE再生 公開I/F
------------------------------------------------------------------- */
void SOUND::KOE_Play(int id)
{
	int tid, num;

	if ( (!koe)||(!KOE_SW) ) return;
	if ( sys->Version()>=1714 ) {
		// 恋ごころ／継母調教とか。17D'系かな？
		tid = id/100000;
		num = id%100000;
	} else {
		tid = id/10000;
		num = id%10000;
	}
	koe->Play(ini->koedir, tid, num);
}


void SOUND::KOE_Stop(void)
{
	if ( koe ) koe->Stop();
}


int SOUND::KOE_IsPlaying(void)
{
	int ret = 0;
	if ( koe ) ret = koe->IsPlaying();
	return ret;
}


void SOUND::KOE_Disable(int sw)
{
	if ( sw ) KOE_SW = 0; else KOE_SW = 1;
}
