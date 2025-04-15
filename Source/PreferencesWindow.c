#include <string.h>

#include "Globals.h"
#include "Defines.h"

extern void SavePreferences(void);

void UpdatePrefsWindow(void);
void ShowPrefsWindow(void);
void ToggleHeadless(void);
void DoPrefsWindowEvent(short whichItem);
pascal void PrefsHeadlessHintUserItem(WindowPtr theWindow, short itemNo);


#define prefwHeadless	1
#define prefwHint		2



#pragma segment Main
void UpdatePrefsWindow(void) {
	short			iType;
	Handle			iHandle;
	Rect			iRect;
	
	GetDialogItem(prefsWindowPtr, prefwHeadless, &iType, &iHandle, &iRect);
	SetControlValue((ControlHandle)iHandle, gPrefs.headlessMode);
}


#pragma segment Main
void ShowPrefsWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	prefsWindowPtr = GetNewDialog(rPrefsDialog, NULL, (WindowPtr) -1);
	
	GetDialogItem(prefsWindowPtr, prefwHint, &iType, &iHandle, &iRect);
	SetDialogItem(prefsWindowPtr, prefwHint, iType, (Handle) &PrefsHeadlessHintUserItem, &iRect);
	
	UpdatePrefsWindow();
	
	ShowWindow(prefsWindowPtr);
}


#pragma segment Main
void ToggleHeadless(void) {
	gPrefs.headlessMode = !gPrefs.headlessMode;
	
	UpdatePrefsWindow();
	
	SavePreferences();
}

#pragma segment Main
void DoPrefsWindowEvent(short whichItem) {
	switch (whichItem) {
		case prefwHeadless:
			ToggleHeadless();
			break;
	}
}



#pragma segment Main
pascal void PrefsHeadlessHintUserItem(WindowPtr theWindow, short itemNo) {
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	Handle      helpTextRes;
	Handle      helpText;

	helpTextRes = Get1Resource('TEXT', rPrefsDialog);
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