/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#	MultiFinder-Aware Simple Sample Application
#	Sample.c	-	C Source
#	Copyright © 1989-1991, 1994-95 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

#define OLDROUTINENAMES 1

#include <Limits.h>
#include <Types.h>
#include <Resources.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Controls.h>
#include <Menus.h>
#include <Devices.h>
#include <ToolUtils.h>
#include <Memory.h>
#include <Processes.h>
#include <SegLoad.h>
#include <Files.h>
#include <StandardFile.h>
#include <OSUtils.h>
#include <DiskInit.h>
#include <Packages.h>
#include <Traps.h>

#include "AIRConfig.h"		/* bring in all the #defines for Sample */

/* The "g" prefix is used to emphasize that a variable is global. */

/* GMac is used to hold the result of a SysEnvirons call. This makes
   it convenient for any routine to check the environment. */
SysEnvRec	gMac;				/* set up by Initialize */

/* GHasWaitNextEvent is set at startup, and tells whether the WaitNextEvent
   trap is available. If it is false, we know that we must call GetNextEvent. */
Boolean		gHasWaitNextEvent;	/* set up by Initialize */

/* GInBackground is maintained by our osEvent handling routines. Any part of
   the program can check it to find out if it is currently in the background. */
Boolean		gInBackground;		/* maintained by Initialize and DoEvent */

/* The qd global has been removed from the libraries */
QDGlobals qd;


/* App variables */
typedef struct {
	FSSpec			outFileSpec;
	FSSpec			inFileSpec;
	
	char *			inputString;
	long			inputStringLength;
	
	long			totalEntries;
	long 			totalTextLength;
	
	Str255			resourceName;
	short			resourceId;
} AppState;

AppState 		gState;


typedef struct {
	DialogPtr		ptr;
	Handle			inputStatus;
	Handle			outputStatus;
	ControlHandle	doItButton;
	
} MainWindow;

MainWindow 		mainWindow;

#define		mwInputButton	1
#define		mwInputStatus	2
#define		mwOutputButton	3
#define		mwOutputStatus	4
#define		mwDoItButton	5
#define		mwInputTitle	6
#define		mwOutputTitle	7


/* Here are declarations for all of the C routines. In MPW 3.0 and later we can use
   actual prototypes for parameter type checking. */

void ShowMainWindow(void);
void UpdateMainWindow(void);
void DoMainWindowEvent(short whichItem);

void PickInputFile( void );
void AbortInputFile(void);
void LoadInputFile( void );
void PickOutputFile( void );
void AbortOutputFile(void);
void LoadOutputFile( void );
Boolean WriteOutputFile( void );
Handle FormatResource(void);
short GetTunnelResId(void);
Boolean IsDelimiter(char c);

void EventLoop( void );
void DoEvent( EventRecord *event );
void AdjustCursor( Point mouse, RgnHandle region );
void GetGlobalMouse( Point *mouse );
void DoUpdate( WindowPtr window );
void DoActivate( WindowPtr window, Boolean becomingActive );
void DoContentClick( WindowPtr window );
void DrawWindow( WindowPtr window );
void AdjustMenus( void );
void DoMenuCommand( long menuResult );
Boolean DoCloseWindow( WindowPtr window );
void Terminate( void );
void Initialize( void );
void ForceEnvirons( void );
Boolean IsAppWindow( WindowPtr window );
Boolean IsDAWindow( WindowPtr window );
Boolean TrapAvailable( short tNumber, TrapType tType );
void FatalAlertMessage( Str255 message, Str255 param2 );
void FatalAlert( void );

void AlertErrorMessage(Str255 message, Str255 param2);
void AlertWarningMessage(Str255 message, Str255 param2);
void AlertInfoMessage(Str255 message, Str255 param2);

Handle StringInsert(Str255 baseString, Str255 subsString);


/* Define HiWrd and LoWrd macros for efficiency. */
#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define LoWrd(aLong)	((aLong) & 0xFFFF)

/* Define TopLeft and BotRight macros for convenience. Notice the implicit
   dependency on the ordering of fields within a Rect */
#define TopLeft(aRect)	(* (Point *) &(aRect).top)
#define BotRight(aRect)	(* (Point *) &(aRect).bottom)


extern void _DataInit(void);

/* This routine is part of the MPW runtime library. This external
   reference to it is done so that we can unload its segment, %A5Init. */


#pragma segment Main
void main(void)
{

	UnloadSeg((Ptr) _DataInit);		/* note that _DataInit must not be in Main! */
	
	/* 1.01 - call to ForceEnvirons removed */
	
	/*	If you have stack requirements that differ from the default,
		then you could use SetApplLimit to increase StackSpace at 
		this point, before calling MaxApplZone. */
	MaxApplZone();					/* expand the heap so code segments load at the top */

	Initialize();					/* initialize the program */
	
	UnloadSeg((Ptr) Initialize);	/* note that Initialize must not be in Main! */
	
	ShowMainWindow();
	
	EventLoop();					/* call the main event loop */
}

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
	GetDItem(mainWindow.ptr, mwOutputStatus, &iType, &mainWindow.outputStatus, &iRect);
	GetDItem(mainWindow.ptr, mwDoItButton, &iType, &iHandle, &iRect);
	mainWindow.doItButton = (ControlHandle) iHandle;
	
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
		SetIText(mainWindow.inputStatus, "\pNo input selected");
		isReady = false;
	} else {
		NumToString(gState.totalEntries, numStr);
		formattedString = StringInsert("\pLoaded ^0 entries", numStr);
		BlockMove(*formattedString, displayString, GetHandleSize(formattedString));
		
		SetIText(mainWindow.inputStatus, displayString);
	}
	
	if (gState.outFileSpec.vRefNum == 0) {
		SetIText(mainWindow.outputStatus, "\pNo output selected");
		isReady = false;
	} else {
		formattedString = StringInsert("\pFound IP tunnel on \"^0\"", gState.resourceName);
		BlockMove(*formattedString, displayString, GetHandleSize(formattedString));
		
		SetIText(mainWindow.outputStatus, displayString);
	}
	
	HiliteControl(mainWindow.doItButton, isReady ? 0 : 255);
}

#pragma segment Main
void DoMainWindowEvent(short whichItem) {
	switch (whichItem) {
		case mwInputButton:
			PickInputFile();
			break;
			
		case mwOutputButton:
			PickOutputFile();
			break;
			
		case mwDoItButton:
			if (WriteOutputFile()) {
				AlertInfoMessage("\pSuccessfully saved changes.", nil);
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
	
	LoadInputFile();
}

#pragma segment Main
void AbortInputFile(void) {
	gState.inFileSpec.vRefNum = 0;
	gState.inputString = nil;
	gState.totalEntries = 0;
	gState.totalTextLength = 0;
}

#pragma segment Main
void LoadInputFile(void)
{
	short		inFileRef;
	OSErr 		error;
	int         i;
	short       loopMode = 0;
	
	error = FSpOpenDF(&gState.inFileSpec, fsRdPerm, &inFileRef);
	if (error != noErr) {
		AlertErrorMessage("\pCant open input file", nil);
		AbortInputFile();
		return;
	}
	
	SetFPos(inFileRef, fsFromStart, 0);
	GetEOF(inFileRef, &gState.inputStringLength);
	gState.inputString = NewPtr(gState.inputStringLength);
	FSRead(inFileRef, &gState.inputStringLength, gState.inputString);
	FSClose(inFileRef);
	
	//Calculate the number of lines and the info needed for output length
	gState.totalEntries = 0;
	gState.totalTextLength = 0;
	
	for (i = 0; i < gState.inputStringLength; i++) {
		if (loopMode == 1 && IsDelimiter(gState.inputString[i])) {
			continue;
		}
		
		loopMode = 0;
		
		if (IsDelimiter(gState.inputString[i])) {
			loopMode = 1;
			gState.totalEntries++;
			continue;
		}
		
		gState.totalTextLength++;
	}
	
	gState.totalEntries++;
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
	
	LoadOutputFile();
}


#pragma segment Main
void AbortOutputFile(void) {
	gState.outFileSpec.vRefNum = 0;
	gState.resourceId = 0;
	gState.resourceName[0] = 0;
}

#pragma segment Main
void LoadOutputFile(void)
{
	short		outFileRef;
	
	Handle      existingRes;
	short       existingResId;
	ResType     existingResType;
	Str255      existingResName;
	
	Str255 		numStr;
	
	outFileRef = FSpOpenResFile(&gState.outFileSpec, fsRdPerm);
	if (outFileRef == -1) {
		NumToString(ResError(), numStr);
		AlertErrorMessage("\pError opening output file for writing ", numStr);
		AbortOutputFile();
		return;
	}
	
	UseResFile(outFileRef);
	
	existingResId = GetTunnelResId();
	if (existingResId == -1) {
		NumToString(existingResId, numStr);
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", numStr);
		AbortOutputFile();
		return;
	}
	
	existingRes = Get1Resource('acfg', existingResId);
	if (existingRes == NULL) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", nil);
		AbortOutputFile();
		return;
	}
	
	GetResInfo(existingRes, &existingResId, &existingResType, existingResName);
	GetResAttrs(existingRes);
	
	gState.resourceId = existingResId;
	BlockMove(existingResName, gState.resourceName, sizeof(Str255));
	
	CloseResFile(outFileRef);
}

#pragma segment Main
Handle FormatResource(void) {
	
	int         i;
	short       loopMode = 0;
	
	Handle		writeData;
	char *		writeDataPointer;
	
	char *		thisStringStartSrc;
	char *		thisStringStartDest;
	long 		thisStringLength = 0;
	
	writeData = NewHandleClear(6 + gState.totalEntries + gState.totalTextLength);
	HLock(writeData);
	writeDataPointer = *writeData;
	
	//00 00 00 00     Unknown header
	writeDataPointer++;
	writeDataPointer++;
	writeDataPointer++;
	writeDataPointer++;
	
	//00 01           Number of entries
	writeDataPointer++; /* TODO make long */
	*writeDataPointer = gState.totalEntries;
	writeDataPointer++;
	
	//[list of pascal strings]
	thisStringLength = 0;
	thisStringStartSrc = gState.inputString;
	thisStringStartDest = writeDataPointer;
	writeDataPointer++; /* reserve space for length */
	
	for (i = 0; i < gState.inputStringLength; i++) {
		if (loopMode == 1 && IsDelimiter(gState.inputString[i])) {
			continue;
		}
		
		loopMode = 0;
		
		if (IsDelimiter(gState.inputString[i])) {
			//End the previous string by setting its length
			thisStringStartDest[0] = thisStringLength;
			
			//Start the next string
			thisStringLength = 0;
			thisStringStartSrc = gState.inputString + i;
			thisStringStartDest = writeDataPointer;
			writeDataPointer++; /* reserve space for length */
				
			loopMode = 1;
			continue;
		}
		
		//Copy a char
		*writeDataPointer = gState.inputString[i];
		writeDataPointer++;
		thisStringLength++;
	}
	
	thisStringStartDest[0] = thisStringLength;
	
	HUnlock(writeData);
	writeDataPointer = 0;
	
	return writeData;
}


#pragma segment Main
Boolean WriteOutputFile(void)
{
	Handle		writeData;
	short		outFileRef;
	
	Handle      existingRes;
	short       existingResId;
	ResType     existingResType;
	Str255      existingResName;
	
	Str255 		numStr;
	
	
	writeData = FormatResource();
	
	outFileRef = FSpOpenResFile(&gState.outFileSpec, fsRdWrPerm);
	if (outFileRef == -1) {
		NumToString(ResError(), numStr);
		AlertErrorMessage("\pError opening output file for writing ", numStr);
		AbortOutputFile();
		return false;
	}
	
	UseResFile(outFileRef);
	
	existingRes = Get1Resource('acfg', gState.resourceId);
	if (existingRes == NULL) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", nil);
		AbortOutputFile();
		return false;
	}
	
	GetResInfo(existingRes, &existingResId, &existingResType, existingResName);
	GetResAttrs(existingRes);
	RemoveResource(existingRes);
	
	AddResource(writeData, 'acfg', existingResId, existingResName);
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	} else {
		NumToString(ResError(), numStr);
		AlertErrorMessage("\pError adding resource ", numStr);
		AbortOutputFile();
		return false;
	}
	
	//Write out comment
	PtrToHand("\pAIRConfig for GlobalTalk", &writeData, 25);
	existingRes = Get1Resource('STR ', existingResId);
	if (existingRes != NULL) {
		GetResInfo(existingRes, &existingResId, &existingResType, existingResName);
		GetResAttrs(existingRes);
		RemoveResource(existingRes);
	
		AddResource(writeData, 'STR ', existingResId, existingResName);
	
		if (ResError() == noErr) {
			WriteResource(writeData);
		}
	}
	
	CloseResFile(outFileRef);
	
	return true;
}


#pragma segment Main
short GetTunnelResId(void)
{
	short       resId;
	ResType     resType;
	Str255      resName;
	
	short	i;
	short	countRes;
	Handle	resHnd;
	char*	resPtr;
	Boolean done = false;
	
	/* 
	    //Testing return 16384;
		The AIR file will contain resources for several port configurations,
		with ID numbers starting with 16384.
		
		How to identify which ID number refers to the IP Tunnel configuration.
		By looking at config files in ResEdit, the following can be found in common:
		
		* 'paid' resource starts with 00 01 49
		* 'port' resource starts with 04
	 */
	
	countRes = Count1Resources('paid');
	
	for (i = 1; i <= countRes; i++) {
		resHnd = Get1IndResource('paid', i);
		if (resHnd == nil) {
			continue;
		}
		
		resPtr = *resHnd;
		if (resPtr[0] == 0x00 && resPtr[1] == 0x01 && resPtr[2] == 0x49) {
			GetResInfo(resHnd, &resId, &resType, resName);
			ReleaseResource(resHnd);
			return resId;
		}
		
		ReleaseResource(resHnd);
	}
	
	return -1;
}

#pragma segment Main
Boolean IsDelimiter(char c) {
	return (c == '\n' || c == '\r' || c == ',' || c == '\t' || c == ';');
}

#pragma segment Main
void EventLoop(void)
{
	RgnHandle	cursorRgn;
	Boolean		gotEvent;
	EventRecord	event;
	Point		mouse;

	cursorRgn = NewRgn();			/* weÕll pass WNE an empty region the 1st time thru */
	do {
		/* use WNE if it is available */
		if ( gHasWaitNextEvent ) {
			GetGlobalMouse(&mouse);
			AdjustCursor(mouse, cursorRgn);
			gotEvent = WaitNextEvent(everyEvent, &event, LONG_MAX, cursorRgn);
		}
		else {
			SystemTask();
			gotEvent = GetNextEvent(everyEvent, &event);
		}
		if ( gotEvent ) {
			/* make sure we have the right cursor before handling the event */
			AdjustCursor(event.where, cursorRgn);
			DoEvent(&event);
		}
		/*	If you are using modeless dialogs that have editText items,
			you will want to call IsDialogEvent to give the caret a chance
			to blink, even if WNE/GNE returned FALSE. However, check FrontWindow
			for a non-NIL value before calling IsDialogEvent. */
	} while ( true );	/* loop forever; we quit via ExitToShell */
} /*EventLoop*/


#pragma segment Main
void DoEvent(EventRecord *event)
{
	short		part, err;
	WindowPtr	window;
	Boolean		hit;
	char		key;
	Point		aPoint;
	
	DialogPtr	whichDialog;
	short		whichItem;

	if (IsDialogEvent(event)) {
		if (DialogSelect(event, &whichDialog, &whichItem)) {
			if (whichDialog == mainWindow.ptr) {
				DoMainWindowEvent(whichItem);
			}
		}
	}
		
	switch ( event->what ) {
		case mouseDown:
			part = FindWindow(event->where, &window);
			switch ( part ) {
				case inMenuBar:				/* process a mouse menu command (if any) */
					AdjustMenus();
					DoMenuCommand(MenuSelect(event->where));
					break;
				case inSysWindow:			/* let the system handle the mouseDown */
					SystemClick(event, window);
					break;
				case inContent:
					if ( window != FrontWindow() ) {
						SelectWindow(window);
						/*DoEvent(event);*/	/* use this line for "do first click" */
					} else
						DoContentClick(window);
					break;
				case inDrag:				/* pass screenBits.bounds to get all gDevices */
					DragWindow(window, event->where, &qd.screenBits.bounds);
					break;
				case inGrow:
					break;
				case inZoomIn:
				case inZoomOut:
					hit = TrackBox(window, event->where, part);
					if ( hit ) {
						SetPort(window);				/* the window must be the current port... */
						EraseRect(&window->portRect);	/* because of a bug in ZoomWindow */
						ZoomWindow(window, part, true);	/* note that we invalidate and erase... */
						InvalRect(&window->portRect);	/* to make things look better on-screen */
					}
					break;
			}
			break;
		case keyDown:
		case autoKey:						/* check for menukey equivalents */
			key = event->message & charCodeMask;
			if ( event->modifiers & cmdKey )			/* Command key down */
				if ( event->what == keyDown ) {
					AdjustMenus();						/* enable/disable/check menu items properly */
					DoMenuCommand(MenuKey(key));
				}
			break;
		case activateEvt:
			DoActivate((WindowPtr) event->message, (event->modifiers & activeFlag) != 0);
			break;
		case updateEvt:
			DoUpdate((WindowPtr) event->message);
			break;
		/*	1.01 - It is not a bad idea to at least call DIBadMount in response
			to a diskEvt, so that the user can format a floppy. */
		case diskEvt:
			if ( HiWord(event->message) != noErr ) {
				SetPt(&aPoint, kDILeft, kDITop);
				err = DIBadMount(aPoint, event->message);
			}
			break;
		case kOSEvent:
		/*	1.02 - must BitAND with 0x0FF to get only low byte */
			switch ((event->message >> 24) & 0x0FF) {		/* high byte of message */
				case kSuspendResumeMessage:		/* suspend/resume is also an activate/deactivate */
					gInBackground = (event->message & kResumeMask) == 0;
					DoActivate(FrontWindow(), !gInBackground);
					break;
			}
			break;
	}
} /*DoEvent*/


#pragma segment Main
void AdjustCursor(Point	mouse, RgnHandle region)
{
	WindowPtr	window;
	RgnHandle	arrowRgn;
	RgnHandle	plusRgn;
	Rect		globalPortRect;

	window = FrontWindow();	/* we only adjust the cursor when we are in front */
	if ( (! gInBackground) && (! IsDAWindow(window)) ) {
		/* calculate regions for different cursor shapes */
		arrowRgn = NewRgn();
		plusRgn = NewRgn();

		/* start with a big, big rectangular region */
		SetRectRgn(arrowRgn, kExtremeNeg, kExtremeNeg, kExtremePos, kExtremePos);

		/* calculate plusRgn */
		if ( IsAppWindow(window) ) {
			SetPort(window);	/* make a global version of the viewRect */
			SetOrigin(-window->portBits.bounds.left, -window->portBits.bounds.top);
			globalPortRect = window->portRect;
			RectRgn(plusRgn, &globalPortRect);
			SectRgn(plusRgn, window->visRgn, plusRgn);
			SetOrigin(0, 0);
		}

		/* subtract other regions from arrowRgn */
		DiffRgn(arrowRgn, plusRgn, arrowRgn);

		/* change the cursor and the region parameter */
		if ( PtInRgn(mouse, plusRgn) ) {
			SetCursor(*GetCursor(plusCursor));
			CopyRgn(plusRgn, region);
		} else {
			SetCursor(&qd.arrow);
			CopyRgn(arrowRgn, region);
		}

		/* get rid of our local regions */
		DisposeRgn(arrowRgn);
		DisposeRgn(plusRgn);
	}
} /*AdjustCursor*/


#pragma segment Main
void GetGlobalMouse(Point *mouse)
{
	EventRecord	event;
	
	OSEventAvail(kNoEvents, &event);	/* we aren't interested in any events */
	*mouse = event.where;				/* just the mouse position */
} /*GetGlobalMouse*/


#pragma segment Main
void DoUpdate(WindowPtr	window)
{
	if ( IsAppWindow(window) ) {
		BeginUpdate(window);				/* this sets up the visRgn */
		if ( ! EmptyRgn(window->visRgn) )	/* draw if updating needs to be done */
			DrawWindow(window);
		EndUpdate(window);
	}
} /*DoUpdate*/


#pragma segment Main
void DoActivate(WindowPtr window, Boolean becomingActive)
{
	if ( IsAppWindow(window) ) {
		if ( becomingActive )
			/* do whatever you need to at activation */ ;
		else
			/* do whatever you need to at deactivation */ ;
	}
} /*DoActivate*/


#pragma segment Main
void DoContentClick(WindowPtr window)
{
	if (window) {
		
	}
} /*DoContentClick*/


#pragma segment Main
void DrawWindow(WindowPtr window)
{
	SetPort(window);
	
} /*DrawWindow*/


#pragma segment Main
void AdjustMenus(void)
{
	WindowPtr	window;
	MenuHandle	menu;

	window = FrontWindow();

	menu = GetMenuHandle(mFile);
	if ( IsDAWindow(window) )		/* we can allow desk accessories to be closed from the menu */
		EnableItem(menu, iClose);
	else
		DisableItem(menu, iClose);	/* but not our traffic light window */

	menu = GetMenuHandle(mEdit);
	if ( IsDAWindow(window) ) {		/* a desk accessory might need the edit menuÉ */
		EnableItem(menu, iUndo);
		EnableItem(menu, iCut);
		EnableItem(menu, iCopy);
		EnableItem(menu, iClear);
		EnableItem(menu, iPaste);
	} else {						/* Ébut we donÕt use it */
		DisableItem(menu, iUndo);
		DisableItem(menu, iCut);
		DisableItem(menu, iCopy);
		DisableItem(menu, iClear);
		DisableItem(menu, iPaste);
	}

} /*AdjustMenus*/


#pragma segment Main
void DoMenuCommand(long	menuResult)
{
	short		menuID;				/* the resource ID of the selected menu */
	short		menuItem;			/* the item number of the selected menu */
	short		itemHit;
	Str255		daName;
	short		daRefNum;
	Boolean		handledByDA;

	menuID = HiWord(menuResult);	/* use macros for efficiency to... */
	menuItem = LoWord(menuResult);	/* get menu item number and menu number */
	switch ( menuID ) {
		case mApple:
			switch ( menuItem ) {
				case iAbout:		/* bring up alert for About */
					itemHit = Alert(rAboutAlert, nil);
					break;
				default:			/* all non-About items in this menu are DAs */
					/* type Str255 is an array in MPW 3 */
					GetMenuItemText(GetMenuHandle(mApple), menuItem, daName);
					daRefNum = OpenDeskAcc(daName);
					break;
			}
			break;
		case mFile:
			switch ( menuItem ) {
				case iClose:
					DoCloseWindow(FrontWindow());
					break;
				case iQuit:
					Terminate();
					break;
			}
			break;
		case mEdit:					/* call SystemEdit for DA editing & MultiFinder */
			handledByDA = SystemEdit(menuItem-1);	/* since we donÕt do any Editing */
			break;
	}
	HiliteMenu(0);					/* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/


#pragma segment Main
Boolean DoCloseWindow(WindowPtr	window)
{
	if ( IsDAWindow(window) )
		CloseDeskAcc(((WindowPeek) window)->windowKind);
	else if ( IsAppWindow(window) )
		CloseWindow(window);
	return true;
} /*DoCloseWindow*/


#pragma segment Main
void Terminate(void)
{
/*	WindowPtr	aWindow;*/
/*	Boolean		closed;*/
/*	*/
/*	closed = true;*/
/*	do {*/
/*		aWindow = FrontWindow();				<¥ get the current front window ¥>*/
/*		if (aWindow != nil)*/
/*			closed = DoCloseWindow(aWindow);	<¥ close this window ¥>	*/
/*	}*/
/*	while (closed && (aWindow != nil));*/
/*	if (closed)*/
		ExitToShell();							/* exit if no cancellation */
} /*Terminate*/


#pragma segment Initialize
void Initialize(void)
{
	Handle		menuBar;
	/*WindowPtr	window;*/
	long		total, contig;
	EventRecord event;
	short		count;

	gInBackground = false;

	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	for (count = 1; count <= 3; count++)
		EventAvail(everyEvent, &event);
	 
	SysEnvirons(kSysEnvironsVersion, &gMac);
	
	if (gMac.machineType < 0) FatalAlertMessage("\pOS too old.", nil);
		
	gHasWaitNextEvent = TrapAvailable(_WaitNextEvent, ToolTrap);
	 
	if ((long) GetApplLimit() - (long) ApplicationZone() < kMinHeap) FatalAlertMessage("\pNot enough RAM", nil);

	PurgeSpace(&total, &contig);
	if (total < kMinSpace) FatalAlertMessage("\pNot enough RAM", nil);
	
	/*
	window = (WindowPtr) NewPtr(sizeof(WindowRecord));
	if ( window == nil ) FatalAlertMessage("\pNot enough RAM", nil);
	window = GetNewWindow(rWindow, (Ptr) window, (WindowPtr) -1);
	*/
	
	menuBar = GetNewMBar(rMenuBar);			/* read menus into menu bar */
	if ( menuBar == nil ) FatalAlertMessage("\pNot enough RAM", nil);
	SetMenuBar(menuBar);					/* install menus */
	DisposeHandle(menuBar);
	AppendResMenu(GetMenuHandle(mApple), 'DRVR');	/* add DA names to Apple menu */
	DrawMenuBar();
	
} /*Initialize*/



#pragma segment Main
Boolean IsAppWindow(WindowPtr window)
{
	short		windowKind;

	if ( window == nil )
		return false;
	else {	/* application windows have windowKinds = userKind (8) */
		windowKind = ((WindowPeek) window)->windowKind;
		return ( windowKind == userKind );
	}
} /*IsAppWindow*/


#pragma segment Main
Boolean IsDAWindow(WindowPtr window)
{
	if ( window == nil )
		return false;
	else	/* DA windows have negative windowKinds */
		return ( ((WindowPeek) window)->windowKind < 0 );
} /*IsDAWindow*/


#pragma segment Initialize
Boolean TrapAvailable(short	tNumber, TrapType tType)
{
	if ( ( tType == ToolTrap ) &&
		( gMac.machineType > envMachUnknown ) &&
		( gMac.machineType < envMacII ) ) {		/* it's a 512KE, Plus, or SE */
		tNumber = tNumber & 0x03FF;
		if ( tNumber > 0x01FF )					/* which means the tool traps */
			tNumber = _Unimplemented;			/* only go to 0x01FF */
	}
	return NGetTrapAddress(tNumber, tType) != NGetTrapAddress(_Unimplemented, ToolTrap);
} /*TrapAvailable*/


#pragma segment Main
void FatalAlertMessage(Str255 message, Str255 param2)
{
	short		itemHit;

	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rFatalAlert, nil);
	ExitToShell();
} /* AlertUser */


#pragma segment Main
void AlertErrorMessage(Str255 message, Str255 param2)
{
	short		itemHit;

	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rFatalAlert, nil);
} /* AlertUser */

#pragma segment Main
void AlertWarningMessage(Str255 message, Str255 param2)
{
	short		itemHit;

	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rWarningAlert, nil);
} /* AlertUser */

#pragma segment Main
void AlertInfoMessage(Str255 message, Str255 param2)
{
	short		itemHit;

	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rInfoAlert, nil);
} /* AlertUser */






#pragma segment Main
Handle StringInsert(Str255 baseString, Str255 subsString) {
	Handle 	baseHandle;
	Handle 	subsHandle;
	Str15 	keyStr = "\p^0";
	long	sizeL;
	long	length;
	
	Str255	output;
	Handle 	outputHand;
	
	sizeL = baseString[0];
	PtrToHand(&baseString[1], &baseHandle, sizeL);
	
	sizeL = subsString[0];
	PtrToHand(&subsString[1], &subsHandle, sizeL);
	
	ReplaceText(baseHandle, subsHandle, keyStr);
	
	length = GetHandleSize(baseHandle);
	
	if (length > 254) {
		length = 254;
	}
	
	//construct Pascal string
	output[0] = (char) length;
	BlockMove(*baseHandle, &output[1], length);
	
	//Have to return a handle
	PtrToHand(&output, &outputHand, length + 1);
	
	//This function definitely leaks memory...
	return outputHand;
}
