/*--------------------------------------------------------------------------------*/
/* VMR_CLIENT: Reference example to use all functions of the VMR-API              */
/*--------------------------------------------------------------------------------*/
/* 'C' Sample Code to use VoicemeeterRemote                  V.Burel (c)2015-2021 */
/*                                                                                */
/*  Based on Minimal program ('C' Programming Under WIN32)                        */
/*  WEB  : http://pagesperso-orange.fr/vb-audio/fr/pub/programming/index.htm      */
/*  mail : infoprog@vb-audio.com                                                  */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to link VoicemeeterRemote.dll                                           */
/*  - How to Login / logout                                                       */
/*  - How to use GetParameter Function.                                           */
/*  - How to use SetParameter Function.                                           */
/*  - How to display new parameters values.                                       */
/*  - How to display levels.                                                      */
/*  - How to get audio device list.                                               */
/*  - How to change input and output audio device.                                */
/*  - How to send txt request, by a simple file.                                  */
/*  - How to send special Command.                                                */
/*  - How to receive M.I.D.I. message.                                            */
/*  - How to install Audio Callback.                                              */
/*  - How to manage Potato appplication gain/mute state (in virtual strip).       */
/*  - How to manage MacroButtons states.                                          */
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
/*  This source code can be compiled for 32bit or 64 bits targets as well         */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  LICENSING: VoicemeeterRemote.dll usage is driven by a license agreement       */
/*             given in VoicemeeterRemoteAPI.pdf or readme.txt                    */
/*                                                                                */
/*--------------------------------------------------------------------------------*/

#ifndef __cplusplus
	#ifndef STRICT
		#define STRICT
	#endif
#endif

#include <windows.h>
#include <stdio.h>

#include "vmr_client.h"
#include "VoicemeeterRemote.h"




/*******************************************************************************/
/**                           GET VOICEMEETER DIRECTORY                       **/
/*******************************************************************************/

static char uninstDirKey[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

#define INSTALLER_UNINST_KEY	"VB:Voicemeeter {17359A74-1236-5467}"


void RemoveNameInPath(char * szPath)
{
	long ll;
	ll=(long)strlen(szPath);
	while ((ll>0) && (szPath[ll]!='\\')) ll--;
	if (szPath[ll] == '\\') szPath[ll]=0;
}

#ifndef KEY_WOW64_32KEY
	#define KEY_WOW64_32KEY 0x0200
#endif

BOOL __cdecl RegistryGetVoicemeeterFolder(char * szDir)
{
	char szKey[256];
	char sss[1024];
	DWORD nnsize=1024;
	HKEY hkResult;
	LONG rep;
	DWORD pptype=REG_SZ;
	sss[0]=0;

	// build Voicemeeter uninstallation key
	strcpy(szKey,uninstDirKey);
	strcat(szKey,"\\");
	strcat(szKey,INSTALLER_UNINST_KEY);

	// open key
	rep=RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey,0, KEY_READ, &hkResult);
	if (rep != ERROR_SUCCESS)
	{
		// if not present we consider running in 64bit mode and force to read 32bit registry
		rep=RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey,0, KEY_READ | KEY_WOW64_32KEY, &hkResult); 
	}
	if (rep != ERROR_SUCCESS) return FALSE;
	// read uninstall profram path
	rep=RegQueryValueEx(hkResult,"UninstallString",0,&pptype,(unsigned char *)sss,&nnsize);
	RegCloseKey(hkResult);
	
	if (pptype != REG_SZ) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	// remove name to get the path only
	RemoveNameInPath(sss);
	if (nnsize>512) nnsize=512;
	strncpy(szDir,sss,nnsize);
	
	return TRUE;
}






/*******************************************************************************/
/**                                GET DLL INTERFACE                          **/
/*******************************************************************************/

static HMODULE G_H_Module=NULL;
static T_VBVMR_INTERFACE iVMR;

//if we directly link source code (for development only)
#ifdef VBUSE_LOCALLIB

	long InitializeDLLInterfaces(void)
	{
		iVMR.VBVMR_Login					=VBVMR_Login;
		iVMR.VBVMR_Logout					=VBVMR_Logout;
		iVMR.VBVMR_RunVoicemeeter			=VBVMR_RunVoicemeeter;
		iVMR.VBVMR_GetVoicemeeterType		=VBVMR_GetVoicemeeterType;
		iVMR.VBVMR_GetVoicemeeterVersion	=VBVMR_GetVoicemeeterVersion;
		iVMR.VBVMR_IsParametersDirty		=VBVMR_IsParametersDirty;
		iVMR.VBVMR_GetParameterFloat		=VBVMR_GetParameterFloat;
		iVMR.VBVMR_GetParameterStringA		=VBVMR_GetParameterStringA;
		iVMR.VBVMR_GetParameterStringW		=VBVMR_GetParameterStringW;

		iVMR.VBVMR_GetLevel					=VBVMR_GetLevel;
		iVMR.VBVMR_GetMidiMessage			=VBVMR_GetMidiMessage;
		iVMR.VBVMR_SetParameterFloat		=VBVMR_SetParameterFloat;
		iVMR.VBVMR_SetParameters			=VBVMR_SetParameters;
		iVMR.VBVMR_SetParametersW			=VBVMR_SetParametersW;
		iVMR.VBVMR_SetParameterStringA		=VBVMR_SetParameterStringA;
		iVMR.VBVMR_SetParameterStringW		=VBVMR_SetParameterStringW;

		iVMR.VBVMR_Output_GetDeviceNumber	=VBVMR_Output_GetDeviceNumber;
		iVMR.VBVMR_Output_GetDeviceDescA	=VBVMR_Output_GetDeviceDescA;
		iVMR.VBVMR_Output_GetDeviceDescW	=VBVMR_Output_GetDeviceDescW;
		iVMR.VBVMR_Input_GetDeviceNumber	=VBVMR_Input_GetDeviceNumber;
		iVMR.VBVMR_Input_GetDeviceDescA		=VBVMR_Input_GetDeviceDescA;
		iVMR.VBVMR_Input_GetDeviceDescW		=VBVMR_Input_GetDeviceDescW;

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
		iVMR.VBVMR_AudioCallbackRegister	=VBVMR_AudioCallbackRegister;
		iVMR.VBVMR_AudioCallbackStart		=VBVMR_AudioCallbackStart;
		iVMR.VBVMR_AudioCallbackStop		=VBVMR_AudioCallbackStop;
		iVMR.VBVMR_AudioCallbackUnregister	=VBVMR_AudioCallbackUnregister;
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
		iVMR.VBVMR_MacroButton_IsDirty		=VBVMR_MacroButton_IsDirty;
		iVMR.VBVMR_MacroButton_GetStatus	=VBVMR_MacroButton_GetStatus;
		iVMR.VBVMR_MacroButton_SetStatus	=VBVMR_MacroButton_SetStatus;

#endif
		return 0;
	}

//Dynamic link to DLL in 'C' (regular use)
#else

	long InitializeDLLInterfaces(void)
	{
		char szDllName[1024];
		memset(&iVMR,0,sizeof(T_VBVMR_INTERFACE));

		//get folder where is installed Voicemeeter
		if (RegistryGetVoicemeeterFolder(szDllName) == FALSE) 
		{
			// voicemeeter not installed
			return -100; 
		}
		//use right dll according O/S type
		if (sizeof(void*) == 8) strcat(szDllName,"\\VoicemeeterRemote64.dll");
		else strcat(szDllName,"\\VoicemeeterRemote.dll");
		
		// Load Dll
		G_H_Module=LoadLibrary(szDllName);
		if (G_H_Module == NULL) return -101;

		// Get function pointers
		iVMR.VBVMR_Login					=(T_VBVMR_Login)GetProcAddress(G_H_Module,"VBVMR_Login");
		iVMR.VBVMR_Logout					=(T_VBVMR_Logout)GetProcAddress(G_H_Module,"VBVMR_Logout");
		iVMR.VBVMR_RunVoicemeeter			=(T_VBVMR_RunVoicemeeter)GetProcAddress(G_H_Module,"VBVMR_RunVoicemeeter");
		iVMR.VBVMR_GetVoicemeeterType		=(T_VBVMR_GetVoicemeeterType)GetProcAddress(G_H_Module,"VBVMR_GetVoicemeeterType");
		iVMR.VBVMR_GetVoicemeeterVersion	=(T_VBVMR_GetVoicemeeterVersion)GetProcAddress(G_H_Module,"VBVMR_GetVoicemeeterVersion");
		
		iVMR.VBVMR_IsParametersDirty		=(T_VBVMR_IsParametersDirty)GetProcAddress(G_H_Module,"VBVMR_IsParametersDirty");
		iVMR.VBVMR_GetParameterFloat		=(T_VBVMR_GetParameterFloat)GetProcAddress(G_H_Module,"VBVMR_GetParameterFloat");
		iVMR.VBVMR_GetParameterStringA		=(T_VBVMR_GetParameterStringA)GetProcAddress(G_H_Module,"VBVMR_GetParameterStringA");
		iVMR.VBVMR_GetParameterStringW		=(T_VBVMR_GetParameterStringW)GetProcAddress(G_H_Module,"VBVMR_GetParameterStringW");
		iVMR.VBVMR_GetLevel					=(T_VBVMR_GetLevel)GetProcAddress(G_H_Module,"VBVMR_GetLevel");
		iVMR.VBVMR_GetMidiMessage			=(T_VBVMR_GetMidiMessage)GetProcAddress(G_H_Module,"VBVMR_GetMidiMessage");

		iVMR.VBVMR_SetParameterFloat		=(T_VBVMR_SetParameterFloat)GetProcAddress(G_H_Module,"VBVMR_SetParameterFloat");
		iVMR.VBVMR_SetParameters			=(T_VBVMR_SetParameters)GetProcAddress(G_H_Module,"VBVMR_SetParameters");
		iVMR.VBVMR_SetParametersW			=(T_VBVMR_SetParametersW)GetProcAddress(G_H_Module,"VBVMR_SetParametersW");
		iVMR.VBVMR_SetParameterStringA		=(T_VBVMR_SetParameterStringA)GetProcAddress(G_H_Module,"VBVMR_SetParameterStringA");
		iVMR.VBVMR_SetParameterStringW		=(T_VBVMR_SetParameterStringW)GetProcAddress(G_H_Module,"VBVMR_SetParameterStringW");

		iVMR.VBVMR_Output_GetDeviceNumber	=(T_VBVMR_Output_GetDeviceNumber)GetProcAddress(G_H_Module,"VBVMR_Output_GetDeviceNumber");
		iVMR.VBVMR_Output_GetDeviceDescA	=(T_VBVMR_Output_GetDeviceDescA)GetProcAddress(G_H_Module,"VBVMR_Output_GetDeviceDescA");
		iVMR.VBVMR_Output_GetDeviceDescW	=(T_VBVMR_Output_GetDeviceDescW)GetProcAddress(G_H_Module,"VBVMR_Output_GetDeviceDescW");
		iVMR.VBVMR_Input_GetDeviceNumber	=(T_VBVMR_Input_GetDeviceNumber)GetProcAddress(G_H_Module,"VBVMR_Input_GetDeviceNumber");
		iVMR.VBVMR_Input_GetDeviceDescA		=(T_VBVMR_Input_GetDeviceDescA)GetProcAddress(G_H_Module,"VBVMR_Input_GetDeviceDescA");
		iVMR.VBVMR_Input_GetDeviceDescW		=(T_VBVMR_Input_GetDeviceDescW)GetProcAddress(G_H_Module,"VBVMR_Input_GetDeviceDescW");

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
		iVMR.VBVMR_AudioCallbackRegister	=(T_VBVMR_AudioCallbackRegister)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackRegister");
		iVMR.VBVMR_AudioCallbackStart		=(T_VBVMR_AudioCallbackStart)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackStart");
		iVMR.VBVMR_AudioCallbackStop		=(T_VBVMR_AudioCallbackStop)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackStop");
		iVMR.VBVMR_AudioCallbackUnregister	=(T_VBVMR_AudioCallbackUnregister)GetProcAddress(G_H_Module,"VBVMR_AudioCallbackUnregister");
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
		iVMR.VBVMR_MacroButton_IsDirty		=(T_VBVMR_MacroButton_IsDirty)GetProcAddress(G_H_Module,"VBVMR_MacroButton_IsDirty");
		iVMR.VBVMR_MacroButton_GetStatus	=(T_VBVMR_MacroButton_GetStatus)GetProcAddress(G_H_Module,"VBVMR_MacroButton_GetStatus");
		iVMR.VBVMR_MacroButton_SetStatus	=(T_VBVMR_MacroButton_SetStatus)GetProcAddress(G_H_Module,"VBVMR_MacroButton_SetStatus");
#endif

		// check pointers are valid
		if (iVMR.VBVMR_Login == NULL) return -1;
		if (iVMR.VBVMR_Logout == NULL) return -2;
		if (iVMR.VBVMR_RunVoicemeeter == NULL) return -2;
		if (iVMR.VBVMR_GetVoicemeeterType == NULL) return -3;
		if (iVMR.VBVMR_GetVoicemeeterVersion == NULL) return -4;
		if (iVMR.VBVMR_IsParametersDirty == NULL) return -5;
		if (iVMR.VBVMR_GetParameterFloat == NULL) return -6;
		if (iVMR.VBVMR_GetParameterStringA == NULL) return -7;
		if (iVMR.VBVMR_GetParameterStringW == NULL) return -8;
		if (iVMR.VBVMR_GetLevel == NULL) return -9;
		if (iVMR.VBVMR_SetParameterFloat == NULL) return -10;
		if (iVMR.VBVMR_SetParameters == NULL) return -11;
		if (iVMR.VBVMR_SetParametersW == NULL) return -12;
		if (iVMR.VBVMR_SetParameterStringA == NULL) return -13;
		if (iVMR.VBVMR_SetParameterStringW == NULL) return -14;
		if (iVMR.VBVMR_GetMidiMessage == NULL) return -15;

		if (iVMR.VBVMR_Output_GetDeviceNumber == NULL) return -30;
		if (iVMR.VBVMR_Output_GetDeviceDescA == NULL) return -31;
		if (iVMR.VBVMR_Output_GetDeviceDescW == NULL) return -32;
		if (iVMR.VBVMR_Input_GetDeviceNumber == NULL) return -33;
		if (iVMR.VBVMR_Input_GetDeviceDescA == NULL) return -34;
		if (iVMR.VBVMR_Input_GetDeviceDescW == NULL) return -35;
		
#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
		if (iVMR.VBVMR_AudioCallbackRegister == NULL) return -40;
		if (iVMR.VBVMR_AudioCallbackStart == NULL) return -41;
		if (iVMR.VBVMR_AudioCallbackStop == NULL) return -42;
		if (iVMR.VBVMR_AudioCallbackUnregister == NULL) return -43;
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
		if (iVMR.VBVMR_MacroButton_IsDirty == NULL) return -50;
		if (iVMR.VBVMR_MacroButton_GetStatus == NULL) return -51;
		if (iVMR.VBVMR_MacroButton_SetStatus == NULL) return -52;
#endif
		return 0;
	}


#endif






/*******************************************************************************/
/*                                APPLICATION CONTEXT                          */
/*******************************************************************************/

#define NBPARAM_DISPLAYED 9

typedef struct tagAPP_CONTEXT
{
	HWND		hwnd_MainWindow;
	HINSTANCE	hinstance;
	BOOL		AudioIsStarted;

	HFONT		font;
	char		MIDIString[256];
	size_t		wTimer;

	HWND		hw_EditBox[NBPARAM_DISPLAYED];
} T_APP_CONTEXT, *PT_APP_CONTEXT, *LPT_APP_CONTEXT;

static T_APP_CONTEXT G_MainAppCtx = {NULL, NULL, FALSE};






/*******************************************************************************/
/**                                QUIT & ABOUT                               **/
/*******************************************************************************/

void ManageCloseMessage(HWND hw)
{
	PostMessage(hw,WM_DESTROY,0,0L);
}

void ManageAboutBox(HWND hw)
{
	char title[]="About...";
	char message[512];
	strcpy(message,SZPUBLICNAME);
	strcat(message,"\nVersion : ");
	strcat(message,SZPUBLICVERSION);
	strcat(message,"\nStandalone Application\n");
	strcat(message,"\nExample of 'C' Source code\n");
	
	MessageBox(hw,message,title,MB_APPLMODAL | MB_OK | MB_ICONINFORMATION);
}

void ManageInfoBox(HWND hw)
{
	long rep,vmType,vmVersion,v1,v2,v3,v4;
	char title[]="Server Info...";
	char message[512];
	char sz[128];
	
	//get Voicemeeter Program Type
	strcpy(message,"Server Name: ");
	rep=iVMR.VBVMR_GetVoicemeeterType(&vmType);
	if (rep == 0) 
	{
		if (vmType ==1) strcat(message, "Voicemeeter");
		if (vmType ==2) strcat(message, "Voicemeeter Banana");
		if (vmType ==3) strcat(message, "Voicemeeter Potato");
	}
	else strcat(message, "-not connected-");
	strcat(message,"\nVersion: ");

	//get Voicemeeter Version
	rep=iVMR.VBVMR_GetVoicemeeterVersion(&vmVersion);
	if (rep == 0) 
	{
		v1 = (vmVersion & 0xFF000000) >> 24;
		v2 = (vmVersion & 0x00FF0000) >> 16;
		v3 = (vmVersion & 0x0000FF00) >> 8;
		v4 = vmVersion & 0x000000FF;
		sprintf(sz,"%i.%i.%i.%i",v1,v2,v3,v4);
		strcat(message, sz);
	}
	else strcat(message, "-unknown-");
	strcat(message,"\n");
	
	MessageBox(hw,message,title,MB_APPLMODAL | MB_OK | MB_ICONINFORMATION);
}







/*******************************************************************************/
/*                                                                             */
/*                              AUDIO CALLBACK EXAMPLE                         */
/*                                                                             */
/*******************************************************************************/
/* With VB-Audio Callback Functions, it's now possible to use voicemeeter as   */
/* audio board and get signal from different point to analyze or process it.   */
/*******************************************************************************/


#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE 

typedef struct tagAUDIODSDPCTX
{
	float insertin_maxlevel[64];
	long  insertin_param_mute[64];
	RECT  insertin_param_mute_rect[64];

	float insertout_maxlevel[64];
	long  insertout_param_mute[64];
	RECT  insertout_param_mute_rect[64];

	float main_maxlevel[128];
	long  main_param_mute[64];
	RECT  main_param_mute_rect[128];

} T_AUDIODSDPCTX, *PT_AUDIODSDPCTX, *LPT_AUDIODSDPCTX;

static T_AUDIODSDPCTX G_DSP_Ctx;

void PROCESSING_initmyDSPContext(void)
{
	memset(&G_DSP_Ctx, 0 , sizeof(T_AUDIODSDPCTX));
}


/*******************************************************************************/
/*              DRAWING FUNCTION RELATED TO AUDIO CALLBACK EXAMPLE             */
/*******************************************************************************/

#define PROCESSING_DRAWING_Y0	340

void PROCESSING_DrawLevels_insertin(HDC dc)
{
	HBRUSH oldbrush;
	HPEN oldpen;
	float NormalLevel;
	long x0,y0,dx,dy,yy;
	long vi;
	x0=50;
	y0=PROCESSING_DRAWING_Y0;
	dx=10;
	dy=50;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<34;vi++)
	{
		NormalLevel=G_DSP_Ctx.insertin_maxlevel[vi];
		yy = (long)(dy * NormalLevel);
		if (yy > dy) yy=dy;

		//draw meter level
		if (yy > 0)
		{
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			Rectangle(dc,x0,y0+dy-yy,x0+dx,y0+dy);
		}
		if (yy < dy)
		{
			SelectObject(dc,GetStockObject(WHITE_BRUSH));
			Rectangle(dc,x0,y0,x0+dx,y0+dy-yy);
		}
		x0=x0+14;
	}

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

void PROCESSING_DrawMuteButton_insertin(HDC dc)
{
	RECT rect;
	HBRUSH oldbrush;
	HPEN oldpen;
	long x0,y0,dx,dy;
	long vi;
	x0=50;
	y0=PROCESSING_DRAWING_Y0+50+5;
	dx=10;
	dy=10;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<34;vi++)
	{
		if (G_DSP_Ctx.insertin_param_mute[vi] == 0) SelectObject(dc,GetStockObject(WHITE_BRUSH));
		else SelectObject(dc,GetStockObject(BLACK_BRUSH));
		
		rect.left=x0;
		rect.top=y0;
		rect.right=x0+dx;
		rect.bottom=y0+dy;
		G_DSP_Ctx.insertin_param_mute_rect[vi]=rect;
		Rectangle(dc,x0,y0,x0+dx,y0+dy);

		x0=x0+14;
	}

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

void PROCESSING_DrawLevels_insertout(HDC dc)
{
	HBRUSH oldbrush;
	HPEN oldpen;
	float NormalLevel;
	long x0,y0,dx,dy,yy;
	long vi;
	x0=50;
	y0=PROCESSING_DRAWING_Y0+70;
	dx=10;
	dy=50;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<64;vi++)
	{
		NormalLevel=G_DSP_Ctx.insertout_maxlevel[vi];
		yy = (long)(dy * NormalLevel);
		if (yy > dy) yy=dy;

		//draw meter level
		if (yy > 0)
		{
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			Rectangle(dc,x0,y0+dy-yy,x0+dx,y0+dy);
		}
		if (yy < dy)
		{
			SelectObject(dc,GetStockObject(WHITE_BRUSH));
			Rectangle(dc,x0,y0,x0+dx,y0+dy-yy);
		}
		x0=x0+14;
	}

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

void PROCESSING_DrawMuteButton_insertout(HDC dc)
{
	RECT rect;
	HBRUSH oldbrush;
	HPEN oldpen;
	long x0,y0,dx,dy;
	long vi;
	x0=50;
	y0=PROCESSING_DRAWING_Y0+70+50+5;
	dx=10;
	dy=10;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<64;vi++)
	{
		if (G_DSP_Ctx.insertout_param_mute[vi] == 0) SelectObject(dc,GetStockObject(WHITE_BRUSH));
		else SelectObject(dc,GetStockObject(BLACK_BRUSH));
		
		rect.left=x0;
		rect.top=y0;
		rect.right=x0+dx;
		rect.bottom=y0+dy;
		G_DSP_Ctx.insertout_param_mute_rect[vi]=rect;
		Rectangle(dc,x0,y0,x0+dx,y0+dy);

		x0=x0+14;
	}

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

void PROCESSING_DrawLevels_main(HDC dc)
{
	HBRUSH oldbrush;
	HPEN oldpen;
	float NormalLevel;
	long x0,y0,dx,dy,yy;
	long vi;
	x0=50;
	y0=PROCESSING_DRAWING_Y0+70+80;
	dx=10;
	dy=50;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<(34+64);vi++)
	{
		NormalLevel=G_DSP_Ctx.main_maxlevel[vi];
		yy = (long)(dy * NormalLevel);
		if (yy > dy) yy=dy;

		//draw meter level
		if (yy > 0)
		{
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			Rectangle(dc,x0,y0+dy-yy,x0+dx,y0+dy);
		}
		if (yy < dy)
		{
			SelectObject(dc,GetStockObject(WHITE_BRUSH));
			Rectangle(dc,x0,y0,x0+dx,y0+dy-yy);
		}
		x0=x0+14;
	}

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

void PROCESSING_DrawMuteButton_main(HDC dc)
{
	RECT rect;
	HBRUSH oldbrush;
	HPEN oldpen;
	long x0,y0,dx,dy;
	long vi;
	x0=50 + (34*14);
	y0=PROCESSING_DRAWING_Y0+70+80+50+5;
	dx=10;
	dy=10;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<64;vi++)
	{
		if (G_DSP_Ctx.main_param_mute[vi] == 0) SelectObject(dc,GetStockObject(WHITE_BRUSH));
		else SelectObject(dc,GetStockObject(BLACK_BRUSH));
		
		rect.left=x0;
		rect.top=y0;
		rect.right=x0+dx;
		rect.bottom=y0+dy;
		G_DSP_Ctx.main_param_mute_rect[vi]=rect;
		Rectangle(dc,x0,y0,x0+dx,y0+dy);

		x0=x0+14;
	}

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}


void PROCESSING_ManageLButtonDownOnMuteButton(HWND hw, int mx, int my)
{
	HDC dc;
	RECT rect;
	int vi;

	//check click in INSERT input mute buttons
	for (vi=0;vi<34;vi++)
	{
		rect=G_DSP_Ctx.insertin_param_mute_rect[vi];
		if ((mx>rect.left) && (mx<rect.right) && (my>rect.top) && (my<rect.bottom)) 
		{
			if (G_DSP_Ctx.insertin_param_mute[vi] == 0) G_DSP_Ctx.insertin_param_mute[vi]=1;
			else G_DSP_Ctx.insertin_param_mute[vi]=0;
		}
	}
	//check click in INSERT output mute buttons
	for (vi=0;vi<64;vi++)
	{
		rect=G_DSP_Ctx.insertout_param_mute_rect[vi];
		if ((mx>rect.left) && (mx<rect.right) && (my>rect.top) && (my<rect.bottom)) 
		{
			if (G_DSP_Ctx.insertout_param_mute[vi] == 0) G_DSP_Ctx.insertout_param_mute[vi]=1;
			else G_DSP_Ctx.insertout_param_mute[vi]=0;
		}
	}

	//check click in Main mute buttons
	for (vi=0;vi<64;vi++)
	{
		rect=G_DSP_Ctx.main_param_mute_rect[vi];
		if ((mx>rect.left) && (mx<rect.right) && (my>rect.top) && (my<rect.bottom)) 
		{
			if (G_DSP_Ctx.main_param_mute[vi] == 0) G_DSP_Ctx.main_param_mute[vi]=1;
			else G_DSP_Ctx.main_param_mute[vi]=0;
		}
	}


	dc=GetDC(hw);
	PROCESSING_DrawMuteButton_insertin(dc);
	PROCESSING_DrawMuteButton_insertout(dc);
	PROCESSING_DrawMuteButton_main(dc);
	ReleaseDC(hw,dc);
}




/*******************************************************************************/
/*                             AUDIO CALLBACK EXAMPLE                          */
/*******************************************************************************/

long __stdcall PROCESSING_MyCallback(void * lpUser, long nCommand, void * lpData, long nnn)
{
	float * lpBufferIn;
	float * lpBufferOut;
	float signal, level;
	int nuChannel,nbs;
	LPT_AUDIODSDPCTX lpctx;
	VBVMR_LPT_AUDIOINFO pinfo;
	VBVMR_LPT_AUDIOBUFFER lpa;
	LPT_APP_CONTEXT lpapp;

	lpapp = &G_MainAppCtx;
	lpctx = (LPT_AUDIODSDPCTX)lpUser;
	switch(nCommand)
	{
	
	//--------------------------------------
	// Init/End your object and allocate your memory
	//--------------------------------------
	case VBVMR_CBCOMMAND_STARTING:
		pinfo = (VBVMR_LPT_AUDIOINFO)lpData;
		// this is the first call of your Callback, made to let you initialize your possible different 
		// DSP processing objects, allocate memory, precompute data...
		// REM: this function is called once, even if you registered this callback several times for different streams
		break;		
	case VBVMR_CBCOMMAND_ENDING:
		pinfo = (VBVMR_LPT_AUDIOINFO)lpData;
		// this is the last call of your Callback, to release all your structure previously 
		// allocated in the first call.
		// REM: this function is called once, even if you registered this callback several times for different streams
		break;		
	case VBVMR_CBCOMMAND_CHANGE:
		pinfo = (VBVMR_LPT_AUDIOINFO)lpData;
		// this command is called if the samplerate or buffer size have changed 
		// (Voicemeeter main audio stream has been changed because user changed output A1 device 
		// or change buffer size or preferred samplerate).
		//
		// you can use info structure to change a parameter or set a flag to say that the stream will have to be 
		// restarted. You can also post a message to ask for restart audio callback after having waited for some ms.
		//
		// After this Command, the CALLBCAK will be called again with VBVMR_CBCOMMAND_ENDING to let your release 
		// your different object if any (those possibily initialized on VBVMR_CBCOMMAND_STARTING first call)
		//
		// REM: this function is called once, even if you registered this callback several times for different streams

		// here we simply use a post command to restart the stream
		PostMessage(lpapp->hwnd_MainWindow,WM_COMMAND, IDM_RESTART,0);
		break;		
	//--------------------------------------
	// process buffer for Input INSERT
	//--------------------------------------
	case VBVMR_CBCOMMAND_BUFFER_IN:
		lpa =(VBVMR_LPT_AUDIOBUFFER)lpData;
		//We compute signal to get level on all possible inputs ( audiobuffer_nbi )
		//with this Callback, we have egual number of inputs and outputs.
		for (nuChannel=0;nuChannel<lpa->audiobuffer_nbi;nuChannel++)
		{
			lpBufferIn = lpa->audiobuffer_r[nuChannel];	//get pointer on input related to channel nuChannel
			lpBufferOut = lpa->audiobuffer_w[nuChannel]; //get pointer on output related to channel nuChannel
			lpctx->insertin_maxlevel[nuChannel] = lpctx->insertin_maxlevel[nuChannel] * 0.99f; //factor to decrease meter bar
			
			//for every sample in buffer 
			for (nbs=0;nbs < lpa->audiobuffer_nbs; nbs++)
			{
				signal= lpBufferIn[nbs];	
				
				//compute level according signal 
				level=signal;
				if (level < 0.0f) level=-level;
				if (level > lpctx->insertin_maxlevel[nuChannel]) lpctx->insertin_maxlevel[nuChannel]=level;

				//mute signal if parameter is set
				if (lpctx->insertin_param_mute[nuChannel] != 0) signal=0.0f;
				//set signal in output buffer
				lpBufferOut[nbs] = signal;
			}
		}
		break;
	//--------------------------------------
	// process buffer for Output INSERT
	//--------------------------------------
	case VBVMR_CBCOMMAND_BUFFER_OUT:
		lpa =(VBVMR_LPT_AUDIOBUFFER)lpData;
		//We compute signal to get level on all possible outputs ( audiobuffer_nbi )
		//with this Callback, we have egual number of inputs and outputs.
		for (nuChannel=0;nuChannel<lpa->audiobuffer_nbi;nuChannel++)
		{
			lpBufferIn = lpa->audiobuffer_r[nuChannel];	//get pointer on input related to channel nuChannel
			lpBufferOut = lpa->audiobuffer_w[nuChannel]; //get pointer on output related to channel nuChannel
			lpctx->insertout_maxlevel[nuChannel] = lpctx->insertout_maxlevel[nuChannel] * 0.99f; //factor to decrease meter bar
			
			//for every sample in buffer 
			for (nbs=0;nbs < lpa->audiobuffer_nbs; nbs++)
			{
				signal= lpBufferIn[nbs];	
				
				//compute level according signal 
				level=signal;
				if (level < 0.0f) level=-level;
				if (level > lpctx->insertout_maxlevel[nuChannel]) lpctx->insertout_maxlevel[nuChannel]=level;

				//mute signal if parameter is set
				if (lpctx->insertout_param_mute[nuChannel] != 0) signal=0.0f;
				//set signal in output buffer
				lpBufferOut[nbs] = signal;
			}
		}
		break;
	//--------------------------------------
	// process buffer for All I/O
	//--------------------------------------
	case VBVMR_CBCOMMAND_BUFFER_MAIN:	
		lpa =(VBVMR_LPT_AUDIOBUFFER)lpData;
		//We compute signal to get level on all possible inputs ( audiobuffer_nbi) 
		//and we process all outputs ( audiobuffer_nbo ) to pass trhough the signal.
		//opposite to INSERT Callback we got all I/O as input (nbi inputs + nbo outputs) 
		//and only Voicemeeter outputs as output buffer (2x or 5x or 8x BUS of 8 channels).
		
		//process inputs (all I/O's)
		for (nuChannel=0;nuChannel<lpa->audiobuffer_nbi;nuChannel++)
		{
			lpBufferIn = lpa->audiobuffer_r[nuChannel];	//get pointer on input related to channel nuChannel
			lpctx->main_maxlevel[nuChannel] = lpctx->main_maxlevel[nuChannel] * 0.99f; //factor to decrease meter bar
			
			//for every sample in buffer 
			for (nbs=0;nbs < lpa->audiobuffer_nbs; nbs++)
			{
				signal= lpBufferIn[nbs];	
				
				//compute level according signal 
				level=signal;
				if (level < 0.0f) level=-level;
				if (level > lpctx->main_maxlevel[nuChannel]) lpctx->main_maxlevel[nuChannel]=level;

			}
		}

		//process outputs (all Bus channels)
		for (nuChannel=0;nuChannel<lpa->audiobuffer_nbo;nuChannel++)
		{
			lpBufferIn = lpa->audiobuffer_r[lpa->audiobuffer_nbi-lpa->audiobuffer_nbo+nuChannel];	//get pointer on output related to output BUS
			lpBufferOut = lpa->audiobuffer_w[nuChannel];	//get pointer on output related to channel nuChannel
			//for every sample in buffer 
			for (nbs=0;nbs < lpa->audiobuffer_nbs; nbs++)
			{
				signal= lpBufferIn[nbs];	
				//mute signal if parameter is set
				if (lpctx->main_param_mute[nuChannel] != 0) signal=0.0f;
				//set signal in output buffer
				lpBufferOut[nbs] = signal;
			}
		}		
		break;
	}
	return 0;
}


//--------------------------------------------------------------------------------
// in the following function , there is 6x examples of Audio Callback Registration
// to let you understand the possibility to process 3 different streams in Voicemeeter
//   - MAIN: all I/O : as virtual audio board you receive all inputs and outputs, and you can change all outputs channels
//	 - IN: Input INSERT to receive all Voicemeeter Input to be able to process them as INSERT 
//	 - OUT: Output INSERT to receive all Voicemeeter Pre Fader BUS Output to be able to process them as INSERT 
//--------------------------------------------------------------------------------

long PROCESSING_RegisterCallback()
{
	char szClientName[64]="VB-Audio Client test";
	//EXAMPLE 1
	//register our callback to process input insert stream
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_IN, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);
	
	//EXAMPLE 2
	//register our callback to process input insert stream
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_OUT, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);

	//EXAMPLE 3
	//register our callback to process 2x Streams: input and output insert stream
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_IN | VBVMR_AUDIOCALLBACK_OUT, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);

	//EXAMPLE 4
	//register our callback to process main stream containing all Voicemeeter I/O
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_MAIN, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);

	//EXAMPLE 5
	//register our callback to process all 3x streams
	iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_MAIN | VBVMR_AUDIOCALLBACK_IN | VBVMR_AUDIOCALLBACK_OUT, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);

	//EXAMPLE 6
	//register our callback to process all 3x streams by calling 3x times the Register Functions
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_MAIN, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_IN, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);
	//iVMR.VBVMR_AudioCallbackRegister(VBVMR_AUDIOCALLBACK_OUT, PROCESSING_MyCallback, &G_DSP_Ctx, szClientName);
	return 0;
}

long PROCESSING_UnregisterCallback()
{
	
	iVMR.VBVMR_AudioCallbackUnregister();
	return 0;
}


#endif /*VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE */










/*******************************************************************************/
/*                                  Manage Menu                                */
/*******************************************************************************/

void DrawCurrentLevels(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	HBRUSH oldbrush;
	HPEN oldpen;
	float NormalLevel;
	long x0,y0,dx,dy,yy;
	long vi;
	x0=350;
	y0=30;
	dx=10;
	dy=230;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));

	for (vi=0;vi<34;vi++)
	{
		NormalLevel=0.0f;
		iVMR.VBVMR_GetLevel(0, vi, &NormalLevel);
		yy = (long)(dy * NormalLevel);
		if (yy > dy) yy=dy;

		//draw meter level
		if (yy > 0)
		{
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			Rectangle(dc,x0,y0+dy-yy,x0+dx,y0+dy);
		}
		if (yy < dy)
		{
			SelectObject(dc,GetStockObject(WHITE_BRUSH));
			Rectangle(dc,x0,y0,x0+dx,y0+dy-yy);
		}

		x0=x0+14;
	}


	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

void DrawCurrentMIDIMessage(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	RECT rect;
	HFONT oldfont;
	HBRUSH oldbrush;
	HPEN oldpen;

	rect.left=10+250+10;
	rect.top=310;
	rect.right=rect.left+370;
	rect.bottom=rect.top+20;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));
	oldfont = (HFONT)SelectObject(dc,lpapp->font);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(0,0,0));

	
	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,7,7);
	rect.left +=5;
	rect.right-=5;
	DrawText(dc,lpapp->MIDIString,(int)strlen(lpapp->MIDIString),&rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
		

	SelectObject(dc,oldfont);
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	
}	

void DrawCurrentValues(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	RECT rect;
	char szParam[128];
	char sss[512];
	long vi,rep;
	float fff;
	HFONT oldfont;
	HBRUSH oldbrush;
	HPEN oldpen;

	rect.left=10+250+10;
	rect.top=30;
	rect.right=rect.left+70;
	rect.bottom=rect.top+20;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));
	oldfont = (HFONT)SelectObject(dc,lpapp->font);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(0,0,0));

	for (vi=0;vi<NBPARAM_DISPLAYED;vi++)
	{
		szParam[0]=0;
		GetWindowText(lpapp->hw_EditBox[vi],szParam,128);
		
		if (vi == (NBPARAM_DISPLAYED-1))
		{
			rect.right +=300;
			RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,7,7);
			rep=iVMR.VBVMR_GetParameterStringA(szParam, sss);
			if (rep < 0) sprintf(sss,"Err:%i",rep);
		}
		else
		{
			RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,7,7);
			rep=iVMR.VBVMR_GetParameterFloat(szParam, &fff);
			if (rep < 0) sprintf(sss,"Err:%i",rep);
			else sprintf(sss,"%0.3f",fff);
		}
		DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		
		rect.top +=30;
		rect.bottom +=30;
	}

	SelectObject(dc,oldfont);
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);

}

#ifdef VMR_INCLUDE_MACROBUTTONS_REMOTING

#define NB_MACRO_BUTTON 80
#define NB_MACRO_BUTTON_X 10
#define NB_MACRO_BUTTON_Y 8

void DrawCurrentButtonState(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	char sss[128];
	HFONT oldfont;
	HBRUSH oldbrush;
	HPEN oldpen;
	float Value;
	long xx0, x0,y0,nbc,vi, dx, dy;
	RECT rect;
	x0=850;
	y0=35;
	dx=26;
	dy=26;

	xx0=x0;
	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(WHITE_BRUSH));
	oldfont = (HFONT)SelectObject(dc,lpapp->font);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(0,0,0));

	nbc=0;
	for (vi= 0; vi< NB_MACRO_BUTTON ;vi++)
	{
		rect.left=x0;
		rect.top=y0;
		rect.right=rect.left+dx;
		rect.bottom=rect.top+dy;
		iVMR.VBVMR_MacroButton_GetStatus(vi, &Value, VBVMR_MACROBUTTON_MODE_DEFAULT);
		if (Value == 0.0f)
		{
			SelectObject(dc,GetStockObject(LTGRAY_BRUSH));
		}
		else
		{
			SelectObject(dc,GetStockObject(WHITE_BRUSH));
		}
		Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
		sprintf(sss,"%02i",vi);
		DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		x0=x0+dx+2;
		nbc++;
		if (nbc >= NB_MACRO_BUTTON_X)
		{
			nbc=0;
			x0=xx0;
			y0+=dy+2;
		}
	}

	SelectObject(dc,oldfont);
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);

}

void MACROBUTTONS_ManageLButtonDown(HWND hw, int mx, int my)
{
	float Value;
	long xx0, x0,y0,nbc,vi, dx, dy;
	RECT rect;
	x0=850;
	y0=35;
	dx=26;
	dy=26;

	xx0=x0;

	nbc=0;
	for (vi= 0; vi< NB_MACRO_BUTTON ;vi++)
	{
		rect.left=x0;
		rect.top=y0;
		rect.right=rect.left+dx;
		rect.bottom=rect.top+dy;
		
		if ((mx >= rect.left) && (mx < rect.right) && (my >= rect.top) && (my < rect.bottom))
		{
			iVMR.VBVMR_MacroButton_GetStatus(vi, &Value, VBVMR_MACROBUTTON_MODE_DEFAULT);
			if (Value == 0.0f) Value=1.0f;
			else Value = 0.0f;
			iVMR.VBVMR_MacroButton_SetStatus(vi, Value, VBVMR_MACROBUTTON_MODE_DEFAULT);
			return;
		}

		x0=x0+dx+2;
		nbc++;
		if (nbc >= NB_MACRO_BUTTON_X)
		{
			nbc=0;
			x0=xx0;
			y0+=dy+2;
		}
	}
	
}

#endif

void DrawAllStuff(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	HFONT oldfont;
	char sss[128];
	RECT rect;
	
	oldfont = (HFONT)SelectObject(dc,lpapp->font);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(50,50,50));

	// Draw some static Labels
	strcpy(sss,"Parameters Name:");
	rect.left=10;
	rect.top=10;
	rect.right=rect.left+250;
	rect.bottom=rect.top+20;
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	strcpy(sss,"Value:");
	rect.left=rect.right+10;
	rect.right=rect.left+70;
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	strcpy(sss,"Input Levels (Pre-Fader):");
	rect.left=rect.right+10;
	rect.right=rect.left+200;
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	rect.top=310;
	rect.bottom=rect.top+20;
	strcpy(sss,"Incoming M.I.D.I. messages:");
	rect.left=10;
	rect.right=rect.left+200;
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);


	DrawCurrentValues(lpapp, hw, dc);


#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE 
	rect.left=10;
	rect.top=PROCESSING_DRAWING_Y0;
	rect.right=rect.left+40;
	rect.bottom=rect.top+50;
	
	strcpy(sss,"Insert");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	rect.top +=14;
	strcpy(sss,"Input");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	rect.top +=14;
	strcpy(sss,"Level");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	rect.top=PROCESSING_DRAWING_Y0+50+1;
	rect.bottom=rect.top+15;
	strcpy(sss,"Mute:");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	
	PROCESSING_DrawMuteButton_insertin(dc);
	
	rect.left=10;
	rect.top=PROCESSING_DRAWING_Y0+70;
	rect.right=rect.left+40;
	rect.bottom=rect.top+50;
	
	strcpy(sss,"Insert");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	rect.top +=14;
	strcpy(sss,"Output");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	rect.top +=14;
	strcpy(sss,"Level");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	rect.top=PROCESSING_DRAWING_Y0+70+50+1;
	rect.bottom=rect.top+15;
	strcpy(sss,"Mute:");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	PROCESSING_DrawMuteButton_insertout(dc);


	rect.left=10;
	rect.top=PROCESSING_DRAWING_Y0+70+80;
	rect.right=rect.left+40;
	rect.bottom=rect.top+50;
	
	strcpy(sss,"All");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	rect.top +=14;
	strcpy(sss,"I/O");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	rect.top +=14;
	strcpy(sss,"Level");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	rect.top=PROCESSING_DRAWING_Y0+70+80+50+1;
	rect.bottom=rect.top+15;
	strcpy(sss,"Mute:");
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);
	PROCESSING_DrawMuteButton_main(dc);
#endif

#ifdef VMR_INCLUDE_MACROBUTTONS_REMOTING
	strcpy(sss,"Macro Buttons Status:");
	rect.left=850;
	rect.top=10;
	rect.right=rect.left+250;
	rect.bottom=rect.top+20;
	DrawText(dc,sss,(int)strlen(sss),&rect, DT_SINGLELINE | DT_LEFT | DT_TOP);

	DrawCurrentButtonState(lpapp, hw, dc);
#endif
	SelectObject(dc,oldfont);

}

/*******************************************************************************/
/*                                  Load Request File                          */
/*******************************************************************************/

static void LoadRequestFile(LPT_APP_CONTEXT lpapp, HWND hw)
{
	BOOL erreur=FALSE;
	char Buffer[1024*64];
	HANDLE hfile;	
	long nbByte, nbRead;
	BOOL ok;
	char filter[]="Request Script (*.txt)\0*.txt\0All (*.*)\0*.*\0\0";
	char name[MAX_PATH]="*.txt";

	// let the user select a file
	OPENFILENAME cc;
	memset(&cc,0, sizeof(OPENFILENAME));
	
	cc.lStructSize			= sizeof(OPENFILENAME);
	cc.hwndOwner			= hw;
	cc.lpstrFilter			= filter;
	cc.nFilterIndex			= 0;
	cc.lpstrFile			= name;
	cc.nMaxFile				= MAX_PATH;
	cc.lpstrInitialDir		= NULL;
	cc.lpstrTitle			= "Load Request Script";
	cc.Flags				= OFN_NONETWORKBUTTON | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;

	ok=GetOpenFileName(&cc);
	if (ok == TRUE) 
	{
		//open the file
		hfile=CreateFile(name,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (hfile == INVALID_HANDLE_VALUE) return;
		//read it 
		nbByte = GetFileSize (hfile, NULL) ; 
		if ((nbByte > 0) && (nbByte < 64000))
		{
			erreur=ReadFile(hfile,Buffer,nbByte,(unsigned long*)&nbRead,NULL);
			if (erreur != 0) erreur=TRUE;
			if (nbRead == nbByte)
			{
				//send the entire txt file (zero terminal)
				Buffer[nbByte]=0;
				iVMR.VBVMR_SetParameters(Buffer);
			}
		}
		CloseHandle(hfile);
	}
}


/*******************************************************************************/
/*                                  Manage Menu                                */
/*******************************************************************************/

void ManageMenu(LPT_APP_CONTEXT lpapp, HWND hw, WPARAM wparam,LPARAM lparam)
{
	WCHAR wszName[512];
	char szName[512];
	char szParam[128];
	long nuCommand,nNotify,nuDev, nType;
	HDC dc;
	nuCommand=(long)LOWORD(wparam);
	nNotify  =(long)HIWORD(wparam);
	switch(nuCommand)
	{
		case IDM_LOAD:
			LoadRequestFile(lpapp, hw);
			break;
		case IDM_SERVERINFO:
			ManageInfoBox(hw);
			break;
		case IDM_QUIT:
			ManageCloseMessage(hw);
			break;
		case IDM_ABOUT:
			ManageAboutBox(hw);
			break;

		//Simple Request Test
		case IDM_TEST_1:
			iVMR.VBVMR_SetParameterFloat("Strip[0].gain", -10.0f);
			break;
		case IDM_TEST_2:
			iVMR.VBVMR_SetParameterFloat("Strip[0].mute", 1.0f);
			break;
		case IDM_TEST_3:
			iVMR.VBVMR_SetParameterFloat("Strip[0].mute", 0.0f);
			break;
		//Multiple Request Test
		case IDM_MTEST_1:
			iVMR.VBVMR_SetParameters("Strip[0].mute=1;Strip[1].mute=1;Strip[2].mute=1");
			break;
		case IDM_MTEST_2:
			iVMR.VBVMR_SetParameters("Strip[0].gain=-30.0;Strip[0].mute=0\n"
									"Strip[1].gain=-30.0;Strip[1].mute=0\n"
									"Strip[2].gain=-30.0;Strip[2].mute=0\n");
			break;
		case IDM_MTEST_3:
			iVMR.VBVMR_SetParameters("Strip[0].gain=0.0;Strip[0].mute=0\n"
									"Strip[1].gain=0.0;Strip[1].mute=0\n"
									"Strip[2].gain=0.0;Strip[2].mute=0\n");
			break;

		//Recorder command 

		case IDM_COMMAND_REC_STOP:
			iVMR.VBVMR_SetParameters("recorder.stop =1");
			break;
		case IDM_COMMAND_REC_PLAY:
			iVMR.VBVMR_SetParameters("recorder.play =1");
			break;
		case IDM_COMMAND_REC_REC:
			iVMR.VBVMR_SetParameters("recorder.rec =1");
			break;
		case IDM_COMMAND_REC_PAUSE:
			iVMR.VBVMR_SetParameters("recorder.pause =1");
			break;
		case IDM_COMMAND_REC_FF:
			iVMR.VBVMR_SetParameters("recorder.ff =1");
			break;
		case IDM_COMMAND_REC_REW:
			iVMR.VBVMR_SetParameters("recorder.rew =1");
			break;
		case IDM_COMMAND_REC_GOTO:
			iVMR.VBVMR_SetParameters("recorder.goto =0");
			break;
		case IDM_COMMAND_REC_GOTO1:
			iVMR.VBVMR_SetParameters("recorder.goto =01:00");
			break;
		case IDM_COMMAND_REC_GOTO2:
			iVMR.VBVMR_SetParameters("recorder.goto =02:00");
			break;
		case IDM_COMMAND_REC_RESTART:
			iVMR.VBVMR_SetParameters("recorder.replay =1");
			break;

		//Special command 
		case IDM_COMMAND_SHUTDOWN:
			iVMR.VBVMR_SetParameterFloat("Command.shutdown", 1.0f);
			break;
		case IDM_COMMAND_SHOW:
			iVMR.VBVMR_SetParameterFloat("Command.Show", 1.0f);
			break;
		case IDM_COMMAND_HIDE:
			iVMR.VBVMR_SetParameterFloat("Command.Show", 0.0f);
			break;
		case IDM_COMMAND_RESTART:
			iVMR.VBVMR_SetParameterFloat("Command.Restart", 1.0f);
			break;
		case IDM_COMMAND_EJECT:
			iVMR.VBVMR_SetParameterFloat("Command.Eject", 1.0f);
			break;
		case IDM_COMMAND_RESET:
			iVMR.VBVMR_SetParameterFloat("Command.Reset", 1.0f);
			break;
		//volume ctrl test
		case IDM_COMMAND_VOLAPP_1:
			iVMR.VBVMR_SetParameterFloat("Strip(5).App(0).Gain", 0.5f);
			break;
		case IDM_COMMAND_VOLAPP_2:
			iVMR.VBVMR_SetParameterFloat("Strip(6).App(0).Gain", 0.6f);
			break;
		case IDM_COMMAND_VOLAPP_3:
			iVMR.VBVMR_SetParameterFloat("Strip(7).App(0).Gain", 0.7f);
			break;
		case IDM_COMMAND_VOLAPP_4:
			iVMR.VBVMR_SetParameterFloat("Strip(5).App(0).Mute", 1.0f);
			break;
		case IDM_COMMAND_VOLAPP_5:
			iVMR.VBVMR_SetParameterFloat("Strip(6).App(0).Mute", 1.0f);
			break;
		case IDM_COMMAND_VOLAPP_6:
			iVMR.VBVMR_SetParameterFloat("Strip(7).App(0).Mute", 1.0f);
			break;
		case IDM_COMMAND_VOLAPP_7:
			iVMR.VBVMR_SetParameterFloat("Strip(5).App(1).Gain", 0.5f);
			break;
		//Volume control by Appliaction name "Windows Media Player"
		case IDM_COMMAND_VOLAPP_8:
			iVMR.VBVMR_SetParameters("Strip(5).AppGain = (\"Windows\", 0.1)");
			break;
		case IDM_COMMAND_VOLAPP_9:
			iVMR.VBVMR_SetParameters("Strip(5).AppMute = (\"Windows\", 1)");
			break;

		//Label Test
		case IDM_LABEL_STRIP3:
			iVMR.VBVMR_SetParameters("Strip(3).Label=\"Name3\"");
			break;
		case IDM_LABEL_STRIP4:
			iVMR.VBVMR_SetParameters("Strip(4).Label=\"Name4\"");
			break;
		case IDM_LABEL_BUS5:
			iVMR.VBVMR_SetParameters("BUS(5).Label=\"Name5\"");
			break;
		case IDM_LABEL_BUS6:
			iVMR.VBVMR_SetParameters("BUS(6).Label=\"Name6\"");
			break;

		//change device by SetPAramtersW
		case IDM_SETPARAMETERW1:
			iVMR.VBVMR_SetParametersW(L"Bus[0].device.wdm=\"Speakers (High Definition Audio Device)\"");
			break;
		case IDM_SETPARAMETERW2:
			iVMR.VBVMR_SetParametersW(L"Bus[1].device.wdm=\"\"");
			break;
		case IDM_SETPARAMETERW3:
			iVMR.VBVMR_SetParametersW(L"Strip[0].device.wdm=\"Microphone (High Definition Audio Device)\"");
			break;
		case IDM_SETPARAMETERW4:
			iVMR.VBVMR_SetParametersW(L"Strip[0].device = \"\"");
			break;

		case IDM_REMOVEDEVICE_IN1:
			strcpy(szParam,"Strip[0].device");
			iVMR.VBVMR_SetParameterStringA(szParam, "");
			break;
		case IDM_REMOVEDEVICE_OUT1:
			strcpy(szParam,"bus[0].device");
			iVMR.VBVMR_SetParameterStringA(szParam, "");
			break;
		//Audio CallBack
#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
		case IDM_COMMAND_ACB_REGISTER:
			PROCESSING_RegisterCallback();
			break;
		case IDM_COMMAND_ACB_UNREGISTER:
			PROCESSING_UnregisterCallback();
			lpapp->AudioIsStarted=FALSE;			
			break;
		case IDM_COMMAND_ACB_START:
			lpapp->AudioIsStarted=TRUE;			
			iVMR.VBVMR_AudioCallbackStart();
			break;
		case IDM_COMMAND_ACB_STOP:
			lpapp->AudioIsStarted=FALSE;			
			iVMR.VBVMR_AudioCallbackStop();
			break;
		//the stream is restarted because an audio engine restart
		case IDM_RESTART:
			Sleep(50);	//we wait a bit and we restart the stream
			iVMR.VBVMR_AudioCallbackStart();
			break;
#endif

	}

	//something changed in Edit Box's
	if ((nuCommand >=IDC_EDITBOX) && (nuCommand <(IDC_EDITBOX+NBPARAM_DISPLAYED)))
	{
		if (nNotify == EN_CHANGE) 
		{	
			dc=GetDC(hw);
			DrawCurrentValues(lpapp, hw, dc);
			ReleaseDC(hw,dc);
		}
		return;
	}
	//Select input device by Menu
	if ((nuCommand >IDM_DEVICE_IN1) && (nuCommand <(IDM_DEVICE_IN1+256)))
	{
		nuDev = nuCommand -IDM_DEVICE_IN1-1;
		iVMR.VBVMR_Input_GetDeviceDescW(nuDev,&nType, wszName, NULL);
		strcpy(szParam,"Strip[0].device");
		if (nType==VBVMR_DEVTYPE_MME) strcat(szParam,".mme");
		if (nType==VBVMR_DEVTYPE_WDM) strcat(szParam,".wdm");
		if (nType==VBVMR_DEVTYPE_KS) strcat(szParam,".ks");
		iVMR.VBVMR_SetParameterStringW(szParam, wszName);
		return;
	}
	//Select output device by Menu
	if ((nuCommand >IDM_DEVICE_OUT1) && (nuCommand <(IDM_DEVICE_OUT1+256)))
	{
		nuDev = nuCommand -IDM_DEVICE_OUT1-1;
		iVMR.VBVMR_Output_GetDeviceDescA(nuDev,&nType, szName, NULL);
		strcpy(szParam,"bus[0].device");
		if (nType==VBVMR_DEVTYPE_MME) strcat(szParam,".mme");
		if (nType==VBVMR_DEVTYPE_WDM) strcat(szParam,".wdm");
		if (nType==VBVMR_DEVTYPE_KS) strcat(szParam,".ks");
		if (nType==VBVMR_DEVTYPE_ASIO) strcat(szParam,".asio");
		iVMR.VBVMR_SetParameterStringA(szParam, szName);
		
		return;
	}
}

/*******************************************************************************/
/*                               Init / End Software                           */
/*******************************************************************************/

BOOL InitSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	LOGFONT		lf;
	char szTitle[]="Init Error";
	long vi, rep;

	lpapp->hwnd_MainWindow = hw;
	//create font
	memset(&lf,0, sizeof(LOGFONT));
	lf.lfHeight	= 16;
	lf.lfWeight	= 400;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font =CreateFontIndirect(&lf);

	//get DLL interface
	rep=InitializeDLLInterfaces();
	if (rep < 0)
	{
		if (rep == -100) MessageBox(hw,"Voicemeeter is not installed",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		else MessageBox(hw,"Failed To Link To VoicemeeterRemote.dll",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return FALSE;
	}
	//Log in
	rep=iVMR.VBVMR_Login();
	if (rep < 0)
	{
		MessageBox(hw,"Failed To Login",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if (rep == 1)
	{
		iVMR.VBVMR_RunVoicemeeter(3);
		Sleep(1000);
	}
	//call this to get first parameters state (if server already launched)
	//for (vi=0;vi<3;vi++) //3 times to be sure to get the last settings on startup.
	{
	iVMR.VBVMR_IsParametersDirty();
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
	iVMR.VBVMR_MacroButton_IsDirty();
#endif
	}
	return TRUE;
}

BOOL EndSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	if (iVMR.VBVMR_Logout != NULL) iVMR.VBVMR_Logout();
	if (iVMR.VBVMR_AudioCallbackUnregister != NULL) iVMR.VBVMR_AudioCallbackUnregister();

	if (lpapp->font != NULL) DeleteObject(lpapp->font);
	lpapp->font=NULL;
	return TRUE;
}

long CreateOurWindowControls(LPT_APP_CONTEXT lpapp, HWND hw)
{
	long vi,x0,y0;
	char sss[256];
	x0=10;
	y0=30;
	for (vi=0;vi<NBPARAM_DISPLAYED;vi++)
	{
		if (vi < 4) 
		{
			if (vi == 0) sprintf(sss,"Strip[%i].gain",vi);
			if (vi == 1) sprintf(sss,"Strip[%i].A1",vi);
			if (vi == 2) sprintf(sss,"Strip[%i].Mute",vi);
			if (vi == 3) sprintf(sss,"Strip[%i].Solo",vi);
		}
		else sprintf(sss,"Bus[%i].gain",vi-4);
		if (vi == (NBPARAM_DISPLAYED-1)) sprintf(sss,"Bus[0].device.name");
		lpapp->hw_EditBox[vi]=CreateWindowEx(WS_EX_CLIENTEDGE,"edit",sss,
						 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP,
						 x0,y0,250,25,
						 hw,(HMENU)(IDC_EDITBOX+vi),lpapp->hinstance,NULL);
		if (lpapp->hw_EditBox[vi] ==NULL) return -1;
		SendMessage(lpapp->hw_EditBox[vi],WM_SETFONT,(WPARAM)lpapp->font,MAKELPARAM(1,0));
		SendMessage(lpapp->hw_EditBox[vi],EM_SETMARGINS,EC_LEFTMARGIN | EC_RIGHTMARGIN,(LPARAM) MAKELONG(2,2));

		y0=y0+30;
	}

	return 0;
}

void UpdateDeviceMenu(LPT_APP_CONTEXT lpapp, HMENU hMenu)
{
	char sss[512];
	char szName[256];
	char szHardwareId[256];
	HMENU hSubMenu;
	long vi,nb,style,nType,rep;

	//if the code includes Audio Callback example, we activate menu
#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_REGISTER, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_UNREGISTER, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_START, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_STOP, MF_BYCOMMAND | MF_ENABLED);
#else
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_REGISTER, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_UNREGISTER, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_START, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMenu, IDM_COMMAND_ACB_STOP, MF_BYCOMMAND | MF_GRAYED);
#endif
	style=MF_ENABLED | MF_STRING;
	//re-enumerate device
	nb=	iVMR.VBVMR_Input_GetDeviceNumber();

	//Build Input Device Menu
	hSubMenu=CreatePopupMenu();
	for (vi=0;vi<nb;vi++)
	{
		rep = iVMR.VBVMR_Input_GetDeviceDescA(vi,&nType, szName, szHardwareId);
		if (rep == 0)
		{
			switch(nType)
			{
			case VBVMR_DEVTYPE_MME:
				sprintf(sss,"MME: %s",szName);
				break;
			case VBVMR_DEVTYPE_WDM:
				sprintf(sss,"WDM: %s",szName);
				break;
			case VBVMR_DEVTYPE_KS:
				sprintf(sss,"KS: %s",szName);
				break;
			}
			AppendMenu(hSubMenu,MF_STRING,IDM_DEVICE_IN1+vi+1,sss);
		}
	}
	//replace sub menu (this function release possible previous submenu)
	ModifyMenu(hMenu,IDM_DEVICE_IN1,MF_BYCOMMAND | MF_POPUP,(unsigned int)hSubMenu,"Select Input#1 device");

	//Build Output Device Menu
	nb=	iVMR.VBVMR_Output_GetDeviceNumber();
	hSubMenu=CreatePopupMenu();
	for (vi=0;vi<nb;vi++)
	{
		rep = iVMR.VBVMR_Output_GetDeviceDescA(vi,&nType, szName, szHardwareId);
		if (rep == 0)
		{
			switch(nType)
			{
			case VBVMR_DEVTYPE_MME:
				sprintf(sss,"MME: %s",szName);
				break;
			case VBVMR_DEVTYPE_WDM:
				sprintf(sss,"WDM: %s",szName);
				break;
			case VBVMR_DEVTYPE_KS:
				sprintf(sss,"KS: %s",szName);
				break;
			case VBVMR_DEVTYPE_ASIO:
				sprintf(sss,"ASIO: %s",szName);
				break;
			}
			AppendMenu(hSubMenu,MF_STRING,IDM_DEVICE_OUT1+vi+1,sss);
		}
	}
	//replace sub menu (this function release possible previous submenu)
	ModifyMenu(hMenu,IDM_DEVICE_OUT1,MF_BYCOMMAND | MF_POPUP,(unsigned int)hSubMenu,"Select Output A1 device");
}








/*******************************************************************************/
/*                                WIN CALLBACK                                 */
/*******************************************************************************/

#define MYTIMERID 16489

LRESULT CALLBACK MainWindowManageEvent(HWND hw,			//handle of the window.
											UINT msg,   //Message Ident.
											WPARAM wparam,	//parameter 1.
											LPARAM lparam)	//parameter 2
{
	unsigned char pBuffer[1024];
	char sz[64];
	long fDisplayParam,nbb,vi,nbmax, fDisplayMIDI;
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
	long fDisplayButton;
#endif
	LPT_APP_CONTEXT lpapp;
	//char sss[256];
	HDC dc;
	PAINTSTRUCT ps;
	lpapp = &G_MainAppCtx;
	switch (msg)
	{


		case WM_CREATE:
			if (InitSoftware(lpapp,hw) == FALSE) return -1;//return -1 here cancel the window creation
			CreateOurWindowControls(lpapp, hw);
			lpapp->wTimer = SetTimer(hw,MYTIMERID, 20,NULL);//50 Hz timer

			break;
		case WM_INITMENU:
			UpdateDeviceMenu(lpapp, (HMENU)wparam);
			break;
		case WM_LBUTTONDOWN:
#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE 
			PROCESSING_ManageLButtonDownOnMuteButton(hw, (short int)LOWORD(lparam),(short int)HIWORD(lparam));
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
			MACROBUTTONS_ManageLButtonDown(hw, (short int)LOWORD(lparam),(short int)HIWORD(lparam));

#endif				
			break;	
		case WM_COMMAND:
			ManageMenu(lpapp,hw,wparam,lparam);
			break;
		case WM_TIMER:
			if (wparam == MYTIMERID)
			{
				//check if we have parameter change
				fDisplayParam=iVMR.VBVMR_IsParametersDirty();
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
				fDisplayButton=iVMR.VBVMR_MacroButton_IsDirty();
#endif

				//check if we have received MIDI messages
				fDisplayMIDI=0;
				nbmax = 32;
				nbb=iVMR.VBVMR_GetMidiMessage(pBuffer, 1024);
				while (nbb > 0)		// while there is M.I.D.I. message in stack
				{
					if (nbmax == 32) lpapp->MIDIString[0]=0;	// reset string at the beginning
					if (nbmax > 0)	// if our string is not full we add MIDI code to our string
					{
						fDisplayMIDI=1;
						if (nbb > 32) nbb=32; //32 code max
						for (vi=0;vi<nbb;vi++)
						{
							sprintf(sz,"%02X ", pBuffer[vi]);
							strcat(lpapp->MIDIString, sz);
						}
						nbmax = nbmax -nbb;
					}
					//get next message if any
					nbb=iVMR.VBVMR_GetMidiMessage(pBuffer, 1024);
				}
				
				//Real Time display 
				dc=GetDC(hw);
				DrawCurrentLevels(lpapp, hw, dc);

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE 
				PROCESSING_DrawLevels_insertin(dc);
				PROCESSING_DrawLevels_insertout(dc);
				PROCESSING_DrawLevels_main(dc);
#endif
				//update MIDI message display if there is something new
				if (fDisplayMIDI != 0) DrawCurrentMIDIMessage(lpapp, hw, dc);
				//update parameters, only if there is change
				if (fDisplayParam == 1) DrawCurrentValues(lpapp, hw, dc);
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
				if (fDisplayButton == 1) DrawCurrentButtonState(lpapp, hw, dc);
#endif				
				ReleaseDC(hw,dc);
			}
			break;
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			DrawAllStuff(lpapp,hw, dc);
			EndPaint(hw,&ps);
	        break;
		case WM_CLOSE:
			ManageCloseMessage(hw);
			break;
		case WM_DESTROY:
			if (lpapp->wTimer != 0) KillTimer(hw,lpapp->wTimer);
			lpapp->wTimer=0;
			EndSoftware(lpapp,hw);
			PostQuitMessage(0);
			break;
		default:
			return (DefWindowProc(hw,msg,wparam,lparam));

	}
	return (0L);
}


/*******************************************************************************/
/**                              MAIN PROCDURE                                **/
/*******************************************************************************/

int APIENTRY WinMain(HINSTANCE handle_app,			//Application hinstance.
							HINSTANCE handle_prev,  //NULL.
							LPTSTR param,           //Command Line Parameter.
							int com_show)           //How to display window (optionnal).
{
	HWND	hh;
	long	wstyle;
	MSG		msg;    
	char	szWindowClassName[]="MainWindowClass";
	char *	title="Sorry";

	WNDCLASS	wc;
	//we first store the APP Hinstance
	G_MainAppCtx.hinstance=handle_app;

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE 
	PROCESSING_initmyDSPContext();
#endif

#ifdef VBUSE_LOCALLIB
	//if we directly link source code (development only)
	VBVMR_SetHinstance(handle_app);
#endif
	//here you can make some early initialization and analyze command line if any.


	//we define a window class to create a window from this class 
	wc.style		=CS_HREDRAW | CS_VREDRAW;  	  		//property.
	wc.lpfnWndProc=(WNDPROC)MainWindowManageEvent;		//Adresse of our Callback.
	wc.cbClsExtra =0;					  				//Possibility to store some byte inside a class object.
	wc.cbWndExtra =0;                          			//Possibility to store some byte inside a window object.
	wc.hInstance  =handle_app; 	                		//handle of the application hinstance.
	wc.hIcon      =LoadIcon(NULL, IDI_APPLICATION);    	//handle of icon displayed in the caption.
	wc.hCursor    =LoadCursor(NULL,IDC_ARROW);			//handle of cursor mouse .
	wc.hbrBackground=(HBRUSH)(COLOR_MENU+1);			//Background color.
	wc.lpszMenuName="MyMainAppMenu";    				//pointer on menu defined in resource.
	wc.lpszClassName=szWindowClassName;       			//pointer on class name.

	//register class.
	if (RegisterClass(&wc)==0)
	{
		MessageBox(NULL,"Failed to register main class...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	
	
	//classical Main Window
	wstyle=WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	hh=CreateWindow(szWindowClassName,					// address of registered class name.
						 SZPUBLICNAME,					// address of window name string
						 wstyle,						// window style
						 CW_USEDEFAULT,					// horizontal position of window
						 CW_USEDEFAULT,					// vertical position of window
						 UI_WIN_DX,						// window width
						 UI_WIN_DY,						// window height
						 NULL,							// parent handle is NULL since it's a main window.
						 NULL,							// menu name defined in resource (NULL if no menu or already defined in the Class).
						 handle_app,					// handle of application instance
						 NULL); 						// address of window-creation data

	if (hh==NULL)
	{
		MessageBox(NULL,"Failed to create window...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	ShowWindow(hh,SW_SHOW);				//Display the window.
	UpdateWindow(hh);					//Send WM_PAINT.
	/*---------------------------------------------------------------------------*/
	/*                             Messages Loop.                                */
	/*---------------------------------------------------------------------------*/
	while (GetMessage(&msg,NULL,0,0))	//Get Message if any.
	{
		TranslateMessage(&msg);			//Translate the virtuel keys event.
		DispatchMessage(&msg);			//DispatchMessage to the related window.
	}

	//here you can make last uninitialization and release
	return (int)(msg.wParam);
}
