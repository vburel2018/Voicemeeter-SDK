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



#ifndef __MIN_VMROSD_H__
#define __MIN_VMROSD_H__




//version information (for program)
#define SZPUBLICVERSION		"1.0.0.1"			//displayed version in about box
#define SZPUBLICNAME		"Remote Voicemeeter - Minimal OSD"	//displayed title in main window 
#define SZPUBLICNAMEW		L"Remote Voicemeeter - Minimal OSD"	//displayed title in main window 


//Information for Main window 
#define UI_WIN_DX	100
#define UI_WIN_DY	300

//version information (used in resource file)
#define __FILEVERSION__			1,0,0,1
#define __PRODUCTVERSION__		1,0,0,1
#define __SZFILEVERSION__		"1, 0, 0, 1\0"
#define __SZPRODUCTVERSION__	"1, 0, 0, 1\0"
 
	#define __COMMENTS__			"Example of OSD source code"
	#define __COMPANYNAME__			"Audio Mechanic & Sound Breeder\0"
	#define __FILEDESCRIPTION__		"Minimal Windows Application showing how to use VoicemeeterRemote.dll\0"
	#define __INTERNALNAME__		"vmrOSD"
	#define __LEGALCOPYRIGHT__		"Copyright V.Burel©2015-2021\0"
	#define __ORIGINALFILENAME__	"vmrOSD.EXE\0"
	#define __PRODUCTNAME__			"vmrOSD\0"


//definitions for MENU

#define SYSTRAY_ID			26747


#define IDT_SYSTRAY_CLOSE		70
#define IDT_SYSTRAY_SHOW		71
#define IDT_SYSTRAY_HIDE		72
#define IDT_SYSTRAY_NOTHING		73


#endif /*__MIN_VMROSD_H__*/


