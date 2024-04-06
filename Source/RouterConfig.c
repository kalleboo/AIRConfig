

void LoadInputFile( void );
void LoadOutputFile( void );

Handle FormatResource(void);
short GetTunnelResId(void);
Boolean IsDelimiter(char c);




#pragma segment Main
void LoadInputFile(void)
{
	short		inFileRef;
	OSErr 		error;
	int         i;
	short		thisEntryLength;
	short       loopMode = 0;
	
	error = FSpOpenDF(&gState.inFileSpec, fsRdPerm, &inFileRef);
	if (error != noErr) {
		AlertErrorMessage("\pCant open input file", error);
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
	
	thisEntryLength = 0;
	
	for (i = 0; i < gState.inputStringLength; i++) {
		if (loopMode == 1 && IsDelimiter(gState.inputString[i])) {
			continue;
		}
		
		loopMode = 0;
		
		if (IsDelimiter(gState.inputString[i])) {
			loopMode = 1;
			if (thisEntryLength > 0) {
				gState.totalEntries++;
			}
			thisEntryLength = 0;
			continue;
		}
		
		gState.totalTextLength++;
		thisEntryLength++;
	}
	
	if (thisEntryLength > 0) {
		gState.totalEntries++;
	}
}


#pragma segment Main
void LoadOutputFile(void)
{
	short		outFileRef;
	
	Handle      existingRes;
	short       existingResId;
	ResType     existingResType;
	Str255      existingResName;
	
	outFileRef = FSpOpenResFile(&gState.outFileSpec, fsRdPerm);
	if (outFileRef == -1) {
		AlertErrorMessage("\pError opening output file for writing ", ResError());
		AbortOutputFile();
		return;
	}
	
	UseResFile(outFileRef);
	
	existingResId = GetTunnelResId();
	if (existingResId == -1) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", ResError());
		AbortOutputFile();
		return;
	}
	
	existingRes = Get1Resource('acfg', existingResId);
	if (existingRes == NULL) {
		AlertErrorMessage("\pCould not find existing IPTunnel configuration", ResError());
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
			if (thisStringLength > 0) {		
				thisStringStartDest[0] = thisStringLength;
			}
			
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
	
	if (thisStringLength > 0) {		
		thisStringStartDest[0] = thisStringLength;
	}
	
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
	
	
	writeData = FormatResource();
	
	outFileRef = FSpOpenResFile(&gState.outFileSpec, fsRdWrPerm);
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
