/*--------------------------------------------------------------------------------*/
/* APA example: 8x8 Gain Matrix Audio processing As Application                   */
/*--------------------------------------------------------------------------------*/
/* 'C' Sample Code to make a Voicemeeter APO                 V.Burel (c)2016-2021 */
/*                                                                                */
/*  THIS PROGRAM PROCESSES A 8x8 GAIN MATRIX ON SINGLE BUS (SELECTABLE)           */
/*                                                                                */
/*  This program example shows                                                    */
/*  - How to link VoicemeeterRemote.dll                                           */
/*  - How to Login / logout                                                       */
/*  - How to install Audio Callback as output insert.                             */
/*                                                                                */
/*  THIS PROGRAM PROCESSES A 8x8 GAIN MATRIX ON SINGLE BUS (SELECTABLE)           */
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


#ifndef __MIN_VMRMATRIX_H__
#define __MIN_VMRMATRIX_H__

//to prevent to launch the same application twice (single instance app)
#define APP_UNIQUEMUTEX "VB-Audio8x8MatrixExampleVoicemeeterAPO" 

//version information (for program)
#define SZPUBLICVERSION		"1.0.0.4"			//displayed version in about box
#define SZPUBLICNAME		"VB-Audio 8x8 Matrix - Voicemeeter APO Example -"	//displayed title in main window 

//Information for Main window 
#define UI_WIN_DX	800
#define UI_WIN_DY	600

//version information (used in resource file)
#define __FILEVERSION__			1,0,0,4
#define __PRODUCTVERSION__		1,0,0,4
#define __SZFILEVERSION__		"1, 0, 0, 4\0"
#define __SZPRODUCTVERSION__	"1, 0, 0, 4\0"
 
	#define __COMMENTS__			"Example of source code to process 8x8 matrix output"
	#define __COMPANYNAME__			"Audio Mechanic & Sound Breeder\0"
	#define __FILEDESCRIPTION__		"Minimal Windows Application showing how to use VoicemeeterRemote.dll\0"
	#define __INTERNALNAME__		"vmr_matrix"
	#define __LEGALCOPYRIGHT__		"Copyright V.Burel©2015-2021\0"
	#define __ORIGINALFILENAME__	"vmrmatrix.EXE\0"
	#define __PRODUCTNAME__			"vmrmatrix\0"



//definitions for MENU

#define IDM_QUIT			100
#define IDM_ABOUT			101

#define IDM_RESTART			200

#endif /*__MIN_VMRMATRIX_H__*/


