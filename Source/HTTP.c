#include "Globals.h"
#include "Errors.h"

#include "compat.h"

#ifdef PROTOS
#include <Types.h>
#include <Memory.h>
#include <Packages.h>
#include <CursorCtl.h>
#endif

#include <String.h>

#include "CvtAddr.h"
#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include "TCPHi.h"

Boolean gCancel = false;

#define kHTTPPort 	80		/* TCP port assigned for finger protocol */
#define kBufSize	16384	/* Size for TCP stream buffer and receive buffer */
#define kTimeOut	20		/* Timeout for TCP commands */


extern void SetInputString(char *newInputString, unsigned long length);

extern void FatalAlertMessage( Str255 message, OSErr errorCode );
extern void AlertWarningMessage(Str255 message, OSErr errorCode);


OSErr FetchHTTP(void);
Boolean LoadInputURL(void);
void ParseURL(Str255 host, Str255 path);
OSErr GetHTTPData(unsigned long stream,Handle *fingerData);
void FixCRLF(char *data);


Boolean LoadInputURL(void) {
	OSErr	err;
	
	if (gPrefs.inputURL[0] == 0) { return false; }
	
	err = FetchHTTP();
	
	if (err == noErr) {
		return true;
	} else if (err == memFullErr) {
		FatalAlertMessage("\pOut of memory error. AIRConfig will now quit to avoid crashing. (this program is full of memory leaks, sorry)", noErr);
		return false;
	} else {
		AlertWarningMessage("\pCould not load URL ", err);
		return false;
	}
}


void ParseURL(Str255 host, Str255 path) {
	Str255	prot = "\phttp://";
	short 	length;
	short 	i;
	short 	outPos;
	char 	c;
	char	scanMode = 0;
	
	outPos = 0;
	length = 0;
	
	
	for (i = 1; i <= gPrefs.inputURL[0]; i++) {
		c = gPrefs.inputURL[i];
		
		if (i <= prot[0]) { //skip "http://"
			if (c != prot[i]) {
				host[0] = 0;
				path[0] = 0;
				return;
			}
		} else if (scanMode == 0) { //host
			if (outPos == 0) {
				outPos = 1;
			}
			
			if (c == '/') {
				host[0] = length;
				scanMode = 1;
				outPos = 1;
				length = 0;
			} else {
				host[outPos] = c;
				outPos++;
				length++;
			}
			
		} else if (scanMode == 1) { //path
			path[outPos] = c;
			outPos++;
			length++;
		}
	}
	
	path[0] = length;
}


OSErr FetchHTTP(void) {
	OSErr 		err;
	unsigned long ipAddress;
	unsigned long stream;
	char 		httpCmd1[256] = "GET /";
	char		path[256];
	char 		httpCmd2[256] = " HTTP/1.1\r\nHost: ";
	char		host[256];
	char 		httpCmd3[256] = "\r\n\r\n";
	Handle 		httpData;
	Ptr 		httpDataPtr;
	Ptr			httpBody;
	Str255		urlHost;
	Str255		urlPath;
	unsigned long	i;
	unsigned long	length;
	
	ParseURL(urlHost, urlPath);
	if (urlHost[0] == 0 || urlPath[0] == 0) {
		return -999;
	}
	
	err = InitNetwork();
	if (err!=noErr) {
		return err;
	}
	
	BlockMove(&urlHost[1], &host, urlHost[0]);
	host[urlHost[0]] = 0;
	
	BlockMove(&urlPath[1], &path, urlPath[0]);
	path[urlPath[0]] = 0;
	
	err = ConvertStringToAddr(host, &ipAddress);
	if (err!=noErr) {
		return err;
	}
	
	err = CreateStream(&stream,kBufSize);
	if (err!=noErr) {
		return err;
	}
	
	err = OpenConnection(stream,ipAddress,kHTTPPort,kTimeOut);
	if (err==noErr) {
		err = SendData(stream,httpCmd1,(unsigned short)strlen(httpCmd1),false);
		if (err!=noErr) { return err; }
		err = SendData(stream,path,(unsigned short)strlen(path),false);
		if (err!=noErr) { return err; }
		err = SendData(stream,httpCmd2,(unsigned short)strlen(httpCmd2),false);
		if (err!=noErr) { return err; }
		err = SendData(stream,host,(unsigned short)strlen(host),false);
		if (err!=noErr) { return err; }
		err = SendData(stream,httpCmd3,(unsigned short)strlen(httpCmd3),false);
		if (err!=noErr) { return err; }
			
		err = GetHTTPData(stream, &httpData);
		if (err!=noErr) {
			return err;
		}
		
		CloseConnectionHi(stream);
	}
	
	ReleaseStream(stream);
	
	//Scan for \n\r\n\r
	
	httpBody = nil;
	
	HLock(httpData);
	httpDataPtr = *httpData;
	length = strlen(httpDataPtr);
	for (i = 0; i < length; i++) {
		if (i <= 4) { continue; }
		
		if (httpDataPtr[i-3] == '\r'
			&& httpDataPtr[i-2] == '\n'
			&& httpDataPtr[i-1] == '\r'
			&& httpDataPtr[i] == '\n') {
			
			httpBody = httpDataPtr + i;
			
			break;
		}
	}
	
	if (httpBody == nil) {
		return -998;
	}
	
	httpBody = httpBody + 1;
	
	SetInputString(httpBody, strlen(httpBody));
	
	HUnlock(httpData);
	DisposeHandle(httpData);
		
	return noErr;
}




OSErr GetHTTPData(unsigned long stream,Handle *fingerData)
{
	OSErr err;
	long bufOffset = 0;
	Ptr data;
	unsigned short dataLength;
	
	*fingerData = NewHandle(kBufSize);
	err = MemError();
	if (err!=noErr)
		return err;
		
	HLock(*fingerData);
	data = **fingerData;
	dataLength = kBufSize;
	
	do {
		err = RecvData(stream,data,&dataLength,false);
		if (err==noErr) {
			bufOffset += dataLength;
			dataLength = kBufSize;
			HUnlock(*fingerData);
			SetHandleSize(*fingerData,bufOffset+kBufSize);
			err = MemError();
			HLock(*fingerData);
			data = **fingerData + bufOffset;
		}
	} while (err==noErr);
	
	data[0] = '\0';
	
	HUnlock(*fingerData);
	if (err == connectionClosing)
		err = noErr;
	
	return err;
}


/* FixCRLF() removes the linefeeds from a	*/
/* text buffer.  This is necessary, since	*/
/* all text on the network is embedded with	*/
/* carraige return linefeed pairs.			*/

void FixCRLF(char *data)
{
	register char *source,*dest;
	long length;
	
	length = strlen(data);
	
	if (*data) {
		source = dest = data;
		while ((source - data) < (length-1)) {
			if (*source == '\r')
				source++;
			*dest++ = *source++;
		}
		if (*source != '\r' && (source - data) < length)
			*dest++ = *source++;
		length = dest - data;
	}
	
	*dest = '\0';
}


