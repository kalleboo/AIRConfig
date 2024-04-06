



#pragma segment Main
Boolean IsValidChar(char c) {
	return c == '.' || c == '-' || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9');
}


#pragma segment Main
Boolean ParseInput(void) {
	int         i;
	short		thisEntryLength;
	short       loopMode = 0;
	char*			string;
	unsigned long	stringLength;
	
	//Calculate the number of lines and the info needed for output length
	gState.totalEntries = 0;
	gState.totalTextLength = 0;
	
	HLock(gState.loadedInputString);
	string = *gState.loadedInputString;
	stringLength = GetHandleSize(gState.loadedInputString);
	
	thisEntryLength = 0;
	
	for (i = 0; i < stringLength; i++) {
		if (loopMode == 1 && IsDelimiter(string[i])) {
			continue;
		}
		
		loopMode = 0;
		
		if (IsDelimiter(string[i])) {
			loopMode = 1;
			if (thisEntryLength > 0) {
				gState.totalEntries++;
			}
			thisEntryLength = 0;
			continue;
		}
		
		if (!IsValidChar(string[i])) {
			gState.totalEntries = 0;
			gState.totalTextLength = 0;
			AlertErrorMessage("\pInvalid character found in input. Is this really a list of IPs and domains?", noErr);
			HUnlock(gState.loadedInputString);
			return false;
		}
		
		gState.totalTextLength++;
		thisEntryLength++;
	}
	
	if (thisEntryLength > 0) {
		gState.totalEntries++;
	}
	
	HUnlock(gState.loadedInputString);
		
	return gState.totalEntries > 0;
}



#pragma segment Main
Boolean LoadInputFile(void)
{
	OSErr 		error;
	
	FSSpec		inFileSpec;
	Boolean		wasChanged;
	
	short		inFileRef;
	
	long			readLength;
	Ptr				readPointer;

	
	if (gPrefs.inputFileAlias == nil) {
		return false;
	}
	
	error = ResolveAlias(nil, gPrefs.inputFileAlias, &inFileSpec, &wasChanged);
	if (error != noErr) {
		AlertErrorMessage("\pError finding input file ", error);
		return false;
	}
	
	if (wasChanged) {
		SavePreferences();
	}
		
	error = FSpOpenDF(&inFileSpec, fsRdPerm, &inFileRef);
	if (error != noErr) {
		AlertErrorMessage("\pCant open input file", error);
		return false;
	}
	
	SetFPos(inFileRef, fsFromStart, 0);
	GetEOF(inFileRef, &readLength);
	readPointer = NewPtr(readLength);
	FSRead(inFileRef, &readLength, readPointer);
	FSClose(inFileRef);
	
	SetInputString(readPointer, readLength);
	
	DisposPtr(readPointer);
	
	return true;
}


#pragma segment Main
Boolean LoadOutputFile(void)
{
	OSErr		error;
	
	FSSpec		outFileSpec;
	Boolean		wasChanged;
	
	short		outFileRef;
	
	Handle      existingRes;
	short       existingResId;
	ResType     existingResType;
	Str255      existingResName;
	
	if (gPrefs.outputFileAlias == nil) {
		return false;
	}

	error = ResolveAlias(nil, gPrefs.outputFileAlias, &outFileSpec, &wasChanged);
	if (error != noErr) {
		AlertErrorMessage("\pError finding output file ", error);
		return false;
	}
	
	if (wasChanged) {
		SavePreferences();
	}
		
	outFileRef = FSpOpenResFile(&outFileSpec, fsRdPerm);
	if (outFileRef == -1) {
		AlertErrorMessage("\pError opening output file for writing ", ResError());
		return false;
	}
	
	UseResFile(outFileRef);
	
	existingResId = GetTunnelResId();
	if (existingResId == -1) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", ResError());
		return false;
	}
	
	existingRes = Get1Resource('acfg', existingResId);
	if (existingRes == NULL) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", ResError());
		return false;
	}
	
	GetResInfo(existingRes, &existingResId, &existingResType, existingResName);
	GetResAttrs(existingRes);
	
	gState.resourceId = existingResId;
	BlockMove(existingResName, gState.resourceName, sizeof(Str255));
	
	CloseResFile(outFileRef);
	
	return true;
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
	
	char*			string;
	unsigned long	stringLength;
	
	HLock(gState.loadedInputString);
	string = *gState.loadedInputString;
	stringLength = GetHandleSize(gState.loadedInputString);
	
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
	thisStringStartSrc = string;
	thisStringStartDest = writeDataPointer;
	writeDataPointer++; /* reserve space for length */
	
	for (i = 0; i < stringLength; i++) {
		if (loopMode == 1 && IsDelimiter(string[i])) {
			continue;
		}
		
		loopMode = 0;
		
		if (IsDelimiter(string[i])) {
			//End the previous string by setting its length
			if (thisStringLength > 0) {		
				thisStringStartDest[0] = thisStringLength;
			}
			
			//Start the next string
			thisStringLength = 0;
			thisStringStartSrc = string + i;
			thisStringStartDest = writeDataPointer;
			writeDataPointer++; /* reserve space for length */
				
			loopMode = 1;
			continue;
		}
		
		//Copy a char
		*writeDataPointer = string[i];
		writeDataPointer++;
		thisStringLength++;
	}
	
	if (thisStringLength > 0) {		
		thisStringStartDest[0] = thisStringLength;
	}
	
	HUnlock(writeData);
	writeDataPointer = 0;
	
	HUnlock(gState.loadedInputString);
	
	return writeData;
}


#pragma segment Main
Boolean WriteOutputFile(void)
{
	OSErr		error;
	
	FSSpec		outFileSpec;
	Boolean		wasChanged;
	
	Handle		writeData;
	short		outFileRef;
	
	Handle      existingRes;
	short       existingResId;
	ResType     existingResType;
	Str255      existingResName;
	
	Str255		numStr;
	Str255		displayString;
	
	writeData = FormatResource();
	
	if (gPrefs.outputFileAlias == nil) {
		return false;
	}

	error = ResolveAlias(nil, gPrefs.outputFileAlias, &outFileSpec, &wasChanged);
	if (error != noErr) {
		AlertErrorMessage("\pError finding output file ", error);
		return false;
	}
	
	outFileRef = FSpOpenResFile(&outFileSpec, fsRdWrPerm);
	if (outFileRef == -1) {
		AlertErrorMessage("\pError opening output file for writing ", ResError());
		AbortOutputFile();
		return false;
	}
	
	UseResFile(outFileRef);
	
	existingRes = Get1Resource('acfg', gState.resourceId);
	if (existingRes == NULL) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", ResError());
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
		AlertErrorMessage("\pError adding resource ", ResError());
		AbortOutputFile();
		return false;
	}
	
	//Write out comment
	
	NumToString(gState.totalEntries, numStr);
	StringInsert("\pAIRConfig for GlobalTalk (^0 entries)", numStr, displayString);
	PtrToHand(displayString, &writeData, displayString[0] + 1);
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
