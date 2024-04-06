/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#	MultiFinder-Aware Simple Sample Application
#	Sample.c	-	C Source
#	Copyright © 1989-1991, 1994-95 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/


#include "SysTypes.r"
#include "Types.r"

#include "AIRConfig.h"

Include "Icon.rsrc";

resource 'vers' (1) {
	0x00, 0x01, release, 0x00,
	verUS,
	"0.1",
	"0.1 by @kalleboo@bitbang.social"
};


/* this is a definition for a resource which contains only a rectangle */

type 'RECT' {
	rect;
};


/* we use an MBAR resource to conveniently load all the menus */

resource 'MBAR' (rMenuBar, preload) {
	{ mApple, mFile, mEdit };	/* four menus */
};


resource 'MENU' (mApple, preload) {
	mApple, textMenuProc,
	AllItems & ~MenuItem2,	/* Disable dashed line, enable About and DAs */
	enabled, apple,
	{
		"About Sample…",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain
	}
};

resource 'MENU' (mFile, preload) {
	mFile, textMenuProc,
	MenuItem12,				/* enable Quit only, program enables others */
	enabled, "File",
	{
		"New",
			noicon, "N", nomark, plain;
		"Open",
			noicon, "O", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Close",
			noicon, "W", nomark, plain;
		"Save",
			noicon, "S", nomark, plain;
		"Save As…",
			noicon, nokey, nomark, plain;
		"Revert",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Page Setup…",
			noicon, nokey, nomark, plain;
		"Print…",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Quit",
			noicon, "Q", nomark, plain
	}
};

resource 'MENU' (mEdit, preload) {
	mEdit, textMenuProc,
	NoItems,				/* disable everything, program does the enabling */
	enabled, "Edit",
	 {
		"Undo",
			noicon, "Z", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Cut",
			noicon, "X", nomark, plain;
		"Copy",
			noicon, "C", nomark, plain;
		"Paste",
			noicon, "V", nomark, plain;
		"Clear",
			noicon, nokey, nomark, plain
	}
};


/* this ALRT and DITL are used as an About screen */

resource 'ALRT' (rAboutAlert, purgeable) {
	{40, 20, 160, 297},
	rAboutAlert,
	{ /* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	centerMainScreen
};

resource 'DITL' (rAboutAlert, purgeable) {
	{ /* array DITLarray: 5 elements */
		/* [1] */
		{88, 185, 108, 265},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{8, 8, 24, 214},
		StaticText {
			disabled,
			"Simple Sample (Traffic Light)"
		},
		/* [3] */
		{32, 8, 48, 296},
		StaticText {
			disabled,
			"Copyright © Apple Computer 1989-1990"
		},
		/* [4] */
		{56, 8, 72, 136},
		StaticText {
			disabled,
			"Brought to you by:"
		},
		/* [5] */
		{80, 24, 112, 167},
		StaticText {
			disabled,
			"Macintosh Developer  Technical Support"
		}
	}
};


/* this ALRT and DITL are used as an error screen */

resource 'ALRT' (rUserAlert, purgeable) {
	{40, 20, 150, 260}, /* top left height width */
	rUserAlert,
	{ /* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	centerMainScreen
};


resource 'DITL' (rUserAlert, purgeable) {
	{ /* array DITLarray: 3 elements */
		/* [1] */
		{80, 150, 105, 225},  /* top left bottom right */
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 60, 60, 230},
		StaticText {
			disabled,
			"Error occurred: ^0^1^2^3"
		},
		/* [3] */
		{8, 8, 40, 40},
		Icon {
			disabled,
			2
		}
	}
};


resource 'WIND' (rWindow, preload, purgeable) {
	{60, 40, 290, 160},
	noGrowDocProc, visible, noGoAway, 0x0, "Traffic", noAutoCenter
};


/* here is the quintessential MultiFinder friendliness device, the SIZE resource */

resource 'SIZE' (-1) {
	dontSaveScreen,
	acceptSuspendResumeEvents,
	enableOptionSwitch,
	canBackground,				/* we can background; we don't currently, but our sleep value */
								/* guarantees we don't hog the Mac while we are in the background */
	multiFinderAware,			/* this says we do our own activate/deactivate; don't fake us out */
	backgroundAndForeground,	/* this is definitely not a background-only application! */
	dontGetFrontClicks,			/* change this is if you want "do first click" behavior like the Finder */
	ignoreChildDiedEvents,		/* essentially, I'm not a debugger (sub-launching) */
	not32BitCompatible,			/* this app should not be run in 32-bit address space */
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	kPrefSize * 1024,
	kMinSize * 1024	
};
