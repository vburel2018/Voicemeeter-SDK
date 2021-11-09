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
/*             THIS SOURCE CODE CAN BE USED ONLY IN A PROGRAM USING VOICEMEETER   */
/*             REMOTE API                                                         */
/*                                                                                */
/*--------------------------------------------------------------------------------*/


#ifndef __VMR_STREAMER_H__
#define __VMR_STREAMER_H__

//to prevent to launch the same application twice (single instance app)
#define APP_UNIQUEMUTEX "VB-AudioVMRStreamerViewExample_VoicemeeterCustomGUI" 

//version information (for program)
#define SZPUBLICVERSION		"1.0.0.2"			//displayed version in about box
#define SZPUBLICNAME		"VM-Streamer View (Voicemeeter Custom GUI Example)"	//displayed title in main window 

//Information for Main window 
#define UI_WIN_DX	800
#define UI_WIN_DY	600

//version information (used in resource file)
#define __FILEVERSION__			1,0,0,2
#define __PRODUCTVERSION__		1,0,0,2
#define __SZFILEVERSION__		"1, 0, 0, 2\0"
#define __SZPRODUCTVERSION__	"1, 0, 0, 2\0"
 
	#define __COMMENTS__			"Example of custom GUI mae for Voicemeeter with the Voicemeeter Remote API"
	#define __COMPANYNAME__			"Audio Mechanic & Sound Breeder\0"
	#define __FILEDESCRIPTION__		"Custom GUI for Streamers, Remoting Voicemeeter potato\0"
	#define __INTERNALNAME__		"vmr_streamer"
	#define __LEGALCOPYRIGHT__		"Copyright V.Burel©2015-2021\0"
	#define __ORIGINALFILENAME__	"vmr_streamer.EXE\0"
	#define __PRODUCTNAME__			"vmr_streamer\0"



//definitions for MENU

#define IDM_QUIT				100
#define IDM_ABOUT				101

#define IDM_UPDATE_ALL_CONTROL	200

#define IDMSYS_LINK_DIRECT		300	
#define IDMSYS_LINK_VBAN1		301
#define IDMSYS_LINK_VBAN2		302
#define IDMSYS_LINK_VBAN3		303
#define IDMSYS_LINK_VBAN4		304

#define IDMSYS_VBAN_CONFIG		350


#endif /*__VMR_STREAMER_H__*/


