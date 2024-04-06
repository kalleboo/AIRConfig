
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
	
	GetDItem(dialogPtr, 3, &iType, &iHandle, &iRect);
	SetDItem(dialogPtr, 3, iType, (Handle) &HelpTextUserItem, &iRect);
	
	GetDItem(dialogPtr, 4, &iType, &iHandle, &iRect);
	SetDItem(dialogPtr, 4, iType, (Handle) &DrawDefaultButtonUserItem, &iRect);

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
	
	GetDItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	TextBox(*helpText, strlen(*helpText), &iRect, teJustLeft);
	
	HUnlock(helpText);
	DisposHandle(helpText);
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
	
	GetDItem(dialogPtr, 4, &iType, &iHandle, &iRect);
	SetDItem(dialogPtr, 4, iType, (Handle) &AboutTextUserItem, &iRect);

	GetDItem(dialogPtr, 5, &iType, &iHandle, &iRect);
	SetDItem(dialogPtr, 5, iType, (Handle) &DrawDefaultButtonUserItem, &iRect);
	
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
	
	GetDItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	TextBox(*helpText, strlen(*helpText), &iRect, teJustLeft);
	
	HUnlock(helpText);
	DisposHandle(helpText);
}