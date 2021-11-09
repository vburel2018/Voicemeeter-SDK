/******************************************************************************/
/* Voicemeeter Remote API.                                  V.Burel©2015-2021 */
/******************************************************************************/
/* This Library allows communication with Voicemeeter applications            */
/* 4 Client Applications can be connected to remote Voicemeeter.              */
/* it also allows communication with MacroButtons application.                */
/******************************************************************************/
/*                                                                            */
/*                  OFFICIAL LINK : WWW.VOICEMEETER.COM                       */
/*                                                                            */
/******************************************************************************/
/* COPYRIGHT: Vincent Burel(c)2015-2021 All Rights Reserved                   */
/******************************************************************************/
/*                                                                            */
/*  LICENSING: VoicemeeterRemote.dll usage is driven by a license agreement   */
/*             given in VoicemeeterRemoteAPI.pdf or readme.txt                */
/*                                                                            */
/******************************************************************************/
/* long = 32 bit integer                                                      */
/******************************************************************************/


#ifndef __VOICEMEETER_REMOTE_H__
#define __VOICEMEETER_REMOTE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define VBVMR_RESULT_OK			0

/******************************************************************************/
/*                                                                            */
/*                                    Login                                   */
/*                                                                            */
/******************************************************************************/

/** @name Communication Login / logout
* @{ */

	/** 
	@brief Open Communication Pipe With Voicemeeter (typically called on software startup).
	@return :	 0: OK (no error).
				 1: OK but Voicemeeter Application not launched.
				-1: cannot get client (unexpected)
				-2: unexpected login (logout was expected before).
	*/

long __stdcall VBVMR_Login(void);

	/** 
	@brief Close Communication Pipe With Voicemeeter (typically called on software end).
	@return : 0 if ok.
	*/

long __stdcall VBVMR_Logout(void);


	/** 
	@brief Run Voicemeeter Application (get installation directory and run Voicemeeter Application).
	@param vType : Voicemeeter type  (1 = Voicemeeter, 2= Voicemeeter Banana, 3= Voicemeeter Potato, 6 = Potato x64 bits).
	@return :	 0: Ok.
				-1: not installed (UninstallString not found in registry).
				-2: unknown vType number
	*/

long __stdcall VBVMR_RunVoicemeeter(long vType);


/** @}  */











/******************************************************************************/
/*                                                                            */
/*                             General Information                            */
/*                                                                            */
/******************************************************************************/

/** @name General Information 
* @{ */

	/** 
	@brief Get Voicemeeter Type
	@param pType : Pointer on 32bit long receiving the type (1 = Voicemeeter, 2= Voicemeeter Banana, 3 Potato).

				 VOICEMEETER STRIP/BUS INDEX ASSIGNMENT

				 | Strip 1 | Strip 2 |Virtual Input|  BUS A  |  BUS B  |
				 +---------+---------+-------------+---------+---------+
				 |    0    |    1    |      2      |    0    |    1    |

				 VOICEMEETER BANANA STRIP/BUS INDEX ASSIGNMENT

				 | Strip 1 | Strip 2 | Strip 2 |Virtual Input|Virtual AUX|BUS A1|BUS A2|BUS A3|BUS B1|BUS B2|
				 +---------+---------+---------+-------------+-----------+------+------+------+------+------+
				 |    0    |    1    |    2    |       3     |     4     |   0  |   1  |   2  |   3  |   4  |

				 VOICEMEETER POTATO STRIP/BUS INDEX ASSIGNMENT

				 | Strip 1 | Strip 2 | Strip 2 | Strip 2 | Strip 2 |Virtual Input|Virtual AUX|   VAIO3   |BUS A1|BUS A2|BUS A3|BUS A4|BUS A5|BUS B1|BUS B2|BUS B3|
				 +---------+---------+---------+---------+---------+-------------+-----------+-----------+------+------+------+------+------+------+------+------+
				 |    0    |    1    |    2    |    3    |    4    |      5      |     6     |     7     |   0  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |


	@return :	 0: OK (no error).
				-1: cannot get client (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_GetVoicemeeterType(long * pType);

	/** 
	@brief Get Voicemeeter Version
	@param pType : Pointer on 32bit integer receiving the version (v1.v2.v3.v4)
						v1 = (version & 0xFF000000)>>24;
						v2 = (version & 0x00FF0000)>>16;
						v3 = (version & 0x0000FF00)>>8;
						v4 = version & 0x000000FF;

	@return :	 0: OK (no error).
				-1: cannot get client (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_GetVoicemeeterVersion(long * pVersion);


/** @}  */


















/******************************************************************************/
/*                                                                            */
/*                               Get parameters                               */
/*                                                                            */
/******************************************************************************/

/** @name Getting Parameters
* @{ */

	/** 
	@brief  Check if parameters have changed.
			Call this function periodically (typically every 10 or 20ms).
			(this function must be called from one thread only)
					
	@return:	 0: no new paramters.
				 1: New parameters -> update your display.
				-1: error (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_IsParametersDirty(void);

	/** 
	@brief get parameter value.
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
	@param pValue : Pointer on float (32bit float by reference) receiving the wanted value.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
				-5: structure mismatch
	*/

long __stdcall VBVMR_GetParameterFloat(char * szParamName, float * pValue);

	/** 
	@brief get parameter value.
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
	@param pValue : Pointer on String (512 char or wchar) receiving the wanted value.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
				-5: structure mismatch
	*/

long __stdcall VBVMR_GetParameterStringA(char * szParamName, char * szString);
long __stdcall VBVMR_GetParameterStringW(char * szParamName, unsigned short * wszString);

/** @}  */









/******************************************************************************/
/*                                                                            */
/*                                Get levels                                  */
/*                                                                            */
/******************************************************************************/

/** @name Getting RT Data
* @{ */

	/** 
	@brief Get Current levels.
			(this function must be called from one thread only)

	@param nType:	0= pre fader input levels.
					1= post fader input levels.
					2= post Mute input levels.
					3= output levels.

	@param nuChannel: audio channel zero based index 
						for input 0 = in#1 left, 1= in#1 Right, etc...
						for output 0 = busA ch1, 1 = busA ch2...

					 VOICEMEETER CHANNEL ASSIGNMENT

					 | Strip 1 | Strip 2 |             Virtual Input             |
					 +----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 |

					 |             Output A1 / A2            |             Virtual Output            |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 |

					 VOICEMEETER BANANA CHANNEL ASSIGNMENT

					 | Strip 1 | Strip 2 | Strip 3 |             Virtual Input             |            Virtual Input AUX          |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 |

					 |             Output A1                 |                Output A2              |                Output A3              |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |

					 |            Virtual Output B1          |             Virtual Output B2         |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |

					 VOICEMEETER POTATO CHANNEL ASSIGNMENT

					 | Strip 1 | Strip 2 | Strip 3 | Strip 4 | Strip 5 |             Virtual Input             |            Virtual Input AUX          |                 VAIO3                 |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 25 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 |

					 |             Output A1                 |                Output A2              |                Output A3              |                Output A4              |                Output A5              |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |

					 |            Virtual Output B1          |             Virtual Output B2         |             Virtual Output B3         |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |


	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: no level available
				-4: out of range
	*/

long __stdcall VBVMR_GetLevel(long nType, long nuChannel, float * pValue);


	/** 
	@brief Get MIDI message from M.I.D.I. input device used by Voicemeeter M.I.D.I. mapping.
			(this function must be called from one thread only)

	@param pMIDIBuffer:	pointer on MIDI Buffer. Expected message size is below 4 bytes, 
	                    but it's recommended to use 1024 Bytes local buffer to receive 
						possible multiple M.I.D.I. event message in optimal way: 
						unsigned char pBuffer[1024];


	@return :	>0: number of bytes placed in buffer (2 or 3 byte for usual M.I.D.I. message) 
				-1: error
				-2: no server.
				-5: no MIDI data
				-6: no MIDI data
	*/


long __stdcall VBVMR_GetMidiMessage(unsigned char *pMIDIBuffer, long nbByteMax);


/** @}  */










/******************************************************************************/
/*                                                                            */
/*                               Set Parameters                               */
/*                                                                            */
/******************************************************************************/

/** @name Setting Parameters
* @{ */

	/** 
	@brief Set a single float 32 bits parameters .
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
						example:
						Strip[1].gain
						Strip[0].mute
						Bus[0].gain
						Bus[0].eq.channel[0].cell[0].gain

	@param pValue : float 32bit containing the new value.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
	*/

long __stdcall VBVMR_SetParameterFloat(char * szParamName, float Value);



	/** 
	@brief Set a single string parameters .
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
						example:
						Strip[1].name
						Strip[0].device.mme
						Bus[0].device.asio

	@param szString : zero terminal string.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
	
	*/


long __stdcall VBVMR_SetParameterStringA(char * szParamName, char * szString);
long __stdcall VBVMR_SetParameterStringW(char * szParamName, unsigned short * wszString);




	/** 
	@brief Set one or several parameters by a script ( < 48 kB ).
	@param szParamName : Null Terminal ASCII String giving the script 
						 (script allows to change several parameters in the same time - SYNCHRO).
						 Possible Instuction separators: ',' ';' or '\n'(CR)
						 EXAMPLE:
						 "Strip[0].gain = -6.0
						 Strip[0].A1 = 0
						 Strip[0].B1 = 1
						 Strip[1].gain = -6.0
						 Strip[2].gain = 0.0
						 Strip[3].name = "Skype Caller" "

	@return :	 0: OK (no error).
				>0: number of line causing script error.
				-1: error
				-2: no server.
				-3: unexpected error
				-4: unexpected error
	*/

long __stdcall VBVMR_SetParameters(char * szParamScript);
long __stdcall VBVMR_SetParametersW(unsigned short * szParamScript);


/** @}  */











/******************************************************************************/
/*                            DEVICES ENUMERATOR                              */
/******************************************************************************/


/** @name Device Enumeration Functions
* @{ */

#define VBVMR_DEVTYPE_MME		1
#define VBVMR_DEVTYPE_WDM		3
#define VBVMR_DEVTYPE_KS		4
#define VBVMR_DEVTYPE_ASIO		5

	/** 
	@brief Get number of Audio Output Device available on the system
	@return : return number of device found.
	*/

long __stdcall VBVMR_Output_GetDeviceNumber(void);

	/** 
	@brief Return pointer on Output Device Descriptor according index
	@param zindex : zero based index
	@param nType : Pointer on 32bit long receiving the type (pointer can be NULL).
	@param szName : Pointer on string (256 char min) receiving the device name (pointer can be NULL).
	@param szHardwareId : Pointer on string (256 char min) receiving the hardware ID (pointer can be NULL).
	@return :	 0: OK (no error).
	*/

long __stdcall VBVMR_Output_GetDeviceDescA(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
long __stdcall VBVMR_Output_GetDeviceDescW(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);

	/** 
	@brief Get number of Audio Input Device available on the system
	@return : return number of device found.
	*/

long __stdcall VBVMR_Input_GetDeviceNumber(void);

	/** 
	@brief Return pointer on Input Device Descriptor according index
	@param zindex : zero based index
	@param nType : Pointer on 32bit long receiving the type (pointer can be NULL).
	@param szName : Pointer on string (256 char min) receiving the device name (pointer can be NULL).
	@param szHardwareId : Pointer on string (256 char min) receiving the hardware ID (pointer can be NULL).
	@return :	 0: OK (no error).
	*/

long __stdcall VBVMR_Input_GetDeviceDescA(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
long __stdcall VBVMR_Input_GetDeviceDescW(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);



/** @}  */






/******************************************************************************/
/*                             VB-AUDIO CALLBACK                              */
/******************************************************************************/
/* 4x Functions to process all voicemeeter audio input and output channels    */
/*                                                                            */
/* VBVMR_AudioCallbackRegister	 :to register your audio callback(s)          */
/* VBVMR_AudioCallbackStart	     :to start the audio stream                   */
/* VBVMR_AudioCallbackStop    	 :to stop the audio stream                    */
/* VBVMR_AudioCallbackUnregister :to unregister / Release callback(s)         */
/******************************************************************************/

/** @name VB-Audio Callback Functions
* @{ */


typedef struct tagVBVMR_AUDIOINFO
{
	long samplerate;
	long nbSamplePerFrame;
} VBVMR_T_AUDIOINFO, *VBVMR_PT_AUDIOINFO, *VBVMR_LPT_AUDIOINFO;


typedef struct tagVBVMR_AUDIOBUFFER
{
	long audiobuffer_sr;					//Sampling Rate
	long audiobuffer_nbs;					//number of sample per frame
	long audiobuffer_nbi;					//number of inputs
	long audiobuffer_nbo;					//number of outputs
	float * audiobuffer_r[128];				//nbi input pointers containing frame of nbs sample (of 32bits float)
	float * audiobuffer_w[128];				//nbo output pointers containing frame of nbs sample (of 32bits float)
} VBVMR_T_AUDIOBUFFER, *VBVMR_PT_AUDIOBUFFER, *VBVMR_LPT_AUDIOBUFFER;


	/** 
	@brief VB-AUDIO Callback is called for different task to Initialize, perform and end your process.
	       VB-AUDIO Callback is part of single TIME CRITICAL Thread. 
	       VB-AUDIO Callback is non re-entrant (cannot be called while in process)
	       VB-AUDIO Callback is supposed to be REAL TIME when called to process buffer.
		   (it means that the process has to be performed as fast as possible, waiting cycles are forbidden.
		   do not use O/S synchronization object, even Critical_Section can generate waiting cycle. Do not use 
		   system functions that can generate waiting cycle like display, disk or communication functions for example).
		   
	@param lpUser: User pointer given on callback registration.
	@param ncommand: reason why the callback is called.
	@param lpData: pointer on structure, pending on nCommand.
	@param nnn: additional data, unused

	@return :	 0: always 0 (unused).
	*/


typedef long (__stdcall *T_VBVMR_VBAUDIOCALLBACK)(void * lpUser, long nCommand, void * lpData, long nnn);


#define  VBVMR_CBCOMMAND_STARTING		1	//command to initialize data according SR and buffer size
											//info = (VBVMR_LPT_AUDIOINFO)lpData 

#define  VBVMR_CBCOMMAND_ENDING			2	//command to release data
#define  VBVMR_CBCOMMAND_CHANGE			3	//If change in audio stream, you will have to restart audio 

#define  VBVMR_CBCOMMAND_BUFFER_IN		10	//input insert
#define  VBVMR_CBCOMMAND_BUFFER_OUT		11  //bus output insert
#define  VBVMR_CBCOMMAND_BUFFER_MAIN	20  //all i/o 
											//audiobuffer = (VBVMR_LPT_AUDIOBUFFER)lpData 
											//nnn = synchro = 1 if synchro with Voicemeeter
											
/*                   
 -----------------------------------------------------
    AUDIO BUFFER for VBVMR_CBCOMMAND_BUFFER_IN
 -----------------------------------------------------
		VOICEMEETER 

		 | Strip 1 | Strip 2 |             Virtual Input             |
		 +----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 |

		VOICEMEETER BANANA 

		 | Strip 1 | Strip 2 | Strip 3 |             Virtual Input             |            Virtual Input AUX          |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 |

		VOICEMEETER 8 

		 | Strip 1 | Strip 2 | Strip 3 | Strip 4 | Strip 5 |             Virtual Input             |            Virtual Input AUX          |             Virtual Input 8           |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 |


-----------------------------------------------------
     AUDIO BUFFER for VBVMR_CBCOMMAND_BUFFER_OUT
-----------------------------------------------------
		 VOICEMEETER 

		 |             Output A1 / A2            |             Virtual Output            |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 |

		 VOICEMEETER BANANA 

		 |             Output A1                 |                Output A2              |                Output A3              |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |

		 |            Virtual Output B1          |             Virtual Output B2         |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |


		VOICEMEETER 8 

		 |             Output A1                 |                Output A2              |                Output A3              |                Output A4              |                Output A5              |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |

		 |            Virtual Output B1          |             Virtual Output B2         |             Virtual Output B3         |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |


-----------------------------------------------------
      AUDIO BUFFER for VBVMR_CBCOMMAND_BUFFER_MAIN
-----------------------------------------------------
		 VOICEMEETER 

		 | Strip 1 | Strip 2 |             Virtual Input             |
		 +----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 |

		 |             Output A1 / A2            |             Virtual Output            |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 |
		 Output buffer provides outputs only:
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 |

		 VOICEMEETER BANANA

		 | Strip 1 | Strip 2 | Strip 3 |             Virtual Input             |            Virtual Input AUX          |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 |

		 |             Output A1                 |                Output A2              |                Output A3              |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 |
		 Output buffer provides outputs only:
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |

		 |            Virtual Output B1          |             Virtual Output B2         |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 |
		 Output buffer provides outputs only:
		 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |


		VOICEMEETER 8 

		 | Strip 1 | Strip 2 | Strip 3 | Strip 4 | Strip 5 |             Virtual Input             |            Virtual Input AUX          |             Virtual Input 8           |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 |

		 |             Output A1                 |                Output A2              |                Output A3              |                Output A4              |                Output A5              |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 34 | 35 | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 | 69 | 70 | 71 | 72 | 73 |
		 Output buffer provides outputs only:
		 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |

		 |            Virtual Output B1          |             Virtual Output B2         |             Virtual Output B3         |
		 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
		 | 74 | 75 | 76 | 77 | 78 | 79 | 80 | 81 | 82 | 83 | 84 | 85 | 86 | 87 | 88 | 89 | 90 | 91 | 92 | 93 | 94 | 95 | 96 | 97 |
		 Output buffer provides outputs only:
		 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |


*/

	/** 
	@brief register your audio callback function to receive real time audio buffer
			it's possible to register up to 3x different Audio Callback in the same application or in 
			3x different applications. In the same application, this is possible because Voicemeeter
			provides 3 kind of audio Streams:
				- AUDIO INPUT INSERT (to process all Voicemeeter inputs as insert)	
				- AUDIO OUTPUT INSERT (to process all Voicemeeter BUS outputs as insert)	
				- ALL AUDIO I/O (to process all Voicemeeter i/o).
			Note: a single callback can be used to receive the 3 possible audio streams.

	@param mode : callback type (main, input or bus output) see define below
	@param pCallback : Pointer on your callback function.
	@param lpUser : user pointer (pointer that will be passed in callback first argument).
	@param szClientName[64]: IN: Name of the application registering the Callback.
							 OUT: Name of the application already registered.
	@return :	 0: OK (no error).
				-1: error
				 1: callback already registered (by another application).
	*/

long __stdcall VBVMR_AudioCallbackRegister(long mode, T_VBVMR_VBAUDIOCALLBACK pCallback, void * lpUser, char szClientName[64]);

#define VBVMR_AUDIOCALLBACK_IN		0x00000001	//to process input insert
#define VBVMR_AUDIOCALLBACK_OUT		0x00000002  //to process output bus insert
#define VBVMR_AUDIOCALLBACK_MAIN	0x00000004  //to receive all i/o

	/** 
	@brief	Start / Stop Audio processing 
			
			he Callback will be called with 
	@return :	 0: OK (no error).
				-1: error
				-2: no callback registred.
	*/

long __stdcall VBVMR_AudioCallbackStart(void);
long __stdcall VBVMR_AudioCallbackStop(void);


	/** 
	@brief unregister your callback to release voicemeeter virtual driver
			(this function will automatically call VBVMR_AudioCallbackStop() function)
	@param pCallback : Pointer on your callback function.
	@return :	 0: OK (no error).
				-1: error
				 1: callback already unregistered.
	*/

long __stdcall VBVMR_AudioCallbackUnregister(void);



/** @}  */






/******************************************************************************/
/*                                                                            */
/*                                Macro Buttons                               */
/*                                                                            */
/******************************************************************************/

/** @name Macro Buttons functions
* @{ */

	/** 
	@brief  Check if Macro Buttons states changed.
			Call this function periodically (typically every 50 or 500ms) to know if something happen on MacroButton states .
			(this function must be called from one thread only)
					
	@return:	 0: no new status.
				>0: last nu logical button status changed.
				-1: error (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_MacroButton_IsDirty(void);

	/** 
	@brief get current status of a given button.
	@param nuLogicalButton : button index: 0 to 79)
	@param pValue : Pointer on float (32bit float by reference) receiving the wanted value (0.0 = OFF / 1.0 = ON).
	@param bitmode: define what kind of value you want to read (see MACROBUTTON_MODE below)
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
				-5: structure mismatch
	*/

long __stdcall VBVMR_MacroButton_GetStatus(long nuLogicalButton, float * pValue, long bitmode);

	/** 
	@brief set current button value.
	@param nuLogicalButton : button index: 0 to 79)
	@param fValue : float 32 bit value giving the status (0.0 = OFF / 1.0 = ON).
	@param bitmode: define what kind of value you want to write/modify (see MACROBUTTON_MODE below)
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
				-5: structure mismatch
	*/

long __stdcall VBVMR_MacroButton_SetStatus(long nuLogicalButton, float fValue, long bitmode);

#define VBVMR_MACROBUTTON_MODE_DEFAULT		0x00000000	//PUSH or RELEASE button 
#define VBVMR_MACROBUTTON_MODE_STATEONLY	0x00000002	//change Displayed State only
#define VBVMR_MACROBUTTON_MODE_TRIGGER		0x00000003	//change Trigger State



/** @}  */









/******************************************************************************/
/*                          'C' STRUCTURED INTERFACE                          */
/******************************************************************************/

typedef long (__stdcall *T_VBVMR_Login)(void);
typedef long (__stdcall *T_VBVMR_Logout)(void);
typedef long (__stdcall *T_VBVMR_RunVoicemeeter)(long vType);

typedef long (__stdcall *T_VBVMR_GetVoicemeeterType)(long * pType);
typedef long (__stdcall *T_VBVMR_GetVoicemeeterVersion)(long * pVersion);

typedef long (__stdcall *T_VBVMR_IsParametersDirty)(void);
typedef long (__stdcall *T_VBVMR_GetParameterFloat)(char * szParamName, float * pValue);
typedef long (__stdcall *T_VBVMR_GetParameterStringA)(char * szParamName, char * szString);
typedef long (__stdcall *T_VBVMR_GetParameterStringW)(char * szParamName, unsigned short * wszString);


typedef long (__stdcall *T_VBVMR_GetLevel)(long nType, long nuChannel, float * pValue);
typedef long (__stdcall *T_VBVMR_GetMidiMessage)(unsigned char *pMIDIBuffer, long nbByteMax);

typedef long (__stdcall *T_VBVMR_SetParameterFloat)(char * szParamName, float Value);
typedef long (__stdcall *T_VBVMR_SetParameters)(char * szParamScript);
typedef long (__stdcall *T_VBVMR_SetParametersW)(unsigned short * szParamScript);
typedef long (__stdcall *T_VBVMR_SetParameterStringA)(char * szParamName, char * szString);
typedef long (__stdcall *T_VBVMR_SetParameterStringW)(char * szParamName, unsigned short * wszString);

typedef long (__stdcall *T_VBVMR_Output_GetDeviceNumber)(void);
typedef long (__stdcall *T_VBVMR_Output_GetDeviceDescA)(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
typedef long (__stdcall *T_VBVMR_Output_GetDeviceDescW)(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);
typedef long (__stdcall *T_VBVMR_Input_GetDeviceNumber)(void);
typedef long (__stdcall *T_VBVMR_Input_GetDeviceDescA)(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
typedef long (__stdcall *T_VBVMR_Input_GetDeviceDescW)(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);

typedef long (__stdcall *T_VBVMR_AudioCallbackRegister)(long mode, T_VBVMR_VBAUDIOCALLBACK pCallback, void * lpUser, char szClientName[64]);
typedef long (__stdcall *T_VBVMR_AudioCallbackStart)(void);
typedef long (__stdcall *T_VBVMR_AudioCallbackStop)(void);
typedef long (__stdcall *T_VBVMR_AudioCallbackUnregister)(void);

typedef long (__stdcall *T_VBVMR_MacroButton_IsDirty)(void);
typedef long (__stdcall *T_VBVMR_MacroButton_GetStatus)(long nuLogicalButton, float * pValue, long bitmode);
typedef long (__stdcall *T_VBVMR_MacroButton_SetStatus)(long nuLogicalButton, float fValue, long bitmode);



typedef struct tagVBVMR_INTERFACE
{
	T_VBVMR_Login					VBVMR_Login;
	T_VBVMR_Logout					VBVMR_Logout;
	T_VBVMR_RunVoicemeeter			VBVMR_RunVoicemeeter;
	T_VBVMR_GetVoicemeeterType		VBVMR_GetVoicemeeterType;
	T_VBVMR_GetVoicemeeterVersion	VBVMR_GetVoicemeeterVersion;
	T_VBVMR_IsParametersDirty		VBVMR_IsParametersDirty;
	T_VBVMR_GetParameterFloat		VBVMR_GetParameterFloat;
	T_VBVMR_GetParameterStringA		VBVMR_GetParameterStringA;
	T_VBVMR_GetParameterStringW		VBVMR_GetParameterStringW;

	T_VBVMR_GetLevel				VBVMR_GetLevel;
	T_VBVMR_GetMidiMessage			VBVMR_GetMidiMessage;

	T_VBVMR_SetParameterFloat		VBVMR_SetParameterFloat;
	T_VBVMR_SetParameters			VBVMR_SetParameters;
	T_VBVMR_SetParametersW			VBVMR_SetParametersW;
	T_VBVMR_SetParameterStringA		VBVMR_SetParameterStringA;
	T_VBVMR_SetParameterStringW		VBVMR_SetParameterStringW;

	T_VBVMR_Output_GetDeviceNumber	VBVMR_Output_GetDeviceNumber;
	T_VBVMR_Output_GetDeviceDescA	VBVMR_Output_GetDeviceDescA;
	T_VBVMR_Output_GetDeviceDescW	VBVMR_Output_GetDeviceDescW;
	T_VBVMR_Input_GetDeviceNumber	VBVMR_Input_GetDeviceNumber;
	T_VBVMR_Input_GetDeviceDescA	VBVMR_Input_GetDeviceDescA;
	T_VBVMR_Input_GetDeviceDescW	VBVMR_Input_GetDeviceDescW;

	T_VBVMR_AudioCallbackRegister	VBVMR_AudioCallbackRegister;
	T_VBVMR_AudioCallbackStart		VBVMR_AudioCallbackStart;
	T_VBVMR_AudioCallbackStop		VBVMR_AudioCallbackStop;
	T_VBVMR_AudioCallbackUnregister	VBVMR_AudioCallbackUnregister;

	T_VBVMR_MacroButton_IsDirty		VBVMR_MacroButton_IsDirty;
	T_VBVMR_MacroButton_GetStatus	VBVMR_MacroButton_GetStatus;
	T_VBVMR_MacroButton_SetStatus	VBVMR_MacroButton_SetStatus;

} T_VBVMR_INTERFACE, *PT_VBVMR_INTERFACE, *LPT_VBVMR_INTERFACE;

#ifdef VBUSE_LOCALLIB
	// internal used (not public)
	void __stdcall VBVMR_SetHinstance(HINSTANCE hinst);
#endif




/******************************************************************************/
/*                                VBAN RT PACKET                              */
/******************************************************************************/

#pragma pack(1)

// COMPATIBILITY: defined structure cannot be changed.
// some field could be added at the end of the structure to keep the compatibility in the time.  

typedef struct tagVBAN_VMRT_PACKET
{
	unsigned char voicemeeterType;			// 1 = Voicemeeter, 2= Voicemeeter Banana, 3 Potato
	unsigned char reserved;					// unused
	unsigned short buffersize;				// main stream buffer size
	unsigned long voicemeeterVersion;		// version like for VBVMR_GetVoicemeeterVersion() functino
	unsigned long optionBits;				// unused
	unsigned long samplerate;				// main stream samplerate
	short inputLeveldB100[34];				// pre fader input peak level in dB * 100
	short outputLeveldB100[64];				// bus output peak level in dB * 100
	unsigned long TransportBit;				// Transport Status
	unsigned long stripState[8];			// Strip Buttons Status (see MODE bits below)
	unsigned long busState[8];				// Bus Buttons Status (see MODE bits below)
	short stripGaindB100Layer1[8];			// Strip Gain in dB * 100 
	short stripGaindB100Layer2[8];
	short stripGaindB100Layer3[8];
	short stripGaindB100Layer4[8];
	short stripGaindB100Layer5[8];
	short stripGaindB100Layer6[8];
	short stripGaindB100Layer7[8];
	short stripGaindB100Layer8[8];
	short busGaindB100[8];					// Bus Gain in dB * 100 
	char stripLabelUTF8c60[8][60];			// Strip Label
	char busLabelUTF8c60[8][60];			// Bus Label
} T_VBAN_VMRT_PACKET, *PT_VBAN_VMRT_PACKET, *LPT_VBAN_VMRT_PACKET;

#define expected_size_T_VBAN_VMRT_PACKET  1384 //1436 max

#pragma pack()

#define VMRTSTATE_MODE_MUTE			0x00000001
#define VMRTSTATE_MODE_SOLO			0x00000002
#define VMRTSTATE_MODE_MONO			0x00000004
#define VMRTSTATE_MODE_MUTEC		0x00000008

#define VMRTSTATE_MODE_MIXDOWN		0x00000010
#define VMRTSTATE_MODE_REPEAT		0x00000020
#define VMRTSTATE_MODE_MIXDOWNB		0x00000030
#define VMRTSTATE_MODE_COMPOSITE	0x00000040
#define VMRTSTATE_MODE_UPMIXTV		0x00000050
#define VMRTSTATE_MODE_UPMIX2		0x00000060
#define VMRTSTATE_MODE_UPMIX4		0x00000070
#define VMRTSTATE_MODE_UPMIX6		0x00000080
#define VMRTSTATE_MODE_CENTER		0x00000090
#define VMRTSTATE_MODE_LFE			0x000000A0
#define VMRTSTATE_MODE_REAR			0x000000B0

#define VMRTSTATE_MODE_MASK			0x000000F0

#define VMRTSTATE_MODE_EQ			0x00000100
#define VMRTSTATE_MODE_CROSS		0x00000200
#define VMRTSTATE_MODE_EQB			0x00000800

#define VMRTSTATE_MODE_BUSA			0x00001000
#define VMRTSTATE_MODE_BUSA1		0x00001000
#define VMRTSTATE_MODE_BUSA2		0x00002000
#define VMRTSTATE_MODE_BUSA3		0x00004000
#define VMRTSTATE_MODE_BUSA4		0x00008000
#define VMRTSTATE_MODE_BUSA5		0x00080000

#define VMRTSTATE_MODE_BUSB			0x00010000
#define VMRTSTATE_MODE_BUSB1		0x00010000
#define VMRTSTATE_MODE_BUSB2		0x00020000
#define VMRTSTATE_MODE_BUSB3		0x00040000

#define VMRTSTATE_MODE_PAN0			0x00000000
#define VMRTSTATE_MODE_PANCOLOR		0x00100000
#define VMRTSTATE_MODE_PANMOD		0x00200000
#define VMRTSTATE_MODE_PANMASK		0x00F00000

#define VMRTSTATE_MODE_POSTFX_R		0x01000000
#define VMRTSTATE_MODE_POSTFX_D		0x02000000
#define VMRTSTATE_MODE_POSTFX1		0x04000000
#define VMRTSTATE_MODE_POSTFX2		0x08000000

#define VMRTSTATE_MODE_SEL			0x10000000
#define VMRTSTATE_MODE_MONITOR		0x20000000





/******************************************************************************/
/*                               LOCAL FUNCTIONS                              */
/******************************************************************************/

long VBVMR_LocalInit(void);
long VBVMR_LocalEnd(void);
void * VBVMR_GetRequestVB0STREAMPTR(void);

long VBVMR_SetParametersWEx(unsigned short * szParamScript, long fCopyToClient);

long VBVMR_LoginEx(long properties);

long VBVMR_MB_PushSettings(void * lpParam);





#ifdef __cplusplus
}
#endif

#endif /*__VOICEMEETER_REMOTE_H__*/


