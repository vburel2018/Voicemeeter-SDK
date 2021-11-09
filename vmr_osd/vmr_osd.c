/*--------------------------------------------------------------------------------*/
/* VMR-OSD : On Screen Display application to show last current gains slider move */
/*--------------------------------------------------------------------------------*/
/* 'C' Sample Code to use VoicemeeterRemote                  V.Burel (c)2015-2021 */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to link VoicemeeterRemote.dll                                           */
/*  - How to Login / logout                                                       */
/*  - How to use GetParameter Function.                                           */
/*  - How to display new parameters values.                                       */
/*  - How to make a borderless application.                                       */
/*  - How to place application in system tray.                                    */
/*  - How to get Voicemeeter Application HWND                                     */
/*  - How to manage all slider gain parameters for OSD                            */
/*                                                                                */
/*--------------------------------------------------------------------------------*/
/*                                                                                */
/*  COMPILATION DIRECTIVES:                                                       */
/*                                                                                */
/*  To compile With Microsoft VC2005 and higher you need to create an empty       */
/*  Win32 project with the following options :                                    */
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

#include "vmr_osd.h"
#include "VoicemeeterRemote.h"


/*****************************************************************************/
/*                            CreateWindowInBand                             */
/*****************************************************************************/
/* Undocumented function to CreateWindow able to be displayed in overlay     */
/* on video game                                                             */
/*****************************************************************************/

typedef HWND(WINAPI* TYPE_CreateWindowInBand)(DWORD dwExStyle, ATOM atom, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam, DWORD band);


static TYPE_CreateWindowInBand G_pCreateWindowInBand = NULL;

long VBTOOL_InBandFnct_Init(void)
{
	HMODULE hhh;
	hhh=GetModuleHandle("user32.dll"); //this DLL is normally already loaded 
	if (hhh != NULL)
	{
		G_pCreateWindowInBand= (TYPE_CreateWindowInBand)GetProcAddress(hhh, "CreateWindowInBand");
	}
	return 0;
}



/*****************************************************************************/
/*                                 Shell Function                            */
/*****************************************************************************/

typedef BOOL (WINAPI *LPT_Shell_NotifyIcon)(DWORD dwMessage, PNOTIFYICONDATA lpdata);

static HMODULE VB_Shell_hModule=NULL;
static LPT_Shell_NotifyIcon VB_Shell_NotifyIcon = NULL;

long VBTOOL_Shell_Init(void)
{
	if (VB_Shell_hModule == NULL)
		VB_Shell_hModule=LoadLibrary("SHELL32.DLL");
	if (VB_Shell_hModule != NULL)
	{
		if (VB_Shell_NotifyIcon == NULL)
			VB_Shell_NotifyIcon=(LPT_Shell_NotifyIcon)GetProcAddress(VB_Shell_hModule,"Shell_NotifyIconA");
	}
	if (VB_Shell_NotifyIcon == NULL) return -1;
	return 0;
}

long VBTOOL_Shell_End(void)
{
	if (VB_Shell_hModule != NULL) FreeLibrary(VB_Shell_hModule);
	VB_Shell_hModule=NULL;
	VB_Shell_NotifyIcon=NULL;
	return 0;
}


long VBTOOL_ShellAddIcon(HWND hw, DWORD uId, HICON hicon, char szTip[64])
{
	BOOL fOk;
	NOTIFYICONDATA icondata;
	if (VB_Shell_NotifyIcon == NULL) return -1;

	memset(&icondata, 0, sizeof(NOTIFYICONDATA));
	icondata.cbSize = sizeof(NOTIFYICONDATA);
	icondata.hWnd = hw;
	icondata.uID = uId;
	icondata.uCallbackMessage = uId;
	icondata.hIcon = hicon;

	icondata.uFlags = NIF_MESSAGE;
	if (hicon != NULL) icondata.uFlags |= NIF_ICON;
	if (szTip != NULL)
	{
		strncpy(icondata.szTip, szTip,64);
		icondata.szTip[63]=0;
		icondata.uFlags |= NIF_TIP; 
	}
	fOk=VB_Shell_NotifyIcon(NIM_ADD, &icondata);
	if (fOk != 0) return 0;
	return -1;
}

long VBTOOL_ShellDelIcon(HWND hw, DWORD uId)
{
	BOOL fOk;
	NOTIFYICONDATA icondata;
	if (VB_Shell_NotifyIcon == NULL) return -1;
	memset(&icondata, 0, sizeof(NOTIFYICONDATA));
	icondata.cbSize = sizeof(NOTIFYICONDATA);
	icondata.hWnd = hw;
	icondata.uID = uId;
	icondata.uCallbackMessage = uId;
	icondata.uFlags = NIF_MESSAGE;
	fOk=VB_Shell_NotifyIcon(NIM_DELETE , &icondata);
	return 0;
}


/*******************************************************************************/
/**                       HOW TO KNOW IF VOICEMEETER IS SHOWN                 **/
/*******************************************************************************/

const char G_VoicemeeterWindowsClass[]="VBCABLE0Voicemeeter0MainWindow0";

static HWND G_hwFound=NULL;

BOOL CALLBACK VBEnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	char sss[512];
	G_hwFound=NULL;
	if (hwnd != NULL)
	{
		GetClassName(hwnd,sss,512);
		if (strcmp(sss,G_VoicemeeterWindowsClass) == 0)
		{
			GetWindowText(hwnd,sss,512);
			if (strncmp("VoiceMeeter",sss,11) == 0)
			{
				G_hwFound = hwnd;
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL IsVoicemeeterOnTopScreen(void)
{
	HWND hw;
	EnumWindows(VBEnumWindowsProc, 0);
	if (G_hwFound != NULL)
	{
		hw = GetForegroundWindow();
		if (G_hwFound == hw) return TRUE;
	}
	return FALSE;
}



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
/*                                Color Resources functions                    */
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
/*                                APPLICATION CONTEXT                          */
/*******************************************************************************/

#define NBPARAM_DISPLAYED 9

typedef struct tagAPP_CONTEXT
{
	HWND		hwnd_MainWindow;
	HINSTANCE	hinstance;
	DWORD		msCounter0;
	DWORD		msCurrent;
	HMENU		systray_menu;

	char **		vbvmr_pBUSName;
	char **		vbvmr_pStripName;

	long	vbvmr_connect;
	long	vbvmr_nbBus;
	long	vbvmr_nbStrip;
	float   vbvmr_strip_level[8];
	float   vbvmr_bus_level[8];

	long	nChangedDisplayed;
	long	IsShown;

	size_t		wTimer;

	HFONT		font;
	
	T_COLORPENBRUSH cpb_bkg;
	T_COLORPENBRUSH cpb_slider;
	T_COLORPENBRUSH cpb_slider_cursor;

} T_APP_CONTEXT, *PT_APP_CONTEXT, *LPT_APP_CONTEXT;

static T_APP_CONTEXT G_MainAppCtx = {NULL, NULL};










/*******************************************************************************/
/*                            Detect Voicemeeter Version                       */
/*******************************************************************************/

long UpdateOurParameters(LPT_APP_CONTEXT lpapp)
{
	char szParam[128];
	float fff;
	long vi,rep, nChange=0;
	
	// update BUS
	for (vi=0;vi<lpapp->vbvmr_nbBus; vi++)
	{
		sprintf(szParam,"Bus[%i].Gain", vi);
		rep=iVMR.VBVMR_GetParameterFloat(szParam, &fff);
		if (rep >= 0)
		{
			if (fff != lpapp->vbvmr_bus_level[vi])
			{
				if (nChange == 0) nChange = 0x00020000 | vi;
			}
			lpapp->vbvmr_bus_level[vi] = fff;
		}
	}
	// update Strip
	for (vi=0;vi<lpapp->vbvmr_nbStrip; vi++)
	{
		sprintf(szParam,"Strip[%i].Gain", vi);
		rep=iVMR.VBVMR_GetParameterFloat(szParam, &fff);
		if (rep >= 0)
		{
			if (fff != lpapp->vbvmr_strip_level[vi])
			{
				if (nChange == 0) nChange = 0x00010000 | vi;
			}
			lpapp->vbvmr_strip_level[vi] = fff;
		}
	}
	//return what level has been changed
	return nChange;
}


static char * G_szBUSNameList_v1[2]={"BUS A", "BUS B"};
static char * G_szBUSNameList_v2[5]={"BUS A1", "BUS A2", "BUS A3", "BUS B1", "BUS B2"};
static char * G_szBUSNameList_v3[8]={"BUS A1", "BUS A2", "BUS A3", "BUS A4", "BUS A5", "BUS B1", "BUS B2", "BUS B3"};

static char * G_szStripNameList_v1[3]={"IN #1", "IN #2", "VIN #1"};
static char * G_szStripNameList_v2[5]={"IN #1", "IN #2", "IN #3", "VIN #1", "VIN #2"};
static char * G_szStripNameList_v3[8]={"IN #1", "IN #2", "IN #3", "IN #4", "IN #5", "VIN #1", "VIN #2", "VIN #3"};

void DetectVoicemeeterType(LPT_APP_CONTEXT lpapp, HWND hw)
{
	long rep,vmType;

	lpapp->vbvmr_nbBus		=0;
	lpapp->vbvmr_nbStrip	=0;
	lpapp->vbvmr_pBUSName	=NULL;
	lpapp->vbvmr_pStripName	=NULL;

	rep=iVMR.VBVMR_GetVoicemeeterType(&vmType);
	if (rep == 0) 
	{
		if (lpapp->vbvmr_connect != vmType)
		{
			lpapp->vbvmr_connect =vmType;
			switch(vmType)
			{
			case 1://Voicemeeter
				lpapp->vbvmr_nbStrip	=3;
				lpapp->vbvmr_nbBus		=2;
				lpapp->vbvmr_pBUSName	=G_szBUSNameList_v1;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v1;
				lpapp->nChangedDisplayed=0;
				break;
			case 2://Voicemeeter Banana
				lpapp->vbvmr_nbStrip	=5;
				lpapp->vbvmr_nbBus		=5;
				lpapp->vbvmr_pBUSName	=G_szBUSNameList_v2;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v2;
				lpapp->nChangedDisplayed=0;
				break;
			case 3://Voicemeeter 8 (Potato)
				lpapp->vbvmr_nbStrip	=8;
				lpapp->vbvmr_nbBus		=8;
				lpapp->vbvmr_pBUSName	=G_szBUSNameList_v3;
				lpapp->vbvmr_pStripName	=G_szStripNameList_v3;
				lpapp->nChangedDisplayed=0;
				break;
			}
			//InvalidateRect(hw,NULL,TRUE);
		}
	}
	UpdateOurParameters(lpapp);
}




/*******************************************************************************/
/*                                  Manage Menu                                */
/*******************************************************************************/

void ShowOSDWindows(LPT_APP_CONTEXT lpapp, HWND hw)
{
	long x0,y0;
	RECT rect;

	// do nothing if already shown
	if (lpapp->IsShown != 0) return;
	
	// get main monitor RECT
	SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)&rect,0);
	
	// define position (left - bottom of the sscreen) 
	x0 = rect.right - UI_WIN_DX -20;
	y0 = rect.bottom - UI_WIN_DY -20;
	// set Windows position and show it.
	SetWindowPos(hw,HWND_TOP,x0,y0,UI_WIN_DX,UI_WIN_DY,SWP_NOSIZE | SWP_SHOWWINDOW);	
	ShowWindow(hw,SW_SHOWNORMAL);
	lpapp->IsShown = 1;
}

void HideOSDWindows(LPT_APP_CONTEXT lpapp, HWND hw)
{
	// do nothing if already hidden
	if (lpapp->IsShown == 0) return;
	ShowWindow(hw,SW_HIDE);
	lpapp->IsShown = 0;
}


void ManageMenu(LPT_APP_CONTEXT lpapp, HWND hw, WPARAM wparam,LPARAM lparam)
{
	long nuCommand,nNotify;
	nuCommand=(long)LOWORD(wparam);
	nNotify  =(long)HIWORD(wparam);
	switch(nuCommand)
	{
		case IDT_SYSTRAY_SHOW:
			lpapp->msCounter0 = lpapp->msCurrent;
			ShowOSDWindows(lpapp, hw);
			return;
		case IDT_SYSTRAY_HIDE:
			HideOSDWindows(lpapp, hw);
			return;

		case IDT_SYSTRAY_CLOSE:
			PostMessage(hw,WM_DESTROY,0,0L); 		
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
	long rep;

	lpapp->hwnd_MainWindow = hw;
	//create font
	memset(&lf,0, sizeof(LOGFONT));
	lf.lfHeight	= 20;
	lf.lfWeight	= 800;
	strcpy(lf.lfFaceName,"Arial");
	lpapp->font =CreateFontIndirect(&lf);

	CreateColorPenBrush(&(lpapp->cpb_bkg), RGB(18,30,40));
	CreateColorPenBrush(&(lpapp->cpb_slider), RGB(44,61,77));
	CreateColorPenBrush(&(lpapp->cpb_slider_cursor), RGB(110,190,150));

	//create system tray menu
	lpapp->systray_menu=CreatePopupMenu();
	AppendMenu(lpapp->systray_menu,MF_STRING,IDT_SYSTRAY_SHOW,"Show VM-OSD");
	AppendMenu(lpapp->systray_menu,MF_STRING,IDT_SYSTRAY_HIDE,"Hide VM-OSD");
	AppendMenu(lpapp->systray_menu,MF_SEPARATOR,0,NULL);
	AppendMenu(lpapp->systray_menu,MF_STRING,IDT_SYSTRAY_CLOSE,"Shut Down VM-OSD");
	AppendMenu(lpapp->systray_menu,MF_STRING,IDT_SYSTRAY_NOTHING,"Exit Menu");

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

	//call this to get first parameters state (if server already launched)
	rep= iVMR.VBVMR_IsParametersDirty();
	if (rep == 0)
	{
		DetectVoicemeeterType(lpapp, hw);
	}
	else lpapp->vbvmr_connect=0;
	return TRUE;
}

BOOL EndSoftware(LPT_APP_CONTEXT lpapp, HWND hw)
{
	if (iVMR.VBVMR_Logout != NULL) iVMR.VBVMR_Logout();
	if (iVMR.VBVMR_AudioCallbackUnregister != NULL) iVMR.VBVMR_AudioCallbackUnregister();

	if (lpapp->font != NULL) DeleteObject(lpapp->font);
	lpapp->font=NULL;
	DestroyColorPenBrush(&(lpapp->cpb_bkg));
	DestroyColorPenBrush(&(lpapp->cpb_slider));
	DestroyColorPenBrush(&(lpapp->cpb_slider_cursor));

	if (lpapp->systray_menu != NULL) DestroyMenu(lpapp->systray_menu);
	lpapp->systray_menu=NULL;
	return TRUE;
}




void DrawAllStuff(LPT_APP_CONTEXT lpapp, HWND hw, HDC dc, long nChange)
{
	HPEN oldpen;
	HBRUSH oldbrush;
	HFONT oldfont;
	float dBValue=0.0f;
	float dBMin =-60.0f;
	float dBMax =+12.0f;
	RECT rect0,rect;
	char *lpc=NULL;
	long x0,y0, yy, dx,dy, nu,fBUS=0;
	if (nChange == 0) nChange = 0x00020000;
	if ((nChange & 0x00020000) != 0) fBUS=1;
	nu = nChange & 0x0000FFFF;
	GetClientRect(hw,&rect0);
	rect=rect0;
	rect0.bottom = rect0.top+40;
	rect.top=rect0.bottom;
	//Strip
	if (fBUS == 0)
	{
		if (lpapp->vbvmr_pStripName != NULL) lpc=lpapp->vbvmr_pStripName[nu];
		dBValue= lpapp->vbvmr_strip_level[nu];
	}
	//BUS
	else
	{
		if (lpapp->vbvmr_pBUSName != NULL) lpc=lpapp->vbvmr_pBUSName[nu];
		dBValue= lpapp->vbvmr_bus_level[nu];
		
	}
	//Display level name
	oldpen=(HPEN)SelectObject(dc,lpapp->cpb_bkg.pen);
	oldbrush=(HBRUSH)SelectObject(dc,lpapp->cpb_bkg.brush);
	oldfont=(HFONT)SelectObject(dc,lpapp->font);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,RGB(200, 200, 200));
	if (lpc != NULL)
	{
		Rectangle(dc, rect0.left,rect0.top,rect0.right,rect0.bottom);
		DrawText(dc, lpc, (int)strlen(lpc), &rect0, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	//display slider
	dx=20;
	dy=rect.bottom-rect.top-40;
	x0=(rect.right-rect.left-dx)>>1;	
	y0=rect.top+20;
	
	SelectObject(dc,GetStockObject(BLACK_PEN));
	SelectObject(dc,lpapp->cpb_slider.brush);
	Rectangle(dc, x0,y0-5,x0+dx,y0+dy+5);

	yy = (long)((dBValue - dBMin) * dy / (dBMax-dBMin));
	if (yy < 0) yy=0;
	if (yy > dy) yy=dy;
	SelectObject(dc,lpapp->cpb_slider_cursor.brush);
	Rectangle(dc, x0,y0+dy-yy-5,x0+dx,y0+dy-yy+5);
	
	//ending
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
	lpapp->nChangedDisplayed=nChange;
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
	POINT po;
	RECT rect;
	HPEN oldpen;
	HBRUSH oldbrush;
	long rep, nChange;
	LPT_APP_CONTEXT lpapp;
	//char sss[256];
	HDC dc;
	PAINTSTRUCT ps;
	lpapp = &G_MainAppCtx;
	switch (msg)
	{


		case WM_CREATE:
			if (InitSoftware(lpapp,hw) == FALSE) return -1;//return -1 here cancel the window creation
			lpapp->wTimer = SetTimer(hw,MYTIMERID, 50,NULL);//20 Hz timer
			lpapp->msCounter0=GetTickCount();
			//Set application in System tray
			VBTOOL_ShellAddIcon(hw, SYSTRAY_ID, LoadIcon(lpapp->hinstance,MAKEINTRESOURCE(100)), "VM-OSD");
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			lpapp->msCounter0 = lpapp->msCurrent;
			GetCursorPos(&po);
			TrackPopupMenu(lpapp->systray_menu,TPM_RIGHTBUTTON,po.x,po.y,0,hw,NULL);
			break;	
		case WM_COMMAND:
			ManageMenu(lpapp,hw,wparam,lparam);
			break;
		case SYSTRAY_ID:
			switch(LOWORD(lparam))
			{
			case WM_LBUTTONDOWN:
				lpapp->msCounter0 = lpapp->msCurrent;
				ShowOSDWindows(lpapp, hw);
				break;
			case WM_RBUTTONDOWN:
				GetCursorPos(&po);
				TrackPopupMenu(lpapp->systray_menu,TPM_RIGHTBUTTON,po.x,po.y,0,hw,NULL);
				break;
			}
			break;
		case WM_TIMER:
			if (wparam == MYTIMERID)
			{
				lpapp->msCurrent = GetTickCount();
				//check if we have parameter change
				rep=iVMR.VBVMR_IsParametersDirty();
				if (rep >= 0)
				{
					//if not already connected we detect Voicemeeter type
					if (lpapp->vbvmr_connect == 0)
					{
						DetectVoicemeeterType(lpapp, hw);
					}
					//if param change
					if (rep == 1)
					{
						nChange = UpdateOurParameters(lpapp);							
						// if the change concerns our parameters
						if (nChange != 0)
						{
							if (IsVoicemeeterOnTopScreen() == FALSE)
							{
								ShowOSDWindows(lpapp, hw);
								lpapp->msCounter0 = lpapp->msCurrent;
							}

							if (lpapp->IsShown != 0)
							{
								// Draw client Area
								dc=GetDC(hw);			
								DrawAllStuff(lpapp,hw, dc, nChange);
								ReleaseDC(hw,dc);
							}
						}
					}
					else //if no change
					{
						// We Hide Windows After 5 second.
						if ((lpapp->msCurrent - lpapp->msCounter0) > 5000)
						{
							HideOSDWindows(lpapp, hw);
						}
					}
				}
				// else we are losing connection to Voicemeeter
				else 
				{
					if (lpapp->vbvmr_connect != 0)
					{
						//Voicemeeter has been shut down
						lpapp->vbvmr_connect	=0;
						lpapp->vbvmr_nbBus		=0;
						lpapp->vbvmr_nbStrip	=0;
						HideOSDWindows(lpapp, hw);
					}
				}
			}
			break;
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			DrawAllStuff(lpapp,hw, dc, lpapp->nChangedDisplayed);
			EndPaint(hw,&ps);
	        break;
		case WM_ERASEBKGND:
			dc=(HDC)wparam;
			GetClientRect(hw,&rect);
			oldpen=(HPEN)SelectObject(dc,lpapp->cpb_bkg.pen);
			oldbrush=(HBRUSH)SelectObject(dc,lpapp->cpb_bkg.brush);
			Rectangle(dc, 0,0,rect.right,rect.bottom);
			SelectObject(dc,oldpen);
			SelectObject(dc,oldbrush);
			return 1;

		case WM_CLOSE:
			PostMessage(hw,WM_DESTROY,0,0L);
			break;
		case WM_DESTROY:
			if (lpapp->wTimer != 0) KillTimer(hw,lpapp->wTimer);
			lpapp->wTimer=0;
			EndSoftware(lpapp,hw);
			
			//Remove From system tray
			VBTOOL_ShellDelIcon(hw,SYSTRAY_ID);
			//quit
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
	ATOM regClassAtom;
	HWND	hh;
	long	style, styleEx;
	MSG		msg;    
	char	szWindowClassName[]="VBAudio0Example0OSD0MainWindowClass";
	char *	title="Sorry";

	WNDCLASS	wc;
	//we first reset context and store the APP Hinstance
	memset(&G_MainAppCtx, 0, sizeof(T_APP_CONTEXT));
	G_MainAppCtx.hinstance=handle_app;

	//here you can make some early initialization and analyze command line if any.
	VBTOOL_Shell_Init();
	VBTOOL_InBandFnct_Init();

	//we define a window class to create a window from this class 
	wc.style		=CS_HREDRAW | CS_VREDRAW;  	  		//property.
	wc.lpfnWndProc=(WNDPROC)MainWindowManageEvent;		//Adresse of our Callback.
	wc.cbClsExtra =0;					  				//Possibility to store some byte inside a class object.
	wc.cbWndExtra =0;                          			//Possibility to store some byte inside a window object.
	wc.hInstance  =handle_app; 	                		//handle of the application hinstance.
	wc.hIcon      =LoadIcon(handle_app, MAKEINTRESOURCE(100));    			//handle of icon displayed in the caption.
	wc.hCursor    =LoadCursor(NULL,IDC_ARROW);			//handle of cursor mouse .
	wc.hbrBackground=(HBRUSH)(COLOR_MENU+1);			//Background color.
	wc.lpszMenuName=NULL;    							//pointer on menu defined in resource.
	wc.lpszClassName=szWindowClassName;       			//pointer on class name.

	//register class.
	regClassAtom = RegisterClass(&wc);
	if (regClassAtom ==0)
	{
		MessageBox(NULL,"Failed to register main class...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	
	
	//to display above all windows
	styleEx = WS_EX_TOPMOST;	
	//windows without caption and border and not visible
	style=WS_POPUP;

	if (G_pCreateWindowInBand == NULL)
	{
	
	hh=CreateWindowEx(styleEx, szWindowClassName,		// address of registered class name.
						 SZPUBLICNAME,					// address of window name string
						 style,							// window style
						 CW_USEDEFAULT,					// horizontal position of window
						 CW_USEDEFAULT,					// vertical position of window
						 UI_WIN_DX,						// window width
						 UI_WIN_DY,						// window height
						 NULL,							// parent handle is NULL since it's a main window.
						 NULL,							// menu name defined in resource (NULL if no menu or already defined in the Class).
						 handle_app,					// handle of application instance
						 NULL); 						// address of window-creation data
	}
	else
	{
	hh=G_pCreateWindowInBand(styleEx, regClassAtom,		// address of registered class name.
						 SZPUBLICNAMEW,					// address of window name string
						 style,							// window style
						 CW_USEDEFAULT,					// horizontal position of window
						 CW_USEDEFAULT,					// vertical position of window
						 UI_WIN_DX,						// window width
						 UI_WIN_DY,						// window height
						 NULL,							// parent handle is NULL since it's a main window.
						 NULL,							// menu name defined in resource (NULL if no menu or already defined in the Class).
						 handle_app,					// handle of application instance
						 NULL,
						 1); 							// InBand 0 = Default / 1= Desktop

	}

	if (hh==NULL)
	{
		MessageBox(NULL,"Failed to create window...",title,MB_APPLMODAL | MB_OK | MB_ICONERROR);
		return 0;
	}
	ShowWindow(hh,SW_HIDE);				//Display the window.
	/*---------------------------------------------------------------------------*/
	/*                             Messages Loop.                                */
	/*---------------------------------------------------------------------------*/
	while (GetMessage(&msg,NULL,0,0))	//Get Message if any.
	{
		TranslateMessage(&msg);			//Translate the virtuel keys event.
		DispatchMessage(&msg);			//DispatchMessage to the related window.
	}

	//here you can make last uninitialization and release
	VBTOOL_Shell_End();
	return (int)(msg.wParam);
}
