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




#define VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE //to include audio callback mechanism example

#define VMR_INCLUDE_MACROBUTTONS_REMOTING	//to include function related to MacroButtons 


#ifndef __MIN_VMRCLIENT_H__
#define __MIN_VMRCLIENT_H__






//version information (for program)
#define SZPUBLICVERSION		"1.0.1.0"			//displayed version in about box
#define SZPUBLICNAME		"Remote Voicemeeter - Minimal Program Example"	//displayed title in main window 

//Information for Main window 
#define UI_WIN_DX	1024
#define UI_WIN_DY	640

//version information (used in resource file)
#define __FILEVERSION__			1,0,1,0
#define __PRODUCTVERSION__		1,0,1,0
#define __SZFILEVERSION__		"1, 0, 1, 0\0"
#define __SZPRODUCTVERSION__	"1, 0, 1, 0\0"
 
	#define __COMMENTS__			"Example of source code"
	#define __COMPANYNAME__			"Audio Mechanic & Sound Breeder\0"
	#define __FILEDESCRIPTION__		"Minimal Windows Application showing how to use VoicemeeterRemote.dll\0"
	#define __INTERNALNAME__		"vmrClient"
	#define __LEGALCOPYRIGHT__		"Copyright V.Burel©2015-2021\0"
	#define __ORIGINALFILENAME__	"vmrClient.EXE\0"
	#define __PRODUCTNAME__			"vmrClient\0"


//definitions for MENU

#define IDM_LOAD			50
#define IDM_SERVERINFO		60

#define IDM_RESTART			80

#define IDM_QUIT			100
#define IDM_ABOUT			101

#define IDM_TEST_1			200
#define IDM_TEST_2			201
#define IDM_TEST_3			202

#define IDM_MTEST_1			210
#define IDM_MTEST_2			211
#define IDM_MTEST_3			212

#define IDM_COMMAND_REC_STOP	220
#define IDM_COMMAND_REC_PLAY	221
#define IDM_COMMAND_REC_REC		222
#define IDM_COMMAND_REC_FF		223
#define IDM_COMMAND_REC_REW		224
#define IDM_COMMAND_REC_GOTO	225	
#define IDM_COMMAND_REC_GOTO1	226
#define IDM_COMMAND_REC_GOTO2	227
#define IDM_COMMAND_REC_RESTART	228
#define IDM_COMMAND_REC_PAUSE	229

#define IDM_COMMAND_SHUTDOWN	250
#define IDM_COMMAND_SHOW		251
#define IDM_COMMAND_HIDE		252
#define IDM_COMMAND_RESTART		253
#define IDM_COMMAND_EJECT		254
#define IDM_COMMAND_RESET		255

#define IDM_COMMAND_VOLAPP_1	260
#define IDM_COMMAND_VOLAPP_2	261
#define IDM_COMMAND_VOLAPP_3	262
#define IDM_COMMAND_VOLAPP_4	263
#define IDM_COMMAND_VOLAPP_5	264
#define IDM_COMMAND_VOLAPP_6	265
#define IDM_COMMAND_VOLAPP_7	266
#define IDM_COMMAND_VOLAPP_8	267
#define IDM_COMMAND_VOLAPP_9	268

#define IDM_COMMAND_ACB_REGISTER	300
#define IDM_COMMAND_ACB_UNREGISTER	301
#define IDM_COMMAND_ACB_START		302
#define IDM_COMMAND_ACB_STOP		303

#define IDM_SETPARAMETERW1	400
#define IDM_SETPARAMETERW2	401
#define IDM_SETPARAMETERW3	402
#define IDM_SETPARAMETERW4	403

#define IDM_LABEL_STRIP3	405	
#define IDM_LABEL_STRIP4	406
#define IDM_LABEL_BUS5		407
#define IDM_LABEL_BUS6		408

#define IDM_REMOVEDEVICE_IN1	410
#define IDM_REMOVEDEVICE_OUT1	411


#define IDC_EDITBOX			500

#define IDM_DEVICE_IN1		1024
#define IDM_DEVICE_OUT1		2048


#endif /*__MIN_VMRCLIENT_H__*/


