/*----------------------------------------------------------
#
#	finger -- MPW Tool
#
#	Written by Steven Falkenburg
#
#-----------------------------------------------------------
#
#	TCPHi.c
#
#	This module contains high-level routines for setting
#	up connections using MacTCP and sending/receiving data
#	on these connections.
#
#-----------------------------------------------------------*/

#pragma segment lowlevel

#include "compat.h"
#include <string.h>

#ifdef PROTOS
#include <Types.h>
#include <Memory.h>
#include <Dialogs.h>
#include <Resources.h>
#include <CursorCtl.h>
#include <Strings.h>
#include <Lists.h>
#endif

#include "MacTCPCommonTypes.h"
#include "GetMyIPAddr.h"
#include "TCPPB.h"
#include "TCPRoutines.h"
#include "TCPHi.h"

#define DisposPtr DisposePtr

/* InitNetwork opens the network driver
*/

OSErr InitNetwork(void)
{
	return OpenTCPDriver();
}


/* CreateStream() creates an unconnected network stream to be
   used later by OpenConnection.  The length of the receive
   buffer must be specified in the call */
   
OSErr CreateStream(unsigned long *stream,unsigned long recvLen)
{
	Ptr recvPtr;
	OSErr err;
	
	recvPtr = NewPtr(recvLen);
	err = MemError();
	if (err==noErr)
		err = LowTCPCreateStream(stream,recvPtr,recvLen,(TCPNotifyProc)nil);
	return err;
}


/* OpenConnection() initiates a connection to a remote machine,
   given that machine's network number and connection port.  A timeout
   value for the call must be given, and the stream identifier is returned. */

OSErr OpenConnection(unsigned long stream,long remoteHost,short remotePort,Byte timeout)
{
	ip_addr localHost;
	tcp_port localPort = 0;
	
	return LowTCPOpenConnection(stream,timeout,remoteHost,remotePort,&localHost,
								&localPort);
}


/* WaitForConnection() listens for a connection on a particular port from a
	particular host.  It returns when a connection has been established */

OSErr WaitForConnection(unsigned long stream,Byte timeout,short localPort,
						long *remoteHost,short *remotePort)
{
	ip_addr localHost;
	
	return LowTCPWaitForConnection(stream,timeout,(ip_addr *)remoteHost,
				(tcp_port *)remotePort,&localHost,(tcp_port *)&localPort,false,nil);
}


/* AsyncWaitForConnection() listens for a connection on a particular port from a
	particular host.  It is executed asynchronously and returns immediately */

void AsyncWaitForConnection(unsigned long stream,Byte timeout,short localPort,
				long remoteHost,short remotePort,TCPiopb **returnBlock)
{
	ip_addr localHost;
	
	LowTCPWaitForConnection(stream,timeout,(ip_addr *)&remoteHost,
				(tcp_port *)&remotePort,&localHost,(tcp_port *)&localPort,true,returnBlock);
}


/* AsyncGetConnectionData() should be called when a call to AsyncWaitForConnection
	completes (when returnBlock->ioResult <= 0).  This call retrieves the information
	about this new connection and disposes the parameter block. */
	
OSErr AsyncGetConnectionData(TCPiopb *returnBlock,long *remoteHost,short *remotePort)
{
	ip_addr localHost;
	tcp_port localPort;
	
	return LowFinishTCPWaitForConn(returnBlock,(ip_addr *)remoteHost,
						(tcp_port *)remotePort,&localHost,&localPort);
}


/* CloseConnection() terminates a connection to a remote host, given the
   stream identifier of the connection */
   
OSErr CloseConnectionHi(unsigned long stream)
{
	return LowTCPClose(stream,10);
}


/* AbortConnection() aborts a connection to a remote host, given the
   stream identifier of the connection */
   
OSErr AbortConnectionHi(unsigned long stream)
{
	return LowTCPAbort(stream);
}
	

/* ReleaseStream() frees the allocated buffer space for a given connection
   stream.  This call should be made after CloseConnection. */
   
OSErr ReleaseStream(unsigned long stream)
{
	OSErr err;
	Ptr recvPtr;
	unsigned long recvLen;
	
	if ((err = LowTCPRelease(stream,&recvPtr,&recvLen)) == noErr)
			DisposPtr(recvPtr);
	
	return err;
}


/* SendData() sends data along a connection stream to a remote host. */

OSErr SendData(unsigned long stream,Ptr data,unsigned short length,Boolean retry)
{	
	OSErr err;
	struct wdsEntry myWDS[2];	/* global write data structure */

	myWDS[0].length = length;
	myWDS[0].ptr = data;
	myWDS[1].length = 0;
	myWDS[1].ptr = nil;
	do
		err = LowTCPSendData(stream,20,false,false,(Ptr) myWDS,false,nil);
	while (retry && err==commandTimeout);
	return err;
}

/* SendMultiData() is similar to SendData, but takes an array of strings to send
   to the remote host. */

OSErr SendMultiData(unsigned long stream,Str255 data[],short numData,Boolean retry)
{
	struct wdsEntry *theWDS;
	short i;
	OSErr err;
	
	theWDS = (wdsEntry *)NewPtr((numData+1) * sizeof(wdsEntry));
	if (MemError())
		return MemError();	
	theWDS[numData].length = 0;
	theWDS[numData].ptr = nil;
	for (i=0; i<numData; i++) {
		theWDS[i].ptr = (char *)data[i];
		theWDS[i].length = strlen((char *)data[i]);
	}
	do
		err = LowTCPSendData(stream,20,false,false,(Ptr) theWDS,false,nil);
	while (retry && err==commandTimeout);
	DisposPtr((Ptr)theWDS);
	return err;
}


/* SendDataAsync() sends data to a remote host asynchronously.  The ioResult
   parameter block variable should be checked, and SendDataDone() called when
   this flag is zero or negative */

void SendDataAsync(unsigned long stream, Ptr data,unsigned short length,TCPiopb **returnBlock)
{	
	struct wdsEntry *theWDS;

	//InitNetCursor();
	theWDS = (wdsEntry *)NewPtr( (2*sizeof(wdsEntry)) );
	theWDS[0].length = length;
	theWDS[0].ptr = (char *)data;
	theWDS[1].length = 0;
	theWDS[1].ptr = 0;
	LowTCPSendData(stream,20,false,false,(Ptr) theWDS,true,returnBlock);
	//TrashNetCursor();
}


/* SendDataDone() should be called in response to the completion of a SendDataAsync
   call.  It returns any error which occurred in the send. */

OSErr SendAsyncDone(TCPiopb *returnBlock)
{
	DisposPtr((Ptr)returnBlock->csParam.send.wdsPtr);
	return LowFinishTCPSend(returnBlock);
}


/* RecvData() waits for data to be received on a connection stream.  When data
   arrives, it is copied into the data buffer and the call terminates. */

OSErr RecvData(unsigned long stream,Ptr data,unsigned short *length,Boolean retry)
{
	Boolean	urgent,mark;
	OSErr	err;
	unsigned short recvLength;

	do {
		recvLength = *length;
		err = LowTCPRecvData(stream,20,&urgent,&mark,data,&recvLength,false,nil);
	}
	while (retry && err==commandTimeout);
	*length = recvLength;
	if (err == noErr) {
		*(data+*length) = '\0';
	}
	return err;
}


/* RecvDataAsync() is identical to RecvData above, but in this case, the call is
   made asynchronously. */

void RecvDataAsync(unsigned long stream,Ptr data,unsigned short length,TCPiopb **returnBlock)
{
	Boolean urgent,mark;
	
	LowTCPRecvData(stream,20,&urgent,&mark,data,&length,true,returnBlock);
}


/* GetDataLength() should be called in response to the completion of the
   RecvDataAsync call. */

OSErr GetDataLength(TCPiopb *returnBlock,unsigned short *length)
{
	Boolean urgent,mark;
	
	return LowFinishTCPRecv(returnBlock,&urgent,&mark,length);
}
