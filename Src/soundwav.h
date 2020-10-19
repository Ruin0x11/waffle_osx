/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  soundwav.h
     WAV�T�E���h�Ǘ��N���X
=======================================================================*/

#ifndef _soundwav_h
#define _soundwav_h

#include <QuickTimeComponents.h>
#include <Sound.h>

#define SNDBUFSIZE 44100		// 4�Ŋ���؂���[�ɂ��Ƃ��Ăˁi1frame�ő傪16bit x stereo�Ȃ̂Łj


/************************************************************************
  class SOUNDWAV
************************************************************************/

// WAV�f�[�^���ߗv���R�[���o�b�N�ɓn�����f�[�^
struct WAVINFO {
	int playflag;			// �Đ����t���O
	int playingflag;		// �Đ����t���O
	int loopflag;			// ���[�v�Đ�
	int start;			// WAV�f�[�^��́A�t�@�C���擪����̃I�t�Z�b�g
	int end;			// �f�[�^�I�[�ʒu
	int repeat;			// ���s�[�g���̊����߂��ʒu
	int pos;			// ���݂̃t�@�C���|�C���g
	int channel;			// �`���l�����i1:mono 2:stereo)
	int bits;			// �f�[�^�r�b�g���i8 or 16�j
	int linear;			//  ���`���k�t���O
	long backupedA5;		// A5�̃o�b�N�A�b�v
};


// �t�@�C���ǂݍ��ݏI���R�[���o�b�N�ɓn�����f�[�^�iParamBlockRec�̊g���j
struct WAVFILEINFO {
	ParamBlockRec pb;		// �p�����[�^�u���b�N���R�[�h�iPBReadAsync�Ŏg�p�j
	SndDoubleBufferPtr buf;		// DoubleBuffer�̕Ђ��ۂ̃|�C���^
	int num;			// �o�b�t�@�ԍ��i0��1�j
	int inuse;			// �f�B�X�N�ǂݍ��ݎ��s���t���O
	WAVINFO* wav;			// WAVINFO�ւ̃|�C���^
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
	~SOUNDWAV(void);

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
	~SOUNDKOE(void);

	void Play(char* f, int tid, int id);
	void Stop(void);
	int IsPlaying(void);
};

#endif
