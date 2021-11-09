/*--------------------------------------------------------------------------------*/
/* VMR example: Voicemeeter Custom GUI for Streamers                              */
/*--------------------------------------------------------------------------------*/
/* 'C' Sample Code to make a Voicemeeter Custom GUI          V.Burel (c)2016-2021 */
/*                                                                                */
/*  THIS PROGRAM PROVIDES A GUI TO CONTROL VOICEMEETER                            */
/*               PROVIDES A SIMPLIFIED GUI WITH ONLY USED STRIP/BUS               */
/*               PROVIDES A RESIZABLE GRAPHIC USER INTERFACE                      */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to link VoicemeeterRemote.dll                                           */
/*  - How to Login / logout                                                       */
/*  - How to Manage regular parameters to make a custom controller.               */
/*  - How to make a VBAN layer to also manage distant Voicemeeter.                */
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
/*  WARNING: FOR 64x COMPILATION, ADD PREPROCESSOR DEFINE: VB_X64                 */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  LICENSING: VoicemeeterRemote.dll usage is driven by a license agreement       */
/*             given in VoicemeeterRemoteAPI.pdf or readme.txt                    */
/*             This Source Code can be used only in a program using Voicemeeter   */
/*             Remote API.                                                        */
/*                                                                                */
/*--------------------------------------------------------------------------------*/


#ifndef __cplusplus
	#ifndef STRICT
		#define STRICT
	#endif
#endif

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "VoicemeeterRemote.h"
#include "vmr_streamer.h"
#include "streamer_ctrl.h"
#include "vban_cmd.h"

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

//if we directly link source code (for vb-audio development only)
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

		iVMR.VBVMR_AudioCallbackRegister	=VBVMR_AudioCallbackRegister;
		iVMR.VBVMR_AudioCallbackStart		=VBVMR_AudioCallbackStart;
		iVMR.VBVMR_AudioCallbackStop		=VBVMR_AudioCallbackStop;
		iVMR.VBVMR_AudioCallbackUnregister	=VBVMR_AudioCallbackUnregister;
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
		
		return 0;
	}


#endif


/*******************************************************************************/
/*                       CONSTANT FUNCTION OF VOICEMEETER TYPE                 */
/*******************************************************************************/


static char * G_szBUSNameList_v1[2]={"A", "B"};
static char * G_szBUSNameList_v2[5]={"A1", "A2", "A3", "B1", "B2"};
static char * G_szBUSNameList_v3[8]={"A1", "A2", "A3", "A4", "A5", "B1", "B2", "B3"};

static char * G_szStripNameList_v1[3]={"IN #1", "IN #2", "VIN #1"};
static char * G_szStripNameList_v2[5]={"IN #1", "IN #2", "IN #3", "VIN #1", "VIN #2"};
static char * G_szStripNameList_v3[8]={"IN #1", "IN #2", "IN #3", "IN #4", "IN #5", "VIN #1", "VIN #2", "VIN #3"};

static long	G_nbChannelPerStrip_v1[3]={2, 2, 8};
static long	G_nbChannelPerStrip_v2[5]={2, 2, 2, 8, 8};
static long	G_nbChannelPerStrip_v3[8]={2, 2, 2, 2, 2, 8, 8, 8};


/*******************************************************************************/
/*                                    COLOR / PEN TOOLS                        */
/*******************************************************************************/

typedef struct tagCOLORPENBRUSH
{
	COLORREF	color;
	HPEN		pen;
	HBRUSH		brush;
} T_COLORPENBRUSH, *PT_COLORPENBRUSH, *LPT_COLORPENBRUSH;

long CreateColorPenBrush(LPT_COLORPENBRUSH lpgdi, COLORREF color)
{
	LOGBRUSH	lb;
	if (lpgdi == NULL) return -1;
	lpgdi->color=color;
	lpgdi->pen=CreatePen(PS_SOLID,0,color);

	lb.lbStyle=BS_SOLID;
	lb.lbColor=color;
	lpgdi->brush=CreateBrushIndirect(&lb);
	return 0;
}

long DestroyColorPenBrush(LPT_COLORPENBRUSH lpgdi)
{
	if (lpgdi == NULL) return -1;
	
	lpgdi->color=0;
	if (lpgdi->pen != NULL) DeleteObject(lpgdi->pen);
	lpgdi->pen=NULL;
	if (lpgdi->brush != NULL) DeleteObject(lpgdi->brush);
	lpgdi->brush=NULL;
	return 0;
}




/*******************************************************************************/
/*                                OUR PARAM STRUCTURE                          */
/*                     to store all voicemeeter parameter we manage            */
/*******************************************************************************/



typedef struct tagMYVM_PARAMETERS
{
	long nbStrip;
	T_VMSCTL_STRIP_DATA strip[VMSCTL_MAX_NBSTRIP];
	
	long nbBus;
	T_VMSCTL_BUS_DATA bus[VMSCTL_MAX_NBBUS];

	long monitorbus;
} T_MYVM_PARAMETERS, *PT_MYVM_PARAMETERS, *LPT_MYVM_PARAMETERS;


/*******************************************************************************/
/*                                APPLICATION CONTEXT                          */
/*******************************************************************************/

#define SLIDERLINKMODE_INDEPENDENT	0
#define SLIDERLINKMODE_ABSLINK		1
#define SLIDERLINKMODE_RELLINK		2

#define SLIDERLINKMODE_DX			80

typedef struct tagVBAN_CMD_PARAM
{
	char		VBAN_szStreamName[32];
	char		VBAN_szIPAddress[32];
	long		VBAN_UDPport;	
} T_VBAN_CMD_PARAM, *PT_VBAN_CMD_PARAM, *LPT_VBAN_CMD_PARAM;


typedef struct tagAPP_CONTEXT
{
	HWND		hwnd_MainWindow;
	HINSTANCE	hinstance;
	size_t		wTimer;
	long		hw_dx;
	long		hw_dy;
	long		vbvmr_NotInstalled;	
	long		vbvmr_error;
	long		vbvmr_connect;
	long		vbvmr_nbbus;
	long		vbvmr_nbstrip;
	long		vbvmr_multilayer;
	long		vbvmr_version;
	long		vbvmr_monitor_support;

	char **		vbvmr_pBUSName;
	char **		vbvmr_pStripName;
	long *		vbvmr_pStripChannel;
	DWORD		msLastTimeForVBANRegister;	// time reference in ms
	DWORD		msLastTimeForVBANCheck;		// time reference in ms
	DWORD		LastCounter;				// time reference in ms
	BOOL		VBANisConnected;			// time reference in ms
	T_MYVM_PARAMETERS current_param;
	long		StripSliderMode;		

	long		VMConnectionType;				// 0 = direct,  1 = VBAN1, 2 = VBAN2, 3 = VBAN3, 4 = VBAN4.
	T_VBAN_CMD_PARAM VMLinkVBAN[4];

	HWND		hw_StripList[VMSCTL_MAX_NBSTRIP];
	HWND		hw_BusList[VMSCTL_MAX_NBBUS];

	HFONT		font0;
	HFONT		font1;
	HFONT		font2;

	T_COLORPENBRUSH	gdiobjects_black;
	T_COLORPENBRUSH	gdiobjects_bkg;
	T_COLORPENBRUSH	gdiobjects_bkg_ctl;
	T_COLORPENBRUSH	gdiobjects_blue0;
	T_COLORPENBRUSH	gdiobjects_blue1;
	T_COLORPENBRUSH	gdiobjects_blue2;
	T_COLORPENBRUSH	gdiobjects_blue3;
	T_COLORPENBRUSH	gdiobjects_buttongreen;
	T_COLORPENBRUSH	gdiobjects_slidergreen;
	T_COLORPENBRUSH	gdiobjects_sliderred;
	T_COLORPENBRUSH	gdiobjects_white;
	T_COLORPENBRUSH	gdiobjects_meter_blue;
	T_COLORPENBRUSH	gdiobjects_meter_green;
	T_COLORPENBRUSH	gdiobjects_meter_red;
	T_COLORPENBRUSH	gdiobjects_mute;
	T_COLORPENBRUSH	gdiobjects_monitor;
	T_COLORPENBRUSH	gdiobjects_grey;

	RECT		CurrentWindowRect;
	RECT		rect_inputs;
	RECT		rect_outputs;
	RECT		rect_SliderMode;
	RECT		rect_VMConnectionType;
} T_APP_CONTEXT, *PT_APP_CONTEXT, *LPT_APP_CONTEXT;

static T_APP_CONTEXT G_MainAppCtx = {NULL, NULL};



long InitResources(LPT_APP_CONTEXT lpapp)
{
	LOGFONT		lf;

	if (lpapp == NULL) return -1;

	//make Font
	memset(&lf,0, sizeof(LOGFONT));
	lf.lfHeight	= 14;
	lf.lfWeight	= 400;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font0 =CreateFontIndirect(&lf);

	lf.lfHeight	= 16;
	lf.lfWeight	= 400;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font1 =CreateFontIndirect(&lf);

	lf.lfHeight	= 18;
	lf.lfWeight	= 600;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font2 =CreateFontIndirect(&lf);
	
	//make pen brush 
	CreateColorPenBrush(&(lpapp->gdiobjects_black), RGB(0,0,0));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg), RGB(18,32,41));
	CreateColorPenBrush(&(lpapp->gdiobjects_bkg_ctl), RGB(44,61,77));
	CreateColorPenBrush(&(lpapp->gdiobjects_blue0), RGB(71,90,103));
	CreateColorPenBrush(&(lpapp->gdiobjects_blue1), RGB(95,120,137));
	CreateColorPenBrush(&(lpapp->gdiobjects_blue2), RGB(111,140,160));
	CreateColorPenBrush(&(lpapp->gdiobjects_blue3), RGB(104,230,248));
	CreateColorPenBrush(&(lpapp->gdiobjects_buttongreen), RGB(36,167,49));
	CreateColorPenBrush(&(lpapp->gdiobjects_slidergreen), RGB(112,195,153));
	CreateColorPenBrush(&(lpapp->gdiobjects_sliderred), RGB(248,99,77));
	CreateColorPenBrush(&(lpapp->gdiobjects_white), RGB(255,255,255));

	CreateColorPenBrush(&(lpapp->gdiobjects_meter_blue), RGB(130,170,180));
	CreateColorPenBrush(&(lpapp->gdiobjects_meter_green), RGB(30,255,90));
	CreateColorPenBrush(&(lpapp->gdiobjects_meter_red), RGB(250,0,0));

	CreateColorPenBrush(&(lpapp->gdiobjects_mute), RGB(246,91,81));
	CreateColorPenBrush(&(lpapp->gdiobjects_monitor), RGB(203,174,130));
	CreateColorPenBrush(&(lpapp->gdiobjects_grey), RGB(100,100,100));
	
	return 0;
}

long EndResources(LPT_APP_CONTEXT lpapp)
{
	if (lpapp == NULL) return -1;
	//Delete font Object
	if (lpapp->font0 != NULL) DeleteObject(lpapp->font0);
	lpapp->font0=NULL;
	if (lpapp->font1 != NULL) DeleteObject(lpapp->font1);
	lpapp->font1=NULL;
	if (lpapp->font2 != NULL) DeleteObject(lpapp->font2);
	lpapp->font2=NULL;
	//Delete Pen Brush
	DestroyColorPenBrush(&(lpapp->gdiobjects_black));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg));
	DestroyColorPenBrush(&(lpapp->gdiobjects_bkg_ctl));
	DestroyColorPenBrush(&(lpapp->gdiobjects_blue0));
	DestroyColorPenBrush(&(lpapp->gdiobjects_blue1));
	DestroyColorPenBrush(&(lpapp->gdiobjects_blue2));
	DestroyColorPenBrush(&(lpapp->gdiobjects_blue3));
	DestroyColorPenBrush(&(lpapp->gdiobjects_buttongreen));
	DestroyColorPenBrush(&(lpapp->gdiobjects_slidergreen));
	DestroyColorPenBrush(&(lpapp->gdiobjects_sliderred));
	DestroyColorPenBrush(&(lpapp->gdiobjects_white));

	DestroyColorPenBrush(&(lpapp->gdiobjects_meter_blue));
	DestroyColorPenBrush(&(lpapp->gdiobjects_meter_green));
	DestroyColorPenBrush(&(lpapp->gdiobjects_meter_red));
	DestroyColorPenBrush(&(lpapp->gdiobjects_mute));
	DestroyColorPenBrush(&(lpapp->gdiobjects_monitor));
	return 0;
}


/*******************************************************************************/
/*                               LOCAL TOOL Functions                          */
/*******************************************************************************/


#ifndef SM_CXPADDEDBORDER
	#define SM_CXPADDEDBORDER 92
#endif

typedef struct tagVBNONCLIENTMETRICS 
{  
	UINT cbSize;
	int iBorderWidth;
	int iScrollWidth;
	int iScrollHeight;
	int iCaptionWidth;
	int iCaptionHeight;
	LOGFONT lfCaptionFont;
	int iSmCaptionWidth;
	int iSmCaptionHeight;
	LOGFONT lfSmCaptionFont;
	int iMenuWidth;
	int iMenuHeight;
	LOGFONT lfMenuFont;
	LOGFONT lfStatusFont;
	LOGFONT lfMessageFont;
	int iPaddedBorderWidth;
} VBNONCLIENTMETRICS,  *LPVBNONCLIENTMETRICS;


void TOOL_GetWindowsMetricInfo(long * win_border, long * win_title,long * win_menu)
{
	long xyadjust=0;
	BOOL ok;
	VBNONCLIENTMETRICS ncm;
	ncm.cbSize=sizeof(VBNONCLIENTMETRICS);
	ok=SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(VBNONCLIENTMETRICS),&ncm,0L); 
	if (ok != 0)
	{
		xyadjust=ncm.iPaddedBorderWidth;//+2;
		if (win_menu != NULL)			*win_menu		=ncm.iMenuHeight;
		if (win_border != NULL)			*win_border		=xyadjust+ncm.iBorderWidth;
		if (win_title != NULL)			*win_title		=ncm.iCaptionHeight;
		return;
	}
	xyadjust=GetSystemMetrics(SM_CXPADDEDBORDER);
	if (win_menu != NULL)	*win_menu=GetSystemMetrics(SM_CYMENU);
	if (win_border != NULL) *win_border= xyadjust+GetSystemMetrics(SM_CXDLGFRAME); //SM_CXDLGFRAME SM_CXBORDER ncm.iBorderWidth;
	if (win_title != NULL)	*win_title=GetSystemMetrics(SM_CYCAPTION);//ncm.iCaptionHeight;
}



void TOOL_RemoveUnauthorizedChar(char * szString)
{
	char cc,* lpTarget,*lpSource;
	if (szString == NULL) return;
	lpSource=szString;
	lpTarget=szString;
	
	while(*lpSource != 0)
	{
		cc=*lpSource;
		if (((cc > 47) && (cc<58)) || 
			((cc>64) && (cc<91)) ||
			((cc>96) && (cc<123)) || 
			(cc == '-') || (cc == '_') || (cc == '{') || (cc == '}') || (cc =='(') || 
			(cc == ')') || (cc == '[') || (cc == ']') || (cc == 32))
		{
			*lpTarget=cc;
			lpTarget++;
		}
		lpSource++;
	}
	*lpTarget=0;
}


long TOOL_IsItIPAddress(char* sss)
{
	long nbn=0,num=0,fdigi=0;
	char cc;
	cc=*sss++;
	while (cc > 31)
	{
		if ((cc > 47) && (cc < 58)) 
		{
			num = (num*10) + (cc - 48);
			fdigi=1;
		}
		else
		{
			if (cc == '.')
			{
				fdigi=0;
				if ((num >= 0) && (num < 256))	nbn++;
				else return -2;
				num=0;
			}
			else
			{
				if (cc != 32) return -1;
			}
		}
		cc=*sss++;
	}
	if (fdigi != 0)
	{
		if ((num >= 0) && (num < 256))	nbn++;
	}
	if (nbn  == 4) return 1;
	return 0;
}

DWORD TOOL_GetDeltaMsDword(DWORD previous_ms, DWORD current_ms)
{
	DWORD delta;
	if (current_ms >= previous_ms) delta =  current_ms-previous_ms;
	else
	{
		delta = (0xFFFFFFFF-previous_ms);
		delta +=current_ms;
	}
	return delta;
}



/*******************************************************************************/
/*                                 REGISTRY TOOL                               */
/*******************************************************************************/


BOOL __cdecl TOOL_RegistrySaveUserString(char * key,char * ValueName,char * strvalue)
{
	DWORD nbc;
	HKEY hkResult;
	LONG rep;
	rep=RegOpenKey(HKEY_CURRENT_USER,key,&hkResult); 
	if (rep != ERROR_SUCCESS) 
	{
		rep=RegCreateKey(HKEY_CURRENT_USER,key,&hkResult);
		if (rep != ERROR_SUCCESS) return FALSE;
	}
	nbc= (DWORD)((strlen(strvalue)+1)*sizeof(char));
	rep=RegSetValueEx(hkResult,	ValueName,0,REG_SZ,(unsigned char *)strvalue,nbc);
	RegCloseKey(hkResult);
	if (rep != ERROR_SUCCESS) return FALSE;
	return TRUE;
}

BOOL __cdecl TOOL_RegistryLoadUserString(char * key,char * ValueName,char * strvalue)
{
	char sss[4100];
	DWORD nnsize=4096;
	HKEY hkResult;
	LONG rep;
	DWORD pptype=REG_SZ;
	sss[0]=0;
	rep=RegOpenKey(HKEY_CURRENT_USER,key,&hkResult); 
	if (rep != ERROR_SUCCESS) return FALSE;
	strvalue[0]=0;
	rep=RegQueryValueEx(hkResult,ValueName,0,&pptype,(unsigned char *)sss,&nnsize);
	RegCloseKey(hkResult);
	if (pptype != REG_SZ) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	if (nnsize>4050) nnsize=4050;
	strncpy(strvalue,sss,nnsize);
	return TRUE;
}

BOOL __cdecl TOOL_RegistrySaveUserDWORD(char * key,char * ValueName,DWORD value)
{
	HKEY hkResult;
	LONG rep;
	rep=RegOpenKey(HKEY_CURRENT_USER,key,&hkResult); 
	if (rep != ERROR_SUCCESS) 
	{
		rep=RegCreateKey(HKEY_CURRENT_USER,key,&hkResult);
		if (rep != ERROR_SUCCESS) return FALSE;
	}
	rep=RegSetValueEx(hkResult,	ValueName,0,REG_DWORD,(unsigned char *)&value,sizeof(DWORD));
	RegCloseKey(hkResult);
	if (rep != ERROR_SUCCESS) return FALSE;
	return TRUE;
}

BOOL __cdecl TOOL_RegistryLoadUserDWORD(char * key,char * ValueName,DWORD * pvalue)
{
	HKEY hkResult;
	LONG rep;
	DWORD bSize = sizeof(DWORD);
	DWORD bType = REG_DWORD;

	rep=RegOpenKey(HKEY_CURRENT_USER,key,&hkResult); 
	if (rep != ERROR_SUCCESS) return FALSE;
	rep=RegQueryValueEx(hkResult, ValueName,0,&bType,(unsigned char *)pvalue,&bSize);
	RegCloseKey(hkResult);
	if (bType != REG_DWORD) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	return TRUE;
}








/*******************************************************************************/
/*                        VBAN CONFIGURATION DIALOG BOX                        */
/*******************************************************************************/


//VBAN Edit Dialog Box Context
typedef struct tagVBANOPTION_DIALOGBOX
{
	HGLOBAL				hdlgtemplate;
	HWND				hparent;
	HWND				hw;
	int					fInit;
	long				x0,y0,dx,dy;
	long				style;

	HWND				hw_StreamName[4];
	HWND				hw_StreamIPTo[4];
	HWND				hw_StreamPort[4];
	HWND				hw_StreamCheck[4];

} T_VBANOPTION_DIALOGBOX, *PT_VBANOPTION_DIALOGBOX, *LPT_VBANOPTION_DIALOGBOX;

static T_VBANOPTION_DIALOGBOX G_VBANDialog_CTX={NULL,NULL, NULL};


#define IDC_VBANEDIT_NAME		100
#define IDC_VBANEDIT_IP			200
#define IDC_VBANEDIT_CHECK		300
#define IDC_VBANEDIT_PORT		400

void VBANDIALOG_CreateControls(LPT_VBANOPTION_DIALOGBOX lpctx, HWND hwParent)
{
	LPT_VBAN_CMD_PARAM lpVBAN_param;
	char sz[128];
	long x0,y0,vi;
	HWND hh;
	LPT_APP_CONTEXT lpapp;	
	lpapp=&G_MainAppCtx;

	y0=10+30+20;
	for (vi=0;vi<4;vi++)
	{
		lpVBAN_param = &(lpapp->VMLinkVBAN[vi]);
		x0=10;

		//name 
		x0=x0+80;
		hh=CreateWindowEx(WS_EX_CLIENTEDGE,"edit",lpVBAN_param->VBAN_szStreamName,
						 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | ES_AUTOHSCROLL,
						 x0,y0,200,26,
						 hwParent,(HMENU)(IDC_VBANEDIT_NAME+vi),lpapp->hinstance,NULL);
		SendMessage(hh,WM_SETFONT,(WPARAM)lpapp->font1,MAKELPARAM(1,0));
		SendMessage(hh,EM_SETMARGINS,EC_LEFTMARGIN | EC_RIGHTMARGIN,(LPARAM) MAKELONG(2,2));
		lpctx->hw_StreamName[vi]=hh;

		//IP to
		x0=x0+210;
		hh=CreateWindowEx(WS_EX_CLIENTEDGE,"edit",lpVBAN_param->VBAN_szIPAddress,
						 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | ES_AUTOHSCROLL,
						 x0,y0,180,26,
						 hwParent,(HMENU)(IDC_VBANEDIT_IP+vi),lpapp->hinstance,NULL);
		SendMessage(hh,WM_SETFONT,(WPARAM)lpapp->font1,MAKELPARAM(1,0));
		SendMessage(hh,EM_SETMARGINS,EC_LEFTMARGIN | EC_RIGHTMARGIN,(LPARAM) MAKELONG(2,2));
		lpctx->hw_StreamIPTo[vi]=hh;

		//
		x0=x0+190;
		sprintf(sz,"%i", lpVBAN_param->VBAN_UDPport);
		hh=CreateWindowEx(WS_EX_CLIENTEDGE,"edit",sz,
						 WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | ES_AUTOHSCROLL,
						 x0,y0,80,26,
						 hwParent,(HMENU)(IDC_VBANEDIT_PORT+vi),lpapp->hinstance,NULL);
		SendMessage(hh,WM_SETFONT,(WPARAM)lpapp->font1,MAKELPARAM(1,0));
		SendMessage(hh,EM_SETMARGINS,EC_LEFTMARGIN | EC_RIGHTMARGIN,(LPARAM) MAKELONG(2,2));
		lpctx->hw_StreamPort[vi]=hh;

		y0=y0+40;
	}


	//
	// button Ok/Cancel	
	//
	hh=CreateWindow("button","Ok",WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					 lpctx->dx-200,lpctx->dy-40,90,30,
					 hwParent,(HMENU)1,lpapp->hinstance,NULL);

	hh=CreateWindow("button","Cancel",WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					 lpctx->dx-100,lpctx->dy-40,90,30,
					 hwParent,(HMENU)2,lpapp->hinstance,NULL);
	
	//

}

static void VBANDIALOG_ValidateOption(LPT_VBANOPTION_DIALOGBOX lpctx)
{
	LPT_VBAN_CMD_PARAM lpVBAN_param;
	char sz[128];
	long nnn,vi;
	LPT_APP_CONTEXT lpapp;	
	lpapp=&G_MainAppCtx;

	for (vi=0;vi<4;vi++)
	{
		lpVBAN_param = &(lpapp->VMLinkVBAN[vi]);

		GetWindowText(lpctx->hw_StreamName[vi],sz,64);
		TOOL_RemoveUnauthorizedChar(sz);
		sz[17]=0;
		strcpy(lpVBAN_param->VBAN_szStreamName, sz);

		GetWindowText(lpctx->hw_StreamIPTo[vi],sz,64);
		if (TOOL_IsItIPAddress(sz) == 1) 
		{
			strcpy(lpVBAN_param->VBAN_szIPAddress, sz);
		}

		GetWindowText(lpctx->hw_StreamPort[vi],sz,64);
		nnn=atoi(sz);
		nnn=nnn & 0x0000FFFF;
		if (nnn > 0)
		{
			lpVBAN_param->VBAN_UDPport= nnn;
		}
	}
}



void VBANDIALOG_DrawStuff(LPT_VBANOPTION_DIALOGBOX lpctx, HDC dc)
{
	long vi, x0,y0;
	char sss[512];
	HBRUSH oldbrush;
	HPEN oldpen;
	HFONT oldfont;
	LPT_APP_CONTEXT lpapp;	
	lpapp=&G_MainAppCtx;

	oldbrush=(HBRUSH)SelectObject(dc,lpapp->gdiobjects_grey.brush);
	oldpen=(HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
	oldfont=(HFONT)SelectObject(dc,lpapp->font1);
	SetBkMode(dc,TRANSPARENT);
	
	x0=10;
	y0=10;

	SetTextColor(dc,RGB(0,0,0));
	strcpy(sss,"VBAN Connections Configuration:");
	TextOut(dc,x0+5,y0+2,sss,(int)strlen(sss));	

	SetTextColor(dc,RGB(255,255,255));
	y0=y0+30;
	x0=x0+80;
	strcpy(sss,"Stream Name:");
	Rectangle(dc,x0,y0,x0+200,y0+20);
	TextOut(dc,x0+5,y0+2,sss,(int)strlen(sss));	

	x0=x0+210;
	strcpy(sss,"IP Addres To:");
	Rectangle(dc,x0,y0,x0+200,y0+20);
	TextOut(dc,x0+5,y0+2,sss,(int)strlen(sss));	

	x0=x0+190;
	strcpy(sss,"UDP port:");
	Rectangle(dc,x0,y0,x0+80,y0+20);
	TextOut(dc,x0+5,y0+2,sss,(int)strlen(sss));	

	
	y0=10+30+20;
	x0=10;
	SetTextColor(dc,RGB(0,0,0));
	for (vi=0;vi<4;vi++)
	{
		sprintf(sss,"VBAN #%i", vi+1);
		TextOut(dc,x0+5,y0+2,sss,(int)strlen(sss));	
		y0=y0+40;
	}

	strcpy(sss,"WARNING: VBAN Incoming Command Stream must be activated on Voicemeeter.");
	TextOut(dc,x0+45,y0+2,sss,(int)strlen(sss));	

	SelectObject(dc,oldfont);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldpen);

}

static BOOL CALLBACK VBANDIALOG_DialogProc(HWND hw,    //handle de la fenetre concernés.
										 UINT te,    //Type d'événement.
										 WPARAM p1,  //paramètre 1.
										 LPARAM p2)  //Ligne de paramètre 2
{
	char sss[128];
	LPT_VBANOPTION_DIALOGBOX lpctx;
	HPEN	oldpen;
	HBRUSH	oldbrush;
	long win_border, win_title, win_menu;
	RECT rect;
	HDC dc;
	PAINTSTRUCT ps;
	long dx,dy,nnn,nNotify;
	lpctx=&G_VBANDialog_CTX;
	switch (te)
	{
		case WM_INITDIALOG:
			lpctx->hw=hw;
			//--------------------------------------------			
			strcpy(sss,"VBAN Configuration...");
			SetWindowText(hw,sss);
			VBANDIALOG_CreateControls(lpctx, hw);
			TOOL_GetWindowsMetricInfo(&win_border, &win_title,&win_menu);

			dx=lpctx->dx+(win_border*2);
			dy=lpctx->dy+(win_border*2)+win_title;
			SetWindowPos(hw,HWND_TOP,lpctx->x0,lpctx->y0,dx,dy,SWP_SHOWWINDOW);
			SetFocus(hw);
			lpctx->fInit=1;
			break;

		case WM_COMMAND:
			if (lpctx->fInit == 0) break;
			nnn=LOWORD(p1);
			nNotify=HIWORD(p1);
			switch(nnn)
			{
				case 1:
					//CancelEditFlag(lpctx, lpctx->dialopt_track,FALSE);
					VBANDIALOG_ValidateOption(lpctx);
					EndDialog(hw,1);
					break;
				case 2:
					EndDialog(hw,0);
					break;

			}
			break;
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			VBANDIALOG_DrawStuff(lpctx, dc);
			EndPaint(hw,&ps);
			break;

		case WM_ERASEBKGND:
			dc=(HDC)p1;
			GetClientRect(hw,&rect);
			oldpen=(HPEN)SelectObject(dc,GetStockObject(WHITE_PEN));
			oldbrush=(HBRUSH)SelectObject(dc,GetStockObject(LTGRAY_BRUSH));
			Rectangle(dc, 0,0,rect.right,rect.bottom);
			SelectObject(dc,oldpen);
			SelectObject(dc,oldbrush);
			return TRUE;

		case WM_KEYDOWN:
			switch(p1)
			{
				case VK_RETURN:
					PostMessage(hw,WM_COMMAND,1,0);
					return 0;
				case VK_ESCAPE:
					PostMessage(hw,WM_COMMAND,2,0);
					return 0;
			}
			return -1;
		case WM_CLOSE:
			PostMessage(hw,WM_COMMAND,2,0);
			return TRUE;
		case WM_DESTROY:
			GlobalFree(lpctx->hdlgtemplate);
			lpctx->hw=NULL;
			break;
	}
	return 0;
}

BOOL VBANDIALOG_DialogGo(HWND h_parent,DWORD style)
{
	LPDLGTEMPLATE 	lpdlgtemplate;
	RECT			rect;
	LPT_VBANOPTION_DIALOGBOX lpctx;
	LPT_APP_CONTEXT lpapp;	
	lpctx=&G_VBANDialog_CTX;
	lpapp=&G_MainAppCtx;

	if (lpctx->hw != NULL) EndDialog(lpctx->hw, 0);
	GetWindowRect(h_parent, &rect);
	
	lpctx->hparent=h_parent;
	lpctx->style=style;
	lpctx->dx=600;
	lpctx->dy=300;
	lpctx->fInit = 0;

	lpctx->x0 = ((rect.right-rect.left)-lpctx->dx)>>1;
	lpctx->y0 = ((rect.bottom-rect.top)-lpctx->dy)>>1;
	if (lpctx->x0 < 0) lpctx->x0=0;
	if (lpctx->y0 < 0) lpctx->y0=0;
	lpctx->x0 +=rect.left;
	lpctx->y0 +=rect.top;

	//-------------------------------------------------------------
	lpctx->hdlgtemplate=GlobalAlloc(GMEM_ZEROINIT,1024);
	lpdlgtemplate=(LPDLGTEMPLATE)GlobalLock(lpctx->hdlgtemplate);
	lpdlgtemplate->style=WS_POPUP | DS_NOIDLEMSG | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER | WS_CAPTION  | WS_SYSMENU;
	lpdlgtemplate->dwExtendedStyle=0L;
	lpdlgtemplate->cdit=0L;
	GlobalUnlock(lpctx->hdlgtemplate);
	
	return (BOOL)DialogBoxIndirectParam(lpapp->hinstance,(const DLGTEMPLATE *)lpctx->hdlgtemplate,h_parent,(DLGPROC)VBANDIALOG_DialogProc,(LPARAM)lpctx);
}

















/*******************************************************************************/
/*                            LOAD / SAVE CONFIGURATION                        */
/*******************************************************************************/

static char * G_OptionDirKey ="VB-Audio\\VMStreamerView";

static char * G_OptionKey_VBANx_name="VBAN%i_name";
static char * G_OptionKey_VBANx_ip="VBAN%i_ip";
static char * G_OptionKey_VBANx_port="VBAN%i_port";

static char * G_OptionKey_ConnectionType="VMConnectType";
static char * G_OptionKey_StripSliderMode="StripSliderMode";

static char * G_OptionKey_WindowRectLeft="WinPos_X0";
static char * G_OptionKey_WindowRectTop="WinPos_Y0";
static char * G_OptionKey_WindowRectRight="WinPos_X1";
static char * G_OptionKey_WindowRectBottom="WinPos_Y1";


void LoadAppConfiguration(LPT_APP_CONTEXT lpapp)
{
	LPT_VBAN_CMD_PARAM lpVBANParam;
	BOOL ok;
	long nnn;
	int vi;
	char sz[2048];
	char szKey[128];

	// set default parameters
	for (vi=0;vi<4;vi++)
	{
		lpVBANParam = &(lpapp->VMLinkVBAN[vi]);
		strcpy(lpVBANParam->VBAN_szStreamName, "Command1");
		sprintf(lpVBANParam->VBAN_szIPAddress, "192.168.10.%i", 10+vi);
		lpVBANParam->VBAN_UDPport = 6980;
	}

	// windows rect
	ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectLeft, &nnn);
	if (ok == TRUE) lpapp->CurrentWindowRect.left = nnn;
	ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectTop, &nnn);
	if (ok == TRUE) lpapp->CurrentWindowRect.top = nnn;
	ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectRight, &nnn);
	if (ok == TRUE) lpapp->CurrentWindowRect.right = nnn;
	ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectBottom, &nnn);
	if (ok == TRUE) lpapp->CurrentWindowRect.bottom = nnn;
	
	// SliderMode
	ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, G_OptionKey_StripSliderMode, &nnn);
	if (ok == TRUE)
	{
		nnn=nnn & 0x0000000F;
		if (nnn > 2) nnn=0;
		lpapp->StripSliderMode = nnn;
	}

	// Link Type
	ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, G_OptionKey_ConnectionType, &nnn);
	if (ok == TRUE)
	{
		nnn=nnn & 0x0000000F;
		if (nnn > 4) nnn=0;
		lpapp->VMConnectionType = nnn;
	}

	// VBAN Configuration
	for (vi=0;vi<4;vi++)
	{
		lpVBANParam = &(lpapp->VMLinkVBAN[vi]);

		sprintf(szKey,G_OptionKey_VBANx_name, vi+1);
		ok=TOOL_RegistryLoadUserString(G_OptionDirKey, szKey, sz);
		if (ok == TRUE)
		{
			strncpy(lpVBANParam->VBAN_szStreamName,sz,32);
			lpVBANParam->VBAN_szStreamName[32-1]=0;
		}
		sprintf(szKey,G_OptionKey_VBANx_ip, vi+1);
		ok=TOOL_RegistryLoadUserString(G_OptionDirKey, szKey, sz);
		if (ok == TRUE)
		{
			strncpy(lpVBANParam->VBAN_szIPAddress,sz,32);
			lpVBANParam->VBAN_szIPAddress[32-1]=0;
		}
		sprintf(szKey,G_OptionKey_VBANx_port, vi+1);
		ok=TOOL_RegistryLoadUserDWORD(G_OptionDirKey, szKey, &nnn);
		if (ok == TRUE)
		{
			lpVBANParam->VBAN_UDPport = nnn & 0x0000FFFF;
		}
	}
}

void SaveAppConfiguration(LPT_APP_CONTEXT lpapp)
{
	LPT_VBAN_CMD_PARAM lpVBANParam;
	DWORD nnn;
	int vi;
	char szKey[128];

	// store current windows rect
	TOOL_RegistrySaveUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectLeft, lpapp->CurrentWindowRect.left);
	TOOL_RegistrySaveUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectTop, lpapp->CurrentWindowRect.top);
	TOOL_RegistrySaveUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectRight, lpapp->CurrentWindowRect.right);
	TOOL_RegistrySaveUserDWORD(G_OptionDirKey, G_OptionKey_WindowRectBottom, lpapp->CurrentWindowRect.bottom);

	// SliderMode
	nnn=lpapp->StripSliderMode;
	TOOL_RegistrySaveUserDWORD(G_OptionDirKey, G_OptionKey_StripSliderMode, nnn);

	// Link Type
	nnn=lpapp->VMConnectionType;
	TOOL_RegistrySaveUserDWORD(G_OptionDirKey, G_OptionKey_ConnectionType, nnn);
	
	// VBAN Configuration
	for (vi=0;vi<4;vi++)
	{
		lpVBANParam = &(lpapp->VMLinkVBAN[vi]);

		sprintf(szKey,G_OptionKey_VBANx_name, vi+1);
		TOOL_RegistrySaveUserString(G_OptionDirKey, szKey, lpVBANParam->VBAN_szStreamName);

		sprintf(szKey,G_OptionKey_VBANx_ip, vi+1);
		TOOL_RegistrySaveUserString(G_OptionDirKey, szKey, lpVBANParam->VBAN_szIPAddress);
		
		sprintf(szKey,G_OptionKey_VBANx_port, vi+1);
		nnn=lpVBANParam->VBAN_UDPport;
		TOOL_RegistrySaveUserDWORD(G_OptionDirKey, szKey, nnn);
	}
}






/*******************************************************************************/
/*                                 VBAN OR DIRECT                              */
/*******************************************************************************/


long SendParameterToVoicemeeter(char * szParam, float value)
{
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	// If Direct Link (we use Voicemeeter Remote API
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		return iVMR.VBVMR_SetParameterFloat(szParam, value);	// set value in Local Voicemeeter 
	}
	// Otherwise we send the VBAN request.
	else
	{
		return VBANCMD_SendRequest_Float(szParam, value);		// set value by a VBAN request
	}
}


long GetCurrentVoicemeeterType(unsigned long * pType)
{
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	// If Direct Link (we use Voicemeeter Remote API
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		return iVMR.VBVMR_GetVoicemeeterType(pType);
	}
	// Otherwise we ask to our VBANCMD Lib.
	else
	{
		return VBANCMD_GetVoicemeeterType(pType);
	}
}

long GetCurrentVoicemeeterVersion(unsigned long * pVersion)
{
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	// If Direct Link (we use Voicemeeter Remote API
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		return iVMR.VBVMR_GetVoicemeeterVersion(pVersion);
	}
	// Otherwise we ask to our VBANCMD Lib.
	else
	{
		return VBANCMD_GetVoicemeeterVersion(pVersion);
	}
}

long GetCurrentBusLabel(long index, WCHAR * pwsz)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Bus[%i].Label", index);
		return iVMR.VBVMR_GetParameterStringW(szParam, pwsz);
	}
	else return VBANCMD_GetBusLabel(index, pwsz);
}

long GetCurrentBusGain(long index, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Bus[%i].Gain", index);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetBusGain(index, pValue);
}

long GetCurrentBusSel(long index, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Bus[%i].Sel", index);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetBusSel(index, pValue);
}

long GetCurrentBusMute(long index, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Bus[%i].Mute", index);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetBusMute(index, pValue);
}

long GetCurrentBusMonitor(long index, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Bus[%i].Monitor", index);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetBusMonitor(index, pValue);
}


long GetCurrentStripLabel(long index, WCHAR * pwsz)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Strip[%i].Label", index);
		return iVMR.VBVMR_GetParameterStringW(szParam, pwsz);
	}
	else return VBANCMD_GetStripLabel(index, pwsz);
}

long GetCurrentStripGain(long index,  float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Strip[%i].Gain", index);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetStripGain(index, pValue);
}

long GetCurrentStripGainLayer(long index,  long layer, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Strip[%i].GainLayer[%i]", index, layer);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetStripGainLayer(index, layer, pValue);
}

long GetCurrentStripMute(long index, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Strip[%i].Mute", index);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetStripMute(index, pValue);
}

long GetCurrentStripAssignation(long index, long nuBus, float * pValue)
{
	char szParam[128];
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		sprintf(szParam,"Strip[%i].%s", index, lpapp->vbvmr_pBUSName[nuBus]);
		return iVMR.VBVMR_GetParameterFloat(szParam, pValue);
	}
	else return VBANCMD_GetStripAssignation(index, nuBus, pValue);
}

long GetCurrentLevelDB(long index, long nuChannel, float * pLevelDB)
{
	long rep;
	float NormalLevel;
	LPT_APP_CONTEXT lpapp;
	lpapp=&G_MainAppCtx;
	if (lpapp->VMConnectionType == 0)  
	{
		if (lpapp->vbvmr_NotInstalled != 0) return -1;
		NormalLevel =0.0f;
		rep= iVMR.VBVMR_GetLevel(index, nuChannel, &NormalLevel);
		if (NormalLevel > 1.0e-5f) *pLevelDB=(float)(20.0*log10(NormalLevel));
		else *pLevelDB=-100.0f;
		return rep;
	}
	else return VBANCMD_GetLevel(index, nuChannel, pLevelDB);
}


/*******************************************************************************/
/*                                 UPDATE CONTROLS                             */
/*******************************************************************************/

#define VMRS_BITSTRIP	0x00000001
#define VMRS_BITBUS		0x00000100
#define VMRS_BITSTRIPNB	0x00010000
#define VMRS_BITBUSNB	0x00020000
//#define VMRS_BITMONBUT	0x00040000
#define VMRS_BITMON		0x01000000

#define VMRS_BITALL		0xFFFFFFFF


#define VMRS_BITSTRIP_MASK	0x000000FF
#define VMRS_BITBUS_MASK	0x0000FF00


static long UpdateAllControl(LPT_APP_CONTEXT lpapp, long nChangeBit)
{
	BOOL fStripMustBeUpdated;
	LPT_MYVM_PARAMETERS lpp;
	long nu,mask,fUpdateAll;
	lpp = &(lpapp->current_param);
	if (nChangeBit == VMRS_BITALL) fUpdateAll=1;
	else fUpdateAll=0;
	//if BUS parameters are changed
	if ((nChangeBit & VMRS_BITBUS_MASK) != 0)
	{
		mask=VMRS_BITBUS;
		for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
		{
			if ((nChangeBit & mask) != 0)
			{
				VMSCTL_SetDataBUS(lpapp->hw_BusList[nu], &(lpp->bus[nu]), fUpdateAll);
			}
			mask = mask << 1;
		}
	}
	//if STRIP parameters are changed
	fStripMustBeUpdated = ((nChangeBit & VMRS_BITBUSNB) != 0);
	if (((nChangeBit & VMRS_BITSTRIP_MASK) != 0) || (fStripMustBeUpdated == TRUE))
	{
		mask=VMRS_BITSTRIP;
		for (nu=0;nu<lpapp->vbvmr_nbstrip;nu++)
		{
			if (((nChangeBit & mask) != 0) || (fStripMustBeUpdated == TRUE))
			{
				VMSCTL_SetDataSTRIP(lpapp->hw_StripList[nu], &(lpp->strip[nu]), fUpdateAll);
			}
			mask = mask << 1;
		}
	}
	
	return 0;
}




static long AnalyseParamChange(LPT_APP_CONTEXT lpapp)
{
	char  c1,c2, bus_assignbit[VMSCTL_MAX_NBBUS]; //0x10 bus activated / 0x01 bus assigned 
	WCHAR wszString[256]; 
	LPT_MYVM_PARAMETERS lpp;
	float fff;
	long vi, nu, nbstrip, nbbus, rep, updatebit=0;
	long mask, monitorbus=-1;

#ifdef _DEBUG
	if (lpapp->vbvmr_pStripName == NULL)
	{
		return 0;
	}
	if (lpapp->vbvmr_pBUSName == NULL) 
	{
		return 0;
	}
#endif

	lpp = &(lpapp->current_param);
	nbstrip=0;
	nbbus = 0;

	for (vi=0;vi<VMSCTL_MAX_NBBUS; vi++) bus_assignbit[vi]=0;

	//
	//get parameters for bus
	//
	mask = VMRS_BITBUS;
	for (vi=0;vi<lpapp->vbvmr_nbbus; vi++)
	{
		lpp->bus[vi].bus_fMonitorSupport = lpapp->vbvmr_monitor_support;
		strcpy(lpp->bus[vi].bus_nickname,lpapp->vbvmr_pBUSName[vi]);
		// bus label
		rep = GetCurrentBusLabel(vi, wszString);
		if (rep >= 0)
		{
			if (wcscmp(wszString, lpp->bus[vi].bus_namew) != 0)
			{
				updatebit |= mask;
			}
			wcscpy(lpp->bus[vi].bus_namew, wszString);
		}
		// if the BUS is displayed
		if ((lpp->bus[vi].bus_namew[0] != 0) || (vi==0)) 
		{
			bus_assignbit[vi] = 0x10; 
			nbbus++;
		}
		// bus gain
		rep = GetCurrentBusGain(vi, &fff);
		if (rep >= 0)
		{
			if (fff != lpp->bus[vi].bus_gain)
			{
				updatebit |= mask;
			}
			lpp->bus[vi].bus_gain = fff;
		}
		// BUS SEL 
		if (lpapp->vbvmr_multilayer != 0)
		{
			rep=GetCurrentBusSel(vi, &fff);
			if (rep >= 0)
			{
				if ((long)fff != lpp->bus[vi].bus_monitor)
				{
					updatebit |= mask;
				}
				lpp->bus[vi].bus_monitor = (long)fff;
			}
		}
		else lpp->bus[vi].bus_monitor=0;
		// Bus Mute
		rep=GetCurrentBusMute(vi, &fff);
		if (rep >= 0)
		{
			if ((long)fff != lpp->bus[vi].bus_mute)
			{
				updatebit |= mask;
			}
			lpp->bus[vi].bus_mute = (long)fff;
		}
		// Bus Monitor
		rep=GetCurrentBusMonitor(vi, &fff);
		if (rep >= 0)
		{
			if ((fff != 0.0f) && (monitorbus < 0))
			{
				monitorbus = vi;
			}
		}

		mask=mask<<1;
	}
	// check monitoring options
	if (lpapp->vbvmr_multilayer != 0)
	{
		// define what is the BUS monitoring
		if (monitorbus != lpp->monitorbus)
		{
			lpp->monitorbus = monitorbus;
			updatebit |= VMRS_BITMON;
		}
		//check bus monitoring option consistency (only one BUS can be monitored)
		nu=0;
		for (vi=0;vi<lpapp->vbvmr_nbbus; vi++)
		{
			if (lpp->bus[vi].bus_monitor != 0) nu++;
		}
		//otherwise we consider there is no monitoring
		if (nu > 1)
		{
			mask = VMRS_BITBUS;
			for (vi=0;vi<lpapp->vbvmr_nbbus; vi++)
			{
				if (lpp->bus[vi].bus_monitor != 0) updatebit |= mask;
				lpp->bus[vi].bus_monitor =0;
			}
			mask=mask<<1;
		}
	}
	//
	//get parameters for strips
	//
	mask = VMRS_BITSTRIP;
	for (vi=0;vi<lpapp->vbvmr_nbstrip; vi++)
	{
		strcpy(lpp->strip[vi].strip_nickname,lpapp->vbvmr_pStripName[vi]);
		// Strip label
		rep = GetCurrentStripLabel(vi, wszString);
		if (rep >= 0)
		{
			if (wcscmp(wszString, lpp->strip[vi].strip_namew) != 0)
			{
				updatebit |= mask;
			}
			wcscpy(lpp->strip[vi].strip_namew, wszString);
		}
		if (lpp->strip[vi].strip_namew[0] != 0) nbstrip++;

		// Strip Gain
		if (lpapp->vbvmr_multilayer == 0)
		{
			rep = GetCurrentStripGain(vi, &fff);
			if (rep >= 0)
			{
				if (fff != lpp->strip[vi].strip_gain[0])
				{
					updatebit |= mask;
				}
				for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
					lpp->strip[vi].strip_gain[nu] = fff;
			}
		}
		else
		{
			for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
			{
				rep= GetCurrentStripGainLayer(vi,  nu, &fff);
				if (rep >= 0)
				{
					if (fff != lpp->strip[vi].strip_gain[nu])
					{
						updatebit |= mask;
					}
					lpp->strip[vi].strip_gain[nu] = fff;
				}
			}

		}

		// bus assignation on the strip		
		lpp->strip[vi].strip_pBUSNameList = lpapp->vbvmr_pBUSName;
		lpp->strip[vi].strip_nbBusDisplayed = nbbus;
		lpp->strip[vi].strip_fMultiLayer = lpapp->vbvmr_multilayer;

		for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
		{
			c1=0;
			rep= GetCurrentStripAssignation(vi, nu, &fff);
			if (rep >= 0)
			{
				if (fff != 0) c1 = 0x01;
				c2= lpp->strip[vi].strip_assignmentbit[nu] & 0x0F;
				if (c1 != c2)
				{
					updatebit |= mask;
				}
			}
			lpp->strip[vi].strip_assignmentbit[nu] = bus_assignbit[nu] | c1;
		}
		// Strip Mute
		rep =GetCurrentStripMute(vi,&fff);
		if (rep >= 0)
		{
			if (fff != lpp->strip[vi].strip_mute)
			{
				updatebit |= mask;
			}
			lpp->strip[vi].strip_mute = (long)fff;
		}
		mask=mask<<1;
	}


	//check if the number of strip to display is changed...
	if (nbstrip != lpp->nbStrip)
	{
		updatebit |= VMRS_BITSTRIPNB;
		lpp->nbStrip=nbstrip;
	}
	//check if the number of bus to display is changed...
	if (nbbus != lpp->nbBus)
	{
		updatebit |= VMRS_BITBUSNB;
		lpp->nbBus=nbbus;
	}
	return updatebit;
}


void UpdateAllPeakMeters(LPT_APP_CONTEXT lpapp)
{
	float dBlevel;
	long nu, *pChannel, nuChannel, nc, nbMax;
	T_VMSCTL_RT_DATA rtData;

	//update peak meters for all STRIP
	pChannel= lpapp->vbvmr_pStripChannel;
	nuChannel=0;
	for (nu=0;nu<lpapp->vbvmr_nbstrip;nu++)
	{
		nbMax=pChannel[nu];					// get number of channel per Strip
		for (nc = 0; nc <nbMax; nc++)
		{
			GetCurrentLevelDB(0, nuChannel, &dBlevel);
			rtData.dbLevel[nc] = dBlevel;
			nuChannel++;
		}
		for (nc = nbMax; nc <8;nc++)
		{
			rtData.dbLevel[nc] = -100.0f;
		}
		VMSCTL_SetRTDataSTRIP(lpapp->hw_StripList[nu], &rtData);
	}

	//update peak meters for all BUS
	nuChannel =0;
	for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
	{
		for (nc = 0; nc <8; nc++)			// always 8 channel per Bus
		{
			GetCurrentLevelDB(3, nuChannel, &dBlevel);
			rtData.dbLevel[nc] = dBlevel;
			nuChannel++;
		}
		VMSCTL_SetRTDataBUS(lpapp->hw_BusList[nu], &rtData);
	}
}




/*******************************************************************************/
/*                                DISPLAY FUNCTIONS                            */
/*******************************************************************************/

void DrawVMConnectionMode(LPT_APP_CONTEXT lpapp, HDC dc)
{
	HBRUSH oldbrush;
	HPEN oldpen;
	HFONT oldfont;
	char sz1[128];
	char sss[256];
	RECT rect;
	rect=lpapp->rect_VMConnectionType;

	oldpen = (HPEN)SelectObject(dc,lpapp->gdiobjects_blue0.pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpapp->gdiobjects_bkg.brush);
	oldfont = (HFONT)SelectObject(dc,lpapp->font1);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(255,255,255));
	if (lpapp->vbvmr_connect == 0) SetTextColor(dc,RGB(200,0,0));

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,7,7);
	
	if (lpapp->VMConnectionType == 0) strcpy(sz1,"Direct");
	else 
	{
		if (lpapp->VBANisConnected == FALSE) SetTextColor(dc,RGB(200,0,0));
		sprintf(sz1,"VBAN #%i", lpapp->VMConnectionType);
	}
	strcpy(sss,"Connection: ");
	strcat(sss,sz1);

	DrawText(dc,sss,(int)strlen(sss),&rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}

void DrawSliderMode(LPT_APP_CONTEXT lpapp, HDC dc)
{
	HBRUSH oldbrush;
	HPEN oldpen;
	HFONT oldfont;
	char sss[128];
	RECT rect;
	rect=lpapp->rect_SliderMode;

	oldpen = (HPEN)SelectObject(dc,lpapp->gdiobjects_blue0.pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpapp->gdiobjects_blue0.brush);
	oldfont = (HFONT)SelectObject(dc,lpapp->font1);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(255,255,255));

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,7,7);
	
	strcpy(sss,"NO Link");
	if (lpapp->StripSliderMode == SLIDERLINKMODE_ABSLINK) strcpy(sss,"ABS Link");
	if (lpapp->StripSliderMode == SLIDERLINKMODE_RELLINK) strcpy(sss,"REL Link");
	
	DrawText(dc,sss,(int)strlen(sss),&rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}

void DrawAllStuff(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc)
{
	char sss[512];
	long nBus, xCenter;
	long x0,y0,dx,dy;
	RECT rect;
	HBRUSH oldbrush;
	HPEN oldpen;
	HFONT oldfont;

	oldpen = (HPEN)SelectObject(dc,lpapp->gdiobjects_blue0.pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpapp->gdiobjects_blue2.brush);
	oldfont = (HFONT)SelectObject(dc,lpapp->font2);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(0,0,0));

	rect=lpapp->rect_inputs;
	dx=(rect.right-rect.left) / 8;
	dy = rect.bottom-rect.top;
	x0=rect.left;
	y0=rect.top;	

	nBus=lpapp->vbvmr_nbbus;
	if (nBus < 2)
	{
		if (lpapp->VMConnectionType == 0) strcpy(sss,"Voicemeeter is not running...");
		else sprintf(sss,"Voicemeeter is not responding on VBAN #%i", lpapp->VMConnectionType);
		Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
		DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
	else
	{
		SelectObject(dc,lpapp->gdiobjects_bkg.pen);
		SelectObject(dc,lpapp->gdiobjects_bkg.brush);
		SetTextColor(dc,lpapp->gdiobjects_blue2.color);
		SelectObject(dc,lpapp->font1);
		// Display STRIP LABEL
		if (lpapp->current_param.nbStrip < 1)
		{
			strcpy(sss,"Label Voicemeeter Strip you want to see in the INPUTS section...");
			Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
			DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);

		}
		else
		{
			strcpy(sss,"INPUT STRIP");
			Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
			DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);
			lpapp->rect_VMConnectionType = rect;

			// display Strip slider mode
			lpapp->rect_SliderMode.left=rect.right;
			lpapp->rect_SliderMode.top=0;
			if (lpapp->vbvmr_multilayer != 0)
			{
				
				strcpy(sss,"Slider Mode:");
				rect.left = rect.right - 72- SLIDERLINKMODE_DX-10;
				if (rect.left < 120) rect.left=120;
				rect.right = rect.left+300;

				DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);
				
				rect.top+=2;
				rect.bottom-=2;
				lpapp->rect_SliderMode = rect;
				lpapp->rect_SliderMode.left +=80;
				lpapp->rect_SliderMode.right= lpapp->rect_SliderMode.left+SLIDERLINKMODE_DX;

				DrawSliderMode(lpapp, dc);

			}
			// display connection type
			lpapp->rect_VMConnectionType.left =100;
			lpapp->rect_VMConnectionType.right=lpapp->rect_SliderMode.left-100;
			if ((lpapp->rect_VMConnectionType.right - lpapp->rect_VMConnectionType.left) > 140)
			{
				xCenter = ((lpapp->rect_VMConnectionType.left +lpapp->rect_VMConnectionType.right)>>1);
				lpapp->rect_VMConnectionType.left	=xCenter-70;
				lpapp->rect_VMConnectionType.right=xCenter+70;
				DrawVMConnectionMode(lpapp, dc);
			}

		}
		// Display BUS LABEL
		SetTextColor(dc,lpapp->gdiobjects_blue2.color);
		rect=lpapp->rect_outputs;
		strcpy(sss,"OUTPUT BUS");
		Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
		DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		if (lpapp->vbvmr_multilayer != 0)
		{
			rect.left = rect.right - 110;
			if (rect.left < 120) rect.left=120;
			strcpy(sss,"Monitor BUS: ");
			if (lpapp->vbvmr_monitor_support == 0) strcat(sss, "OFF");
			else
			{
				if (lpapp->current_param.monitorbus >=0)
				{
					nBus = (lpapp->current_param.monitorbus) & 0x00000007;
					if (lpapp->vbvmr_pBUSName != NULL) strcat(sss,lpapp->vbvmr_pBUSName[nBus]);
					else strcat(sss, "OFF");
				}
				else strcat(sss, "OFF");
			}
			DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		}
	}	
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}




/*******************************************************************************/
/*                             USER ACTIONS MANAGEMENT                         */
/*******************************************************************************/

#define APP_ZONE_SLIDERMODE	100


long APP_WhereAmI(LPT_APP_CONTEXT lpapp, long x0, long y0, long * pnuOut, long * pnuIn)
{
	RECT rect;

	if (lpapp->vbvmr_multilayer != 0)
	{
		rect= lpapp->rect_SliderMode;
		if ((x0>=rect.left) && (x0<=rect.right) && (y0>=rect.top) && (y0<=rect.bottom)) return APP_ZONE_SLIDERMODE;
	}
	return -1;
}

void APP_ManageLRButtonDown(LPT_APP_CONTEXT lpapp, HWND hw, long x0, long y0, long fRightClick)
{
	HDC dc;
	long rep, nuIn,nuOut;
	rep= APP_WhereAmI(lpapp, x0, y0, &nuOut, &nuIn);	
	if (rep <0) return;

	switch(rep)
	{
	case APP_ZONE_SLIDERMODE:
		lpapp->StripSliderMode++;
		if (lpapp->StripSliderMode > 2) lpapp->StripSliderMode=0;
		dc=GetDC(hw);
		DrawSliderMode(lpapp, dc);
		ReleaseDC(hw,dc);
		break;
	}
}


/*******************************************************************************/
/*                               Init / End Software                           */
/*******************************************************************************/


long DetectVoicemeeterType(LPT_APP_CONTEXT lpapp, HWND hw)
{
	char szParam[128];
	long rep,vmType;

	lpapp->vbvmr_nbbus			=0;
	lpapp->vbvmr_nbstrip		=0;
	lpapp->vbvmr_multilayer		=0;
	lpapp->vbvmr_monitor_support=0;

	rep = GetCurrentVoicemeeterType(&vmType);
	if (rep == 0) 
	{
		rep=GetCurrentVoicemeeterVersion(&(lpapp->vbvmr_version));
		if (lpapp->vbvmr_connect != vmType)
		{
			lpapp->vbvmr_connect =vmType;
			switch(vmType)
			{
			case 1://Voicemeeter
				lpapp->vbvmr_nbbus		=2;
				lpapp->vbvmr_nbstrip	=3;
				lpapp->vbvmr_pBUSName	=G_szBUSNameList_v1;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v1;
				lpapp->vbvmr_pStripChannel =G_nbChannelPerStrip_v1;
				break;
			case 2://Voicemeeter Banana
				lpapp->vbvmr_nbbus		=5;
				lpapp->vbvmr_nbstrip	=5;
				lpapp->vbvmr_pBUSName	=G_szBUSNameList_v2;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v2;
				lpapp->vbvmr_pStripChannel =G_nbChannelPerStrip_v2;
				break;
			case 3://Voicemeeter 8
				lpapp->vbvmr_nbbus		=8;
				lpapp->vbvmr_nbstrip	=8;
				lpapp->vbvmr_multilayer	=1;
				lpapp->vbvmr_pBUSName	=G_szBUSNameList_v3;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v3;
				lpapp->vbvmr_pStripChannel =G_nbChannelPerStrip_v3;
				if (lpapp->vbvmr_version >= 0x02000109)
				{
					// we activate the Monitor on SEL option
					strcpy(szParam,"Option.MonitorOnSEL");
					rep=SendParameterToVoicemeeter(szParam, 1.0f);
					if (rep >= 0)
					{
						lpapp->vbvmr_monitor_support=1;
					}
				}
				break;
			}
			return 1;
		}
	}
	return 0;
}



BOOL InitSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	char szTitle[]="Init Error";
	long rep;

	//init context
	lpapp->hwnd_MainWindow = hw;
	lpapp->vbvmr_connect=0;
	
	//init resources
	InitResources(lpapp);

	//get DLL interface for Voicemeeter remote API (local use)
	//the VM Streamer View can work with Voicemeeter Dll
	lpapp->vbvmr_NotInstalled=0;
	rep=InitializeDLLInterfaces();
	if (rep >= 0)
	{
		//Log in
		rep=iVMR.VBVMR_Login();
		if (rep < 0)
		{
			MessageBox(hw,"Failed To Login",szTitle,MB_APPLMODAL | MB_OK | MB_ICONERROR);
			lpapp->vbvmr_NotInstalled=1;
		}
		//call this to get first parameters state (if server already launched)
		lpapp->vbvmr_error = iVMR.VBVMR_IsParametersDirty();
		if (lpapp->vbvmr_error == 0)
		{
			DetectVoicemeeterType(lpapp, hw);
			AnalyseParamChange(lpapp);
		}
	}
	else lpapp->vbvmr_NotInstalled=1;
	// we return always true, because the application can work with VBAN connection only
	return TRUE;
}

BOOL EndSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	VBANCMD_StopThread();
	if (iVMR.VBVMR_Logout != NULL) iVMR.VBVMR_Logout();
	EndResources(lpapp);
	return TRUE;
}



void ManageMenu(LPT_APP_CONTEXT lpapp, HWND hw, WPARAM wparam,	LPARAM lparam)
{
	switch(LOWORD(wparam))
	{
		case IDM_UPDATE_ALL_CONTROL:
			UpdateAllControl(lpapp, VMRS_BITALL);
			break;

	}
}


/*******************************************************************************/
/*                              CONTROLS CALLBACK                              */
/*******************************************************************************/

void __stdcall APP_InputCtlCallback(void * lpuser,long Ident, long ICtl, float value)
{
	float delta, vvv;
	char szParam[128], c1,  bus_assignbit;
	long rep,nuStrip, nuVisibleSlider, index, nu;
	LPT_MYVM_PARAMETERS lpp;
	LPT_APP_CONTEXT lpapp;
	lpapp=(LPT_APP_CONTEXT)lpuser;
	lpp = &(lpapp->current_param);
	nuStrip = Ident;

	// from gain slider (8 possible slider
	if ((ICtl >= VMSCTL_STRIP_ID_GAIN) && (ICtl < (VMSCTL_STRIP_ID_GAIN + VMSCTL_MAX_NBBUS)))
	{
		nuVisibleSlider = ICtl - VMSCTL_STRIP_ID_GAIN;
		index = VMSCTL_GetIndexSTRIP(lpapp->hw_StripList[nuStrip],nuVisibleSlider);

		// no multi layer
		if (lpapp->vbvmr_multilayer == 0)
		{
			sprintf(szParam,"Strip[%i].Gain", nuStrip);
			rep=SendParameterToVoicemeeter(szParam, value);
			if (rep >= 0)
			{
				for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
					lpp->strip[nuStrip].strip_gain[nu] = value;
			}
			VMSCTL_SetDataSTRIP(lpapp->hw_StripList[nuStrip], &(lpp->strip[nuStrip]), FALSE);
		}
		// multi layer
		else
		{
			delta=0.0f;
			sprintf(szParam,"Strip[%i].GainLayer[%i]", nuStrip, index);
			rep=SendParameterToVoicemeeter(szParam, value);
			if (rep >= 0)
			{
				delta= value-lpp->strip[nuStrip].strip_gain[index];
				lpp->strip[nuStrip].strip_gain[index] = value;
			}
			switch(lpapp->StripSliderMode)
			{
			case SLIDERLINKMODE_ABSLINK: // absolute link = all slider get the same value
				for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
				{
					// we update other slider / visible on the strip
					if ((nu != index) && ((lpp->strip[nuStrip].strip_assignmentbit[nu] & 0xF0) != 0))
					{
						sprintf(szParam,"Strip[%i].GainLayer[%i]", nuStrip, nu);
						rep=SendParameterToVoicemeeter(szParam, value);
						if (rep >= 0)
						{
							lpp->strip[nuStrip].strip_gain[nu] = value;
						}
					}
				}
				VMSCTL_SetDataSTRIP(lpapp->hw_StripList[nuStrip], &(lpp->strip[nuStrip]), FALSE);
				break;
			case SLIDERLINKMODE_RELLINK:// relative link = all slider are moved by the same amount (delta)
				for (nu=0;nu<lpapp->vbvmr_nbbus;nu++)
				{
					// we update other slider / visible on the strip
					if ((nu != index) && ((lpp->strip[nuStrip].strip_assignmentbit[nu] & 0xF0) != 0))
					{
						vvv = lpp->strip[nuStrip].strip_gain[nu] + delta;
						sprintf(szParam,"Strip[%i].GainLayer[%i]", nuStrip, nu);
						rep=SendParameterToVoicemeeter(szParam, vvv);
						if (rep >= 0)
						{
							lpp->strip[nuStrip].strip_gain[nu] = vvv;
						}
					}
				}
				VMSCTL_SetDataSTRIP(lpapp->hw_StripList[nuStrip], &(lpp->strip[nuStrip]), FALSE);
				break;
			}
		}
		return;
	}
	// from BUS Assignation button
	if ((ICtl >= VMSCTL_STRIP_ID_ASSIGN) && (ICtl < (VMSCTL_STRIP_ID_ASSIGN + VMSCTL_MAX_NBBUS)))
	{
		nuVisibleSlider = ICtl - VMSCTL_STRIP_ID_ASSIGN;
		index = VMSCTL_GetIndexSTRIP(lpapp->hw_StripList[nuStrip],nuVisibleSlider);

		if ((index >=0) && (lpapp->vbvmr_pBUSName!=NULL))
		{
			sprintf(szParam,"Strip[%i].%s", nuStrip, lpapp->vbvmr_pBUSName[index]);
			rep=SendParameterToVoicemeeter(szParam, value);
			if (rep >= 0)
			{
				c1=0x00;
				bus_assignbit = lpp->strip[nuStrip].strip_assignmentbit[index] & 0xF0;
				if (value != 0.0f) c1=0x01;
				lpp->strip[nuStrip].strip_assignmentbit[index] = bus_assignbit | c1;
			}			
		}
		return;
	}

	// other sub control
	switch(ICtl)
	{
	case VMSCTL_STRIP_ID_MUTE:
		sprintf(szParam,"Strip[%i].Mute", nuStrip);
		rep=SendParameterToVoicemeeter(szParam, value);		// set value in Voicemeeter
		if (rep >= 0) lpp->strip[nuStrip].strip_mute = (long)value;	// update internal parameters.
		break;
	}
}

void __stdcall APP_OutputCtlCallback(void * lpuser,long Ident, long ICtl, float value)
{
	char szParam[128];
	LPT_MYVM_PARAMETERS lpp;
	long rep,nuBus, vi;
	LPT_APP_CONTEXT lpapp;
	lpapp=(LPT_APP_CONTEXT)lpuser;
	lpp = &(lpapp->current_param);
	nuBus = Ident;
	switch(ICtl)
	{
	// From Mute button
	case VMSCTL_BUS_ID_MUTE:
		sprintf(szParam,"Bus[%i].Mute", nuBus);
		rep=SendParameterToVoicemeeter(szParam, value);		// set value in Voicemeeter
		if (rep >= 0) lpp->bus[nuBus].bus_mute = (long)value;	// update internal parameters.
		break;
	// From Gain Slider
	case VMSCTL_BUS_ID_GAIN:
		sprintf(szParam,"Bus[%i].Gain", nuBus);
		rep=SendParameterToVoicemeeter(szParam, value);		// set value in Voicemeeter
		if (rep >= 0) lpp->bus[nuBus].bus_gain = value;	// update internal parameters.
		break;
	// from monitor
	case VMSCTL_BUS_ID_MONITOR:
		sprintf(szParam,"Bus[%i].Sel", nuBus);
		rep=SendParameterToVoicemeeter(szParam, value);		// set value in Voicemeeter
		if (rep >= 0) 
		{
			lpp->bus[nuBus].bus_monitor = (long)value;	// update internal parameters.
			if (value != 0.0)
			{
				// Remove all other SEL (Monitor = exclusive SEL)
				for (vi=0;vi<lpapp->vbvmr_nbbus;vi++)
				{
					if (vi != nuBus)
					{
						sprintf(szParam,"Bus[%i].Sel", vi);
						rep=SendParameterToVoicemeeter(szParam, 0.0f);		
						if (rep >= 0)
						{
							lpp->bus[nuBus].bus_monitor=0;
							VMSCTL_SetDataBUS(lpapp->hw_BusList[vi], &(lpp->bus[vi]), FALSE);
						}
					}
				}
			}
		}
		break;
	}
}



/*******************************************************************************/
/*                                 CONTROLS                                    */
/*******************************************************************************/

static long CreateAllControls(LPT_APP_CONTEXT lpapp, HWND hParent)
{
	T_VMSCTL_STRIPPARAM param_strip;
	T_VMSCTL_BUSPARAM param_bus;
	long vi;

	// Create all STRIP Control	
	memset(&param_strip, 0, sizeof(T_VMSCTL_STRIPPARAM));

	param_strip.vmin	=-60.0f;
	param_strip.vmax	=+12.0f;

	param_strip.font0		=lpapp->font0;
	param_strip.font1		=lpapp->font1;
	param_strip.font2		=lpapp->font2;
	param_strip.bkg0_brush	=lpapp->gdiobjects_bkg.brush;
	param_strip.bkg1_brush	=lpapp->gdiobjects_bkg_ctl.brush;

	param_strip.lpCallback	=APP_InputCtlCallback;
	param_strip.lpuser		=lpapp;

	param_strip.assignbus_pen	=lpapp->gdiobjects_blue1.pen;
	param_strip.assignbus_color	=lpapp->gdiobjects_blue1.color;
	param_strip.name_color		=lpapp->gdiobjects_white.color;
	param_strip.meter_brush1	=lpapp->gdiobjects_meter_blue.brush;
	param_strip.meter_brush2	=lpapp->gdiobjects_meter_green.brush;
	param_strip.meter_brush3	=lpapp->gdiobjects_meter_red.brush;

	param_strip.slider_pen_green	=lpapp->gdiobjects_slidergreen.pen;
	param_strip.slider_brush_green	=lpapp->gdiobjects_slidergreen.brush;
	param_strip.slider_pen_red		=lpapp->gdiobjects_sliderred.pen;
	param_strip.slider_brush_red	=lpapp->gdiobjects_sliderred.brush;

	param_strip.mutered_color	=lpapp->gdiobjects_mute.color;
	param_strip.mutered_pen		=lpapp->gdiobjects_mute.pen;
	param_strip.assigned_color	=lpapp->gdiobjects_slidergreen.color;
	param_strip.assigned_pen	=lpapp->gdiobjects_slidergreen.pen;


	for (vi=0;vi<VMSCTL_MAX_NBSTRIP; vi++)
	{
		lpapp->hw_StripList[vi] = VMSCTL_CreateSTRIP(hParent, vi, 10,10, &param_strip);
	}

	
	// Create all BUS Control	
	memset(&param_bus, 0, sizeof(T_VMSCTL_STRIPPARAM));

	param_bus.vmin	=-60.0f;
	param_bus.vmax	=+12.0f;

	param_bus.font0			=lpapp->font0;
	param_bus.font1			=lpapp->font1;
	param_bus.font2			=lpapp->font2;
	param_bus.bkg0_brush	=lpapp->gdiobjects_bkg.brush;
	param_bus.bkg1_brush	=lpapp->gdiobjects_bkg_ctl.brush;
	param_bus.assignbus_pen	=lpapp->gdiobjects_blue1.pen;
	param_bus.assignbus_color=lpapp->gdiobjects_blue1.color;
	param_bus.name_color	=lpapp->gdiobjects_white.color;
	param_bus.meter_brush1	=lpapp->gdiobjects_meter_blue.brush;
	param_bus.meter_brush2	=lpapp->gdiobjects_meter_green.brush;
	param_bus.meter_brush3	=lpapp->gdiobjects_meter_red.brush;

	param_bus.slider_pen_green	=lpapp->gdiobjects_slidergreen.pen;
	param_bus.slider_brush_green=lpapp->gdiobjects_slidergreen.brush;
	param_bus.slider_pen_red	=lpapp->gdiobjects_sliderred.pen;
	param_bus.slider_brush_red	=lpapp->gdiobjects_sliderred.brush;

	param_bus.mutered_color	=lpapp->gdiobjects_mute.color;
	param_bus.mutered_pen	=lpapp->gdiobjects_mute.pen;

	param_bus.monitor_brush	=lpapp->gdiobjects_monitor.brush;
	param_bus.monitor_pen	=lpapp->gdiobjects_monitor.pen;

	param_bus.lpCallback	=APP_OutputCtlCallback;
	param_bus.lpuser		=lpapp;
	
	for (vi=0;vi<VMSCTL_MAX_NBBUS; vi++)
	{
		//lpapp->hw_BusList[vi];
		lpapp->hw_BusList[vi] = VMSCTL_CreateBUS(hParent, vi, 10,10, &param_bus);

	}
	return 0;	
}


static void DestroyAllControls(LPT_APP_CONTEXT lpapp)
{
	long vi;
	for (vi=0;vi<VMSCTL_MAX_NBSTRIP; vi++)
	{
		if (lpapp->hw_StripList[vi] != NULL) DestroyWindow(lpapp->hw_StripList[vi]);
		lpapp->hw_StripList[vi]=NULL;

	}
	for (vi=0;vi<VMSCTL_MAX_NBBUS; vi++)
	{
		if (lpapp->hw_BusList[vi] != NULL) DestroyWindow(lpapp->hw_BusList[vi]);
		lpapp->hw_BusList[vi]=NULL;
	}
}

#define APP_MARGIN			10
#define APP_SECTIONLABEL	25

static void APP_RecomputeLayout(LPT_APP_CONTEXT lpapp, long dx, long dy)
{
	RECT rect;
	LPT_MYVM_PARAMETERS lpp;
	long fShow;
	long vi, x0, y0;
	long nbStrip, nbBus;
	long dx_strip, dy_strip;
	long dx_bus, dy_bus;
	long fRecompute = 0;
	if ((dx == 0) || (dy == 0)) fRecompute=1;
	if (dx > 0)
	{
		if (lpapp->hw_dx !=dx) fRecompute=1;
		lpapp->hw_dx =dx;
	}
	else  dx = lpapp->hw_dx;
	if (dy > 0)
	{
		if (lpapp->hw_dy !=dy) fRecompute=1;
		lpapp->hw_dy =dy;
	}
	else dy = lpapp->hw_dy;
	if (fRecompute == 0) return;

	rect.left=APP_MARGIN;
	rect.right=dx-APP_MARGIN;
	rect.top=APP_MARGIN;
	rect.bottom=dy-APP_MARGIN;

	dx=dx-APP_MARGIN;
	dy=dy-APP_MARGIN;
	lpp = &(lpapp->current_param);
	nbStrip	=lpp->nbStrip;
	nbBus	=lpp->nbBus;

	dx_bus = dx-APP_MARGIN; 
	dy_bus = 40;
	if (nbStrip > 0) dx_strip = dx / nbStrip; 
	else dx_strip=dx;
	dy_strip = dy - (dy_bus*nbBus) - APP_SECTIONLABEL - APP_SECTIONLABEL-APP_MARGIN-APP_MARGIN;

	lpapp->rect_inputs=rect;
	lpapp->rect_inputs.bottom=lpapp->rect_inputs.top+APP_SECTIONLABEL;

	lpapp->rect_outputs=rect;
	lpapp->rect_outputs.top += APP_SECTIONLABEL + dy_strip+APP_MARGIN;
	lpapp->rect_outputs.bottom=lpapp->rect_outputs.top+APP_SECTIONLABEL;

	//position strip
	x0 = APP_MARGIN; 
	y0 = APP_MARGIN+APP_SECTIONLABEL+10;
	dy_strip -=10;
	for (vi = 0; vi < VMSCTL_MAX_NBSTRIP; vi++)
	{
		if (lpp->strip[vi].strip_namew[0] != 0)
		{
			fShow=1;
		}
		else fShow=0;
		VMSCTL_PositionSTRIP(lpapp->hw_StripList[vi], x0,y0, dx_strip-APP_MARGIN, dy_strip, fShow);
		if (fShow != 0) x0=x0+dx_strip;
	}

	//position bus
	x0 = APP_MARGIN; 
	y0 = lpapp->rect_outputs.bottom+10;
	for (vi = 0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		if ((lpp->bus[vi].bus_namew[0] != 0) || (vi==0))
		{
			fShow=1;
		}
		else fShow=0;
		VMSCTL_PositionBUS(lpapp->hw_BusList[vi], x0,y0, dx_bus, dy_bus-APP_MARGIN, fShow);
		if (fShow != 0) y0=y0+dy_bus;
	}
}

static void APP_ResetData(LPT_APP_CONTEXT lpapp)
{
	long vi;
	memset(&(lpapp->current_param), 0 , sizeof(T_MYVM_PARAMETERS )); //reset parameters
	for (vi = 0; vi < VMSCTL_MAX_NBSTRIP; vi++)
	{
		VMSCTL_ResetDataSTRIP(lpapp->hw_StripList[vi]);
	}
	for (vi = 0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		VMSCTL_ResetDataBUS(lpapp->hw_BusList[vi]);
	}
}



/*******************************************************************************/
/*                                WIN CALLBACK                                 */
/*******************************************************************************/

#define MYTIMERID 16489

static void StartVBANConnection(LPT_APP_CONTEXT lpapp, HWND hw, long nuVBAN)
{
	LPT_VBAN_CMD_PARAM lpVBANParam;
	lpVBANParam = &(lpapp->VMLinkVBAN[nuVBAN]);
	VBANCMD_StartThread(lpVBANParam->VBAN_szStreamName, lpVBANParam->VBAN_szIPAddress, (unsigned short)lpVBANParam->VBAN_UDPport);
	lpapp->VMConnectionType =nuVBAN+1;
	// send request to register for RT-Packet.
	VBANCMD_SendRequest_RegisterRTPacket(15, "VMStreamerView");
	//disconnect to force reconnection
	lpapp->vbvmr_connect	=0;
	if (hw != NULL) InvalidateRect(hw,NULL,TRUE);
}


LRESULT CALLBACK MainWindowManageEvent(HWND hw,			//handle of the window.
											UINT msg,   //Message Ident.
											WPARAM wparam,	//parameter 1.
											LPARAM lparam)	//parameter 2
{
	HMENU sysmenu;
	HPEN oldpen;
	HBRUSH oldbrush;
	RECT rect;
	long dx,dy, nChange, nNewLogin, fUpdatePaint;
	long nCommand, nuVBAN;
	BOOL LastConnectedState;
	DWORD ms, delta, nnn;
	LPT_APP_CONTEXT lpapp;
	HDC dc;
	PAINTSTRUCT ps;

	lpapp = &G_MainAppCtx;
	switch (msg)
	{


		case WM_CREATE:
			if (InitSoftware(lpapp,hw) == FALSE) return -1;//return -1 here cancel the window creation
			CreateAllControls(lpapp, hw);

			// add item in system menu
			sysmenu=GetSystemMenu(hw,FALSE);
			if (sysmenu != NULL)
			{
				AppendMenu(sysmenu,MF_SEPARATOR,0,NULL);
				AppendMenu(sysmenu,MF_STRING,IDMSYS_LINK_DIRECT,"Direct Connection");
				AppendMenuA(sysmenu,MF_STRING,IDMSYS_LINK_VBAN1,"VBAN #1 Connection");
				AppendMenuA(sysmenu,MF_STRING,IDMSYS_LINK_VBAN2,"VBAN #2 Connection");
				AppendMenuA(sysmenu,MF_STRING,IDMSYS_LINK_VBAN3,"VBAN #3 Connection");
				AppendMenuA(sysmenu,MF_STRING,IDMSYS_LINK_VBAN4,"VBAN #4 Connection");
				AppendMenu(sysmenu,MF_SEPARATOR,0,NULL);
				AppendMenuA(sysmenu,MF_STRING,IDMSYS_VBAN_CONFIG,"VBAN Configuration...");
			}			
			PostMessage(hw,WM_COMMAND, IDM_UPDATE_ALL_CONTROL,0);
			lpapp->wTimer = SetTimer(hw,MYTIMERID, 30,NULL);//30 Hz timer
			
			if (lpapp->VMConnectionType > 0)
			{
				StartVBANConnection(lpapp, NULL, lpapp->VMConnectionType-1);
			}
			break;
		case WM_INITMENU:
			sysmenu = GetSystemMenu(hw,FALSE);
			if (sysmenu != NULL)
			{
				if (lpapp->VMConnectionType == 0) CheckMenuItem(sysmenu, IDMSYS_LINK_DIRECT, MF_BYCOMMAND | MF_CHECKED);
				else CheckMenuItem(sysmenu, IDMSYS_LINK_DIRECT, MF_BYCOMMAND | MF_UNCHECKED);
				if (lpapp->VMConnectionType == 1) CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN1, MF_BYCOMMAND | MF_CHECKED);
				else CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN1, MF_BYCOMMAND | MF_UNCHECKED);
				if (lpapp->VMConnectionType == 2) CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN2, MF_BYCOMMAND | MF_CHECKED);
				else CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN2, MF_BYCOMMAND | MF_UNCHECKED);
				if (lpapp->VMConnectionType == 3) CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN3, MF_BYCOMMAND | MF_CHECKED);
				else CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN3, MF_BYCOMMAND | MF_UNCHECKED);
				if (lpapp->VMConnectionType == 4) CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN4, MF_BYCOMMAND | MF_CHECKED);
				else CheckMenuItem(sysmenu, IDMSYS_LINK_VBAN4, MF_BYCOMMAND | MF_UNCHECKED);
			}
			break;
		case WM_LBUTTONDOWN:
			APP_ManageLRButtonDown(lpapp, hw, (short int)LOWORD(lparam),(short int)HIWORD(lparam),0);
			break;	
		case WM_SIZE:
			dx=(short)LOWORD(lparam);
			dy=(short)HIWORD(lparam);
			APP_RecomputeLayout(lpapp, dx,dy);
			break;

		case WM_COMMAND:
			ManageMenu(lpapp,hw,wparam,lparam);
			break;

		case WM_SYSCOMMAND:
			nCommand = LOWORD(wparam);
			switch(nCommand)
			{
			case IDMSYS_VBAN_CONFIG:
				VBANDIALOG_DialogGo(hw,0);
				break;
			case IDMSYS_LINK_DIRECT:
				if (lpapp->VMConnectionType == 0) break;
				VBANCMD_StopThread();
				lpapp->VMConnectionType =0;
				//disconnect to force reconnection
				lpapp->vbvmr_connect	=0;
				InvalidateRect(hw,NULL,TRUE);
				break;
			case IDMSYS_LINK_VBAN1:
			case IDMSYS_LINK_VBAN2:
			case IDMSYS_LINK_VBAN3:
			case IDMSYS_LINK_VBAN4:
				nuVBAN = nCommand-IDMSYS_LINK_VBAN1;
				if (lpapp->VMConnectionType == (nuVBAN+1)) break;
				StartVBANConnection(lpapp, hw, nuVBAN);
				break;

			default:
				return DefWindowProc(hw,msg,wparam,lparam);
			}
			break;
		case WM_TIMER:
			if (wparam == MYTIMERID)
			{
				ms = GetTickCount();

				// if direct connection
				if ((lpapp->VMConnectionType == 0) && (lpapp->vbvmr_NotInstalled == 0))
				{
					//check what is changed
					lpapp->vbvmr_error=iVMR.VBVMR_IsParametersDirty();
				}
				// if VBAN connection
				else
				{
					// every 1 seconds
					delta = TOOL_GetDeltaMsDword(lpapp->msLastTimeForVBANCheck, ms);
					if (delta > 1000)
					{
						LastConnectedState = lpapp->VBANisConnected;
						nnn = VBANCMD_GetIncomingRequestCounter();
						if (nnn != lpapp->LastCounter) lpapp->VBANisConnected=TRUE;
						else lpapp->VBANisConnected=FALSE;
						lpapp->LastCounter = nnn;
						lpapp->msLastTimeForVBANCheck = ms;
						// if the VBAN connection is changed, we update the display
						if (LastConnectedState != lpapp->VBANisConnected)
						{
							dc=GetDC(hw);
							DrawVMConnectionMode(lpapp, dc);
							ReleaseDC(hw,dc);
						}
					}
					// every 10 seconds
					delta = TOOL_GetDeltaMsDword(lpapp->msLastTimeForVBANRegister, ms);
					if (delta > 10000)
					{
						// we ask for 15 second timeout = we will get RT-Packet for 15 seconds
						VBANCMD_SendRequest_RegisterRTPacket(15, "VMStreamerView");
						lpapp->msLastTimeForVBANRegister = ms;
					}
					lpapp->vbvmr_error=VBANCMD_IsParameterDirty();
				}
				// manage behavior
				if (lpapp->vbvmr_error >= 0)
				{
					//if not already connected we detect Voicemeeter type
					nNewLogin=0;
					if (lpapp->vbvmr_connect == 0)
					{
						nNewLogin = DetectVoicemeeterType(lpapp, hw);
						if (nNewLogin != 0) APP_ResetData(lpapp);
					}
					//if param change
					if ((lpapp->vbvmr_error == 1) || (nNewLogin != 0))
					{
						fUpdatePaint=0;
						nChange=AnalyseParamChange(lpapp);
						if (nNewLogin != 0) nChange=VMRS_BITALL;
						//if layout change
						if ((nChange & (VMRS_BITSTRIPNB | VMRS_BITBUSNB)) != 0)
						{
							APP_RecomputeLayout(lpapp, 0,0);
							fUpdatePaint=1;
						}
						if ((nChange & VMRS_BITMON) != 0) fUpdatePaint=1;
						if (nChange != 0) 
						{
							UpdateAllControl(lpapp, nChange);
							if (fUpdatePaint != 0) InvalidateRect(hw,NULL,FALSE);
						}
					}
				}
				// otherwise we consider it's not connected
				else 
				{
					// manage disconnection
					if (lpapp->vbvmr_connect != 0)
					{
						//Voicemeeter has been shut down / disconnected
						lpapp->vbvmr_connect	=0;
						InvalidateRect(hw,NULL,TRUE);
					}
				}
				//Peak Meter display 
				if (lpapp->vbvmr_connect != 0) UpdateAllPeakMeters(lpapp);
			}
			break;
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			DrawAllStuff(lpapp,hw, dc);
			EndPaint(hw,&ps);
	        break;
		case WM_ERASEBKGND:
			dc=(HDC)wparam;
			GetClientRect(hw,&rect);
			oldbrush = (HBRUSH)SelectObject(dc,lpapp->gdiobjects_bkg.brush);
			oldpen = (HPEN)SelectObject(dc,lpapp->gdiobjects_bkg.pen);
			Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
			SelectObject(dc,oldpen);
			SelectObject(dc,oldbrush);
			return 0;
		case WM_DESTROY:
			GetWindowRect(hw, &(lpapp->CurrentWindowRect));
			SaveAppConfiguration(lpapp);
			if (lpapp->wTimer != 0) KillTimer(hw,lpapp->wTimer);
			lpapp->wTimer=0;
			DestroyAllControls(lpapp);
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
	HANDLE	hMutex;
	HWND	hh;
	long	wstyle, wdx, wdy;
	MSG		msg;    
	char	szWindowClassName[]="MainWindowClass";
	char *	title="Sorry";

	WNDCLASS	wc;
	LPT_APP_CONTEXT lpapp;
	lpapp = &G_MainAppCtx;

	//we first store the APP Hinstance
	memset(lpapp, 0,  sizeof(T_APP_CONTEXT));
	lpapp->hinstance=handle_app;

	//
	// Use Mutex to prevent to launch several instance of the application
	//
	hMutex = CreateMutex(NULL,TRUE,APP_UNIQUEMUTEX);
	if (hMutex == NULL) return -1;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(hMutex);
		return -1;
	}
	else WaitForSingleObject(hMutex,INFINITE);

#ifdef VBUSE_LOCALLIB
	//if we directly link source code (VB local development only)
	VBVMR_SetHinstance(handle_app);
#endif
	//here you can make some early initialization and analyze command line if any.
	VMSCTL_InitLib(handle_app);
	VBANCMD_InitLib(0);

	//load last configuration and get Windows position/size
	lpapp->CurrentWindowRect.left = CW_USEDEFAULT;
	lpapp->CurrentWindowRect.top = CW_USEDEFAULT;
	lpapp->CurrentWindowRect.right = CW_USEDEFAULT+UI_WIN_DX;
	lpapp->CurrentWindowRect.bottom = CW_USEDEFAULT+UI_WIN_DY;
	LoadAppConfiguration(lpapp);
	wdx = lpapp->CurrentWindowRect.right-lpapp->CurrentWindowRect.left;
	wdy = lpapp->CurrentWindowRect.bottom - lpapp->CurrentWindowRect.top;

	//we define a window class to create a window from this class 
	wc.style		=CS_HREDRAW | CS_VREDRAW;  	  		//property.
	wc.lpfnWndProc=(WNDPROC)MainWindowManageEvent;		//Adresse of our Callback.
	wc.cbClsExtra =0;					  				//Possibility to store some byte inside a class object.
	wc.cbWndExtra =0;                          			//Possibility to store some byte inside a window object.
	wc.hInstance  =handle_app; 	                		//handle of the application hinstance.
	wc.hIcon      =LoadIcon(handle_app, MAKEINTRESOURCE(100));   	//handle of icon displayed in the caption.
	wc.hCursor    =LoadCursor(NULL,IDC_ARROW);			//handle of cursor mouse .
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);//Background color.
	wc.lpszMenuName=NULL;			    				//pointer on menu defined in resource.
	wc.lpszClassName=szWindowClassName;       			//pointer on class name.

	//register class.
	if (RegisterClass(&wc)==0)
	{
		MessageBox(NULL,"Failed to register main class...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}

	
	//classical Main Window
	wstyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	hh=CreateWindow(szWindowClassName,					// address of registered class name.
						 SZPUBLICNAME,					// address of window name string
						 wstyle,						// window style
						 lpapp->CurrentWindowRect.left,					// horizontal position of window
						 lpapp->CurrentWindowRect.top,					// vertical position of window
						 wdx,						// window width
						 wdy,						// window height
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
	VBANCMD_EndLib();
	VMSCTL_EndLib();
	return (int)(msg.wParam);
}
