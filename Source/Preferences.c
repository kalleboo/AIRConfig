#include "Globals.h"
#include "Defines.h"

FSSpec		gPrefsFileSpec;

#define hasSeenHelpPrefID	128
#define headlessModePrefID	129
#define inputSourcePrefID	130
#define inputURLPrefID		131

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
	short		fileRef;
	Handle      resHandle;
	
	gPrefs.hasSeenHelp = false;
	gPrefs.headlessMode = false;
	gPrefs.inputSource = kInputSourceFile;
	gPrefs.inputFileAlias = nil;
	gPrefs.outputFileAlias = nil;
	gPrefs.inputURL[0] = 0;
	
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
	
	resHandle = Get1Resource('pref', inputSourcePrefID);
	if (resHandle != NULL) {
		gPrefs.inputSource = **resHandle;
		ReleaseResource(resHandle);
	}
	
	resHandle = Get1Resource('pref', inputURLPrefID);
	if (resHandle != NULL) {
		BlockMove(*resHandle, &gPrefs.inputURL, GetHandleSize(resHandle));
		ReleaseResource(resHandle);
	}
	
	resHandle = Get1Resource('alis', inputFileAlisID);
	if (resHandle != NULL) {
		gPrefs.inputFileAlias = (AliasHandle)resHandle;
		DetachResource(resHandle);
	}
	
	resHandle = Get1Resource('alis', outputFileAlisID);
	if (resHandle != NULL) {
		gPrefs.outputFileAlias = (AliasHandle)resHandle;
		DetachResource(resHandle);
	}
	
	CloseResFile(fileRef);
}

#pragma segment Main
void SavePreferences(void)
{
	short		fileRef;
	Handle		writeData;
	Handle      existingRes;
	Handle		copyHandle;
	
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
	
	AddResource(writeData, 'pref', headlessModePrefID, "\pheadlessMode");
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	}
	
	
	//inputSource
	writeData = NewHandle(1);
	**writeData = gPrefs.inputSource;
	
	existingRes = Get1Resource('pref', inputSourcePrefID);
	
	if (existingRes != NULL) {
		RemoveResource(existingRes);
	}
	
	AddResource(writeData, 'pref', inputSourcePrefID, "\pinputSourcePrefID");
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	}
	
	
	//inputURL
	writeData = NewHandle(gPrefs.inputURL[0] + 1);
	BlockMove(&gPrefs.inputURL, *writeData, GetHandleSize(writeData));
	
	existingRes = Get1Resource('pref', inputURLPrefID);
	
	if (existingRes != NULL) {
		RemoveResource(existingRes);
	}
	
	AddResource(writeData, 'pref', inputURLPrefID, "\pinputURL");
	
	if (ResError() == noErr) {
		WriteResource(writeData);
	}
	
	
	//Input file
	existingRes = Get1Resource('alis', inputFileAlisID);
	RemoveResource(existingRes);
	
	if (gPrefs.inputFileAlias != nil) {
		copyHandle = (Handle)gPrefs.inputFileAlias;
		HandToHand(&copyHandle);
		
		AddResource(copyHandle, 'alis', inputFileAlisID, "\pinputFile");
		
		if (ResError() == noErr) {
			WriteResource(copyHandle);
		}
	}
	
	//Output file
	existingRes = Get1Resource('alis', outputFileAlisID);
	RemoveResource(existingRes);
	
	if (gPrefs.outputFileAlias != nil) {
		copyHandle = (Handle)gPrefs.outputFileAlias;
		HandToHand(&copyHandle);
		
		AddResource(copyHandle, 'alis', outputFileAlisID, "\poutputFile");
		
		if (ResError() == noErr) {
			WriteResource(copyHandle);
		}
	}
	
	CloseResFile(fileRef);
}
