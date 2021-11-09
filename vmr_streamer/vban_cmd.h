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

#ifndef __VBAN_CMD_H__
#define __VBAN_CMD_H__


#pragma pack(1)

struct tagVBAN_HEADER 
{
	unsigned long vban;			// contains 'V' 'B', 'A', 'N'
	unsigned char format_SR;	// SR index (see SRList above) 
	unsigned char format_nbs;	// nb sample per frame (1 to 256) 
	unsigned char format_nbc;	// nb channel (1 to 256)
	unsigned char format_bit;	// mask = 0x07 (see DATATYPE table below)
	char streamname[16];		// stream name
	unsigned long nuFrame;		// growing frame number
};

#pragma pack()

typedef struct tagVBAN_HEADER T_VBAN_HEADER;
typedef struct tagVBAN_HEADER *PT_VBAN_HEADER;
typedef struct tagVBAN_HEADER *LPT_VBAN_HEADER;


#define VBAN_PROTOCOL_MASK			0xE0
#define VBAN_PROTOCOL_SERVICE		0x60

#define VBAN_SERVICE_RTPACKETREGISTER	32
#define VBAN_SERVICE_RTPACKET			33


/******************************************************************************/
/* FUNCTIONS                                                                  */
/******************************************************************************/

long VBANCMD_SendRequest_String(char * pString);
long VBANCMD_SendRequest_Float(char * szParam, float value);

long VBANCMD_SendRequest_RegisterRTPacket(unsigned char sTimeOut, char * szVBANStreamName);

// to check the VBAN connection works
long VBANCMD_GetIncomingRequestCounter(void);


/******************************************************************************/
/* CURRENT PARAMETERS FUNCTIONS                                               */
/******************************************************************************/
// we provide same function than Voicemeeter Remote API to simplify the process          

long VBANCMD_IsParameterDirty(void);

long VBANCMD_GetVoicemeeterType(unsigned long * pType);
long VBANCMD_GetVoicemeeterVersion(unsigned long * pVersion);

long VBANCMD_GetBusLabel(long index, WCHAR *pwsz);
long VBANCMD_GetBusGain(long index, float * pValue);
long VBANCMD_GetBusSel(long index, float * pValue);
long VBANCMD_GetBusMute(long index, float * pValue);
long VBANCMD_GetBusMonitor(long index, float * pValue);


long VBANCMD_GetStripLabel(long index, WCHAR *pwsz);
long VBANCMD_GetStripGain(long index, float * pValue);
long VBANCMD_GetStripGainLayer(long index, long layer, float * pValue);
long VBANCMD_GetStripMute(long index, float * pValue);
long VBANCMD_GetStripAssignation(long index, long nBus, float * pValue);

long VBANCMD_GetLevel(long index, long nuChannel, float * pLevelDB);



/******************************************************************************/
/* START / STOP THREAD                                                        */
/******************************************************************************/

long VBANCMD_StartThread(char * szVBANStreamName, char * szIPAddressTo, unsigned short UDPport);
long VBANCMD_StopThread(void);

/******************************************************************************/
/* INIT / END LIB                                                             */
/******************************************************************************/

long VBANCMD_EndLib(void);
long VBANCMD_InitLib(long mode);


#endif /* __VBAN_CMD_H__*/