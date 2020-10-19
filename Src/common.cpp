/*=======================================================================
  AVG32-like scriptor for Macintosh
  Copyright 2000, K.Takagi(Kenjo)

  common.cpp
    エラー表示ダイアログなど
=======================================================================*/

#include <stdio.h>
#include <string.h>
#include "common.h"

void Error(char* msg)
{
	DialogPtr hDlg = NULL;
	unsigned char buf[256];
	DialogItemType type;
	Handle item;
	Rect rect;
	short num;

	hDlg = GetNewDialog(134, NULL, (WindowPtr)-1);
	if ( hDlg ) {
		SetDialogDefaultItem(hDlg, 1);
		strncpy((char*)&buf[1], msg, 255);
		if ( strlen(msg)>255 )
			buf[0] = 255;
		else
			buf[0] = strlen(msg);
		GetDialogItem(hDlg, 2, &type, &item, &rect);
		SetDialogItemText(item, buf);
		while (1) {
			ModalDialog(NULL, &num);
			if ( num == 1 ) {
				break;
			}
		}
		DisposeDialog(hDlg);
	}
}
