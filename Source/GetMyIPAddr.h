/* 
	GetMyIPAddr.h	
	C definitions of parameter block entries needed for IP calls

    Copyright Apple Computer, Inc. 1989 
    All rights reserved
	
*/

#ifndef _GETMYIP_
#define _GETMYIP_

#define ipctlGetAddr		15			/* csCode to get our IP address */

#define ParamBlockHeader 	\
	struct QElem *qLink; 	\
	short qType; 			\
	short ioTrap; 			\
	Ptr ioCmdAddr; 			\
	ProcPtr ioCompletion; 	\
	OSErr ioResult; 		\
	StringPtr ioNamePtr; 	\
	short ioVRefNum;		\
	short ioCRefNum;		\
	short csCode

struct IPParamBlock {
	ParamBlockHeader;			/* standard I/O header */
	ip_addr	ourAddress;			/* our IP address */
	long	ourNetMask;			/* our IP net mask */
	};
	
#endif _GETMYIP_