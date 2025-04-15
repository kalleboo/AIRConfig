#include <string.h>

#include "Globals.h"
#include "Defines.h"

extern void ParseURL(Str255 host, Str255 path);
extern void LoadCurrentInput(void);
extern void AlertWarningMessage(Str255 message, OSErr errorCode);
extern void SavePreferences(void);
extern pascal void DrawDefaultButtonUserItem(WindowPtr theWindow, short itemNo);
extern Boolean DoCloseWindow( WindowPtr window );

void ShowOpenURLWindow(void);
void UpdateOpenURLWindow(void);
void FinishOpenURLWindow(void);
pascal void OpenURLHintUserItem(WindowPtr theWindow, short itemNo);
void DoOpenURLWindowEvent(short whichItem);

#define urlwOK			1
#define urlwCancel		2
#define urlwEdit		4
#define urlwHint		5
#define urlwDefault		6




#pragma segment Main
void ShowOpenURLWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	itemHit;

	openURLWindowPtr = GetNewDialog(rOpenURLDialog, NULL, (WindowPtr) -1);
	
	GetDialogItem(openURLWindowPtr, urlwHint, &iType, &iHandle, &iRect);
	SetDialogItem(openURLWindowPtr, urlwHint, iType, (Handle) &OpenURLHintUserItem, &iRect);
	
	GetDialogItem(openURLWindowPtr, urlwDefault, &iType, &iHandle, &iRect);
	SetDialogItem(openURLWindowPtr, urlwDefault, iType, (Handle) &DrawDefaultButtonUserItem, &iRect);
	
	GetDialogItem(openURLWindowPtr, urlwEdit, &iType, &iHandle, &iRect);
	SetDialogItemText(iHandle, gPrefs.inputURL);
	
	UpdateOpenURLWindow();
	
	//ShowWindow(openURLWindowPtr);
	
	do {
		ModalDialog(0, &itemHit);
	} while (itemHit != ok && itemHit != cancel);
	
	FinishOpenURLWindow();
	DisposeDialog(openURLWindowPtr);
}



#pragma segment Main
void UpdateOpenURLWindow(void) {

}


#pragma segment Main
void FinishOpenURLWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	
	Str255 	host;
	Str255	path;
	
	GetDialogItem(openURLWindowPtr, urlwEdit, &iType, &iHandle, &iRect);
	GetDialogItemText(iHandle, gPrefs.inputURL);
	
	ParseURL(host, path);
	
	if (host[0] == 0 || path[0] == 0) {
		AlertWarningMessage("\pThe URL could not be parsed. Check that it is a full URL of format http://example.com/dir/file.txt", noErr);
	}
	
	
	SavePreferences();
	LoadCurrentInput();
}


#pragma segment Main
void DoOpenURLWindowEvent(short whichItem) {
	switch (whichItem) {
		case urlwOK:
			FinishOpenURLWindow();
			DoCloseWindow(openURLWindowPtr);
			break;
		case urlwCancel:
			DoCloseWindow(openURLWindowPtr);
			break;
	}
}


#pragma segment Main
pascal void OpenURLHintUserItem(WindowPtr theWindow, short itemNo) {
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	Handle      helpTextRes;
	Handle      helpText;

	helpTextRes = Get1Resource('TEXT', rOpenURLDialog);
	if (helpTextRes == NULL) {
		return;
	}
	
	helpText = NewHandleClear(GetHandleSize(helpTextRes) + 1);
	HLock(helpText);
	HLock(helpTextRes);
	BlockMove(*helpTextRes, *helpText, GetHandleSize(helpTextRes));
	HUnlock(helpTextRes);
	
	GetDialogItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	TETextBox(*helpText, strlen(*helpText), &iRect, teJustLeft);
	
	HUnlock(helpText);
}