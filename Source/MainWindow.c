
void ShowMainWindow(void);
void UpdateMainWindow(void);
void RedrawMainWindow(void);
void DoMainWindowEvent(short whichItem);

pascal void DrawFilename(WindowPtr theWindow, short itemNo);

void PickInputFile( void );
void PickOutputFile( void );


#define		mwDoItButton		1
#define		mwInputButton		2
#define		mwInputStatus		3
#define		mwOutputButton		4
#define		mwOutputStatus		5
#define		mwInputIcon			6
#define		mwOutputIcon		7
#define		mwInputTitle		8
#define		mwOutputTitle		9
#define		mwDivider1			10
#define		mwDivider2			11
#define		mwInputFilename		12
#define		mwOutputFilename	13
#define		mwInputIconBlank	14
#define		mwOutputIconBlank	15


#pragma segment Main
void ShowMainWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	mainWindow.ptr = GetNewDialog(130, NULL, (WindowPtr) -1);
	
	GetDItem(mainWindow.ptr, mwInputTitle, &iType, &iHandle, &iRect);
	SetIText(iHandle, "\pHost IDs:");
	GetDItem(mainWindow.ptr, mwOutputTitle, &iType, &iHandle, &iRect);
	SetIText(iHandle, "\pRouter Config:");
	
	GetDItem(mainWindow.ptr, mwInputStatus, &iType, &mainWindow.inputStatus, &iRect);
	GetDItem(mainWindow.ptr, mwInputButton, &iType, &iHandle, &iRect);
	mainWindow.inputButton = (ControlHandle) iHandle;
	
	GetDItem(mainWindow.ptr, mwOutputStatus, &iType, &mainWindow.outputStatus, &iRect);
	GetDItem(mainWindow.ptr, mwOutputButton, &iType, &iHandle, &iRect);
	mainWindow.outputButton = (ControlHandle) iHandle;
	
	GetDItem(mainWindow.ptr, mwDoItButton, &iType, &iHandle, &iRect);
	mainWindow.doItButton = (ControlHandle) iHandle;
	
	GetDItem(mainWindow.ptr, mwDivider1, &iType, &iHandle, &iRect);
	SetDItem(mainWindow.ptr, mwDivider1, iType, (Handle) &MyDrawRect, &iRect);
	
	GetDItem(mainWindow.ptr, mwDivider2, &iType, &iHandle, &iRect);
	SetDItem(mainWindow.ptr, mwDivider2, iType, (Handle) &MyDrawRect, &iRect);
	
	GetDItem(mainWindow.ptr, mwInputFilename, &iType, &iHandle, &iRect);
	SetDItem(mainWindow.ptr, mwInputFilename, iType, (Handle) &DrawFilename, &iRect);
	
	GetDItem(mainWindow.ptr, mwOutputFilename, &iType, &iHandle, &iRect);
	SetDItem(mainWindow.ptr, mwOutputFilename, iType, (Handle) &DrawFilename, &iRect);
	
	UpdateMainWindow();
	
	ShowWindow(mainWindow.ptr);
}

#pragma segment Main
void UpdateMainWindow(void) {
	Boolean		isReady = true;
	Str255		numStr;
	Handle		formattedString;
	Str255		displayString;
	
	
	if (gState.totalEntries == 0) {
		SetIText(mainWindow.inputStatus, "\p");
		
		SetCTitle(mainWindow.inputButton, "\pOpen...");
		HideDItem(mainWindow.ptr, mwInputIcon);
		ShowDItem(mainWindow.ptr, mwInputIconBlank);
		isReady = false;
	} else {
		NumToString(gState.totalEntries, numStr);
		formattedString = StringInsert("\pLoaded ^0 entries", numStr);
		BlockMove(*formattedString, displayString, GetHandleSize(formattedString));
		
		SetIText(mainWindow.inputStatus, displayString);
		
		SetCTitle(mainWindow.inputButton, "\pClose");
		ShowDItem(mainWindow.ptr, mwInputIcon);
		HideDItem(mainWindow.ptr, mwInputIconBlank);
	}
	
	if (gState.outFileSpec.vRefNum == 0) {
		SetIText(mainWindow.outputStatus, "\p");
		SetCTitle(mainWindow.outputButton, "\pOpen...");
		HideDItem(mainWindow.ptr, mwOutputIcon);
		ShowDItem(mainWindow.ptr, mwOutputIconBlank);
		isReady = false;
	} else {
		formattedString = StringInsert("\pFound IP tunnel on \"^0\"", gState.resourceName);
		BlockMove(*formattedString, displayString, GetHandleSize(formattedString));
		
		SetIText(mainWindow.outputStatus, displayString);
		SetCTitle(mainWindow.outputButton, "\pClose");
		ShowDItem(mainWindow.ptr, mwOutputIcon);
		HideDItem(mainWindow.ptr, mwOutputIconBlank);
	}
	
	HiliteControl(mainWindow.doItButton, isReady ? 0 : 255);
	
	RedrawMainWindow();
}

#pragma segment Main
void RedrawMainWindow(void) {

	//BeginUpdate(mainWindow.ptr);
	UpdtDialog(mainWindow.ptr, mainWindow.ptr->visRgn);
	//EndUpdate(mainWindow.ptr);
}

#pragma segment Main
pascal void DrawFilename(WindowPtr theWindow, short itemNo) {
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	short		length = 0;
	Str63		defaultName = "\p<no file open>";
	Handle		fileNameHandle;
	
	if (itemNo == mwInputFilename && gState.inFileSpec.vRefNum != 0) {
		length = gState.inFileSpec.name[0];
		PtrToHand(&gState.inFileSpec.name[1], &fileNameHandle, length);
		
	} else if (itemNo == mwOutputFilename && gState.outFileSpec.vRefNum != 0) {
		length = gState.outFileSpec.name[0];
		PtrToHand(&gState.outFileSpec.name[1], &fileNameHandle, length);

	} else {
		length = defaultName[0];
		PtrToHand(&defaultName[1], &fileNameHandle, length);
	}
	
	GetDItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	
	HLock(fileNameHandle);
	TextBox(*fileNameHandle, length, &iRect, teJustCenter);
	HUnlock(fileNameHandle);
	
	TextFont(systemFont);
	TextSize(12);
}


#pragma segment Main
void DoMainWindowEvent(short whichItem) {
	switch (whichItem) {
		case mwInputButton:
			if (gState.inFileSpec.vRefNum == 0) {
				PickInputFile();
			} else {
				AbortInputFile();
			}
			break;
			
		case mwOutputButton:
			if (gState.outFileSpec.vRefNum == 0 ) {
				PickOutputFile();
			} else {
				AbortOutputFile();
			}
			break;
			
		case mwDoItButton:
			if (WriteOutputFile()) {
				AlertInfoMessage("\pSuccessfully saved changes.", noErr);
			}
			break;
	}
	
	UpdateMainWindow();
}




#pragma segment Main
void PickInputFile(void)
{
	SFTypeList				typeList;
	StandardFileReply		reply;
	
	typeList[0] = 'TEXT';
	StandardGetFile(NULL, 1, typeList, &reply);
	if (!reply.sfGood) {
		return;
	}
	
	gState.inFileSpec = reply.sfFile;
	
	if (LoadInputFile()) {
		SavePreferences();
	} else {
		AbortInputFile();
	}
}

#pragma segment Main
void PickOutputFile(void)
{
	SFTypeList				typeList;
	StandardFileReply		reply;
	
	typeList[0] = 'ARSD';
	StandardGetFile(NULL, 1, typeList, &reply);
	if (!reply.sfGood) {
		return;
	}
	
	gState.outFileSpec = reply.sfFile;
	
	if (LoadOutputFile()) {
		SavePreferences();
	} else {
		AbortOutputFile();
	}
}


