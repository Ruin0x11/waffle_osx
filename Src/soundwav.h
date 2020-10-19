/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  soundwav.h
     WAVサウンド管理クラス
=======================================================================*/

#ifndef _soundwav_h
#define _soundwav_h

#include <QuickTimeComponents.h>
#include <Sound.h>

#define SNDBUFSIZE 44100		// 4で割り切れるよーにしといてね（1frame最大が16bit x stereoなので）


/************************************************************************
  class SOUNDWAV
************************************************************************/

// WAVデータ埋め要求コールバックに渡されるデータ
struct WAVINFO {
	int playflag;			// 再生中フラグ
	int playingflag;		// 再生中フラグ
	int loopflag;			// ループ再生
	int start;			// WAVデータ列の、ファイル先頭からのオフセット
	int end;			// データ終端位置
	int repeat;			// リピート時の巻き戻し位置
	int pos;			// 現在のファイルポイント
	int channel;			// チャネル数（1:mono 2:stereo)
	int bits;			// データビット数（8 or 16）
	int linear;			//  線形圧縮フラグ
	long backupedA5;		// A5のバックアップ
};


// ファイル読み込み終了コールバックに渡されるデータ（ParamBlockRecの拡張）
struct WAVFILEINFO {
	ParamBlockRec pb;		// パラメータブロックレコード（PBReadAsyncで使用）
	SndDoubleBufferPtr buf;		// DoubleBufferの片っぽのポインタ
	int num;			// バッファ番号（0か1）
	int inuse;			// ディスク読み込み実行中フラグ
	WAVINFO* wav;			// WAVINFOへのポインタ
};


class SOUNDWAV
{
private:
	SndChannelPtr hsnd;
	SndDoubleBufferHeader2 head;
	WAVFILEINFO wfile[2];
	WAVINFO wav;
	IOCompletionUPP ioc;
	SndCallBackUPP sndcb;
	int linear;
public:
	SOUNDWAV(int l);
	‾SOUNDWAV(void);

	void Play(char* f, int loop, int cutsize);
	void Stop(void);
	void GetVolume(int* l, int* r);
	void SetVolume(int l, int r);
	int IsPlaying(void);
};



/************************************************************************
  class SOUNDKOE
 ***********************************************************************/
struct KOETABLE {
	short id;
	short symbol;
	unsigned int pos;
};


class SOUNDKOE
{
private:
	SndChannelPtr hsnd;
	ExtSoundHeader head;
	SndCallBackUPP sndcb;

	KOETABLE* table;
	int tableid;
	int maxnum;
	int rate;
	signed short* koebuf;
	int playflag;
	int koetype;
	int linear;

	void MakeTable(char* f, int tid);

	short table1[256];
	unsigned char table2[256];
//short table2[256];

public:
	SOUNDKOE(int type, int l);
	‾SOUNDKOE(void);

	void Play(char* f, int tid, int id);
	void Stop(void);
	int IsPlaying(void);
};

#endif
