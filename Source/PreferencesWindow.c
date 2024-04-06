

pascal void PrefsHeadlessHintUserItem(WindowPtr theWindow, short itemNo);

#define prefwHeadless	1
#define prefwHint		2



#pragma segment Main
void UpdatePrefsWindow(void) {
	short			iType;
	Handle			iHandle;
	Rect			iRect;
	
	GetDItem(prefsWindowPtr, prefwHeadless, &iType, &iHandle, &iRect);
	SetCtlValue((ControlHandle)iHandle, gPrefs.headlessMode);
}


#pragma segment Main
void ShowPrefsWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	prefsWindowPtr = GetNewDialog(rPrefsDialog, NULL, (WindowPtr) -1);
	
	GetDItem(prefsWindowPtr, prefwHint, &iType, &iHandle, &iRect);
	SetDItem(prefsWindowPtr, prefwHint, iType, (Handle) &PrefsHeadlessHintUserItem, &iRect);
	
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
	
	GetDItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	TextBox(*helpText, strlen(*helpText), &iRect, teJustLeft);
	
	HUnlock(helpText);
}