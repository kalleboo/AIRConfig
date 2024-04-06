/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#	MultiFinder-Aware Simple Sample Application
#	Sample.c	-	C Source
#	Copyright © 1989-1991, 1994-95 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

/*	These #defines correspond to values defined in the Pascal source code.
	Sample.c and Sample.r include this file. */
	
#define kMinSize	23				/* application's minimum size (in K) */

/*	We made the preferred size bigger than the minimum size by 12K, so that
	there would be even more room for the scrap, FKEYs, etc. */

#define kPrefSize	35				/* application's preferred size (in K) */

#define	rMenuBar	128				/* application's menu bar */
#define	rAboutAlert	128				/* about alert */
#define	rUserAlert	129				/* error user alert */
#define	rWindow		128				/* application's window */

/* kSysEnvironsVersion is passed to SysEnvirons to tell it which version of the
   SysEnvRec we understand. */

#define	kSysEnvironsVersion		1

/* kOSEvent is the event number of the suspend/resume and mouse-moved events sent
   by MultiFinder. Once we determine that an event is an osEvent, we look at the
   high byte of the message sent to determine which kind it is. To differentiate
   suspend and resume events we check the resumeMask bit. */

#define	kOSEvent				app4Evt	/* event used by MultiFinder */
#define	kSuspendResumeMessage	1		/* high byte of suspend/resume event message */
#define	kResumeMask				1		/* bit of message field for resume vs. suspend */
#define	kMouseMovedMessage		0xFA	/* high byte of mouse-moved event message */
#define	kNoEvents				0		/* no events mask */

/* The following constants are used to identify menus and their items. The menu IDs
   have an "m" prefix and the item numbers within each menu have an "i" prefix. */

#define	mApple					128		/* Apple menu */
#define	iAbout					1

#define	mFile					129		/* File menu */
#define	iNew					1
#define	iClose					4
#define	iQuit					12

#define	mEdit					130		/* Edit menu */
#define	iUndo					1
#define	iCut					3
#define	iCopy					4
#define	iPaste					5
#define	iClear					6

/*	1.01 - kTopLeft - This is for positioning the Disk Initialization dialogs. */

#define kDITop					0x0050
#define kDILeft					0x0070

/*	1.01 - kMinHeap - This is the minimum result from the following
	equation:
		
		ORD(GetApplLimit) - ORD(ApplicZone)
		
	for the application to run. It will insure that enough memory will
	be around for reasonable-sized scraps, FKEYs, etc. to exist with the
	application, and still give the application some 'breathing room'.
	To derive this number, we ran under a MultiFinder partition that was
	our requested minimum size, as given in the 'SIZE' resource. */
	 
#define kMinHeap				21 * 1024
	
/*	1.01 - kMinSpace - This is the minimum result from PurgeSpace, when called
	at initialization time, for the application to run. This number acts
	as a double-check to insure that there really is enough memory for the
	application to run, including what has been taken up already by
	pre-loaded resources, the scrap, code, and other sundry memory blocks. */
	 
#define kMinSpace				8 * 1024

/* kExtremeNeg and kExtremePos are used to set up wide open rectangles and regions. */

#define kExtremeNeg				-32768
#define kExtremePos				32767 - 1 /* required to address an old region bug */

/* these #defines are used to set enable/disable flags of a menu */

#define AllItems	0b1111111111111111111111111111111	/* 31 flags */
#define NoItems		0b0000000000000000000000000000000
#define MenuItem1	0b0000000000000000000000000000001
#define MenuItem2	0b0000000000000000000000000000010
#define MenuItem3	0b0000000000000000000000000000100
#define MenuItem4	0b0000000000000000000000000001000
#define MenuItem5	0b0000000000000000000000000010000
#define MenuItem6	0b0000000000000000000000000100000
#define MenuItem7	0b0000000000000000000000001000000
#define MenuItem8	0b0000000000000000000000010000000
#define MenuItem9	0b0000000000000000000000100000000
#define MenuItem10	0b0000000000000000000001000000000
#define MenuItem11	0b0000000000000000000010000000000
#define MenuItem12	0b0000000000000000000100000000000
