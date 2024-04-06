

void UpdateOpenURLWindow(void);
void FinishOpenURLWindow(void);
pascal void OpenURLHintUserItem(WindowPtr theWindow, short itemNo);

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
	
	GetDItem(openURLWindowPtr, urlwHint, &iType, &iHandle, &iRect);
	SetDItem(openURLWindowPtr, urlwHint, iType, (Handle) &OpenURLHintUserItem, &iRect);
	
	GetDItem(openURLWindowPtr, urlwDefault, &iType, &iHandle, &iRect);
	SetDItem(openURLWindowPtr, urlwDefault, iType, (Handle) &DrawDefaultButtonUserItem, &iRect);
	
	GetDItem(openURLWindowPtr, urlwEdit, &iType, &iHandle, &iRect);
	SetIText(iHandle, gPrefs.inputURL);
	
	UpdateOpenURLWindow();
	
	//ShowWindow(openURLWindowPtr);
	
	do {
		ModalDialog(0, &itemHit);
	} while (itemHit != ok && itemHit != cancel);
	
	FinishOpenURLWindow();
	DisposDialog(openURLWindowPtr);
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
	
	GetDItem(openURLWindowPtr, urlwEdit, &iType, &iHandle, &iRect);
	GetIText(iHandle, gPrefs.inputURL);
	
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
	
	GetDItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	TextBox(*helpText, strlen(*helpText), &iRect, teJustLeft);
	
	HUnlock(helpText);
}