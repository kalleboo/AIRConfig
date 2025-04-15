/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#	MultiFinder-Aware Simple Sample Application
#	Sample.c	-	C Source
#	Copyright © 1989-1991, 1994-95 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

#include <string.h>
#include <limits.h>

#include "Defines.h"

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
//QDGlobals qd;


#include "Types.c"

AppState 		gState;
AppPrefs		gPrefs;
MainWindow 		mainWindow;
DialogPtr		prefsWindowPtr;
DialogPtr		openURLWindowPtr;
DialogPtr		busyWindowPtr;


extern void SetInputString(char *newInputString, unsigned long length);
extern Boolean isOptionKeyPressed();
extern void LoadPreferences(void);
extern void SavePreferences(void);
extern void ShowMainWindow(void);
extern void ShowHelpDialog(void);
extern void ShowLoadingBusyDialog(void);
extern void HideBusyDialog(void);
extern void UpdateMainWindow(void);
extern void PickInputFile(void);
extern void ShowAboutDialog(void);
extern void ShowPrefsWindow(void);
extern void FatalAlertMessage( Str255 message, OSErr errorCode );
extern void AlertInfoMessage(Str255 message, OSErr errorCode);
extern Boolean LoadInputURL(void);
extern Boolean ParseInput(void);

extern void DoMainWindowEvent(short whichItem);
extern void DoPrefsWindowEvent(short whichItem);
extern void DoOpenURLWindowEvent(short whichItem);


void SetInputString(char *newInputString, unsigned long length);
void ClearInputString(void);

void LoadCurrentInput(void);
void LoadCurrentOutput(void);

void AbortInputFile(void);
void AbortOutputFile(void);

void EventLoop( void );
void DoEvent( EventRecord *event );
void AdjustCursor( Point mouse, RgnHandle region );
pascal void GetGlobalMouse( Point *mouse );
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
Boolean GiveTime(short sleepTime);


Boolean LoadInputFile( void );
Boolean LoadOutputFile( void );

Handle FormatResource(void);
Boolean WriteOutputFile( void );

short GetTunnelResId(void);
Boolean IsDelimiter(char c);



/* this routine would normally be a callback for giving time to background
	apps */

Cursor watchCurs;

Boolean GiveTime(short sleepTime)
{
	//SetCursor(&watchCurs);
	short x = sleepTime;
	return true;
}


/* Define HiWrd and LoWrd macros for efficiency. */
#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define LoWrd(aLong)	((aLong) & 0xFFFF)

/* Define TopLeft and BotRight macros for convenience. Notice the implicit
   dependency on the ordering of fields within a Rect */
#define TopLeft(aRect)	(* (Point *) &(aRect).top)
#define BotRight(aRect)	(* (Point *) &(aRect).bottom)


//extern void _DataInit(void);

/* This routine is part of the MPW runtime library. This external
   reference to it is done so that we can unload its segment, %A5Init. */


#pragma segment Main
void main(void)
{

//	UnloadSeg((Ptr) _DataInit);		/* note that _DataInit must not be in Main! */
	
	/* 1.01 - call to ForceEnvirons removed */
	
	/*	If you have stack requirements that differ from the default,
		then you could use SetApplLimit to increase StackSpace at 
		this point, before calling MaxApplZone. */
	MaxApplZone();					/* expand the heap so code segments load at the top */

	Initialize();					/* initialize the program */
	
	UnloadSeg((Ptr) Initialize);	/* note that Initialize must not be in Main! */
	
	if (isOptionKeyPressed()) { 
		gPrefs.hasSeenHelp = true;
		SavePreferences();
	} else {
		LoadPreferences();
	}
	
	LoadCurrentInput();
	LoadCurrentOutput();
	
	if (gPrefs.headlessMode) {
		if (gState.inputIsValid
			&& gState.outputIsValid
			&& gState.totalEntries > 0
			&& WriteOutputFile()) {
			
			ExitToShell();
			return;
		} else {
			gPrefs.headlessMode = false;
			SavePreferences();
		}
	}
	
	ShowMainWindow();
	
	if (gPrefs.hasSeenHelp == false) {
		ShowHelpDialog();
	}
	
	//HTTPTest();
	
	EventLoop();					/* call the main event loop */
}


#pragma segment Main 
void LoadCurrentInput(void) {
	gState.inputIsValid = false;
	ClearInputString();
	gState.totalEntries = 0;
	gState.totalTextLength = 0;
	
	ShowLoadingBusyDialog();
	
	if (gPrefs.inputSource == kInputSourceFile) {
		gState.inputIsValid = LoadInputFile() && ParseInput();
		
		if (!gState.inputIsValid) {
			AbortInputFile();
		}
		
	} else if (gPrefs.inputSource == kInputSourceURL) {
		gState.inputIsValid = LoadInputURL() && ParseInput();
		HideBusyDialog();
		
		if (!gState.inputIsValid) {
			gState.inputIsValid = false;
			ClearInputString();
			gState.totalEntries = 0;
			gState.totalTextLength = 0;
		}
	}
	
	HideBusyDialog();
}

#pragma segment Main 
void LoadCurrentOutput(void) {
	gState.outputIsValid = LoadOutputFile();
	
	if (!gState.outputIsValid) {
		AbortOutputFile();
	}
}


#pragma segment Main
void AbortInputFile(void) {
	gState.inputIsValid = false;
	gPrefs.inputFileAlias = nil;
	ClearInputString();
	gState.totalEntries = 0;
	gState.totalTextLength = 0;
	
	SavePreferences();
}


#pragma segment Main
void AbortOutputFile(void) {
	gState.outputIsValid = false;
	gPrefs.outputFileAlias = nil;
	gState.resourceId = 0;
	gState.resourceName[0] = 0;
	
	SavePreferences();
}

#pragma segment Main
void SetInputString(char *newInputString, unsigned long length) {
	ClearInputString();
	
	gState.loadedInputString = NewHandle(length);
	
	HLock(gState.loadedInputString);
	BlockMove(newInputString, *gState.loadedInputString, length);
	HUnlock(gState.loadedInputString);
}

#pragma segment Main
void ClearInputString(void) {
	if (gState.loadedInputString == nil) { return; }
	
	DisposeHandle(gState.loadedInputString);
	gState.loadedInputString = nil;
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
			} else if (whichDialog == prefsWindowPtr) {
				DoPrefsWindowEvent(whichItem);
			} else if (whichDialog == openURLWindowPtr) {
				DoOpenURLWindowEvent(whichItem);
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
				case inGoAway:
					if (TrackGoAway(window, event->where)) {
						if (window == mainWindow.ptr) {
							ExitToShell();
						} else {
							DoCloseWindow(window);
						}
					}
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
pascal void GetGlobalMouse(Point *mouse)
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

	menu = GetMenuHandle(mApple);
	EnableItem(menu, iHelp);
	
	menu = GetMenuHandle(mFile);
	if ( IsDAWindow(window) )		/* we can allow desk accessories to be closed from the menu */
		EnableItem(menu, iClose);
	else {
		EnableItem(menu, iOpen);
		
		if (gState.inputIsValid || FrontWindow() != mainWindow.ptr) {
			EnableItem(menu, iClose);	
		} else {
			DisableItem(menu, iClose);
		}

		if (gState.inputIsValid && gState.outputIsValid) {
			EnableItem(menu, iSave);
		} else {
			DisableItem(menu, iSave);
		}
	}
	EnableItem(menu, iQuit);

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
		EnableItem(menu, iPrefs);
	}
	
	
	

} /*AdjustMenus*/

 
#pragma segment Main
void DoMenuCommand(long	menuResult)
{
	short		menuID;				/* the resource ID of the selected menu */
	short		menuItem;			/* the item number of the selected menu */
	Str255		daName;
	short		daRefNum;
	Boolean		handledByDA;

	menuID = HiWord(menuResult);	/* use macros for efficiency to... */
	menuItem = LoWord(menuResult);	/* get menu item number and menu number */
	switch ( menuID ) {
		case mApple:
			switch ( menuItem ) {
				case iAbout:		/* bring up alert for About */
					ShowAboutDialog();
					break;
				case iHelp:		/* bring up alert for About */
					ShowHelpDialog();
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
				case iOpen:
					gPrefs.inputSource = kInputSourceFile;
					PickInputFile();
					UpdateMainWindow();
					break;
				case iClose:
					if (FrontWindow() == mainWindow.ptr) {
						gPrefs.inputSource = kInputSourceFile;
						AbortInputFile();
						UpdateMainWindow();
					} else {
						DoCloseWindow(FrontWindow());
					}
					break;
				case iSave:
					if (WriteOutputFile()) {
						AlertInfoMessage("\pSuccessfully saved changes.", noErr);
					}
					break;
				case iQuit:
					Terminate();
					break;
			}
			break;
		case mEdit:					/* call SystemEdit for DA editing & MultiFinder */
			switch ( menuItem ) {
				case iPrefs:
					ShowPrefsWindow();
					break;
				default:
					handledByDA = SystemEdit(menuItem-1);	/* since we donÕt do any Editing */
					break;
			}
			break;
		case kHMHelpMenuID:
			ShowHelpDialog();
			break;
	}
	HiliteMenu(0);					/* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/


#pragma segment Main
Boolean DoCloseWindow(WindowPtr	window)
{
	if ( IsDAWindow(window) )
		CloseDeskAcc(((WindowPeek) window)->windowKind);
	else if ( IsAppWindow(window) || window == prefsWindowPtr)
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
	long		total, contig;
	EventRecord event;
	short		count;
	OSErr		err;
	MenuHandle	mh;
	CursHandle	cursH;

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
	
	if (gMac.machineType < 0) FatalAlertMessage("\pOS too old.", noErr);
		
	gHasWaitNextEvent = TrapAvailable(_WaitNextEvent, ToolTrap);
	 
	if ((long) GetApplLimit() - (long) ApplicationZone() < kMinHeap) FatalAlertMessage("\pNot enough RAM", noErr);

	PurgeSpace(&total, &contig);
	if (total < kMinSpace) FatalAlertMessage("\pNot enough RAM", noErr);
		
	menuBar = GetNewMBar(rMenuBar);			/* read menus into menu bar */
	if ( menuBar == nil ) FatalAlertMessage("\pNot enough RAM", noErr);
	SetMenuBar(menuBar);					/* install menus */
	DisposeHandle(menuBar);

	AppendResMenu(GetMenuHandle(mApple), 'DRVR');	/* add DA names to Apple menu */
	
	err = HMGetHelpMenuHandle(&mh);
	if ((err == noErr) && mh) {
		AppendMenu(mh, "\pAIRConfig HelpÉ");
	}
	
	DrawMenuBar();
	
	cursH = GetCursor(watchCursor);
	HLock((Handle)cursH);
	watchCurs = **cursH;
	
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


