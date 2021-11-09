/**********************************************************************************/
/* VBAN COMMAND	                                             V.Burel(c)2015-2021  */
/**********************************************************************************/
/*                                                                                */
/*  THIS PROGRAM PROVIDES A MINIMAL VBAN-SERVICE                                  */
/*               PROVIDES A FUNCTION TO SEND VBAN-TEXT REQUEST                    */
/*               PROVIDES A METHOD TO MANAGE VBAN RT-PACKET                       */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to use Windows Socket                                                   */
/*  - How to send and recieve VBAN request                                        */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  COMPILATION DIRECTIVES:                                                       */
/*                                                                                */
/*  To compile With Microsoft VC2005 or higher, you need to create an             */
/*  empty Win32 project with the following options:                               */
/*  - Configuration Properties / General : Char Set = NOT SET                     */
/*  - Configuration Properties / C/C++ / Preprocessor : _CRT_SECURE_NO_DEPRECATE  */
/*                                                                                */
/*  LINKER: Ws2_32.lib                                                            */
/*                                                                                */
/*  This source code can be compiled for 32bit or 64 bits targets as well         */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  LICENSING: VoicemeeterRemote.dll usage is driven by a license agreement       */
/*             given in VoicemeeterRemoteAPI.pdf or readme.txt                    */
/*             THIS SOURCE CODE CAN BE USED ONLY IN A PROGRAM USING VOICEMEETER   */
/*             REMOTE API                                                         */
/*                                                                                */
/*--------------------------------------------------------------------------------*/


#ifndef __cplusplus
	#ifndef STRICT
		#define STRICT
	#endif
#endif

#include <stdio.h>
//#include <math.h>
#include <windows.h>
//#include <tchar.h>
#include <winsock.h>

#include "vban_cmd.h"
#include "VoicemeeterRemote.h"



typedef struct tagVBANCMD_CONTEXT
{
	long				mode;
	BOOL				ThreadStarted, finitlib;
	HANDLE				Thread_handle;
	DWORD				Thread_ident;
	BOOL				Thread_flagOK;
	long				WSAStartupReply;
	WSADATA				WSAData;
	SOCKET				vban_socket;
	unsigned long		IPv4Address;
	unsigned short		UDPport;
	long				fPortUsedByOtherApplication;
	CRITICAL_SECTION	ctc_section;
	T_VBAN_VMRT_PACKET	CurrentRTPPacket;
	long				CurrentRTPPacket_dirtyflag;
	DWORD				CurrentRTPPacket_counter;
	T_VBAN_VMRT_PACKET	PublicRTPPacket;
	unsigned char		last_voicemeeterType;

	T_VBAN_HEADER		VBANHeader;

} T_VBANCMD_CONTEXT, *PT_VBANCMD_CONTEXT, *LPT_VBANCMD_CONTEXT;


static T_VBANCMD_CONTEXT G_vban_ctx={0, 0, 0, NULL};


long VBANCMD_GetIncomingRequestCounter(void)
{
	return G_vban_ctx.CurrentRTPPacket_counter;
}

/******************************************************************************/
/* SOCKET TOOL                                                                */
/******************************************************************************/


static long VBAN_CloseSocket(SOCKET *psocket)
{
	long rep;
	if (*psocket != INVALID_SOCKET)
	{
		shutdown(*psocket, 2);
		rep=closesocket(*psocket );
		*psocket =INVALID_SOCKET;
	}
	return 0;
}


static long VBAN_SetSocketNonblocking(SOCKET socket)
{
    u_long flags;
	flags=1;
    return ioctlsocket(socket, FIONBIO, &flags);
}     

static long VBAN_InitSocket(LPT_VBANCMD_CONTEXT lpctx, SOCKET *psocket, unsigned short nuPort, long SocketBufferSize)
{
	SOCKADDR_IN local_sin;
	long rep,nnn;
	long nsize, nbByte;
	long nError;

	*psocket  = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP);
	if (*psocket == INVALID_SOCKET) return -1;
	//set non blocking socket
	rep=VBAN_SetSocketNonblocking(*psocket);
	if (rep != 0)
	{
		closesocket(*psocket);
		*psocket=INVALID_SOCKET;
		return -2;
	}
	//disable debug mode
	nsize=sizeof(rep);
	nnn=getsockopt(*psocket , SOL_SOCKET,SO_DEBUG, (char*)&(rep),&nsize);
	if (nnn == 0)
	{
		if (rep != 0)
		{
			rep=0;
			nsize=sizeof(rep);
			setsockopt(*psocket , SOL_SOCKET,SO_DEBUG, (char*)&rep,nsize);
		}
	}
	//set Time Out = ZERO.
	rep=0;
	nsize=sizeof(rep);
	setsockopt(*psocket , SOL_SOCKET,SO_SNDTIMEO, (char*)&rep,nsize);
	setsockopt(*psocket , SOL_SOCKET,SO_RCVTIMEO, (char*)&rep,nsize);

	//set Socket buffer size
	nsize=sizeof(SocketBufferSize);
	setsockopt(*psocket , SOL_SOCKET,SO_RCVBUF, (char*)&SocketBufferSize,nsize);

	//128 kB buffer is senough to send VBAN-TEXT request
	nbByte = 1024*128;
	nsize=sizeof(nbByte);
	setsockopt(*psocket , SOL_SOCKET,SO_SNDBUF, (char*)&nbByte,nsize);

	//define socket port and ipaddress to
	memset(&local_sin,0,sizeof(SOCKADDR_IN));
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons ((unsigned short)nuPort);  
	local_sin.sin_addr.s_addr = htonl (INADDR_ANY);
			
	lpctx->fPortUsedByOtherApplication=0;
	if (bind (*psocket , (struct sockaddr *) &(local_sin), sizeof(local_sin)) == SOCKET_ERROR)
	{
       	nError=WSAGetLastError();
		if (nError == WSAEADDRINUSE)
		{
			lpctx->fPortUsedByOtherApplication=1;
		}
		closesocket(*psocket);
		*psocket=INVALID_SOCKET;
		return -5;
	}
	return 0;
}


/******************************************************************************/
/* FUNCTIONS REQUEST                                                          */
/******************************************************************************/

long VBANCMD_SendRequest_String(char * pString)
{
	SOCKADDR_IN dest;
	LPT_VBAN_HEADER lpHeader;
	char Buffer[2048];
	long rep, nbByte;
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->ThreadStarted == 0) return -9;	
	if (pString == NULL) return -10;

	lpHeader = (LPT_VBAN_HEADER)Buffer;
	// build VBAN Header from our current VBAN Header
	*lpHeader = lpctx->VBANHeader;
	lpctx->VBANHeader.nuFrame++;
	strcpy((char*)(lpHeader+1), pString);
	//send request
	memset(&dest,0, sizeof(SOCKADDR_IN));
	dest.sin_family = AF_INET;
	dest.sin_port = htons (lpctx->UDPport);		
	dest.sin_addr.s_addr = lpctx->IPv4Address;

	nbByte = sizeof(T_VBAN_HEADER) + (long)strlen(pString);
	rep = sendto(lpctx->vban_socket,Buffer,nbByte,0,(struct sockaddr*)&dest, sizeof(dest));
	if (rep == SOCKET_ERROR) return -20;
	return 0;
}

long VBANCMD_SendRequest_Float(char * szParam, float value)
{
	char String[512];
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;

	if (szParam == NULL) return -1;
	if (szParam[0] == 0) return -2;

	sprintf(String, "%s = %f;", szParam, value);
	return VBANCMD_SendRequest_String(String);
}



long VBANCMD_SendRequest_RegisterRTPacket(unsigned char sTimeOut, char * szVBANStreamName)
{
	SOCKADDR_IN dest;
	T_VBAN_HEADER Header;
	long rep,nbByte;
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->ThreadStarted == 0) return -9;	

	Header.vban			='NABV';
	Header.format_SR	=0x60;
	Header.format_nbs	=0;
	Header.format_nbc	=VBAN_SERVICE_RTPACKETREGISTER;
	Header.format_bit	=sTimeOut & 0x000000FF;
	if (szVBANStreamName != NULL) strcpy(Header.streamname, "Register RTP");
	else strncpy(Header.streamname, szVBANStreamName,16);
	Header.nuFrame	=0;


	//send request
	memset(&dest,0, sizeof(SOCKADDR_IN));
	dest.sin_family = AF_INET;
	dest.sin_port = htons (lpctx->UDPport);		
	dest.sin_addr.s_addr = lpctx->IPv4Address;

	nbByte = sizeof(T_VBAN_HEADER);
	rep = sendto(lpctx->vban_socket,(char*)&Header,nbByte,0,(struct sockaddr*)&dest, sizeof(dest));
	if (rep == SOCKET_ERROR) return -20;
	return 0;
}






/******************************************************************************/
/* CURRENT PARAMETERS FUNCTIONS                                               */
/******************************************************************************/

long VBANCMD_IsParameterDirty(void)
{
	unsigned char vmType;
	long reply;
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	reply= lpctx->CurrentRTPPacket_dirtyflag;
	if (reply != 0)
	{
		// if dirty we copy our buffer in public structure
		EnterCriticalSection(&(lpctx->ctc_section));
		lpctx->PublicRTPPacket = lpctx->CurrentRTPPacket; 
		LeaveCriticalSection(&(lpctx->ctc_section));
	}
	// to fit the same behavior than the remote API function
	vmType = lpctx->PublicRTPPacket.voicemeeterType;
	if (vmType == 0) reply=-1;
	if (lpctx->last_voicemeeterType != vmType) reply=-2;
	lpctx->last_voicemeeterType = vmType;
	return reply;
}

// then all information will come directly from our public RTPacket structure

long VBANCMD_GetVoicemeeterType(unsigned long * pType)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	*pType = (unsigned long)(lpctx->PublicRTPPacket.voicemeeterType);
	return 0;
}

long VBANCMD_GetVoicemeeterVersion(unsigned long * pVersion)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	*pVersion = lpctx->PublicRTPPacket.voicemeeterVersion;
	return 0;
}


void TOOL_ConvertUTF8ToWCHAR(char * UTF8_String, WCHAR * wString, long nbCharMax)
{
	if (UTF8_String[0] == 0) 
	{
		wString[0]=0;
		return;
	}
	memset(wString,0, nbCharMax*sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8,0,(LPCSTR)UTF8_String,(int)strlen(UTF8_String),wString,nbCharMax);
	wString[nbCharMax-1]=0;
}


long VBANCMD_GetBusLabel(long index, WCHAR *pwsz)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	TOOL_ConvertUTF8ToWCHAR(lpctx->PublicRTPPacket.busLabelUTF8c60[index], pwsz, 64);
	return 0;
}

long VBANCMD_GetBusGain(long index, float * pValue)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	*pValue = ((float)lpctx->PublicRTPPacket.busGaindB100[index]) * 0.01f;
	return 0;
}

long VBANCMD_GetBusSel(long index, float * pValue)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if ((lpctx->PublicRTPPacket.busState[index] & VMRTSTATE_MODE_SEL)  != 0) *pValue =1.0f;
	else *pValue =0.0f;
	return 0;
}

long VBANCMD_GetBusMute(long index, float * pValue)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if ((lpctx->PublicRTPPacket.busState[index] & VMRTSTATE_MODE_MUTE)  != 0) *pValue =1.0f;
	else *pValue =0.0f;
	return 0;
}

long VBANCMD_GetBusMonitor(long index, float * pValue)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if ((lpctx->PublicRTPPacket.busState[index] & VMRTSTATE_MODE_MONITOR)  != 0) *pValue =1.0f;
	else *pValue =0.0f;
	return 0;
}



long VBANCMD_GetStripLabel(long index, WCHAR *pwsz)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	TOOL_ConvertUTF8ToWCHAR(lpctx->PublicRTPPacket.stripLabelUTF8c60[index], pwsz, 64);
	return 0;
}

long VBANCMD_GetStripGain(long index, float * pValue)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	*pValue = ((float)lpctx->PublicRTPPacket.stripGaindB100Layer1[index]) * 0.01f;
	return 0;
}

long VBANCMD_GetStripGainLayer(long index, long layer, float * pValue)
{
	short * pGain;
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	
	pGain = lpctx->PublicRTPPacket.stripGaindB100Layer1;
	pGain = pGain + (layer * 8);
	*pValue = ((float)pGain[index]) * 0.01f;
	return 0;
}

long VBANCMD_GetStripMute(long index, float * pValue)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if ((lpctx->PublicRTPPacket.stripState[index] & VMRTSTATE_MODE_MUTE)  != 0) *pValue =1.0f;
	else *pValue =0.0f;
	return 0;
}


static long G_MaskBusAssign_v1[8]={VMRTSTATE_MODE_BUSA1, VMRTSTATE_MODE_BUSB1, 0, 0, 0, 0, 0, 0};
static long G_MaskBusAssign_v2[8]={VMRTSTATE_MODE_BUSA1, VMRTSTATE_MODE_BUSA2, VMRTSTATE_MODE_BUSA3, VMRTSTATE_MODE_BUSB1, VMRTSTATE_MODE_BUSB2, 0, 0, 0};
static long G_MaskBusAssign_v3[8]={VMRTSTATE_MODE_BUSA1, VMRTSTATE_MODE_BUSA2, VMRTSTATE_MODE_BUSA3, VMRTSTATE_MODE_BUSA4, VMRTSTATE_MODE_BUSA5, VMRTSTATE_MODE_BUSB1, VMRTSTATE_MODE_BUSB2, VMRTSTATE_MODE_BUSB3};

long VBANCMD_GetStripAssignation(long index, long nBus, float * pValue)
{
	long * pMask;
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	pMask = NULL;
	if (lpctx->PublicRTPPacket.voicemeeterType == 1) pMask=G_MaskBusAssign_v1;
	if (lpctx->PublicRTPPacket.voicemeeterType == 2) pMask=G_MaskBusAssign_v2;
	if (lpctx->PublicRTPPacket.voicemeeterType == 3) pMask=G_MaskBusAssign_v3;

	if ((lpctx->PublicRTPPacket.stripState[index] & pMask[nBus])  != 0) *pValue =1.0f;
	else *pValue =0.0f;
	return 0;
}


long VBANCMD_GetLevel(long index, long nuChannel, float * pLevelDB)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	switch(index)
	{
	case 0:
		*pLevelDB = ((float)lpctx->PublicRTPPacket.inputLeveldB100[nuChannel]) * 0.01f;
		return 0;
	case 3:
		*pLevelDB = ((float)lpctx->PublicRTPPacket.outputLeveldB100[nuChannel]) * 0.01f;
		return 0;
	default:
		*pLevelDB=-100.0f;
		break;
	}
	return -1;
}


/******************************************************************************/
/* START / STOP VBAN CLIENT                                                   */
/******************************************************************************/


static DWORD WINAPI VBANCMD_Thread(LPT_VBANCMD_CONTEXT lpctx)
{
	SOCKADDR_IN from;
	LPT_VBAN_HEADER lpHeader;
	long rep, datasize, nbByte, mstemp, fromlen;
	char  Buffer[2048];
	unsigned char protocol;
	DWORD current_ipv4;
	HANDLE h_t;
	h_t=GetCurrentThread();
	SetThreadPriority(h_t,THREAD_PRIORITY_HIGHEST);
	CloseHandle(h_t);
	lpHeader = (LPT_VBAN_HEADER)Buffer;
	current_ipv4 = lpctx->IPv4Address;
	while (lpctx->Thread_flagOK == TRUE)
	{
		mstemp=1; 
		fromlen=sizeof(SOCKADDR_IN);
		rep= recvfrom(lpctx->vban_socket,Buffer,2048,0,(struct sockaddr*)&from,&fromlen);
		if (rep != SOCKET_ERROR)
		{
			mstemp = 0;
			// if we recieve something
			datasize = rep;
			if ((datasize > sizeof(T_VBAN_HEADER)) && (from.sin_addr.S_un.S_addr == current_ipv4))
			{
				// if it's a VBAN packet
				if (lpHeader->vban == 'NABV')
				{
					protocol =lpHeader->format_SR & VBAN_PROTOCOL_MASK;
					// if it's a VBAN-SERVICE packet
					if (protocol == VBAN_PROTOCOL_SERVICE)
					{
						if (lpHeader->format_nbc == VBAN_SERVICE_RTPACKET)
						{
							// if it's a RTPacket
							nbByte = datasize-sizeof(T_VBAN_HEADER);
							if (nbByte >= sizeof(T_VBAN_VMRT_PACKET)) 
							{
								// just copy it into our context structure and set dirty flag
								EnterCriticalSection(&(lpctx->ctc_section));
								memcpy(&(lpctx->CurrentRTPPacket), lpHeader+1, sizeof(T_VBAN_VMRT_PACKET));
								lpctx->CurrentRTPPacket_dirtyflag=1;
								lpctx->CurrentRTPPacket_counter++;
								LeaveCriticalSection(&(lpctx->ctc_section));
							}
						}
					}
				}
			}
		}
		// if we did nothing we wait for 1 ms, otherwise we wait for next O/S scheduler cycle
		Sleep(mstemp);
	}
	return 0;
}


long VBANCMD_StopThread(void)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->finitlib == 0) return -1;

	lpctx->ThreadStarted=0;	
	// Stop Thread
	lpctx->Thread_flagOK=FALSE;
	if (lpctx->Thread_handle != NULL)
	{
		WaitForSingleObject(lpctx->Thread_handle, INFINITE);
		CloseHandle(lpctx->Thread_handle);
		lpctx->Thread_handle=NULL;
	}
	// Close Soccket
	VBAN_CloseSocket(&(lpctx->vban_socket));
	
	memset(&(lpctx->CurrentRTPPacket), 0, sizeof(T_VBAN_VMRT_PACKET));
	memset(&(lpctx->PublicRTPPacket), 0, sizeof(T_VBAN_VMRT_PACKET));
	lpctx->CurrentRTPPacket_dirtyflag=1;
	return 0;
}


long VBANCMD_StartThread(char * szVBANStreamName, char * szIPAddressTo, unsigned short UDPport)
{
	long rep, reply=0;
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	if (lpctx->finitlib == 0) return -1;
	VBANCMD_StopThread();

	// build our header to send VBAN-TEXT request
	lpctx->VBANHeader.vban			='NABV';
	lpctx->VBANHeader.format_SR		=0x52;
	lpctx->VBANHeader.format_nbs	=0;
	lpctx->VBANHeader.format_nbc	=0;
	lpctx->VBANHeader.format_bit	=0x10;
	if (szVBANStreamName == NULL) strcpy(lpctx->VBANHeader.streamname, "Command1");
	else strncpy(lpctx->VBANHeader.streamname, szVBANStreamName,16);
	lpctx->VBANHeader.nuFrame		=0;
	
	// Init Socket with 2 MB on RCV buffer 
	lpctx->IPv4Address = inet_addr(szIPAddressTo);
	lpctx->UDPport = UDPport;
	rep=VBAN_InitSocket(lpctx, &(lpctx->vban_socket), UDPport, 1024*1024*2);
	if (rep != 0) return -2;

	// Start Thread 
	lpctx->Thread_flagOK=TRUE;
	lpctx->Thread_handle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )VBANCMD_Thread,(void *)lpctx,0,&(lpctx->Thread_ident));	
	if (lpctx->Thread_handle == NULL) 
	{
		reply=-100;
	}
	// we set STARTED flag only if all is good.
	lpctx->ThreadStarted=1;	
	return reply;
}







/******************************************************************************/
/* INIT / END LIB                                                             */
/******************************************************************************/

long VBANCMD_EndLib(void)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;

	if (lpctx->finitlib == 0) return 0;
	VBANCMD_StopThread();
	
	DeleteCriticalSection(&(lpctx->ctc_section));
	memset(lpctx,0, sizeof(T_VBANCMD_CONTEXT));
	return 0;
}

long VBANCMD_InitLib(long mode)
{
	LPT_VBANCMD_CONTEXT lpctx;
	lpctx=&G_vban_ctx;
	memset(lpctx,0,sizeof(T_VBANCMD_CONTEXT));
	lpctx->mode=mode;
	lpctx->finitlib=1;
	lpctx->WSAStartupReply=WSAStartup(MAKEWORD(1,1), &(lpctx->WSAData));
	InitializeCriticalSection(&(lpctx->ctc_section));
	lpctx->vban_socket = INVALID_SOCKET;
	return 0;	
}

