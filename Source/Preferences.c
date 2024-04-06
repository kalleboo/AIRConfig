

FSSpec		gPrefsFileSpec;

#define hasSeenHelpPrefID	128
#define headlessModePrefID	129

#define inputFileAlisID		201
#define outputFileAlisID	202

Boolean FindPreferences(void);
void LoadPreferences(void);
void SavePreferences(void);


#pragma segment Main
Boolean FindPreferences(void)
{
	OSErr	error;
	short	prefsFolderVRefNum;
	long	prefsFolderDirID;
	
	error = FindFolder(kOnSystemDisk, 
						kPreferencesFolderType, 
						kCreateFolder, 
						&prefsFolderVRefNum,
						&prefsFolderDirID);
	
	if (error != noErr) {
		return false;
	}
	
	error = FSMakeFSSpec(prefsFolderVRefNum, prefsFolderDirID, "\pAIRConfig Prefs", &gPrefsFileSpec);
	
	if (error != noErr && error != fnfErr) {
		return false;
	}
	
	FSpCreateResFile(&gPrefsFileSpec, 'AIRc', 'pref', smSystemScript);
	
	if (ResError() != noErr && ResError() != dupFNErr) {
		return false;
	}
	
	return true;
}

#pragma segment Main
void LoadPreferences(void)
{
	OSErr		error;
	short		fileRef;
	Handle      resHandle;
	Boolean		wasChanged;
	
	gPrefs.hasSeenHelp = false;
	
	if (!FindPreferences()) {
		return;
	}
	
	fileRef = FSpOpenResFile(&gPrefsFileSpec, fsRdPerm);
	UseResFile(fileRef);
	
	resHandle = Get1Resource('pref', hasSeenHelpPrefID);
	if (resHandle != NULL) {
		gPrefs.hasSeenHelp = **resHandle;
		ReleaseResource(resHandle);
	}
	
	resHandle = Get1Resource('pref', headlessModePrefID);
	if (resHandle != NULL) {
		gPrefs.headlessMode = **resHandle;
		ReleaseResource(resHandle);
	}
	
	resHandle = Get1Resource('alis', inputFileAlisID);
	if (resHandle != NULL) {
		error = ResolveAlias(nil, (AliasHandle)resHandle, &gState.inFileSpec, &wasChanged);
		if (error != noErr || !LoadInputFile()) {
			AbortInputFile();
		}
	}
	
	resHandle = Get1Resource('alis', outputFileAlisID);
	if (resHandle != NULL) {
		error = ResolveAlias(nil, (AliasHandle)resHandle, &gState.outFileSpec, &wasChanged);
		if (error != noErr || !LoadOutputFile()) {
			AbortOutputFile();
		}
	}
	
	CloseResFile(fileRef);
}

#pragma segment Main
void SavePreferences(void)
{
	OSErr		error;
	short		fileRef;
	Handle		writeData;
	Handle      existingRes;
	AliasHandle	aliasHandle;
	
	if (!FindPreferences()) {
		return;
	}
		
	fileRef = FSpOpenResFile(&gPrefsFileSpec, fsWrPerm);
	UseResFile(fileRef);
	
	
	//Has seen prefs
	writeData = NewHandle(1);
	**writeData = gPrefs.hasSeenHelp;
	
	existingRes = Get1Resource('pref', hasSeenHelpPrefID);
	
	if (existingRes != NULL) {
		RemoveResource(existingRes);
	}
	
	AddResource(writeData, 'pref', hasSeenHelpPrefID, "\phasSeenHelp");
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	}
	
	//Headless mode
	writeData = NewHandle(1);
	**writeData = gPrefs.headlessMode;
	
	existingRes = Get1Resource('pref', headlessModePrefID);
	
	if (existingRes != NULL) {
		RemoveResource(existingRes);
	}
	
	AddResource(writeData, 'pref', headlessModePrefID, "\pheadlessModePrefID");
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	}
	
	
	//Input file
	existingRes = Get1Resource('alis', inputFileAlisID);
	RemoveResource(existingRes);
	
	if (gState.totalEntries > 0 && gState.inFileSpec.vRefNum != 0) {
		error = NewAlias(NULL, &gState.inFileSpec, &aliasHandle);
		
		if (error == noErr && writeData != NULL) {
			AddResource((Handle)aliasHandle, 'alis', inputFileAlisID, "\pinputFile");
			
			if (ResError() == noErr) {
				WriteResource((Handle)aliasHandle);
			}
		}
	}
	
	//Output file
	existingRes = Get1Resource('alis', outputFileAlisID);
	RemoveResource(existingRes);
	
	if (gState.totalEntries > 0 && gState.outFileSpec.vRefNum != 0) {
		error = NewAlias(NULL, &gState.outFileSpec, &aliasHandle);
		
		if (error == noErr && writeData != NULL) {
			AddResource((Handle)aliasHandle, 'alis', outputFileAlisID, "\poutputFile");
			
			if (ResError() == noErr) {
				WriteResource((Handle)aliasHandle);
			}
		}
	}
	
	CloseResFile(fileRef);
}
