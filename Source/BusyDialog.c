
#define busywBusyText	1

Boolean	gShowingBusyDialog;

void ShowBusyDialog(Str255 message);


void ShowLoadingBusyDialog(void) {
	OSErr		error;
	Str63		fetchName = "\p";
	Str255		urlHost;
	Str255		urlPath;
	Str255		displayString;
	
	if (gPrefs.inputSource == kInputSourceFile) {
		if (gPrefs.inputFileAlias != nil) {
			error = GetAliasInfo(gPrefs.inputFileAlias, asiZoneName, fetchName);
		
			if (error == noErr && fetchName[0] != 0) {
				StringInsert("\pLoading input file in zone \"^0\"...", fetchName, displayString);
			
				ShowBusyDialog(displayString);
			}
		}
		
	} else if (gPrefs.inputSource == kInputSourceURL) {
		if (gPrefs.inputURL[0] > 0) {
			ParseURL(urlHost, urlPath);
			
			StringInsert("\pFetching input from ^0...", urlHost, displayString);
			
			ShowBusyDialog(displayString);
		}
	}
}

void ShowBusyDialog(Str255 message) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	busyWindowPtr = GetNewDialog(rBusyDialog, NULL, (WindowPtr) -1);
	
	GetDItem(busyWindowPtr, busywBusyText, &iType, &iHandle, &iRect);
	SetIText(iHandle, message);
	
	ShowWindow(busyWindowPtr);
	gShowingBusyDialog = true;
	
	UpdtDialog(busyWindowPtr, busyWindowPtr->visRgn);
}

void HideBusyDialog(void) {
	//SetCursor(&arrow);
	
	if (!gShowingBusyDialog) { return; }
	
	DisposeWindow(busyWindowPtr);
	gShowingBusyDialog = false;
}