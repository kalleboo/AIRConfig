#include <string.h>

#include "Globals.h"
#include "Defines.h"

extern pascal void DrawDefaultButtonUserItem(WindowPtr theWindow, short itemNo);
extern void SavePreferences(void);

void ShowHelpDialog();
pascal void HelpTextUserItem(WindowPtr theWindow, short itemNo);

void ShowAboutDialog();
pascal void AboutTextUserItem(WindowPtr theWindow, short itemNo);


#pragma segment Main
void ShowHelpDialog()
{
	DialogPtr	dialogPtr;
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	short		itemHit;
	
	
	dialogPtr = GetNewDialog(rHelpDialog, 0, (WindowPtr) - 1);
	
	GetDialogItem(dialogPtr, 3, &iType, &iHandle, &iRect);
	SetDialogItem(dialogPtr, 3, iType, (Handle) &HelpTextUserItem, &iRect);
	
	GetDialogItem(dialogPtr, 4, &iType, &iHandle, &iRect);
	SetDialogItem(dialogPtr, 4, iType, (Handle) &DrawDefaultButtonUserItem, &iRect);

	SetCursor(&qd.arrow);
	ModalDialog(0, &itemHit);
	
	CloseDialog(dialogPtr);
	
	gPrefs.hasSeenHelp = true;
	SavePreferences();
}


#pragma segment Main
pascal void HelpTextUserItem(WindowPtr theWindow, short itemNo) {
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	Handle      helpTextRes;
	Handle      helpText;

	helpTextRes = Get1Resource('TEXT', rHelpDialog);
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
	DisposeHandle(helpText);
}



#pragma segment Main
void ShowAboutDialog()
{
	DialogPtr	dialogPtr;
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	short		itemHit;
	
	
	dialogPtr = GetNewDialog(rAboutDialog, 0, (WindowPtr) - 1);
	
	GetDialogItem(dialogPtr, 4, &iType, &iHandle, &iRect);
	SetDialogItem(dialogPtr, 4, iType, (Handle) &AboutTextUserItem, &iRect);

	GetDialogItem(dialogPtr, 5, &iType, &iHandle, &iRect);
	SetDialogItem(dialogPtr, 5, iType, (Handle) &DrawDefaultButtonUserItem, &iRect);
	
	SetCursor(&qd.arrow);
	ModalDialog(0, &itemHit);
	
	CloseDialog(dialogPtr);
}


#pragma segment Main
pascal void AboutTextUserItem(WindowPtr theWindow, short itemNo) {
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	Handle      helpTextRes;
	Handle      helpText;

	helpTextRes = Get1Resource('TEXT', rAboutDialog);
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
	DisposeHandle(helpText);
}