/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  soundwav.cpp
    サウンド関連
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fp.h>

#include "soundwav.h"
#include "common.h"
#include "debug.h"


static int LinearTableFlag = 0;
static short LinearTable[256];


inline bool CheckHeader(unsigned char* b, char* header)
{
	int i = 0;
	while(header[i]) {
		if ( (char)(b[i]) != header[i] ) return false;
		i++;
	}
	return true;
};


// 偽装8bitデータから16bitPCMへの変換テーブル
inline void SetupLinearTable(void)
{
	short i, data;
	if ( !LinearTableFlag ) {
		for (i=-0x80; i<0x80; i++) {
			data = i*2*abs(i);
			LinearTable[i+0x80] = data;
		}
	}
};

/************************************************************************
  class SOUNDWAV
    WAVサウンド関連
************************************************************************/

// Little <-> Big Endian
#ifdef powerc		// PPC版。恐ろしい程意味不明（ぉぃ
static asm void Endian16BitBuffer (Ptr buf, unsigned long count)
{
#pragma unused (buf, count)
		b		test
loop:	subi	r4, r4, 0x04
		lwzx	r5, r3, r4
		rlwinm	r5, r5, 0x10, 0x00, 0x1F
		stwbrx	r5, r3, r4
test:	cmpwi	r4, 0x03
		bgt		loop
		blr
}
#else				// 68K版（一応）。こっちは理解できる ^^;
static asm void Endian16BitBuffer (Ptr buf, unsigned long count)
{
		movea.l	0x04(sp), a0
		move.l	0x08(sp), d0
		lsr.l	#0x02, d0
		bra		test
loop:	move.l	(a0), d1
		ror.w	#0x08, d1
		swap	d1
		ror.w	#0x08, d1
		swap	d1
		move.l	d1, (a0)+
test:	dbra	d0, loop
		rts
}
#endif


static void Linear8BitBuffer (Ptr buf, unsigned long count)
{
	int i;
	unsigned char *olddata = &((unsigned char*)buf)[count-1];
	short *newdata = &((short*)buf)[count-1];
	for (i=0; i<count; i++) {
		*newdata = LinearTable[*olddata];
		olddata--;
		newdata--;
	}
}


static pascal void WAVFileCallBack(WAVFILEINFO* wfile)
{
	WAVINFO* wav = wfile->wav;
	int count = wfile->pb.ioParam.ioReqCount;
	Ptr buf = (Ptr)(wfile->buf->dbSoundData);

#ifndef powerc
	long A5bk = SetA5(wav->backupedA5);
#endif

	if ( wav->bits==16 ) {				// 16bitデータならEndian調整
		if ( wav->linear ) {
			Linear8BitBuffer(buf, count);
			count <<= 1;
		} else
			Endian16BitBuffer(buf, count);
	}
	wfile->inuse = 0;
	wfile->buf->dbNumFrames = (count/(wav->channel*wav->bits/8));
	wfile->buf->dbFlags |= dbBufferReady;

#ifndef powerc
	SetA5(A5bk);
#endif
}


static pascal void WAVCallBack(SndChannelPtr /*ch*/, SndDoubleBufferPtr db)
{
	WAVFILEINFO* wfile = (WAVFILEINFO*)(db->dbUserInfo[0]);
	WAVINFO* wav = wfile->wav;
	long len, avail;

#ifndef powerc
	long A5bk = SetA5(wav->backupedA5);
#endif

	if ( (wav->playflag)&&(wfile->pb.ioParam.ioRefNum) ) {
		if ( !wfile->inuse ) {
			avail = wav->end-wav->pos;
			len = SNDBUFSIZE;
			if ( wav->linear ) len >>= 1;
			if ( avail<len ) len = avail;
			wfile->pb.ioParam.ioReqCount = len;
			wfile->pb.ioParam.ioPosOffset = wav->pos;
			wav->pos += len;
			if ( wav->pos>=wav->end ) {
				if ( wav->loopflag ) {
					wav->pos = wav->repeat;
				} else {
					db->dbFlags |= dbLastBuffer;
					wav->playflag = 0;
				}
			}
			wfile->pb.ioParam.ioBuffer = (Ptr)db->dbSoundData;
			wfile->inuse = 1;
			PBReadAsync(&wfile->pb);
		}
	} else {
		wav->playflag = 0;
	}
#ifndef powerc
	SetA5(A5bk);
#endif
}


static pascal void WAVEndCallBack(SndChannelPtr /*ch*/, SndCommand* cmd)
{
	WAVINFO* wav = (WAVINFO*)(cmd->param2);
	wav->playingflag = 0;
}


SOUNDWAV::SOUNDWAV(int l)
{
	int i;
	OSErr err;
	SndDoubleBufferPtr db = 0;

	hsnd = 0;
	sndcb = 0;
	memset(&head, 0, sizeof(SndDoubleBufferHeader2));
	memset(&wav, 0, sizeof(WAVINFO));
	memset(&wfile[0], 0, sizeof(WAVFILEINFO));
	memset(&wfile[1], 0, sizeof(WAVFILEINFO));

	linear = l;
	if ( l ) SetupLinearTable();

	sndcb = NewSndCallBackProc(WAVEndCallBack);	// 終了コールバックになるみたい
	err = SndNewChannel(&hsnd, sampledSynth, initStereo+initNoInterp+initNoDrop, sndcb);
	if ( err!=noErr ) {
		hsnd = 0;
		Error("Cannot make SndChannel.");
	}
	if ( hsnd ) {
		head.dbhDoubleBack = NewSndDoubleBackProc(WAVCallBack);
		head.dbhFormat = 'NONE';
		ioc = NewIOCompletionProc(WAVFileCallBack);
		for (i=0; i<2; i++) {
			db = (SndDoubleBufferPtr)(new unsigned char[SNDBUFSIZE+sizeof(SndDoubleBuffer)]);
			if ( db ) {
				HoldMemory(db, SNDBUFSIZE+sizeof(SndDoubleBuffer));
				memset(db, 0, SNDBUFSIZE+sizeof(SndDoubleBuffer));
				head.dbhBufferPtr[i] = db;
				head.dbhBufferPtr[i]->dbUserInfo[0] = (long)(&wfile[i]);
				wfile[i].pb.ioParam.ioCompletion = ioc;
				wfile[i].pb.ioParam.ioVRefNum = fsCurPerm;
				wfile[i].pb.ioParam.ioPosMode = fsFromStart | noCacheMask;
				wfile[i].wav = &wav;
				wfile[i].num = i;
				wfile[i].buf = db;
			}
		}
		wav.backupedA5 = SetCurrentA5();
	}
};


SOUNDWAV::~SOUNDWAV(void)
{
	int i;

	Stop();

	for (i=0; i<2; i++) {
		if ( head.dbhBufferPtr[i] ) {
			delete[] (unsigned char*)(head.dbhBufferPtr[i]);
			head.dbhBufferPtr[i] = 0;
		}
	}
	if ( ioc ) DisposePtr((Ptr)ioc);
	if ( sndcb ) DisposePtr((Ptr)sndcb);
	if ( hsnd ) {
		DisposePtr((Ptr)head.dbhDoubleBack);
		SndDisposeChannel(hsnd, true);
		hsnd = 0;
	}
};


void SOUNDWAV::Play(char* file, int loop, int cutsize)
{
	unsigned char header[128];
	int i;
	unsigned int rate, channel, bit;
	unsigned char f[256];
	long l;
	short fp;
	SndCommand cmd;

	Stop();

	strcpy((char*)&f[1], file);
	f[0] = strlen(file);
	if ( FSOpen(f, fsCurPerm, &fp)==noErr ) {
//	if ( OpenDF(f, fsCurPerm, &fp)==noErr ) {
		l = 128;
		FSRead(fp, &l, (void*)header);
		wav.end = 0;
		wav.repeat = cutsize;
		// 'data'領域は必ずしも Offset:$22 から始まるとは限らないらしい (BabyFace)
		for (i=36; i<128; i+=2) {		// ので探します・・・
			if ( CheckHeader(&header[i], "data") ) {
				wav.end = ((header[i+4])|(header[i+5]<<8)|(header[i+6]<<16)|(header[i+7]<<24));
				wav.start = i+8;
				break;
			}
		}
		if ( (CheckHeader(header, "RIFF"))&&			// この辺は決め打ち
			 (CheckHeader(&header[8], "WAVE"))&&
			 (CheckHeader(&header[12], "fmt"))&&
			 (wav.end>4) ) {
			rate = ((header[24])|(header[25]<<8)|(header[26]<<16)|(header[27]<<24));
			channel = ((header[22])|(header[23]<<8));
			bit = ((header[34])|(header[35]<<8));
			head.dbhNumChannels = channel;
			if ( (bit==8)&&(linear) ) {
				bit = 16;
				wav.linear = 1;
				wav.repeat >>= 1;		// リニア時にはCutSizeの半分に
			} else {
				wav.linear = 0;
			}
			head.dbhSampleSize = bit;
			head.dbhSampleRate = (rate<<16);
			wav.end += wav.start;
			wav.repeat += wav.start;
			wav.pos = wav.start;
			wav.loopflag = loop;
			wav.channel = channel;
			wav.bits = bit;
			wav.playflag = 1;
			wav.playingflag = 1;
			for (i=0; i<2; i++) {
				wfile[i].pb.ioParam.ioRefNum = fp;
				head.dbhBufferPtr[i]->dbFlags = 0;
				WAVCallBack(hsnd, head.dbhBufferPtr[i]);
			}
			SndPlayDoubleBuffer(hsnd, (SndDoubleBufferHeaderPtr)(&head));
			cmd.cmd = callBackCmd;
			cmd.param1 = 0;
			cmd.param2 = (long)(&wav);
			SndDoCommand(hsnd, &cmd, true);
		}
	}
};


void SOUNDWAV::Stop(void)
{
	SndCommand cmd;

	wav.playflag = 0;
	wav.playingflag = 0;
	wav.loopflag = 0;

	if ( hsnd ) {
		cmd.param1 = 0;
		cmd.param2 = 0;
		cmd.cmd = quietCmd;
		SndDoImmediate(hsnd, &cmd);
		cmd.cmd = flushCmd;
		SndDoImmediate(hsnd, &cmd);
	}
	if ( wfile[0].pb.ioParam.ioRefNum ) {
		FSClose(wfile[0].pb.ioParam.ioRefNum);
		wfile[0].pb.ioParam.ioRefNum = 0;
		wfile[1].pb.ioParam.ioRefNum = 0;
	}
};


void SOUNDWAV::GetVolume(int* l, int* r)
{
//	SndCommand cmd;

//	if ( wav.playflag ) {
//		cmd.param1 = 0;
//		cmd.param2 = 0;
//		cmd.cmd = getVolumeCmd;
//		SndDoImmediate(hsnd, &cmd);

//		*l = (cmd.param2&0xffff);
//		*r = (cmd.param2>>16);
//		if ( *l>0x100 ) *l = 0x100;
//		if ( *r>0x100 ) *r = 0x100;
//		if ( *l<0     ) *l = 0;
//		if ( *r<0     ) *r = 0;
//	} else {		// default
		*l = 0x100;
		*r = 0x100;
//	}
};


void SOUNDWAV::SetVolume(int l, int r)
{
	SndCommand cmd;

	if ( l>0x100 ) l = 0x100;
	if ( r>0x100 ) r = 0x100;
	if ( l<0     ) l = 0;
	if ( r<0     ) r = 0;

	cmd.param1 = 0;
	cmd.param2 = (r<<16)|l;
	cmd.cmd = volumeCmd;
	SndDoImmediate(hsnd, &cmd);
};


int SOUNDWAV::IsPlaying(void)
{
	return wav.playingflag;
}



/************************************************************************
  class SOUNDKOE
    KOE関連
************************************************************************/

#define MAXKOETABLE 1024

// SOUNDKOE用コールバック。IsPlayingフラグを落とすのが目的
static pascal void KOECallBack(SndChannelPtr /*ch*/, SndCommand* cmd)
{
	int* flag = (int*)(cmd->param2);
	*flag = 0;
}


SOUNDKOE::SOUNDKOE(int type, int l)
{
	OSErr err;
	extended80 extFreq;
	double freq = 44100;
	int i, n;

	dtox80(&freq, &extFreq);
	tableid = -1;
	table = 0;
	maxnum = 0;
	rate = 22050;
	koetype = type;
	linear = l;

	koebuf = 0;
	hsnd = 0;
	playflag = 0;
	sndcb = 0;

	memset(&head, 0, sizeof(ExtSoundHeader));
	head.encode = extSH;
	head.AIFFSampleRate = extFreq;
	head.numChannels = 1;
	head.sampleSize = 16;

	sndcb = NewSndCallBackProc(KOECallBack);
	err = SndNewChannel(&hsnd, sampledSynth, 0/*initStereo+initNoInterp+initNoDrop*/, sndcb);
	if ( err!=noErr ) {
		hsnd = 0;
		Error("Cannot make SndChannel.");
	}

	SetupLinearTable();
	// 変換テーブル作成
	// table1 : 8bitサンプル -> 16bitサンプル変換
	for (i=-128; i<128; i++) {
		table1[i+128] = (short)(i*0x100);			// $00～$FF -> -32768～+32767 ・・・になると思う（汗
	}
	// table2 : 4/8bit DifferencialPCM nibble? -> バイト差分データ
	for (i=0; i<256; i++) {
		n = (i>>1);
		if ( i&1 ) n ^= 0xff;						// 最下位ビットがnibbleの符号臭い
		table2[i] = (256-n)&0xff;
//if ( i&1 ) n = -n;
//table2[i] = -n;
	}
}


SOUNDKOE::~SOUNDKOE(void)
{
	Stop();
	if ( table ) delete[] table;
	if ( hsnd ) {
		SndDisposeChannel(hsnd, true);
		hsnd = 0;
	}
	if ( koebuf ) delete[] koebuf;
	if ( sndcb ) DisposePtr((Ptr)sndcb);
}

void SOUNDKOE::MakeTable(char* f, int tid)
{
	FILE* fp = 0;
	unsigned char head[32];
	int i;

	// 今のとおんなじテーブルならそのまま
	if ( tableid==tid ) return;

	if ( table ) delete[] table;
	tableid = -1;
	table = 0;
	maxnum = 0;

	fp = fopen(f, "rb");
	if ( fp ) {
		fread(head, 32, 1, fp);
		if ( !strcmp((char*)head, "KOEPAC") ) {
			maxnum = ((head[16])|(head[17]<<8)|(head[18]<<16)|(head[19]<<24));
			rate = ((head[24])|(head[25]<<8)|(head[26]<<16)|(head[27]<<24));
			if ( !rate ) rate = 22050;		// flowers, 0だとデフォルト値みたい
			if ( (maxnum>0)&&(maxnum<MAXKOETABLE) ) {
				table = new KOETABLE[maxnum+1];
				if ( table ) {
					fread((unsigned char*)table, maxnum<<3, 1, fp);
					Endian16BitBuffer((Ptr)table, maxnum<<3);
					for (i=0; i<maxnum; i++) {		// longはエンディアン変更だけでは対応できないので
						table[i].pos = ((table[i].pos>>16)|((table[i].pos&0xffff)<<16));
					}
					tableid = tid;
					fseek(fp, 0, 2);
					table[maxnum].pos = ftell(fp);
					table[maxnum].symbol = 0;
					table[maxnum].id = 999999;		// ま、安全のため
				}
			}
		}
		fclose(fp);
	}
}


// バイトバッファ s[] の n 番目の nibble を得る
// バッファ内容が $AB $CD $EF ... とすると、n=0,1,2,3...時の返値は
// $0B $0A $0D $0C ... となります。
#define nibble(s,n) ((s[n>>1]>>((n&1)<<2))&15)


void SOUNDKOE::Play(char* f, int tid, int id)
{
	int i, j, num;
	int srcsize = 0, dstsize = 0;
	unsigned short* buf = 0;
	unsigned char *srcbuf = 0, *src, s, data;
	signed short *dst = 0;
	FILE* fp = 0;
	SndCommand cmd;
	char file[256];
	unsigned char header[128];

	Stop();

	// サウンドデバイスがないなら帰る
	if ( !hsnd ) return;

	if ( koetype ) {			// Differencial PCM / PAC の時 (DOUBLE_PAC)
		// テーブル読み込み
		sprintf(file, ":%s:Z%03d.KOE", f, tid);
		MakeTable(file, tid);
		if ( !table ) return;

		// テーブル検索
		for (i=0; i<maxnum; i++) {
			if ( table[i].id==id ) break;		// め～っけ
		}
		if ( i==maxnum ) return;				// めっからなかった・・・

		// シンボルリスト読み込み
		num = table[i].symbol;
		buf = new unsigned short[num];
		fp = fopen(file, "rb");
		if ( (fp)&&(buf) ) {
			fseek(fp, table[i].pos, 0);
			fread((unsigned char*)buf, num, 2, fp);
			fclose(fp);
		} else {
			if ( fp ) fclose(fp);
			if ( buf ) delete[] buf;
			return;
		}

		// シンボルリストからサイズ計算
		for (j=0; j<num; j++) {
			buf[j] = ((buf[j]>>8)|((buf[j]&0xff)<<8));		// Endianへんこー
			srcsize += buf[j];
			if ( buf[j] ) {
				if ( buf[j]==0x400 )
					dstsize += 0x400;			// $400時はそのままコピーなので
				else
					dstsize += (buf[j]<<1);		// $001～$3FF時、最大で2倍に増える
			} else {
				dstsize += 0x400;				// 無音／撥音？
			}
		}

		// サイズチェック
		if ( srcsize>(table[i+1].pos-table[i].pos) ) return;	// サイズが変

		// そーすを読み込む＆サンプリングバッファ用意
		srcbuf = new unsigned char[srcsize];
		if ( srcbuf ) {
			fp = fopen(file, "rb");
			if ( !fp ) {
				delete[] srcbuf;
				return;
			}
			fseek(fp, table[i].pos+num*2, 0);
			fread(srcbuf, srcsize, 1, fp);
			fclose(fp);
			koebuf = new signed short[dstsize];
			if ( !koebuf ) {
				delete[] srcbuf;
				return;
			}
		}

		// でこーどするよ
		src = srcbuf;
		dst = koebuf;
		for (i=0; i<num; i++) {
			if ( buf[i] ) {
				if ( buf[i]==0x400 ) {
					for (j=0; j<0x400; j++) {
//						*dst++ = table1[*src++];
*dst++ = LinearTable[*src++];
					}
				} else {
					data = 0;
					for (j=0; j<buf[i]*2; ) {
						s = nibble(src,j);					// まず1nibble拾う
						j++;
						if ( s==15 ) {						// nibbleが15の時は2バイト差分
							s = nibble(src,j);				// nibbleを2個拾う
							j++;
							s |= (nibble(src,j)<<4);
							j++;
						}
						data += table2[s&255];				// 8bitサンプリングデータに加算
//						*dst++ = table1[data&255];			// 16ビットに変換
*dst++ = LinearTable[data&255];
					}
					src += buf[i];
				}
			} else {					// 無音／撥音？
				for (j=0; j<0x400; j++) {
					*dst++ = 0;
				}
			}
		}
		delete[] srcbuf;
		dstsize = dst-koebuf;
	} else {			// 生WAVデータタイプの場合（Linear指定の処理が入ってない・・・）
		sprintf(file, ":%s:V:%03d:Z%03d%04d.WAV", f, tid, tid, id);
		fp = fopen(file, "rb");
		if ( fp ) {
			fread(header, 128, 1, fp);
			for (i=36; i<128; i+=2) {
				if ( CheckHeader(&header[i], "data") ) {
					num = i+8;
					srcsize = ((header[i+4])|(header[i+5]<<8)|(header[i+6]<<16)|(header[i+7]<<24));
					srcsize -= num;
					break;
				}
			}
			srcbuf = new unsigned char[srcsize+4];			// +4 は Endian16BitBuffer が 4 バイト単位で計算するため
			if ( (CheckHeader(header, "RIFF")) &&			// この辺は決め打ち
				 (CheckHeader(&header[8], "WAVE")) &&
				 (CheckHeader(&header[12], "fmt")) &&
				 (srcsize>4) &&
				 (srcbuf) ) {
				fseek(fp, num, 0);
				fread(srcbuf, srcsize, 1, fp);
				fclose(fp);
				rate = ((header[24])|(header[25]<<8)|(header[26]<<16)|(header[27]<<24));
				head.numChannels = ((header[22])|(header[23]<<8));
				head.sampleSize = ((header[34])|(header[35]<<8));
				dstsize = (srcsize/(head.numChannels*head.sampleSize/8));
				if ( head.sampleSize==16 ) Endian16BitBuffer((Ptr)srcbuf, srcsize);
				koebuf = (short*)srcbuf;
			} else {
				fclose(fp);
				return;
			}
		} else {
			return;
		}
	}

	// 再生開始～
	head.sampleRate = (rate<<16);
	head.numFrames = dstsize;
	head.samplePtr = (Ptr)koebuf;
	cmd.cmd = bufferCmd;
	cmd.param1 = 0;
	cmd.param2 = (long)(&head);
	SndDoCommand(hsnd, &cmd, true);
	playflag = 1;
	cmd.cmd = callBackCmd;
	cmd.param1 = 0;
	cmd.param2 = (long)(&playflag);
	SndDoCommand(hsnd, &cmd, true);
}


void SOUNDKOE::Stop(void)
{
	SndCommand cmd;

	playflag = 0;
	if ( hsnd ) {
		cmd.param1 = 0;
		cmd.param2 = 0;
		cmd.cmd = quietCmd;
		SndDoImmediate(hsnd, &cmd);
		cmd.cmd = flushCmd;
		SndDoImmediate(hsnd, &cmd);
	}
	if ( koebuf ) {
		delete[] koebuf;
		koebuf = 0;
	}
}


int SOUNDKOE::IsPlaying(void)
{
	return playflag;
}
