/* 
	AddressXlation.h		
	MacTCP name to address translation routines.

    Copyright Apple Computer, Inc. 1988 
    All rights reserved
	
*/	

#define NUM_ALT_ADDRS	4

typedef struct hostInfo {
	long rtnCode;
	char cname[255];
	unsigned long addr[NUM_ALT_ADDRS];
};

typedef enum AddrClasses {
	A = 1,
	NS,
	CNAME = 5,
	lastClass = 65535
}; 

typedef struct cacheEntryRecord {
	char *cname;
	unsigned short type;
	enum AddrClasses class;
	unsigned long ttl;
	union {
		char *name;
		ip_addr addr;
	} rdata;
};

#ifdef THINK_C
typedef void *EnumResultProcPtr;
typedef void *ResultProcPtr;
#else
typedef pascal void (*EnumResultProcPtr)(struct cacheEntryRecord *cacheEntryRecordPtr, char *userDataPtr);
typedef pascal void (*ResultProcPtr)(struct hostInfo *hostInfoPtr, char *userDataPtr);
#endif

extern OSErr OpenResolver(char *fileName);

extern OSErr StrToAddr(char *hostName, struct hostInfo *hostInfoPtr, ResultProcPtr ResultProc, char *userDataPtr);

extern OSErr AddrToStr(unsigned long addr, char *addrStr);

extern OSErr EnumCache(EnumResultProcPtr enumResultProc, char *userDataPtr);

extern OSErr AddrToName(ip_addr addr, struct hostInfo *hostInfoPtr, ResultProcPtr ResultProc, char *userDataPtr);

extern OSErr CloseResolver(void);