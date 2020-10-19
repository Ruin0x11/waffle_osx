/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  nvlfont.cpp
    NVL_SYSTEM=1 ���̃t�H���g�f�[�^�̃N���X
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "nvlfont.h"
#include "system.h"
#include "common.h"
#include "debug.h"

#define FONTDATASIZE 2544768

/************************************************************************
  class NOVELFONT
************************************************************************/

// ���񂷂Ƃ炭��
NOVELFONT::NOVELFONT(SYSTEM* s)
{
	int temp;
	sys = s;
	buf = sys->ReadFile("FN.DAT", "OTH", &temp);
};


// �ł��Ƃ炭��
NOVELFONT::~NOVELFONT(void)
{
	if ( buf ) delete[] buf;
};


// SJIS -> JIS �R�[�h�ϊ����[�`��
// �ь�̐X�̎q�L�B�ɖ���Ă����ɂ�i����j
int NOVELFONT::SJIS2JIS(int sjis) {

	int	codeh, tmp, codel;

	codel = (sjis&0xff);
	codeh = (sjis - 0x8100) & 0xff00;

	tmp = 0x2100;
	if (codel > 0x7f) {
		if (codel >= 0x9f) {
			if (codel > 0xfc) {
				return(0);
			}
			tmp += 0x100;
			codel -= 0x5f;
		}
		else {
			codel--;
		}
	}
	else if ((codel == 0x7f) || (codel < 0x40)) {
		return(0);
	}
	codel -= 0x1f;
	if (codeh >= 0x1f00) {
		if ((codeh < 0x5f00) || (codeh > 0x6e00)) {
			return(0);
		}
		codeh -= 0x5f00;
		tmp += 0x5f00 - 0x2100;
	}
	return (((codeh << 1) + tmp) + codel);
}


// �t�H���g�̃A�h���X�����߂�
// JIS�R�[�h����$2121�������āA(��ʃo�C�g x $5e + ���ʃo�C�g) x 288(1����������̃o�C�g��)
// �E�E�E�ō����Ă�Ǝv���񂾂��ǁi��
unsigned char* NOVELFONT::GetFont(int num)
{
	int h, l, pos;
	if ( buf ) {
		l = SJIS2JIS(num);
		l -= 0x2121;
		h = (l>>8);
		l &= 0xff;
		pos = (h*0x5e+l)*12*24;
		if ( pos>(FONTDATASIZE-(12*24)) ) pos = 0;
		return (buf+pos);
	} else {
		return 0;
	}
}
