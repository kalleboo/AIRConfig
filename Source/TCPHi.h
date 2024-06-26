#ifndef _TCPLOW_
#define _TCPLOW_

/* network initialization ------------------------------------------------------*/

OSErr InitNetwork(void);		/* opens the network driver */


/* connection stream creation/removal -------------------------------------------*/

OSErr CreateStream(				/* creates a stream needed to establish a connection*/
	unsigned long *stream,			/* stream identifier (returned)					*/
	unsigned long recvLen);			/* stream buffer length to be allocated			*/
	
OSErr ReleaseStream(			/* disposes of an unused stream and its buffers		*/
	unsigned long stream);			/* stream identifier to dispose					*/


/* connection opening/closing calls ---------------------------------------------*/

OSErr OpenConnection(			/* attempts to establish a connection w/remote host */
	unsigned long stream,			/* stream id to be used for connection			*/
	long remoteHost,				/* network number of remote host				*/
	short remotePort,				/* network port of remote port					*/
	Byte timeout);					/* timeout value for connection					*/
	
OSErr WaitForConnection(		/* listens for a remote connection from a rem. port */
	unsigned long stream,			/* stream id to be used for connection			*/
	Byte timeout,					/* timeout value for open						*/
	short localPort,				/* local port to listen on						*/
	long *remoteHost,				/* remote host connected to (returned)			*/
	short *remotePort);				/* remote port connected to (returned)			*/

void AsyncWaitForConnection(	/* same as above, except executed asynchronously	*/
	unsigned long stream,			/* stream id to be used for connection			*/
	Byte timeout,					/* timeout value for open						*/
	short localPort,				/* local port to listen on						*/
	long remoteHost,				/* remote host to listen for					*/
	short remotePort,				/* remote port to listen for					*/
	TCPiopb **returnBlock);			/* parameter block for call (returned)			*/

OSErr AsyncGetConnectionData(	/* retrieves connection data for above call			*/
	TCPiopb *returnBlock,			/* parameter block for asyncwait call			*/
	long *remoteHost,				/* remote host connected to (returned)			*/
	short *remotePort);				/* remote port connected to (returned)			*/

OSErr CloseConnectionHi(			/* closes an established connection					*/
	unsigned long stream);			/* stream id of stream used for connection		*/
	
OSErr AbortConnectionHi(			/* aborts a connection non-gracefully				*/
	unsigned long stream);			/* stream id of stream used for connection		*/


/* data sending calls ----------------------------------------------------------*/

OSErr SendData(					/* sends data along an open connection				*/
	unsigned long stream,			/* stream used for connection					*/
	Ptr data,						/* pointer to data to send						*/
	unsigned short length,			/* length of data to send						*/
	Boolean retry);					/* if true, call continues until send successful*/
	
OSErr SendMultiData(			/* sends multiple strings of data on a connection	*/
	unsigned long stream,			/* stream used for connection					*/
	Str255 data[],					/* array of send strings						*/	
	short numData,					/* number of strings to send					*/
	Boolean retry);					/* if true, call continues until send successful*/

void SendDataAsync(				/* sends data asynchronously						*/
	unsigned long stream,			/* stream used for connection					*/
	Ptr data,						/* pointer to data to send						*/
	unsigned short length,			/* length of data to send						*/
	TCPiopb **returnBlock);			/* pointer to parameter block (returned)		*/
	
OSErr SendAsyncDone(			/* called when SendDataAsync call completes			*/
	TCPiopb *returnBlock);			/* parameter block to complete connection		*/


/* data receiving calls --------------------------------------------------------*/

OSErr RecvData(					/* waits for data to be received on a connection	*/
	unsigned long stream,			/* stream used for connection					*/ 
	Ptr data,						/* pointer to memory used to hold incoming data	*/
	unsigned short *length,			/* length to data received (returned)			*/
	Boolean retry);					/* if true, call continues until successful		*/
	
void RecvDataAsync(				/* receives data asynchronously						*/
	unsigned long stream,			/* stream used for connection					*/
	Ptr data,						/* pointer to memory used to hold incoming data	*/
	unsigned short length,			/* length of data requested						*/
	TCPiopb **returnBlock);			/* parameter block to complete connection		*/
	
OSErr GetDataLength(			/* called when RecvDataAsync completes				*/
	TCPiopb *returnBlock,			/* parameter block used for receive				*/
	unsigned short *length);		/* length of data received (returned)			*/

#endif _TCPLOW_