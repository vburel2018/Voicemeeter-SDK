/**********************************************************************************/
/* VMSCTL                                                                 V.Burel */
/**********************************************************************************/
/* Contains our custom windows control for vmr_streamer project                   */
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
/*             Remote API anyway                                                  */
/*                                                                                */
/*--------------------------------------------------------------------------------*/



#ifndef __VMSCTL_H__
#define __VMSCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

// if compiling for x64 target define this
//#define VB_X64
	

#define VMSCTL_MAX_NBSTRIP	8
#define VMSCTL_MAX_NBBUS	8


#define VMSCTL_CLASSNAME_STRIP	"VMStreamerCtrl_Strip"
#define VMSCTL_CLASSNAME_BUS	"VMStreamerCtrl_BUS"


//Initialize library
long VMSCTL_InitLib(HINSTANCE hinst);
void VMSCTL_EndLib(void);


typedef struct tagVMSCTL_RT_DATA
{
	float dbLevel[8];	
} T_VMSCTL_RT_DATA, *PT_VMSCTL_RT_DATA, *LPT_VMSCTL_RT_DATA;

/*---------------------------------------------------------------*/
/*                          STRIP CTL                            */
/*---------------------------------------------------------------*/
typedef void (__stdcall *VMSCTL_LPT_SBCALLBACK)(void * lpuser,long Ident, long ICtl, float value);


#define VMSCTL_STRIP_ID_MUTE			100
#define VMSCTL_STRIP_ID_NAME			101
#define VMSCTL_STRIP_ID_METER			102
#define VMSCTL_STRIP_ID_GAIN			132
#define VMSCTL_STRIP_ID_ASSIGN			148

typedef struct tagVMSCTL_STRIPPARAM
{
	HFONT	font0;
	HFONT	font1;
	HFONT	font2;
	float	vmin,vmax;
	HBRUSH	bkg0_brush;
	HBRUSH	bkg1_brush;

	HPEN		assignbus_pen;
	COLORREF	assignbus_color;
	COLORREF	name_color;

	HPEN		slider_pen_green;
	HPEN		slider_pen_red;
	HBRUSH		slider_brush_green;
	HBRUSH		slider_brush_red;

	HBRUSH		meter_brush1;
	HBRUSH		meter_brush2;
	HBRUSH		meter_brush3;

	COLORREF	mutered_color;
	HPEN		mutered_pen;
	COLORREF	assigned_color;
	HPEN		assigned_pen;

	VMSCTL_LPT_SBCALLBACK	lpCallback;
	void *						lpuser;
} T_VMSCTL_STRIPPARAM, *PT_VMSCTL_STRIPPARAM, *LPT_VMSCTL_STRIPPARAM;

HWND VMSCTL_CreateSTRIP(HWND hParent, long Ident, long x0,long y0, LPT_VMSCTL_STRIPPARAM lpparam);

long VMSCTL_PositionSTRIP(HWND hw, long x0,long y0, long dx, long dy, long fShow);


typedef struct tagVMSCTL_STRIP_DATA
{
	WCHAR strip_namew[64];
	char  strip_nickname[8];
	float strip_gain[VMSCTL_MAX_NBBUS];
	long  strip_mute;
	char  strip_assignmentbit[VMSCTL_MAX_NBBUS]; //0x10 bus activated / 0x01 bus assigned 
	long  strip_nbBusDisplayed;
	long  strip_fMultiLayer;
	char ** strip_pBUSNameList;
} T_VMSCTL_STRIP_DATA, *PT_VMSCTL_STRIP_DATA, *LPT_VMSCTL_STRIP_DATA;

long VMSCTL_SetDataSTRIP(HWND hw, LPT_VMSCTL_STRIP_DATA pData, long fUpdateAll);
long VMSCTL_ResetDataSTRIP(HWND hw);

long VMSCTL_SetRTDataSTRIP(HWND hw, LPT_VMSCTL_RT_DATA pRTData);

long VMSCTL_GetIndexSTRIP(HWND hw,long nuVisibleSlider);




/*---------------------------------------------------------------*/
/*                            BUS CTL                            */
/*---------------------------------------------------------------*/

#define VMSCTL_BUS_ID_MUTE			200
#define VMSCTL_BUS_ID_NAME			201
#define VMSCTL_BUS_ID_METER			202
#define VMSCTL_BUS_ID_GAIN			203
#define VMSCTL_BUS_ID_MONITOR		204

typedef struct tagVMSCTL_BUSPARAM
{
	HFONT	font0;
	HFONT	font1;
	HFONT	font2;
	float	vmin,vmax;

	HBRUSH		bkg0_brush;
	HBRUSH		bkg1_brush;
	HPEN		assignbus_pen;
	COLORREF	assignbus_color;
	COLORREF	name_color;

	HPEN		slider_pen_green;
	HPEN		slider_pen_red;
	HBRUSH		slider_brush_green;
	HBRUSH		slider_brush_red;

	HBRUSH		meter_brush1;
	HBRUSH		meter_brush2;
	HBRUSH		meter_brush3;

	COLORREF	mutered_color;
	HPEN		mutered_pen;

	HBRUSH		monitor_brush;
	HPEN		monitor_pen;

	VMSCTL_LPT_SBCALLBACK	lpCallback;
	void *						lpuser;
} T_VMSCTL_BUSPARAM, *PT_VMSCTL_BUSPARAM, *LPT_VMSCTL_BUSPARAM;

HWND VMSCTL_CreateBUS(HWND hParent, long Ident, long x0,long y0, LPT_VMSCTL_BUSPARAM lpparam);

long VMSCTL_PositionBUS(HWND hw, long x0,long y0, long dx, long dy, long fShow);


typedef struct tagVMSCTL_BUS_DATA
{
	WCHAR bus_namew[64];
	char  bus_nickname[8];
	float bus_gain;
	long  bus_mute;
	long  bus_monitor;
	long  bus_fMonitorSupport;
} T_VMSCTL_BUS_DATA, *PT_VMSCTL_BUS_DATA, *LPT_VMSCTL_BUS_DATA;

long VMSCTL_SetDataBUS(HWND hw, LPT_VMSCTL_BUS_DATA pData, long fUpdateAll);
long VMSCTL_ResetDataBUS(HWND hw);

long VMSCTL_SetRTDataBUS(HWND hw, LPT_VMSCTL_RT_DATA pRTData);


//End Of Header.

#ifdef __cplusplus 
}
#endif


#endif /*__VMSCTL_H__*/


