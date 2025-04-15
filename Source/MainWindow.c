#include "Globals.h"
#include "Defines.h"

extern pascal void MyDrawRect(WindowPtr theWindow, short itemNo);
extern void StringInsert(Str255 baseString, Str255 subsString, Str255 resultString);
extern void ParseURL(Str255 host, Str255 path);
extern void ShowOpenURLWindow(void);
extern void AbortInputFile(void);
extern void AbortOutputFile(void);
extern Boolean WriteOutputFile( void );
extern void LoadCurrentInput(void);
extern void LoadCurrentOutput(void);
extern void AlertInfoMessage(Str255 message, OSErr errorCode);
extern void SavePreferences(void);

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
#define		mwInputFilename			12
#define		mwOutputFilename		13
#define		mwInputIconBlank		14
#define		mwOutputIconBlank		15
#define		mwInputRadioFile		16
#define		mwInputRadioInternet	17
#define		mwInputIconNoInternet	18
#define		mwInputIconInternet		19


#pragma segment Main
void ShowMainWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	mainWindow.ptr = GetNewDialog(130, NULL, (WindowPtr) -1);
	
	GetDialogItem(mainWindow.ptr, mwInputTitle, &iType, &iHandle, &iRect);
	SetDialogItemText(iHandle, "\pHost IDs:");
	GetDialogItem(mainWindow.ptr, mwOutputTitle, &iType, &iHandle, &iRect);
	SetDialogItemText(iHandle, "\pRouter Config:");
	
	GetDialogItem(mainWindow.ptr, mwInputStatus, &iType, &mainWindow.inputStatus, &iRect);
	GetDialogItem(mainWindow.ptr, mwInputButton, &iType, &iHandle, &iRect);
	mainWindow.inputButton = (ControlHandle) iHandle;
	
	GetDialogItem(mainWindow.ptr, mwOutputStatus, &iType, &mainWindow.outputStatus, &iRect);
	GetDialogItem(mainWindow.ptr, mwOutputButton, &iType, &iHandle, &iRect);
	mainWindow.outputButton = (ControlHandle) iHandle;
	
	GetDialogItem(mainWindow.ptr, mwDoItButton, &iType, &iHandle, &iRect);
	mainWindow.doItButton = (ControlHandle) iHandle;
	
	GetDialogItem(mainWindow.ptr, mwDivider1, &iType, &iHandle, &iRect);
	SetDialogItem(mainWindow.ptr, mwDivider1, iType, (Handle) &MyDrawRect, &iRect);
	
	GetDialogItem(mainWindow.ptr, mwDivider2, &iType, &iHandle, &iRect);
	SetDialogItem(mainWindow.ptr, mwDivider2, iType, (Handle) &MyDrawRect, &iRect);
	
	GetDialogItem(mainWindow.ptr, mwInputFilename, &iType, &iHandle, &iRect);
	SetDialogItem(mainWindow.ptr, mwInputFilename, iType, (Handle) &DrawFilename, &iRect);
	
	GetDialogItem(mainWindow.ptr, mwOutputFilename, &iType, &iHandle, &iRect);
	SetDialogItem(mainWindow.ptr, mwOutputFilename, iType, (Handle) &DrawFilename, &iRect);
	
	UpdateMainWindow();
	
	ShowWindow(mainWindow.ptr);
}

#pragma segment Main
void UpdateMainWindow(void) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	
	Boolean		isReady = true;
	Str255		numStr;
	Str255		displayString;
	
	GetDialogItem(mainWindow.ptr, mwInputRadioFile, &iType, &iHandle, &iRect);
	SetControlValue((ControlHandle)iHandle, gPrefs.inputSource == kInputSourceFile);
	
	GetDialogItem(mainWindow.ptr, mwInputRadioInternet, &iType, &iHandle, &iRect);
	SetControlValue((ControlHandle)iHandle, gPrefs.inputSource == kInputSourceURL);
	
	if (gPrefs.inputSource == kInputSourceFile) {
		if (gState.inputIsValid) {
			NumToString(gState.totalEntries, numStr);
			StringInsert("\pLoaded ^0 entries", numStr, displayString);
		
			SetDialogItemText(mainWindow.inputStatus, displayString);
		
			SetControlTitle(mainWindow.inputButton, "\pClose");
			HideDialogItem(mainWindow.ptr, mwInputIconNoInternet);
	 		HideDialogItem(mainWindow.ptr, mwInputIconInternet);
			ShowDialogItem(mainWindow.ptr, mwInputIcon);
	 		HideDialogItem(mainWindow.ptr, mwInputIconBlank);
		} else {
			SetDialogItemText(mainWindow.inputStatus, "\p");
		
			SetControlTitle(mainWindow.inputButton, "\pOpen File...");
			HideDialogItem(mainWindow.ptr, mwInputIconNoInternet);
			HideDialogItem(mainWindow.ptr, mwInputIconInternet);
			HideDialogItem(mainWindow.ptr, mwInputIcon);
			ShowDialogItem(mainWindow.ptr, mwInputIconBlank);
			isReady = false;
		}

	} else if (gPrefs.inputSource == kInputSourceURL) {
		if (gState.inputIsValid) {
			NumToString(gState.totalEntries, numStr);
			StringInsert("\pLoaded ^0 entries", numStr, displayString);
		
			SetDialogItemText(mainWindow.inputStatus, displayString);
		
			SetControlTitle(mainWindow.inputButton, "\pOpen URL...");
			HideDialogItem(mainWindow.ptr, mwInputIconNoInternet);
			ShowDialogItem(mainWindow.ptr, mwInputIconInternet);
			HideDialogItem(mainWindow.ptr, mwInputIcon);
			HideDialogItem(mainWindow.ptr, mwInputIconBlank);
		} else {
			SetDialogItemText(mainWindow.inputStatus, "\p");
			SetControlTitle(mainWindow.inputButton, "\pOpen URL...");
			ShowDialogItem(mainWindow.ptr, mwInputIconNoInternet);
			HideDialogItem(mainWindow.ptr, mwInputIconInternet);
			HideDialogItem(mainWindow.ptr, mwInputIcon);
			HideDialogItem(mainWindow.ptr, mwInputIconBlank);
			isReady = false;
		}
	}
	
	if (gState.outputIsValid == false) {
		SetDialogItemText(mainWindow.outputStatus, "\p");
		SetControlTitle(mainWindow.outputButton, "\pOpen File...");
		HideDialogItem(mainWindow.ptr, mwOutputIcon);
		ShowDialogItem(mainWindow.ptr, mwOutputIconBlank);
		isReady = false;
	} else {
		StringInsert("\pFound IP tunnel on \"^0\"", gState.resourceName, displayString);
		
		SetDialogItemText(mainWindow.outputStatus, displayString);
		SetControlTitle(mainWindow.outputButton, "\pClose");
		ShowDialogItem(mainWindow.ptr, mwOutputIcon);
		HideDialogItem(mainWindow.ptr, mwOutputIconBlank);
	}
	
	HiliteControl(mainWindow.doItButton, isReady ? 0 : 255);
	
	RedrawMainWindow();
}

#pragma segment Main
void RedrawMainWindow(void) {

	//BeginUpdate(mainWindow.ptr);
	UpdateDialog(mainWindow.ptr, mainWindow.ptr->visRgn);
	//EndUpdate(mainWindow.ptr);
}

#pragma segment Main
pascal void DrawFilename(WindowPtr theWindow, short itemNo) {
	OSErr		error;
	
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	
	short		length = 0;
	Str63		errorName = "\p<filename unknown>";
	Str63		defaultName = "\p<no file open>";
	Str63		defaultNameURL = "\p<url not loaded>";
	Str63		fetchName = "\p";
	Handle		fileNameHandle;
	
	Str255		urlHost;
	Str255		urlPath;
	
	if (itemNo == mwInputFilename && gPrefs.inputSource == kInputSourceURL) {
	
		if (gState.inputIsValid) {
			ParseURL(urlHost, urlPath);
			length = urlHost[0];
			PtrToHand(&urlHost[1], &fileNameHandle, length);		
		} else {
			length = defaultNameURL[0];
			PtrToHand(&defaultNameURL[1], &fileNameHandle, length);
		}
		
	} else if (itemNo == mwInputFilename && gState.inputIsValid && gPrefs.inputFileAlias != nil) {
		error = GetAliasInfo(gPrefs.inputFileAlias, asiAliasName, fetchName);
		
		if (error == noErr) {
			length = fetchName[0];
			PtrToHand(&fetchName[1], &fileNameHandle, length);
		} else {
			length = errorName[0];
			PtrToHand(&errorName[1], &fileNameHandle, length);
		}
		
	} else if (itemNo == mwOutputFilename && gState.outputIsValid) {
		error = GetAliasInfo(gPrefs.outputFileAlias, asiAliasName, fetchName);
		
		if (error == noErr) {
			length = fetchName[0];
			PtrToHand(&fetchName[1], &fileNameHandle, length);
		} else {
			length = errorName[0];
			PtrToHand(&errorName[1], &fileNameHandle, length);
		}

	} else {
		length = defaultName[0];
		PtrToHand(&defaultName[1], &fileNameHandle, length);
	}
	
	GetDialogItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	
	TextFont(geneva);
	TextSize(10);
	
	HLock(fileNameHandle);
	TETextBox(*fileNameHandle, length, &iRect, teJustCenter);
	HUnlock(fileNameHandle);
	
	TextFont(systemFont);
	TextSize(12);
}


#pragma segment Main
void DoMainWindowEvent(short whichItem) {
	switch (whichItem) {
		case mwInputButton:
			if (gPrefs.inputSource == kInputSourceURL) {
				ShowOpenURLWindow();
			} else if (gState.inputIsValid) {
				AbortInputFile();
			} else {
				PickInputFile();
			}
			break;
			
		case mwOutputButton:
			if (gState.outputIsValid) {
				AbortOutputFile();
			} else {
				PickOutputFile();
			}
			break;
			
		case mwDoItButton:
			if (WriteOutputFile()) {
				AlertInfoMessage("\pChanges successfully written!", noErr);
			}
			break;
		
		case mwInputRadioFile:
			if (gPrefs.inputSource == kInputSourceURL) {
				gPrefs.inputSource = kInputSourceFile;
				LoadCurrentInput();
				SavePreferences();
			}
			break;
			
		case mwInputRadioInternet:
			if (gPrefs.inputSource == kInputSourceFile) {
				gPrefs.inputSource = kInputSourceURL;
				LoadCurrentInput();
				SavePreferences();
			}
			break;
	}
	
	UpdateMainWindow();
}




#pragma segment Main
void PickInputFile(void)
{
	OSErr					error;
	SFTypeList				typeList;
	StandardFileReply		reply;
	
	typeList[0] = 'TEXT';
	StandardGetFile(NULL, 1, typeList, &reply);
	if (!reply.sfGood) {
		return;
	}

	error = NewAlias(NULL, &reply.sfFile, &gPrefs.inputFileAlias);
	
	if (error != noErr) {
		AbortInputFile();
	}
	
	LoadCurrentInput();
	
	SavePreferences();
}

#pragma segment Main
void PickOutputFile(void)
{
	OSErr					error;
	SFTypeList				typeList;
	StandardFileReply		reply;
	
	typeList[0] = 'ARSD';
	StandardGetFile(NULL, 1, typeList, &reply);
	if (!reply.sfGood) {
		return;
	}
	
	error = NewAlias(NULL, &reply.sfFile, &gPrefs.outputFileAlias);
	
	if (error != noErr) {
		AbortOutputFile();
	}
	
	LoadCurrentOutput();
	
	SavePreferences();
}


