#include "Globals.h"
#include "Defines.h"

void FatalAlertMessage( Str255 message, OSErr errorCode );
void FatalAlert( void );

void AlertErrorMessage(Str255 message, OSErr errorCode);
void AlertWarningMessage(Str255 message, OSErr errorCode);
void AlertInfoMessage(Str255 message, OSErr errorCode);



#pragma segment Main
void FatalAlertMessage(Str255 message, OSErr errorCode)
{
	short		itemHit;
	Str255		param2 = "\p";
	
	if (errorCode != noErr) {
		NumToString(errorCode, param2);
	}

	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rFatalAlert, nil);
	ExitToShell();
} /* AlertUser */


#pragma segment Main
void AlertErrorMessage(Str255 message, OSErr errorCode)
{
	short		itemHit;
	Str255		param2 = "\p";

	if (errorCode != noErr) {
		NumToString(errorCode, param2);
	}
	
	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rFatalAlert, nil);
} /* AlertUser */

#pragma segment Main
void AlertWarningMessage(Str255 message, OSErr errorCode)
{
	short		itemHit;
	Str255		param2 = "\p";

	if (errorCode != noErr) {
		NumToString(errorCode, param2);
	}
	
	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rWarningAlert, nil);
} /* AlertUser */

#pragma segment Main
void AlertInfoMessage(Str255 message, OSErr errorCode)
{
	short		itemHit;
	Str255		param2 = "\p";

	if (errorCode != noErr) {
		NumToString(errorCode, param2);
	}
	
	SetCursor(&qd.arrow);
	ParamText(message, param2, nil, nil);
	itemHit = Alert(rInfoAlert, nil);
} /* AlertUser */
