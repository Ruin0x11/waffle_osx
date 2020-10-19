/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  soundmgr.h
     サウンド管理クラス
=======================================================================*/

#ifndef _avg32sound_h
#define _avg32sound_h

class SYSTEM;
class SOUNDWAV;
class SOUNDKOE;
struct INIFILE;

class SOUND
{
private:
	SYSTEM* sys;
	INIFILE* ini;
	SOUNDWAV* bgm;
	SOUNDWAV* wav;
	SOUNDKOE* koe;
	int CD_LoopFlag;
	int WAV_LoopFlag;
	int VolL, VolR;
	int CD_FadeTime;
	int CD_FadeTimeBase;
	int WAV_FadeTime;
	int WAV_FadeTimeBase;
	short cddev;
	char CurBGMName[64];
	int CurTrack;
	int KOE_SW;

	void CDDA_Play(char* trk);
	void CDDA_Stop(void);
	void CDDA_CheckPlay(void);
	void CDDA_SetVolume(int n);
	int CDDA_GetVolume(void);

	void DS_Play(char* trk, int loop);
	void DS_Stop(void);
	void DS_CheckPlay(void);
	void DS_SetVolume(int l, int r);
	void DS_GetVolume(int* l, int* r);

public:
	SOUND(SYSTEM* sys, INIFILE* ini);
	‾SOUND(void);
	void Reset(void);

	void CD_Play(char* trk, int loop);
	void CD_Stop(void);
	void CD_FadeOut(int n);
	void CD_FadeIn(char* trk, int loop, int n);
	void CD_Rewind(void);
	void CD_CheckPlay(void);

	void Sound_Play(char* f, int loop);
	void Sound_Stop(void);
	void Sound_CheckPlay(void);
	int Sound_IsPlaying(void);

	void KOE_Play(int id);
	void KOE_Stop(void);
	int KOE_IsPlaying(void);
	void KOE_Disable(int sw);

	char* GetCurrentBGM(void) { return CurBGMName; };
};

#endif
