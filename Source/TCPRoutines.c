/*----------------------------------------------------------
#
#	finger -- MPW Tool
#
#	Written by Steven Falkenburg
#
#-----------------------------------------------------------
#
#	tcproutines.c
#
#	This code segment contains the low-level interfaces to
#	the MacTCP device driver calls.
#
#-----------------------------------------------------------*/

#include "compat.h"

#ifdef	PROTOS
//#include <SysEqu.h>
#include <Types.h>
#include <Events.h>
#include <OSUtils.h>
#include <Devices.h>
#include <CursorCtl.h>
#include <Memory.h>
#include <Dialogs.h>
#include <Lists.h>
#endif

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include "GetMyIPAddr.h"
#include "TCPRoutines.h"

#define DisposPtr DisposePtr

OSErr NewBlock(TCPiopb **pBlock);
Boolean GiveTime(short sleepTime);

const long cSleepTime = 20;

/* global variables */

extern Boolean gCancel;
short 		refNum;


/* Opens the MacTCP driver.
   This routine must be called prior to any of the below functions. */

OSErr OpenTCPDriver()
{
	OSErr	err;
	
	err = OpenDriver("\p.IPP",&refNum);
	return(err);
}


OSErr NewBlock(TCPiopb **pBlock)
{
	*pBlock = (TCPiopb *)NewPtr(sizeof(TCPiopb));
	if (MemError() != noErr)
		return MemError();
	(*pBlock)->ioCompletion = 0L;
	(*pBlock)->ioCRefNum = refNum;
	return noErr;
}


/* kills any pending calls to the TCP driver */

OSErr LowKillTCP(TCPiopb *pBlock)
{
	return(PBKillIO((ParmBlkPtr)pBlock,false));
}


/* Creates a new TCP stream in preparation for initiating a connection.
   A buffer must be provided for storing incoming data waiting to be processed */

OSErr LowTCPCreateStream(StreamPtr *streamPtr,Ptr connectionBuffer,
			unsigned long connBufferLen,TCPNotifyProc notifPtr)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
		
	pBlock->csCode = TCPCreate;
	pBlock->ioResult = 1;
	pBlock->csParam.create.rcvBuff = connectionBuffer;
	pBlock->csParam.create.rcvBuffLen = connBufferLen;
	pBlock->csParam.create.notifyProc = notifPtr;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
		
	*streamPtr = pBlock->tcpStream;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* If TCPWaitForConnection is called asynchronously, this command retrieves the 
   result of the call.  It should be called when the above command completes. */

OSErr LowFinishTCPWaitForConn(TCPiopb *pBlock,ip_addr *remoteHost,tcp_port *remotePort,
							ip_addr *localHost,tcp_port *localPort)
{	
	OSErr err;
	
	*remoteHost = pBlock->csParam.open.remoteHost;
	*remotePort = pBlock->csParam.open.remotePort;
	*localHost = pBlock->csParam.open.localHost;
	*localPort = pBlock->csParam.open.localPort;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* Waits for a connection to be opened on a specified port from a specified address.
   It completes when a connection is made, or a timeout value is reached.  This call
   may be made asynchronously. */

OSErr LowTCPWaitForConnection(StreamPtr streamPtr,byte timeout,ip_addr *remoteHost,
			tcp_port *remotePort,ip_addr *localHost,tcp_port *localPort,
			Boolean async,TCPiopb **returnBlock)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPPassiveOpen;
	pBlock->ioResult = 1;
	pBlock->ioCompletion = nil;
	pBlock->tcpStream = streamPtr;
	pBlock->csParam.open.ulpTimeoutValue = timeout;
	pBlock->csParam.open.ulpTimeoutAction = 1;
	pBlock->csParam.open.validityFlags = 0xC0;
	pBlock->csParam.open.commandTimeoutValue = timeout;
	pBlock->csParam.open.remoteHost = *remoteHost;
	pBlock->csParam.open.remotePort = *remotePort;
	pBlock->csParam.open.localPort = *localPort;
	pBlock->csParam.open.tosFlags = 0;
	pBlock->csParam.open.precedence = 0;
	pBlock->csParam.open.dontFrag = 0;
	pBlock->csParam.open.timeToLive = 0;
	pBlock->csParam.open.security = 0;
	pBlock->csParam.open.optionCnt = 0;
	PBControl((ParmBlkPtr)pBlock,true);
	if (!async) {
		while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
			;
		if (gCancel)
			return -1;
		return(LowFinishTCPWaitForConn(pBlock,remoteHost,remotePort,localHost,localPort));
	}
	
	*returnBlock = pBlock;
	return noErr;
}


/* Attempts to initiate a connection with a host specified by host and port. */

OSErr LowTCPOpenConnection(StreamPtr streamPtr,byte timeout,ip_addr remoteHost,
			tcp_port remotePort,ip_addr *localHost,tcp_port *localPort)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPActiveOpen;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	pBlock->csParam.open.ulpTimeoutValue = timeout;
	pBlock->csParam.open.ulpTimeoutAction = 1;
	pBlock->csParam.open.validityFlags = 0xC0;
	pBlock->csParam.open.commandTimeoutValue = timeout;
	pBlock->csParam.open.remoteHost = remoteHost;
	pBlock->csParam.open.remotePort = remotePort;
	pBlock->csParam.open.localPort = *localPort;
	pBlock->csParam.open.tosFlags = 0;
	pBlock->csParam.open.precedence = 0;
	pBlock->csParam.open.dontFrag = 0;
	pBlock->csParam.open.timeToLive = 0;
	pBlock->csParam.open.security = 0;
	pBlock->csParam.open.optionCnt = 0;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	*localHost = pBlock->csParam.open.localHost;
	*localPort = pBlock->csParam.open.localPort;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* This routine should be called when a TCPSendData call completes.  It returns the
   error code generated upon completion of the CallTCPSend. */

OSErr LowFinishTCPSend(TCPiopb *pBlock)
{
	OSErr err;
	
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* Sends data through an open connection stream.  Note that the connection must be
   open before any data is sent. This call may be made asynchronously. */

OSErr LowTCPSendData(StreamPtr streamPtr,byte timeout,Boolean push,Boolean urgent,
					Ptr wdsPtr,Boolean async,TCPiopb **returnBlock)
{	
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPSend;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	pBlock->ioCompletion = nil;
	pBlock->csParam.send.ulpTimeoutValue = timeout;
	pBlock->csParam.send.ulpTimeoutAction = 1;
	pBlock->csParam.send.validityFlags = 0xC0;
	pBlock->csParam.send.pushFlag = push;
	pBlock->csParam.send.urgentFlag = urgent;
	pBlock->csParam.send.wdsPtr = wdsPtr;
	PBControl((ParmBlkPtr)pBlock,true);
	if (!async) {
		while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
			;
		if (gCancel)
			return -1;
		return LowFinishTCPSend(pBlock);
		return err;
	}
	
	*returnBlock = pBlock;
	return noErr;
}


OSErr LowFinishTCPNoCopyRcv(TCPiopb *pBlock,Boolean *urgent,Boolean *mark)
{
	OSErr err;
	
	*urgent = pBlock->csParam.receive.urgentFlag;
	*mark = pBlock->csParam.receive.markFlag;
	
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


OSErr LowTCPNoCopyRcv(StreamPtr streamPtr,byte timeout,Boolean *urgent,Boolean *mark,
				Ptr rdsPtr,short numEntry,Boolean async,TCPiopb **returnBlock)
{
	OSErr	err = noErr;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPNoCopyRcv;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	pBlock->ioCompletion = nil;
	pBlock->csParam.receive.commandTimeoutValue = timeout;
	pBlock->csParam.receive.rdsPtr = rdsPtr;
	pBlock->csParam.receive.rdsLength = numEntry;
	PBControl((ParmBlkPtr)pBlock,true);
	if (!async) {
		while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
			;
		if (gCancel)
			return -1;
		return LowFinishTCPNoCopyRcv(pBlock,urgent,mark);
	}
	
	*returnBlock = pBlock;
	return noErr;
}


OSErr LowTCPBfrReturn(StreamPtr streamPtr,Ptr rdsPtr)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPRcvBfrReturn;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	pBlock->csParam.receive.rdsPtr = rdsPtr;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* If the above is called asynchronously, this routine returns the data that was
   received from the remote host. */
   
OSErr LowFinishTCPRecv(TCPiopb *pBlock,Boolean *urgent,Boolean *mark,
					unsigned short *rcvLen)
{
	OSErr err;
	
	*rcvLen = pBlock->csParam.receive.rcvBuffLen;
	*urgent = pBlock->csParam.receive.urgentFlag;
	*mark = pBlock->csParam.receive.markFlag;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* Attempts to pull data out of the incoming stream for a connection. If data is
   not present, the routine waits a specified amout of time before returning with
   a timeout error.  This call may be made asynchronously. */
   
OSErr LowTCPRecvData(StreamPtr streamPtr,byte timeout,Boolean *urgent,Boolean *mark,
				Ptr rcvBuff,unsigned short *rcvLen,Boolean async,TCPiopb **returnBlock)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPRcv;
	pBlock->ioResult = 1;
	pBlock->ioCompletion = nil;
	pBlock->tcpStream = streamPtr;
	pBlock->csParam.receive.commandTimeoutValue = timeout;
	pBlock->csParam.receive.rcvBuff = rcvBuff;
	pBlock->csParam.receive.rcvBuffLen = *rcvLen;
	PBControl((ParmBlkPtr)pBlock,true);
	if (!async) {
		while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
			;
		if (gCancel)
			return -1;
		return(LowFinishTCPRecv(pBlock,urgent,mark,rcvLen));
	}
	
	*returnBlock = pBlock;
	return noErr;
}
	

/* Gracefully closes a connection with a remote host.  This is not always possible,
   and the programmer might have to resort to CallTCPAbort, described next. */

OSErr LowTCPClose(StreamPtr streamPtr,byte timeout)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPClose;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	pBlock->csParam.close.ulpTimeoutValue = timeout;
	pBlock->csParam.close.validityFlags = 0xC0;
	pBlock->csParam.close.ulpTimeoutAction = 1;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* Should be called if a CallTCPClose fails to close a connection properly.
   This call should not normally be used to terminate connections. */
   
OSErr LowTCPAbort(StreamPtr streamPtr)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPAbort;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}

OSErr LowTCPStatus(StreamPtr streamPtr,TCPStatusPB *theStatus)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPStatus;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	theStatus = &(pBlock->csParam.status);
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* Deallocates internal buffers used to hold connection data. This should be
   called after a connection has been closed. */

OSErr LowTCPRelease(StreamPtr streamPtr,Ptr *recvPtr,unsigned long *recvLen)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPRelease;
	pBlock->ioResult = 1;
	pBlock->tcpStream = streamPtr;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	*recvPtr = pBlock->csParam.create.rcvBuff;
	*recvLen = pBlock->csParam.create.rcvBuffLen;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}

OSErr LowTCPGlobalInfo(Ptr *tcpParam,Ptr *tcpStat)
{
	OSErr err;
	TCPiopb *pBlock;
	
	if ((err = NewBlock(&pBlock)) != noErr)
		return err;
	
	pBlock->csCode = TCPGlobalInfo;
	pBlock->ioResult = 1;
	PBControl((ParmBlkPtr)pBlock,true);
	while (pBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	*tcpParam = (Ptr) pBlock->csParam.globalInfo.tcpParamPtr;
	*tcpStat = (Ptr) pBlock->csParam.globalInfo.tcpStatsPtr;
	err = pBlock->ioResult;
	DisposPtr((Ptr)pBlock);
	return err;
}


/* Returns the IP address of the local computer. */

OSErr GetMyIP(ip_addr *ipnum)
{
	struct	IPParamBlock	*IPBlock;
	OSErr	err;
	
	IPBlock = (struct IPParamBlock *)NewPtr(sizeof(struct IPParamBlock));
	if (MemError() != noErr)
		return MemError();
	
	IPBlock->ioResult = 1;
	IPBlock->csCode = ipctlGetAddr;
	IPBlock->ioCompletion = nil;
	IPBlock->ioCRefNum = refNum;
	PBControl((ParmBlkPtr)IPBlock,true);
	while (IPBlock->ioResult > 0 && GiveTime(cSleepTime))
		;
	if (gCancel)
		return -1;
	*ipnum = IPBlock->ourAddress;
	err = IPBlock->ioResult;
	DisposPtr((Ptr)IPBlock);
	return err;
}
