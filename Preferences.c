

FSSpec		gPrefsFileSpec;

#define hasSeenHelpPrefID 128

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
	short		fileRef;
	Handle      resHandle;
	
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
	
	CloseResFile(fileRef);
}

#pragma segment Main
void SavePreferences(void)
{
	short		fileRef;
	Handle		writeData;
	Handle      existingRes;
	
	if (!FindPreferences()) {
		return;
	}
		
	fileRef = FSpOpenResFile(&gPrefsFileSpec, fsWrPerm);
	UseResFile(fileRef);
	
	writeData = NewHandle(1);
	**writeData = gPrefs.hasSeenHelp;
	
	existingRes = Get1Resource('pref', hasSeenHelpPrefID);
	
	if (existingRes != NULL) {
		//GetResInfo(existingRes, &existingResId, &existingResType, existingResName);
		//GetResAttrs(existingRes);
		RemoveResource(existingRes);
	}
	
	AddResource(writeData, 'pref', 128, "\phasSeenHelp");
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	}
	
	CloseResFile(fileRef);
}
