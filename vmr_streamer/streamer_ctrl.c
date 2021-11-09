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
#include "streamer_ctrl.h"

static HINSTANCE G_hinstance=NULL;


#define PEAKMETER_RELEASE_DECREMENT 1.0f

/*---------------------------------------------------------------*/
/*                             TOOL                              */
/*---------------------------------------------------------------*/

void TOOL_StorePointerInWindow(HWND hw,LPVOID pp)
{
#ifdef VB_X64
	SetWindowLongPtr(hw,0,(LONG_PTR)pp);
#else
	SetWindowLong(hw,0,(ULONG)pp);
#endif
}

LPVOID TOOL_RecallPointerFromWindow(HWND hw)
{
#ifdef VB_X64
	return (LPVOID)GetWindowLongPtr(hw,0);
#else
	return (LPVOID)GetWindowLong(hw,0);
#endif
}

















/*---------------------------------------------------------------*/
/*                        STRIP CONTROL                          */
/*---------------------------------------------------------------*/

#define VMSCTL_STRIPBUS_MARGIN			5
#define VMSCTL_STRIP_DY_BUTTON			20
#define VMSCTL_STRIP_DY_ASSIGN			30

#define VMSCTL_STRIP_DX_NICKNAME		40
#define VMSCTL_STRIP_DX_ASSIGN			30

#define VMSCTL_STRIP_DXMIN_PEAKMETER	100
#define VMSCTL_STRIP_DYMIN_SLIDER		100


typedef struct tagVMSCTL_STRIPCTX
{
	HWND	hw;
	long	Ident;
	long	x0,y0,dx,dy;
	long	isShown;

	RECT	rect_nickname;
	RECT	rect_name;
	RECT	rect_meters;
	RECT	rect_slider[VMSCTL_MAX_NBBUS];
	RECT	rect_assign[VMSCTL_MAX_NBBUS];

	long	dx_meter;
	long	dy_meter;
	long	dx_slider;
	long	dx_name;
	long	margin_slider;
	long	dy_slider;
	HBITMAP	tempbmp;
//	long	SliderLinkMode;

	float	start_value;
	HWND	LastCapture;
	long	mouse_y0;
	long	last_y0;
	BOOL	mouseCapture;
	long	ctlRunning;
	long	iStripRunning;

	T_VMSCTL_STRIPPARAM param;
	T_VMSCTL_STRIP_DATA data;
	T_VMSCTL_RT_DATA rtdata;
} T_VMSCTL_STRIPCTX, *PT_VMSCTL_STRIPCTX, *LPT_VMSCTL_STRIPCTX;




static void DrawSTRIP_NickNameMute(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	char sss[64];
	RECT rect;
	HFONT oldfont;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	rect.left=VMSCTL_STRIPBUS_MARGIN;
	rect.top=VMSCTL_STRIPBUS_MARGIN;
	rect.right=rect.left+VMSCTL_STRIP_DX_NICKNAME;
	rect.bottom=rect.top+VMSCTL_STRIP_DY_BUTTON;
	lpobject->rect_nickname=rect;
	
	oldpen = (HPEN)SelectObject(dc,lpobject->param.assignbus_pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font0);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,lpobject->param.assignbus_color);

	if (lpobject->data.strip_mute != 0)
	{
		SetTextColor(dc,lpobject->param.mutered_color);
		SelectObject(dc,lpobject->param.mutered_pen);		
	}

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 5,5);
	strcpy(sss,lpobject->data.strip_nickname);
	DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);

}

static void DrawSTRIP_Name(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	RECT rect;
	HFONT oldfont;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	rect.left=VMSCTL_STRIPBUS_MARGIN+VMSCTL_STRIP_DX_NICKNAME+10;
	rect.top=VMSCTL_STRIPBUS_MARGIN;
	rect.right=rect.left+lpobject->dx_name;
	rect.bottom=rect.top+VMSCTL_STRIP_DY_BUTTON;
	lpobject->rect_name=rect;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font2);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,lpobject->param.name_color);

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 5,5);
	DrawTextW(dc,lpobject->data.strip_namew,(long)wcslen(lpobject->data.strip_namew),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}


static void DrawSTRIP_PeakMetersBkg(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	long dx;
	RECT rect;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	dx = lpobject->dx_meter;
	rect.left=VMSCTL_STRIPBUS_MARGIN;
	rect.top=VMSCTL_STRIPBUS_MARGIN+VMSCTL_STRIP_DY_BUTTON+10;
	rect.right=rect.left+dx;
	rect.bottom=rect.top+lpobject->dy_meter;
	lpobject->rect_meters=rect;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(BLACK_BRUSH));

	Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

static void DrawSTRIP_PeakMeters(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	LPT_VMSCTL_STRIPPARAM lpp;
	float dBValue, dBmin=-80.0f, dBmax=+12.0f;
	long nu,nbMeter=2;
	long dx,dy,xx,x1,xgreen,xred;
	RECT rect;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	dx = lpobject->dx_meter-6;
	rect.left=VMSCTL_STRIPBUS_MARGIN;
	rect.top=VMSCTL_STRIPBUS_MARGIN+VMSCTL_STRIP_DY_BUTTON+10;
	rect.bottom=rect.top+lpobject->dy_meter;
	dy = (rect.bottom-rect.top-4) / nbMeter;
	
	rect.left+=3;
	rect.top+=3;
	rect.right=rect.left+dx;
	rect.bottom = rect.top+dy;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(BLACK_BRUSH));
	xgreen = (long)(((-24.0f - dBmin) * dx)/ (dBmax - dBmin));
	xred = (long)(((0.0f - dBmin) * dx)/ (dBmax - dBmin));
	lpp = &(lpobject->param);

	for (nu=0;nu<nbMeter; nu++)
	{
		dBValue = lpobject->rtdata.dbLevel[nu];
		xx = (long)(((dBValue - dBmin) * dx)/ (dBmax - dBmin));
		if (xx < 0) xx=0;
		if (xx > dx) xx=dx;
		
		// blue
		if (xx > 0)
		{
			SelectObject(dc,lpp->meter_brush1);
			x1=xx;
			if (x1 > xgreen) x1=xgreen;
			Rectangle(dc,rect.left,rect.top,rect.left+x1+1,rect.bottom);
		}
		//green
		if (xx >= xgreen)
		{
			SelectObject(dc,lpp->meter_brush2);
			x1=xx;
			if (x1 > xred) x1=xred;
			Rectangle(dc,rect.left+xgreen,rect.top,rect.left+x1+1,rect.bottom);
		}
		//red
		if (xx >= xred)
		{
			SelectObject(dc,lpp->meter_brush3);
			x1=xx;
			Rectangle(dc,rect.left+xred,rect.top,rect.left+x1+1,rect.bottom);
		}
		//black
		if (xx <= dx)
		{
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			Rectangle(dc,rect.left+xx,rect.top,rect.right+1,rect.bottom);
		}

		rect.top +=dy;
		rect.bottom +=dy;
	}
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

static void DrawSTRIP_BUSAssignmentButton(LPT_VMSCTL_STRIPCTX lpobject, HDC dc, long nuVisibleSlider, long fAssigned, char * szBUSName)
{
	long x0,y0, dx,dy;
	long cursor_height=20;
	RECT rect;
	HPEN oldpen;
	HBRUSH oldbrush;
	HFONT oldfont;
	if (dc == NULL) return;
	//compute local coordintate
	dx = lpobject->dx_slider;
	dy = VMSCTL_STRIP_DY_ASSIGN;
	x0=VMSCTL_STRIPBUS_MARGIN + lpobject->margin_slider+ (lpobject->dx_slider*nuVisibleSlider); 
	//y0=lpobject->dy - VMSCTL_STRIPBUS_MARGIN- VMSCTL_STRIP_DY_ASSIGN;
	
	y0=VMSCTL_STRIPBUS_MARGIN+ VMSCTL_STRIP_DY_BUTTON +10 +lpobject->dy_meter+10+lpobject->dy_slider+10;

	dx = dx -4;
	dy = dy -6;

	rect.left=x0+2;
	rect.top=y0+3;
	rect.right=rect.left+dx;
	rect.bottom=rect.top+dy;
	lpobject->rect_assign[nuVisibleSlider]=rect;
	
	
	oldpen = (HPEN)SelectObject(dc,lpobject->param.assignbus_pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font1);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,lpobject->param.assignbus_color);

	if (fAssigned != 0)
	{
		SetTextColor(dc,lpobject->param.assigned_color);
		SelectObject(dc,lpobject->param.assigned_pen);		
		if (dx >= 40) SelectObject(dc,lpobject->param.font2);
	}

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 7,7);
	DrawText(dc,szBUSName,(long)strlen(szBUSName),&rect,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);


}

static void DrawSTRIP_BUSAssignment(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	LPT_VMSCTL_STRIP_DATA lpdata;
	long vi, nuVisibleSlider;
	nuVisibleSlider=0;
	lpdata = &(lpobject->data);
	if (lpdata->strip_pBUSNameList == NULL) return;
	for (vi=0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		if ((lpdata->strip_assignmentbit[vi] & 0x10) != 0)
		{
			DrawSTRIP_BUSAssignmentButton(lpobject, dc, nuVisibleSlider, (long)(lpdata->strip_assignmentbit[vi] & 0x0F), lpdata->strip_pBUSNameList[vi]);			
			nuVisibleSlider++;
		}
	}
}

static void DrawSTRIP_Slider(LPT_VMSCTL_STRIPCTX lpobject, HDC dc, long nuVisibleSlider, float dBValue)
{
	LPT_VMSCTL_STRIPPARAM lpp;
	HDC dcmem;
	char sss[64];
	long x0,y0, yy, dx,dy,x_middle;
	long cursor_height=20;
	RECT rect,rect2;
	HPEN oldpen, sliderpen;
	HBRUSH oldbrush, sliderbrush;
	HFONT oldfont;
	HBITMAP oldbmp;
	if (dc == NULL) return;

	//compute local coordintate
	dx = lpobject->dx_slider;
	dy = lpobject->dy_slider;
	x0=VMSCTL_STRIPBUS_MARGIN + lpobject->margin_slider+ (lpobject->dx_slider*nuVisibleSlider); 
	y0=VMSCTL_STRIPBUS_MARGIN + VMSCTL_STRIP_DY_BUTTON +10 +lpobject->dy_meter+10;
	
	dx = dx -4;

	rect.left=x0;
	rect.top=y0;
	rect.right=rect.left+dx;
	rect.bottom=rect.top+dy;
	lpobject->rect_slider[nuVisibleSlider]=rect;

	rect.left=0;
	rect.top=0;
	rect.right=rect.left+dx;
	rect.bottom=rect.top+dy;

	x_middle = (rect.left+rect.right)>>1;
	lpp = &(lpobject->param);
	//define slider color according level
	if (dBValue > 0.0f)
	{
		sliderpen = lpp->slider_pen_red;
		sliderbrush = lpp->slider_brush_red;
	}
	else
	{
		sliderpen = lpp->slider_pen_green;
		sliderbrush = lpp->slider_brush_green;
	}
	//compute slider horizontal position
	if (dBValue > lpp->vmax) dBValue=lpp->vmax;
	if (dBValue < lpp->vmin) dBValue=lpp->vmin;
	yy = (long)(((dBValue - lpp->vmin) * (dy - cursor_height))/ (lpp->vmax - lpp->vmin));
	if (yy < 0) yy=0;
	if (yy > dy) yy=dy;

	//we don't draw directly in screen.
	//to avoid flickering we first build a bitmap in memory
	dcmem=CreateCompatibleDC(dc);
	if (lpobject->tempbmp ==NULL) lpobject->tempbmp=CreateCompatibleBitmap(dc,dx,dy);
	oldbmp=(HBITMAP)SelectObject(dcmem,lpobject->tempbmp);
	//Draw background 
	oldpen = (HPEN)SelectObject(dcmem,GetStockObject(NULL_PEN));
	oldbrush = (HBRUSH)SelectObject(dcmem,lpp->bkg1_brush);
	oldfont = (HFONT)SelectObject(dcmem,lpobject->param.font0);
	SetBkMode(dcmem,TRANSPARENT);
	SetTextColor(dcmem,RGB(0,0,0));

	//draw background
	Rectangle(dcmem,rect.left,rect.top,rect.right+1,rect.bottom+1);
	SelectObject(dcmem,sliderpen);
	MoveToEx(dcmem,x_middle, rect.top, NULL);
	LineTo(dcmem,x_middle, rect.bottom);

	//draw cursor
	SelectObject(dcmem,sliderpen);
	SelectObject(dcmem,sliderbrush);
	RoundRect(dcmem,rect.left, rect.bottom-yy-cursor_height,rect.right, rect.bottom-yy-1,7,7);
	rect2=rect;
	rect2.top =rect2.bottom-yy-cursor_height;
	rect2.bottom = rect2.top+cursor_height;
	sprintf(sss,"%0.1f", dBValue);
	DrawText(dcmem,sss,(long)strlen(sss),&rect2,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dcmem,oldpen);
	SelectObject(dcmem,oldbrush);
	SelectObject(dcmem,oldfont);
	//display bitmap on screen at the end.
	BitBlt(dc,x0,y0,dx,dy,dcmem,0,0,SRCCOPY);
	SelectObject(dcmem,oldbmp);
	DeleteDC(dcmem);

}

static void DrawSTRIP_AllSliders(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	LPT_VMSCTL_STRIP_DATA lpdata;
	long vi, nuVisibleSlider;
	nuVisibleSlider=0;
	lpdata = &(lpobject->data);
	for (vi=0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		if ((lpdata->strip_assignmentbit[vi] & 0x10) != 0)
		{
			DrawSTRIP_Slider(lpobject, dc, nuVisibleSlider, lpdata->strip_gain[vi]);			
			nuVisibleSlider++;
		}
	}
}

static void ComputeStripDisplayData(LPT_VMSCTL_STRIPCTX lpobject)
{
	long ddx, dx, dy, nbBus;
	dx= lpobject->dx - VMSCTL_STRIPBUS_MARGIN -VMSCTL_STRIPBUS_MARGIN;
	dy= lpobject->dy - VMSCTL_STRIPBUS_MARGIN -VMSCTL_STRIPBUS_MARGIN;
	if (dx < 80) dx =80;
	if (dy < 150) dy =150;
	lpobject->dx_meter = dx;
	lpobject->dy_meter = 20;
	lpobject->dy_slider = dy - VMSCTL_STRIP_DY_BUTTON -10 -lpobject->dy_meter - 10 - VMSCTL_STRIP_DY_ASSIGN-10;
	
	lpobject->dx_name = dx-VMSCTL_STRIP_DX_NICKNAME-10;

	// Bus number gives the slider number
	nbBus=1;
	if (lpobject->data.strip_nbBusDisplayed > nbBus) nbBus=lpobject->data.strip_nbBusDisplayed;
	
	// compute slider size according slider number.
	ddx = dx / (nbBus+1); // we add a margin
	if (ddx < 35)  
	{
		ddx = (dx - (35 * nbBus)) >> 1;
		if (ddx < 0) ddx =0;
	}
	ddx=ddx>>1;
	dx=dx-ddx-ddx;
	lpobject->margin_slider = ddx;
	lpobject->dx_slider = dx / nbBus;
}

static void DrawSTRIP(LPT_VMSCTL_STRIPCTX lpobject, HDC dc)
{
	ComputeStripDisplayData(lpobject);
	DrawSTRIP_NickNameMute(lpobject, dc);
	DrawSTRIP_Name(lpobject, dc);
	DrawSTRIP_PeakMetersBkg(lpobject, dc);
	DrawSTRIP_AllSliders(lpobject, dc);
	DrawSTRIP_BUSAssignment(lpobject, dc);
}


/*---------------------------------------------------------------*/
/*                          BUS CALLBACK                         */
/*---------------------------------------------------------------*/

static long VMSCTL_Strip_GetStripIndex(LPT_VMSCTL_STRIPCTX lpobject,long nuVisibleSlider)
{
	LPT_VMSCTL_STRIP_DATA lpdata;
	long vi, nu=0;
	lpdata = &(lpobject->data);
	for (vi=0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		if ((lpdata->strip_assignmentbit[vi] & 0x10) != 0)
		{
			if (nuVisibleSlider == nu) return vi;
			nu++;
		}
	}
	return 0;
}

static long VMSCTL_Strip_WhereAmI(LPT_VMSCTL_STRIPCTX lpobject,long x0, long y0)
{
	long nuVisibleSlider;
	RECT rect;
	rect=lpobject->rect_nickname;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_STRIP_ID_MUTE;
	rect=lpobject->rect_name;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_STRIP_ID_NAME;
	rect=lpobject->rect_meters;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_STRIP_ID_METER;

	for (nuVisibleSlider=0; nuVisibleSlider < lpobject->data.strip_nbBusDisplayed; nuVisibleSlider++)
	{
		rect=lpobject->rect_slider[nuVisibleSlider];
		if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_STRIP_ID_GAIN+nuVisibleSlider;
		rect=lpobject->rect_assign[nuVisibleSlider];
		if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_STRIP_ID_ASSIGN+nuVisibleSlider;
	}
	return 0;
}

static long VMSCTL_Strip_ManageLButtonDown(LPT_VMSCTL_STRIPCTX lpobject, HWND hw, long x0, long y0)
{
	LPT_VMSCTL_STRIP_DATA lpdata;
	HDC dc;
	float fValue;
	long nuCtl, iStrip, nuVisibleSlider;
	nuCtl=VMSCTL_Strip_WhereAmI(lpobject,x0, y0);
	if (nuCtl == 0) return 0;

	lpdata = &(lpobject->data);

	// if the control is in the 8 possible displayed Sliders
	if ((nuCtl >= VMSCTL_STRIP_ID_GAIN) && (nuCtl < (VMSCTL_STRIP_ID_GAIN + VMSCTL_MAX_NBBUS)))
	{
		nuVisibleSlider = nuCtl - VMSCTL_STRIP_ID_GAIN;
		iStrip = VMSCTL_Strip_GetStripIndex(lpobject,nuVisibleSlider);
		lpobject->start_value = lpdata->strip_gain[iStrip];
		if (lpobject->start_value > lpobject->param.vmax) lpobject->start_value = lpobject->param.vmax;
		if (lpobject->start_value < lpobject->param.vmin) lpobject->start_value = lpobject->param.vmin;
		lpobject->LastCapture = SetCapture(hw);
		lpobject->mouse_y0=y0;
		lpobject->last_y0=y0;
		lpobject->mouseCapture=TRUE;
		lpobject->ctlRunning=nuCtl;
		lpobject->iStripRunning=iStrip;
		return 0;
	}
	// if the control is in the 8 possible assignation buttton.
	if ((nuCtl >= VMSCTL_STRIP_ID_ASSIGN) && (nuCtl < (VMSCTL_STRIP_ID_ASSIGN + VMSCTL_MAX_NBBUS)))
	{
		nuVisibleSlider = nuCtl - VMSCTL_STRIP_ID_ASSIGN;
		iStrip = VMSCTL_Strip_GetStripIndex(lpobject,nuVisibleSlider);

		if ((lpdata->strip_assignmentbit[iStrip] & 0x0F) == 0)
		{
			lpdata->strip_assignmentbit[iStrip] |= 0x01;
			fValue=1.0f;
		}
		else 
		{
			lpdata->strip_assignmentbit[iStrip] &= 0xF0;
			fValue=0.0f;
		}
		lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);
		dc=GetDC(hw);
		DrawSTRIP_BUSAssignmentButton(lpobject, dc, nuVisibleSlider, (long)(fValue), lpdata->strip_pBUSNameList[iStrip]);			
		ReleaseDC(hw,dc);
		return 0;
	}

	// test other control if required
	switch(nuCtl)
	{
	case VMSCTL_STRIP_ID_MUTE:
		if (lpdata->strip_mute == 0)
		{
			lpdata->strip_mute =1;
			fValue=1.0f;
		}
		else 
		{
			lpdata->strip_mute =0;
			fValue=0.0f;
		}
		lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);
		dc=GetDC(hw);
		DrawSTRIP_NickNameMute(lpobject, dc);
		ReleaseDC(hw,dc);
		break;
	case VMSCTL_BUS_ID_NAME:
		break;
	case VMSCTL_BUS_ID_METER:
		break;
	}

	return 0;
}

static long VMSCTL_Strip_ManageLButtonDbclick(LPT_VMSCTL_STRIPCTX lpobject, HWND hw, long x0, long y0)
{
	HDC dc;
	float fValue;
	long nuCtl, iStrip, nuVisibleSlider;
	nuCtl=VMSCTL_Strip_WhereAmI(lpobject,x0, y0);
	if (nuCtl == 0) return 0;

	// if the control is in the 8 possible displayed Sliders
	if ((nuCtl >= VMSCTL_STRIP_ID_GAIN) && (nuCtl < (VMSCTL_STRIP_ID_GAIN + VMSCTL_MAX_NBBUS)))
	{
		nuVisibleSlider = nuCtl - VMSCTL_STRIP_ID_GAIN;
		iStrip = VMSCTL_Strip_GetStripIndex(lpobject,nuVisibleSlider);

		fValue=0.0f;
		lpobject->data.strip_gain[iStrip]=fValue;
		lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);

		dc=GetDC(hw);
		DrawSTRIP_Slider(lpobject, dc, nuVisibleSlider, fValue);
		ReleaseDC(hw,dc);
		return 0;
	}
	VMSCTL_Strip_ManageLButtonDown(lpobject, hw, x0, y0);
	return 0;
}


static long VMSCTL_Strip_ManageMouseMove(LPT_VMSCTL_STRIPCTX lpobject, HWND hw, long x0, long y0)
{
	HDC dc;
	LPT_VMSCTL_STRIPPARAM lpp;
	long dy,ddy, nuCtl, iStrip, nuVisibleSlider;
	float fValue;
	if (lpobject->mouseCapture == FALSE) return -1;
	lpp = &(lpobject->param);
	nuCtl=lpobject->ctlRunning;
	if ((nuCtl >= VMSCTL_STRIP_ID_GAIN) && (nuCtl < (VMSCTL_STRIP_ID_GAIN + VMSCTL_MAX_NBBUS)))
	{
		iStrip = lpobject->iStripRunning;
		if (y0 != lpobject->last_y0)
		{
			dy=y0-lpobject->mouse_y0;
			ddy=lpobject->dy_slider;

			fValue=lpobject->start_value - (((lpp->vmax - lpp->vmin) * dy) / ddy);
			if (fValue < lpp->vmin) fValue=lpp->vmin;
			if (fValue > lpp->vmax) fValue=lpp->vmax;
			if (fValue != lpobject->data.strip_gain[iStrip])
			{
				lpobject->data.strip_gain[iStrip]=fValue;
				lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);

				nuVisibleSlider = nuCtl - VMSCTL_STRIP_ID_GAIN;
				dc=GetDC(hw);
				DrawSTRIP_Slider(lpobject, dc, nuVisibleSlider, fValue);
				ReleaseDC(hw,dc);
			}
			lpobject->last_y0 = y0;
		}
		
	}
	return 0;
}

static long VMSCTL_Strip_ManageLButtonUp(LPT_VMSCTL_STRIPCTX lpobject, HWND hw, long x0, long y0)
{
	if (lpobject->mouseCapture == FALSE) return -1;
	ReleaseCapture();
	if (lpobject->LastCapture != NULL) SetCapture(lpobject->LastCapture);
	lpobject->mouseCapture=FALSE;
	return 0;
}


static LRESULT CALLBACK VMSCTL_StripCallback(HWND hw,		//handle of the window.
											UINT msg,   //Message Ident.
											WPARAM p1,	//parameter 1.
											LPARAM p2)	//parameter 2
{
	HPEN oldpen;
	HBRUSH oldbrush;
	RECT rect;
	LPCREATESTRUCT   lpcs;
	LPT_VMSCTL_STRIPCTX lpobject;
	HDC dc;
	PAINTSTRUCT ps;
	switch (msg)
	{
		//here we create our structure
		case WM_CREATE:
			lpcs=(LPCREATESTRUCT)p2;
			lpobject=(LPT_VMSCTL_STRIPCTX)lpcs->lpCreateParams;
			lpobject->hw=hw;
			TOOL_StorePointerInWindow(hw,lpobject);
			break;
		//on mouse click we get the focus.
		case WM_MOUSEACTIVATE:
			if (GetFocus()!=hw) SetFocus(hw);
			return MA_ACTIVATE;
		//on Focus change we redraw the control
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			InvalidateRect(hw,NULL,FALSE);
			return 0;
		//here we draw all the control
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject != NULL) DrawSTRIP(lpobject, dc);
			EndPaint(hw,&ps);
			return 0;
		case WM_ERASEBKGND:
			lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) return 0;

			dc=(HDC)p1;
			GetClientRect(hw,&rect);
			oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg0_brush);
			oldpen = (HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
			Rectangle(dc,rect.left-1,rect.top-1,rect.right+1,rect.bottom+1);
			oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
			RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 7,7);
			SelectObject(dc,oldpen);
			SelectObject(dc,oldbrush);
			return 0;
		//mouse behavior
		case WM_LBUTTONDOWN:
			lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Strip_ManageLButtonDown(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		case WM_LBUTTONDBLCLK:
			lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Strip_ManageLButtonDbclick(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		case WM_MOUSEMOVE:
			lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Strip_ManageMouseMove(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		case WM_LBUTTONUP:
			lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Strip_ManageLButtonUp(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		// control ending
		case WM_CLOSE:
			DestroyWindow(hw);
			return 0;
		case WM_DESTROY:
			lpobject=TOOL_RecallPointerFromWindow(hw);
			if (lpobject != NULL) 
			{
				//if (lpobject->tempbmp !=NULL) DeleteObject(lpobject->tempbmp);
				free(lpobject);
				TOOL_StorePointerInWindow(hw,NULL);
			}
			break;

	}
	return DefWindowProc(hw,msg,p1,p2);
}


HWND VMSCTL_CreateSTRIP(HWND hParent, long Ident, long x0,long y0, LPT_VMSCTL_STRIPPARAM lpparam)
{
	HWND hw;
	char szName[64];
	long wstyle, vi;
	LPT_VMSCTL_STRIPCTX lpobject;
	//we allocate memory for our structure
	lpobject=(LPT_VMSCTL_STRIPCTX)malloc(sizeof(T_VMSCTL_STRIPCTX));
	if (lpobject == NULL) return NULL;
	memset(lpobject,0, sizeof(T_VMSCTL_STRIPCTX));
	if (lpparam == NULL) return NULL;
	lpobject->param = *lpparam;
	//define control size according bitmap.
	lpobject->dx=100;
	lpobject->dy=100;
	//init rtdata;
	for (vi=0;vi<8;vi++)
	{
		lpobject->rtdata.dbLevel[vi]=-100.0f;
	}
	//set control ident
	lpobject->Ident=Ident;
	//we create window with window-creation data	
	wstyle=WS_CHILD | WS_TABSTOP; // | WS_VISIBLE
	sprintf(szName,"Strip #%i", Ident);
	hw=CreateWindow(VMSCTL_CLASSNAME_STRIP,	szName,
					wstyle,x0,y0,
					lpobject->dx,			// window width
					lpobject->dy,			// window height
					hParent,			// parent Windows Handle.
					(HMENU)Ident,		// child-window identifier.
					G_hinstance,		// handle of application instance
					lpobject); 			// we communicate our pointer (see WM_CREATE)

	return hw;
}

long VMSCTL_PositionSTRIP(HWND hw, long x0,long y0, long dx, long dy, long fShow)
{
	LPT_VMSCTL_STRIPCTX lpobject;
	BOOL fUpdate=FALSE;
	lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;			
	if (fShow == 0)
	{
		lpobject->x0 = x0;	
		lpobject->y0 = y0;	
		lpobject->dx = dx;	
		lpobject->dy = dy;	
		if (lpobject->isShown != 0)
		{
			ShowWindow(hw, SW_HIDE);
		}
		lpobject->isShown = fShow;
	}
	else
	{
		if (lpobject->x0 != x0) fUpdate=TRUE;	
		if (lpobject->y0 != y0) fUpdate=TRUE;	
		if (lpobject->dx != dx) fUpdate=TRUE;	
		if (lpobject->dy != dy) fUpdate=TRUE;	
		if (lpobject->isShown == 0) fUpdate=TRUE;	
			
		if (fUpdate == TRUE) 
		{
			//internal bitmap size can be changed
			if (lpobject->tempbmp !=NULL) DeleteObject(lpobject->tempbmp);
			lpobject->tempbmp=NULL;
			
			lpobject->x0 = x0;	
			lpobject->y0 = y0;	
			lpobject->dx = dx;	
			lpobject->dy = dy;	

			ComputeStripDisplayData(lpobject);
			SetWindowPos(hw,HWND_TOP,lpobject->x0,lpobject->y0,lpobject->dx,lpobject->dy,SWP_SHOWWINDOW);
			lpobject->isShown = fShow;
		}
	}
	//if (fUpdate == TRUE) InvalidateRect(hw, NULL, TRUE);
	return 0;
}

long VMSCTL_SetDataSTRIP(HWND hw, LPT_VMSCTL_STRIP_DATA pData, long fUpdateAll)
{
	BOOL fUpdateMuteDisplay;
	long vi,nuVisibleSlider;
	BOOL fBusChanged, fAssignChanged, fGainChanged; 
	HDC dc=NULL;
	LPT_VMSCTL_STRIP_DATA pCurrent;
	LPT_VMSCTL_STRIPCTX lpobject;
	lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;			
	
	pCurrent = &(lpobject->data);
	if (lpobject->isShown != 0)
	{
		dc=GetDC(hw);
	}
	
	// NickName Button
	fUpdateMuteDisplay=FALSE;
	if ((fUpdateAll != 0) || (pCurrent->strip_nickname[0] == 0))
	{
		strcpy(pCurrent->strip_nickname, pData->strip_nickname);
		fUpdateMuteDisplay=TRUE;
	}
	// Strip Name
	if ((wcscmp(pCurrent->strip_namew, pData->strip_namew) != 0) || (fUpdateAll != 0))
	{
		wcscpy(pCurrent->strip_namew, pData->strip_namew);
		DrawSTRIP_Name(lpobject, dc);
	}

	// check bus number change
	fBusChanged	= fUpdateAll;
	fAssignChanged = fUpdateAll;
	fGainChanged =fUpdateAll;
	if (pCurrent->strip_nbBusDisplayed != pData->strip_nbBusDisplayed)
	{
		fBusChanged	= TRUE;
		fAssignChanged = TRUE;
		pCurrent->strip_nbBusDisplayed = pData->strip_nbBusDisplayed;
		//internal bitmap size can be changed
		if (lpobject->tempbmp !=NULL) DeleteObject(lpobject->tempbmp);
		lpobject->tempbmp=NULL;
	}
	// check slide bus assignement change
	pCurrent->strip_pBUSNameList = pData->strip_pBUSNameList;
	for (vi=0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		if ((pCurrent->strip_assignmentbit[vi] & 0x10) !=  (pData->strip_assignmentbit[vi] & 0x10)) fBusChanged=TRUE;
		if ((pCurrent->strip_assignmentbit[vi] & 0x01) !=  (pData->strip_assignmentbit[vi] & 0x01)) fAssignChanged=TRUE;
		pCurrent->strip_assignmentbit[vi] = pData->strip_assignmentbit[vi];
	}
	if (fBusChanged == TRUE) ComputeStripDisplayData(lpobject);
	if ((fBusChanged == TRUE) || (fAssignChanged == TRUE)) DrawSTRIP_BUSAssignment(lpobject, dc);

	// check slider gain
	nuVisibleSlider=0;
	for (vi=0; vi < VMSCTL_MAX_NBBUS; vi++)
	{
		fGainChanged=fUpdateAll;
		if ((pCurrent->strip_assignmentbit[vi] & 0x10) != 0)
		{
			if (pCurrent->strip_gain[vi] != pData->strip_gain[vi]) fGainChanged=TRUE;
			// if the slider is used by the mouse we do not update display.
			if ((lpobject->mouseCapture == TRUE) && (lpobject->iStripRunning == vi)) fGainChanged=FALSE;
			if ((fBusChanged == TRUE) || (fGainChanged == TRUE)) 
			{
				pCurrent->strip_gain[vi] = pData->strip_gain[vi];
				DrawSTRIP_Slider(lpobject, dc, nuVisibleSlider, pCurrent->strip_gain[vi]);
			}
			nuVisibleSlider++;
		}
	}
	// Mute Button
	if ((pCurrent->strip_mute != pData->strip_mute) || (fUpdateMuteDisplay == TRUE) || (fUpdateAll == TRUE))
	{
		pCurrent->strip_mute = pData->strip_mute;
		DrawSTRIP_NickNameMute(lpobject, dc);
	}

	if (dc != NULL)	ReleaseDC(hw,dc);

	return 0;
}

long VMSCTL_ResetDataSTRIP(HWND hw)
{
	long vi;
	LPT_VMSCTL_STRIPCTX lpobject;
	lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;	
	memset(&(lpobject->data), 0, sizeof(T_VMSCTL_STRIPPARAM));
	for (vi=0;vi<8;vi++) lpobject->rtdata.dbLevel[vi]=-100.0f;
	return 0;
}


long VMSCTL_SetRTDataSTRIP(HWND hw, LPT_VMSCTL_RT_DATA pRTData)
{
	HDC dc;
	long nu;
	LPT_VMSCTL_STRIPCTX lpobject;
	lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;			

	if (pRTData == NULL) return -2;
	
	//compute value to display, with a release factor
	for (nu=0;nu<2;nu++)	//display 2 channels only
	{
		if (pRTData->dbLevel[nu] > lpobject->rtdata.dbLevel[nu]) lpobject->rtdata.dbLevel[nu]=pRTData->dbLevel[nu];
		else 
		{
			lpobject->rtdata.dbLevel[nu] = lpobject->rtdata.dbLevel[nu] - PEAKMETER_RELEASE_DECREMENT;
			if (lpobject->rtdata.dbLevel[nu] < -100.0f) lpobject->rtdata.dbLevel[nu]=-100.0f;
		}
	}
	//display peak meter.
	if (lpobject->isShown != 0)
	{
		dc=GetDC(hw);
		DrawSTRIP_PeakMeters(lpobject, dc);
		ReleaseDC(hw,dc);
	}

	return 0;
}

long VMSCTL_GetIndexSTRIP(HWND hw,long nuVisibleSlider)
{
	LPT_VMSCTL_STRIPCTX lpobject;
	lpobject=(LPT_VMSCTL_STRIPCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return 0;			
	return VMSCTL_Strip_GetStripIndex(lpobject,nuVisibleSlider);
}























/*---------------------------------------------------------------*/
/*                          BUS CONTROL                          */
/*---------------------------------------------------------------*/

#define VMSCTL_BUS_DX_NICKNAME		30
#define VMSCTL_BUS_DX_MON			40

#define VMSCTL_BUS_DXMIN_NAME		100
#define VMSCTL_BUS_DXMIN_PEAKMETER	100
#define VMSCTL_BUS_DXMIN_SLIDER		100

typedef struct tagVMSCTL_BUSCTX
{
	HWND	hw;
	long	Ident;
	long	x0,y0,dx,dy;
	long	isShown;

	RECT	rect_nickname;
	RECT	rect_name;
	RECT	rect_meters;
	RECT	rect_slider;
	RECT	rect_monitor;

	long	dx_meter;
	long	dx_slider;
	HBITMAP	tempbmp;

	float	start_value;
	HWND	LastCapture;
	long	mouse_x0;
	long	last_x0;
	BOOL	mouseCapture;
	long	ctlrunning;

	float   fMaxLevelToDisplay;
	float   fMaxLevelDisplayed;
	long	nMaxLevelToDisplay;	
	T_VMSCTL_BUSPARAM param;
	T_VMSCTL_BUS_DATA data;
	T_VMSCTL_RT_DATA rtdata;
} T_VMSCTL_BUSCTX, *PT_VMSCTL_vCTX, *LPT_VMSCTL_BUSCTX;


static void DrawBUS_NickNameMute(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	char sss[64];
	RECT rect;
	HFONT oldfont;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	rect.left=VMSCTL_STRIPBUS_MARGIN;
	rect.top=VMSCTL_STRIPBUS_MARGIN;
	rect.right=rect.left+VMSCTL_BUS_DX_NICKNAME;
	rect.bottom=lpobject->dy-5;
	lpobject->rect_nickname=rect;
	
	oldpen = (HPEN)SelectObject(dc,lpobject->param.assignbus_pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font1);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,lpobject->param.assignbus_color);

	if (lpobject->data.bus_mute != 0)
	{
		SetTextColor(dc,lpobject->param.mutered_color);
		SelectObject(dc,lpobject->param.mutered_pen);		
	}

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 5,5);
	strcpy(sss,lpobject->data.bus_nickname);
	DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);

}

static void DrawBUS_Name(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	RECT rect;
	HFONT oldfont;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	rect.left=VMSCTL_STRIPBUS_MARGIN+VMSCTL_BUS_DX_NICKNAME+10;
	rect.top=2;
	rect.right=rect.left+VMSCTL_BUS_DXMIN_NAME;
	rect.bottom=lpobject->dy-2;
	lpobject->rect_name=rect;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font2);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,lpobject->param.name_color);

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 5,5);
	DrawTextW(dc,lpobject->data.bus_namew,(long)wcslen(lpobject->data.bus_namew),&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}


static void DrawBUS_PeakMetersBkg(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	long dx;
	RECT rect;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	dx = lpobject->dx_meter;
	rect.left=VMSCTL_STRIPBUS_MARGIN+VMSCTL_BUS_DX_NICKNAME+10+VMSCTL_BUS_DXMIN_NAME+10;
	rect.top=VMSCTL_STRIPBUS_MARGIN;
	rect.right=rect.left+dx;
	rect.bottom=lpobject->dy-VMSCTL_STRIPBUS_MARGIN;
	lpobject->rect_meters=rect;

	oldpen = (HPEN)SelectObject(dc,GetStockObject(BLACK_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(BLACK_BRUSH));
	Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
}

static void DrawBUS_PeakMeters(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	char sss[64];
	LPT_VMSCTL_BUSPARAM lpp;
	float dBValue, dBmin=-80.0f, dBmax=+12.0f;
	long nu,nbMeter=2;
	long dx,dy,xx,x1,xgreen,xred;
	RECT rect, rectmax;
	HPEN oldpen;
	HBRUSH oldbrush;
	HFONT oldfont;
	if (dc == NULL) return;

	dx = lpobject->dx_meter-6;
	rect.left=VMSCTL_STRIPBUS_MARGIN+VMSCTL_BUS_DX_NICKNAME+10+VMSCTL_BUS_DXMIN_NAME+10;
	rect.top=VMSCTL_STRIPBUS_MARGIN;
	rect.bottom=lpobject->dy-VMSCTL_STRIPBUS_MARGIN;
	dy = (rect.bottom-rect.top-4) / nbMeter;
	
	// compute peak meter zone
	rect.left+=3;
	rect.top+=3;
	rect.right=rect.left+dx;
	rect.bottom = rect.top+dy;

	// compute dB max zone and reduce peak meter zone
	rectmax = rect;
	rect.right -=25;
	dx -=25;
	rectmax.left=rect.right;
	rectmax.bottom +=dy;

	// prepare display and compute different level 
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font0);
	oldpen = (HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
	oldbrush = (HBRUSH)SelectObject(dc,GetStockObject(BLACK_BRUSH));
	xgreen = (long)(((-24.0f - dBmin) * dx)/ (dBmax - dBmin));
	xred = (long)(((0.0f - dBmin) * dx)/ (dBmax - dBmin));
	lpp = &(lpobject->param);

	for (nu=0;nu<nbMeter; nu++)
	{
		dBValue = lpobject->rtdata.dbLevel[nu];
		xx = (long)(((dBValue - dBmin) * dx)/ (dBmax - dBmin));
		if (xx < 0) xx=0;
		if (xx > dx) xx=dx;
		
		// blue
		if (xx > 0)
		{
			SelectObject(dc,lpp->meter_brush1);
			x1=xx;
			if (x1 > xgreen) x1=xgreen;
			Rectangle(dc,rect.left,rect.top,rect.left+x1+1,rect.bottom);
		}
		//green
		if (xx >= xgreen)
		{
			SelectObject(dc,lpp->meter_brush2);
			x1=xx;
			if (x1 > xred) x1=xred;
			Rectangle(dc,rect.left+xgreen,rect.top,rect.left+x1+1,rect.bottom);
		}
		//red
		if (xx >= xred)
		{
			SelectObject(dc,lpp->meter_brush3);
			x1=xx;
			Rectangle(dc,rect.left+xred,rect.top,rect.left+x1+1,rect.bottom);
		}
		//black
		if (xx <= dx)
		{
			SelectObject(dc,GetStockObject(BLACK_BRUSH));
			Rectangle(dc,rect.left+xx,rect.top,rect.right+1,rect.bottom);
		}

		rect.top +=dy;
		rect.bottom +=dy;
	}
	// display max level 
	if (lpobject->fMaxLevelDisplayed != lpobject->fMaxLevelToDisplay)
	{
		SetBkMode(dc,TRANSPARENT);
		SetTextColor(dc,lpobject->param.name_color);
		if (lpobject->fMaxLevelToDisplay < -99.0f) strcpy(sss,"-");
		else sprintf(sss,"%0.1f", lpobject->fMaxLevelToDisplay);
		SelectObject(dc,GetStockObject(BLACK_BRUSH));
		Rectangle(dc,rectmax.left,rectmax.top,rectmax.right+1,rectmax.bottom+1);
		DrawText(dc,sss,(long)strlen(sss),&rectmax,DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		lpobject->fMaxLevelDisplayed=lpobject->fMaxLevelToDisplay;
	}
	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);
}


static void DrawBUS_Slider(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	HDC dcmem;
	char sss[64];
	float dBValue;
	LPT_VMSCTL_BUSPARAM lpp;
	long x0,y0, xx,dx,dy,y_middle;
	long cursor_width=35;
	RECT rect,rect2;
	HPEN oldpen, sliderpen;
	HBRUSH oldbrush, sliderbrush;
	HFONT oldfont;
	HBITMAP oldbmp;
	if (dc == NULL) return;

	//compute local coordintate
	dx = lpobject->dx_slider;
	dy = lpobject->dy-VMSCTL_STRIPBUS_MARGIN-VMSCTL_STRIPBUS_MARGIN;
	x0=VMSCTL_STRIPBUS_MARGIN+VMSCTL_BUS_DX_NICKNAME+10+VMSCTL_BUS_DXMIN_NAME+10+lpobject->dx_meter+10;
	y0=VMSCTL_STRIPBUS_MARGIN;
	
	rect.left=x0;
	rect.top=y0;
	rect.right=rect.left+dx;
	rect.bottom=rect.top+dy;
	lpobject->rect_slider=rect;

	rect.left=0;
	rect.top=0;
	rect.right=rect.left+dx;
	rect.bottom=rect.top+dy;

	y_middle = (rect.bottom+rect.top)>>1;
	lpp = &(lpobject->param);
	dBValue = lpobject->data.bus_gain;
	//define slider color according level
	if (dBValue > 0.0f)
	{
		sliderpen = lpp->slider_pen_red;
		sliderbrush = lpp->slider_brush_red;
	}
	else
	{
		sliderpen = lpp->slider_pen_green;
		sliderbrush = lpp->slider_brush_green;
	}
	//compute slider horizontal position
	xx = (long)(((dBValue - lpp->vmin) * (dx - cursor_width))/ (lpp->vmax - lpp->vmin));
	if (xx < 0) xx=0;
	if (xx > dx) xx=dx;

	//we don't draw directly in screen.
	//to avoid flickering we first build a bitmap in memory
	dcmem=CreateCompatibleDC(dc);
	if (lpobject->tempbmp ==NULL) lpobject->tempbmp=CreateCompatibleBitmap(dc,dx,dy);
	oldbmp=(HBITMAP)SelectObject(dcmem,lpobject->tempbmp);
	//Draw background 
	oldpen = (HPEN)SelectObject(dcmem,GetStockObject(NULL_PEN));
	oldbrush = (HBRUSH)SelectObject(dcmem,lpp->bkg1_brush);
	oldfont = (HFONT)SelectObject(dcmem,lpobject->param.font0);
	SetBkMode(dcmem,TRANSPARENT);
	SetTextColor(dcmem,RGB(0,0,0));

	//draw background
	Rectangle(dcmem,rect.left,rect.top,rect.right+1,rect.bottom+1);
	SelectObject(dcmem,sliderpen);
	MoveToEx(dcmem,rect.left,y_middle, NULL);
	LineTo(dcmem,rect.right, y_middle);

	//draw cursor
	SelectObject(dcmem,sliderpen);
	SelectObject(dcmem,sliderbrush);
	RoundRect(dcmem,rect.left+xx, rect.top,rect.left+xx+cursor_width-1,rect.bottom,7,7);
	rect2=rect;
	rect2.left +=xx;
	rect2.right = rect2.left+cursor_width;
	sprintf(sss,"%0.1f", dBValue);
	DrawText(dcmem,sss,(long)strlen(sss),&rect2,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dcmem,oldpen);
	SelectObject(dcmem,oldbrush);
	SelectObject(dcmem,oldfont);
	//display bitmap on screen at the end.
	BitBlt(dc,x0,y0,dx,dy,dcmem,0,0,SRCCOPY);
	SelectObject(dcmem,oldbmp);
	DeleteDC(dcmem);

}

static void DrawBUS_Monitor(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	char sss[64];
	RECT rect;
	HFONT oldfont;
	HPEN oldpen;
	HBRUSH oldbrush;
	if (dc == NULL) return;

	if (lpobject->data.bus_fMonitorSupport == 0) return;
	
	rect.right = lpobject->dx-VMSCTL_STRIPBUS_MARGIN;
	rect.left=rect.right-VMSCTL_BUS_DX_MON;
	//check if there is enough place to display it.
	if (rect.left < (lpobject->rect_slider.right+10)) rect.left = lpobject->rect_slider.right+10;
	rect.right = rect.left + VMSCTL_BUS_DX_MON;

	rect.top=VMSCTL_STRIPBUS_MARGIN;
	rect.bottom=lpobject->dy-VMSCTL_STRIPBUS_MARGIN;
	lpobject->rect_monitor=rect;

	oldpen = (HPEN)SelectObject(dc,lpobject->param.assignbus_pen);
	oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
	oldfont = (HFONT)SelectObject(dc,lpobject->param.font1);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc,lpobject->param.assignbus_color);

	if (lpobject->data.bus_monitor != 0)
	{
		SetTextColor(dc,RGB(0,0,0));
		SelectObject(dc,lpobject->param.monitor_brush);		
		SelectObject(dc,lpobject->param.monitor_pen);		
	}

	RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 5,5);
	strcpy(sss,"Mon");
	DrawText(dc,sss,(long)strlen(sss),&rect,DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(dc,oldpen);
	SelectObject(dc,oldbrush);
	SelectObject(dc,oldfont);

}

static void ComputeBusDisplayData(LPT_VMSCTL_BUSCTX lpobject)
{
	long dx;
	dx= lpobject->dx - VMSCTL_STRIPBUS_MARGIN- VMSCTL_BUS_DX_NICKNAME-10-VMSCTL_BUS_DXMIN_NAME-VMSCTL_STRIPBUS_MARGIN;
	if (lpobject->data.bus_fMonitorSupport != 0) dx = dx -10 -VMSCTL_BUS_DX_MON;	
	dx= (dx-20) >> 1;
	if (dx < 100) dx =100;
	lpobject->dx_meter = dx;
	lpobject->dx_slider = dx;
	lpobject->fMaxLevelDisplayed=-500.0f; //to force display refresh 
}


static void DrawBUS(LPT_VMSCTL_BUSCTX lpobject, HDC dc)
{
	ComputeBusDisplayData(lpobject);
	DrawBUS_NickNameMute(lpobject, dc);
	DrawBUS_Name(lpobject, dc);
	DrawBUS_PeakMetersBkg(lpobject, dc);
	DrawBUS_Slider(lpobject, dc);
	DrawBUS_Monitor(lpobject, dc);
}


/*---------------------------------------------------------------*/
/*                          BUS CALLBACK                         */
/*---------------------------------------------------------------*/


static long VMSCTL_Bus_WhereAmI(LPT_VMSCTL_BUSCTX lpobject,long x0, long y0)
{
	RECT rect;
	rect=lpobject->rect_nickname;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_BUS_ID_MUTE;
	rect=lpobject->rect_name;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_BUS_ID_NAME;
	rect=lpobject->rect_meters;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_BUS_ID_METER;
	rect=lpobject->rect_slider;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_BUS_ID_GAIN;
	rect=lpobject->rect_monitor;
	if ((x0 >= rect.left) && (x0 <= rect.right) && (y0>= rect.top) && (y0<=rect.bottom)) return VMSCTL_BUS_ID_MONITOR;
	return 0;
}

static long VMSCTL_Bus_ManageLButtonDown(LPT_VMSCTL_BUSCTX lpobject, HWND hw, long x0, long y0)
{
	HDC dc;
	float fValue;
	long nuCtl;
	nuCtl=VMSCTL_Bus_WhereAmI(lpobject,x0, y0);
	if (nuCtl == 0) return 0;

	switch(nuCtl)
	{
	case VMSCTL_BUS_ID_MUTE:
		if (lpobject->data.bus_mute == 0)
		{
			lpobject->data.bus_mute =1;
			fValue=1.0f;
		}
		else 
		{
			lpobject->data.bus_mute =0;
			fValue=0.0f;
		}
		lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);
		dc=GetDC(hw);
		DrawBUS_NickNameMute(lpobject, dc);
		ReleaseDC(hw,dc);
		break;
	case VMSCTL_BUS_ID_NAME:
		break;
	case VMSCTL_BUS_ID_METER:
		break;
	case VMSCTL_BUS_ID_GAIN:
		lpobject->start_value = lpobject->data.bus_gain;
		if (lpobject->start_value > lpobject->param.vmax) lpobject->start_value = lpobject->param.vmax;
		if (lpobject->start_value < lpobject->param.vmin) lpobject->start_value = lpobject->param.vmin;
		lpobject->LastCapture = SetCapture(hw);
		lpobject->mouse_x0=x0;
		lpobject->last_x0=x0;
		lpobject->mouseCapture=TRUE;
		lpobject->ctlrunning=nuCtl;
		break;
	case VMSCTL_BUS_ID_MONITOR:
		if (lpobject->data.bus_monitor == 0)
		{
			lpobject->data.bus_monitor =1;
			fValue=1.0f;
		}
		else 
		{
			lpobject->data.bus_monitor =0;
			fValue=0.0f;
		}
		lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);
		dc=GetDC(hw);
		DrawBUS_Monitor(lpobject, dc);
		ReleaseDC(hw,dc);
		break;
	}
	return 0;
}

static long VMSCTL_Bus_ManageLButtonDbclick(LPT_VMSCTL_BUSCTX lpobject, HWND hw, long x0, long y0)
{
	HDC dc;
	float fValue;
	long nuCtl;
	nuCtl=VMSCTL_Bus_WhereAmI(lpobject,x0, y0);
	if (nuCtl == 0) return 0;

	switch(nuCtl)
	{
	//double click to reset slider to 0.0 dB
	case VMSCTL_BUS_ID_GAIN:
		fValue=0.0f;
		lpobject->data.bus_gain=fValue;
		lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);

		dc=GetDC(hw);
		DrawBUS_Slider(lpobject, dc);
		ReleaseDC(hw,dc);
		break;
	//otherwise we consider it as a simple click
	default:
		VMSCTL_Bus_ManageLButtonDown(lpobject, hw, x0, y0);
		break;
	}
	return 0;
}


static long VMSCTL_Bus_ManageMouseMove(LPT_VMSCTL_BUSCTX lpobject, HWND hw, long x0, long y0)
{
	HDC dc;
	LPT_VMSCTL_BUSPARAM lpp;
	long dx,ddx, nuCtl;
	float fValue;
	if (lpobject->mouseCapture == FALSE) return -1;
	lpp = &(lpobject->param);
	nuCtl=lpobject->ctlrunning;
	switch(nuCtl)
	{
	case VMSCTL_BUS_ID_GAIN:
		if (x0 != lpobject->last_x0)
		{
			dx=x0-lpobject->mouse_x0;
			ddx=lpobject->dx_slider;

			fValue=lpobject->start_value + (((lpp->vmax - lpp->vmin) * dx) / ddx);
			if (fValue < lpp->vmin) fValue=lpp->vmin;
			if (fValue > lpp->vmax) fValue=lpp->vmax;
			if (fValue != lpobject->data.bus_gain)
			{
				lpobject->data.bus_gain=fValue;
				lpobject->param.lpCallback(lpobject->param.lpuser, lpobject->Ident, nuCtl, fValue);

				dc=GetDC(hw);
				DrawBUS_Slider(lpobject, dc);
				ReleaseDC(hw,dc);
			}
			lpobject->last_x0 = x0;
		}
		break;
	}
	return 0;
}

static long VMSCTL_Bus_ManageLButtonUp(LPT_VMSCTL_BUSCTX lpobject, HWND hw, long x0, long y0)
{
	if (lpobject->mouseCapture == FALSE) return -1;
	ReleaseCapture();
	if (lpobject->LastCapture != NULL) SetCapture(lpobject->LastCapture);
	lpobject->mouseCapture=FALSE;
	return 0;
}


static LRESULT CALLBACK VMSCTL_BusCallback(HWND hw,		//handle of the window.
											UINT msg,   //Message Ident.
											WPARAM p1,	//parameter 1.
											LPARAM p2)	//parameter 2
{
	HPEN oldpen;
	HBRUSH oldbrush;
	RECT rect;
	LPCREATESTRUCT   lpcs;
	LPT_VMSCTL_BUSCTX lpobject;
	HDC dc;
	PAINTSTRUCT ps;
	switch (msg)
	{
		//here we create our structure
		case WM_CREATE:
			lpcs=(LPCREATESTRUCT)p2;
			lpobject=(LPT_VMSCTL_BUSCTX)lpcs->lpCreateParams;
			lpobject->hw=hw;
			TOOL_StorePointerInWindow(hw,lpobject);
			break;
		//on mouse click we get the focus.
		case WM_MOUSEACTIVATE:
			if (GetFocus()!=hw) SetFocus(hw);
			return MA_ACTIVATE;
		//on Focus change we redraw the control
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			InvalidateRect(hw,NULL,FALSE);
			return 0;
		//here we draw all the control
		case WM_PAINT:
			dc=BeginPaint(hw,&ps);
			lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject != NULL) DrawBUS(lpobject, dc);
			EndPaint(hw,&ps);
			return 0;
		case WM_ERASEBKGND:
			lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) return 0;
			dc=(HDC)p1;
			GetClientRect(hw,&rect);
			oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg0_brush);
			oldpen = (HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
			Rectangle(dc,rect.left-1,rect.top-1,rect.right+1,rect.bottom+1);
			oldbrush = (HBRUSH)SelectObject(dc,lpobject->param.bkg1_brush);
			RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom, 7,7);
			SelectObject(dc,oldpen);
			SelectObject(dc,oldbrush);
			return 0;
		//mouse behavior
		case WM_LBUTTONDOWN:
			lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Bus_ManageLButtonDown(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		case WM_LBUTTONDBLCLK:
			lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Bus_ManageLButtonDbclick(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		case WM_MOUSEMOVE:
			lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Bus_ManageMouseMove(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;
		case WM_LBUTTONUP:
			lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
			if (lpobject == NULL) break;			
			VMSCTL_Bus_ManageLButtonUp(lpobject, hw, (short int)LOWORD(p2), (short int)HIWORD(p2));
			break;

		//ending
		case WM_CLOSE:
			DestroyWindow(hw);
			return 0;
		case WM_DESTROY:
			lpobject=TOOL_RecallPointerFromWindow(hw);
			if (lpobject != NULL) 
			{
				if (lpobject->tempbmp !=NULL) DeleteObject(lpobject->tempbmp);
				free(lpobject);
				TOOL_StorePointerInWindow(hw,NULL);
			}
			break;

	}
	return DefWindowProc(hw,msg,p1,p2);
}


HWND VMSCTL_CreateBUS(HWND hParent, long Ident, long x0,long y0, LPT_VMSCTL_BUSPARAM lpparam)
{
	HWND hw;
	char szName[64];
	long wstyle,vi;
	LPT_VMSCTL_BUSCTX lpobject;
	//we allocate memory for our structure
	lpobject=(LPT_VMSCTL_BUSCTX)malloc(sizeof(T_VMSCTL_BUSCTX));
	if (lpobject == NULL) return NULL;
	memset(lpobject,0, sizeof(T_VMSCTL_BUSCTX));
	if (lpparam == NULL) return NULL;
	lpobject->param = *lpparam;
	//define control size according bitmap.
	lpobject->dx=100;
	lpobject->dy=100;
	//init rtdata;
	for (vi=0;vi<8;vi++)
	{
		lpobject->rtdata.dbLevel[vi]=-100.0f;
	}
	//set control ident
	lpobject->Ident=Ident;

	//we create window with window-creation data	
	wstyle=WS_CHILD | WS_TABSTOP; // | WS_VISIBLE
	sprintf(szName,"Bus #%i", Ident);
	hw=CreateWindow(VMSCTL_CLASSNAME_BUS,	szName,
					wstyle,x0,y0,
					lpobject->dx,			// window width
					lpobject->dy,			// window height
					hParent,			// parent Windows Handle.
					(HMENU)Ident,		// child-window identifier.
					G_hinstance,		// handle of application instance
					lpobject); 			// we communicate our pointer (see WM_CREATE)

	return hw;
}


long VMSCTL_PositionBUS(HWND hw, long x0,long y0, long dx, long dy, long fShow)
{
	LPT_VMSCTL_BUSCTX lpobject;
	BOOL fUpdate=FALSE;
	lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;			
	if (fShow == 0)
	{
		lpobject->x0 = x0;	
		lpobject->y0 = y0;	
		lpobject->dx = dx;	
		lpobject->dy = dy;	
		if (lpobject->isShown != 0)
		{
			ShowWindow(hw, SW_HIDE);
		}
		lpobject->isShown = fShow;
	}
	else
	{
		if (lpobject->x0 != x0) fUpdate=TRUE;	
		if (lpobject->y0 != y0) fUpdate=TRUE;	
		if (lpobject->dx != dx) fUpdate=TRUE;	
		if (lpobject->dy != dy) fUpdate=TRUE;	
		if (lpobject->isShown == 0) fUpdate=TRUE;	
			
		if (fUpdate == TRUE) 
		{
			if (lpobject->tempbmp !=NULL) DeleteObject(lpobject->tempbmp);
			lpobject->tempbmp=NULL;

			lpobject->x0 = x0;	
			lpobject->y0 = y0;	
			lpobject->dx = dx;	
			lpobject->dy = dy;	

			ComputeBusDisplayData(lpobject);	
			SetWindowPos(hw,HWND_TOP,lpobject->x0,lpobject->y0,lpobject->dx,lpobject->dy,SWP_SHOWWINDOW);
			lpobject->isShown = fShow;
		}
	}
	//if (fUpdate == TRUE) InvalidateRect(hw, NULL, TRUE);
	return 0;
}

long VMSCTL_SetDataBUS(HWND hw, LPT_VMSCTL_BUS_DATA pData, long fUpdateAll)
{
	BOOL fUpdateMuteDisplay;
	HDC dc=NULL;
	LPT_VMSCTL_BUS_DATA pCurrent;
	LPT_VMSCTL_BUSCTX lpobject;
	lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;	
	if (pData == NULL) return -2;
	pCurrent = &(lpobject->data);
	
	if (pCurrent->bus_fMonitorSupport != pData->bus_fMonitorSupport)
	{
		*pCurrent = *pData;
		InvalidateRect(hw,NULL,TRUE);
		return 0;
	}
	if (lpobject->isShown != 0)
	{
		dc=GetDC(hw);
	}
	
	fUpdateMuteDisplay=FALSE;
	if ((fUpdateAll != 0) || (pCurrent->bus_nickname[0] == 0))
	{
		strcpy(pCurrent->bus_nickname, pData->bus_nickname);
		fUpdateMuteDisplay = TRUE;
	}
	if ((wcscmp(pCurrent->bus_namew, pData->bus_namew) != 0) || (fUpdateAll != 0))
	{
		wcscpy(pCurrent->bus_namew, pData->bus_namew);
		DrawBUS_Name(lpobject, dc);
	}
	if ((pCurrent->bus_gain != pData->bus_gain) || (fUpdateAll != 0))
	{
		if ((lpobject->mouseCapture == FALSE) || (fUpdateAll != 0))
		{
			pCurrent->bus_gain = pData->bus_gain;
			DrawBUS_Slider(lpobject, dc);
		}
	}
	if ((pCurrent->bus_mute != pData->bus_mute) || (fUpdateMuteDisplay == TRUE) || (fUpdateAll != 0))
	{
		pCurrent->bus_mute = pData->bus_mute;
		DrawBUS_NickNameMute(lpobject, dc);
	}
	if ((pCurrent->bus_monitor != pData->bus_monitor) || (fUpdateAll != 0))
	{
		pCurrent->bus_monitor = pData->bus_monitor;
		DrawBUS_Monitor(lpobject, dc);
	}

	if (dc != NULL)	ReleaseDC(hw,dc);
	
	return 0;
}

long VMSCTL_ResetDataBUS(HWND hw)
{
	long vi;
	LPT_VMSCTL_BUSCTX lpobject;
	lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;	
	memset(&(lpobject->data), 0, sizeof(T_VMSCTL_BUSPARAM));
	for (vi=0;vi<8;vi++) lpobject->rtdata.dbLevel[vi]=-100.0f;
	return 0;
}

long VMSCTL_SetRTDataBUS(HWND hw, LPT_VMSCTL_RT_DATA pRTData)
{
	float fMax;
	HDC dc;
	long nu;
	LPT_VMSCTL_BUSCTX lpobject;
	lpobject=(LPT_VMSCTL_BUSCTX)TOOL_RecallPointerFromWindow(hw);
	if (lpobject == NULL) return -1;	
	if (pRTData == NULL) return -2;
	
	fMax = -100.0f;
	lpobject->nMaxLevelToDisplay--;
	if (lpobject->nMaxLevelToDisplay < 1) lpobject->fMaxLevelToDisplay=fMax;
	//compute value to display, with a release factor
	for (nu=0;nu<2;nu++)	//display 2 channels only
	{
		if (pRTData->dbLevel[nu] > fMax) fMax=pRTData->dbLevel[nu];
		if (pRTData->dbLevel[nu] > lpobject->rtdata.dbLevel[nu]) lpobject->rtdata.dbLevel[nu]=pRTData->dbLevel[nu];
		else 
		{
			lpobject->rtdata.dbLevel[nu] = lpobject->rtdata.dbLevel[nu] - PEAKMETER_RELEASE_DECREMENT;
			if (lpobject->rtdata.dbLevel[nu] < -100.0f) lpobject->rtdata.dbLevel[nu]=-100.0f;
		}
	}
	// Compute max level to display
	if (fMax > lpobject->fMaxLevelToDisplay)
	{
		lpobject->fMaxLevelToDisplay = 	fMax;
		lpobject->nMaxLevelToDisplay = 200;
	}
	//display peak meter.
	if (lpobject->isShown != 0)
	{
		dc=GetDC(hw);
		DrawBUS_PeakMeters(lpobject, dc);
		ReleaseDC(hw,dc);
	}
	
	return 0;
}















/*---------------------------------------------------------------*/
/*                           INIT / END                          */
/*---------------------------------------------------------------*/
long VMSCTL_InitLib(HINSTANCE hinst)
{
	long rep;
	WNDCLASS    wc;

	G_hinstance=hinst;
	
	//register class for our custom Strip
	wc.style		= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;	//allows double click 	  
	wc.lpfnWndProc	=(WNDPROC)VMSCTL_StripCallback;	//Adresse of the related callback
	wc.cbClsExtra	=0;									//0 per default.
	wc.cbWndExtra	=sizeof(void*);						//alloc some byte to store pointer inside window.
	wc.hInstance	=hinst; 	                		//application hinstance.
	wc.hIcon		=NULL;								//no Icon
	wc.hCursor		=LoadCursor(NULL,IDC_ARROW);		//handle on cursor mouse.
	wc.hbrBackground=NULL;								//no background
	wc.lpszMenuName	=NULL;    							//no menu.
	wc.lpszClassName=VMSCTL_CLASSNAME_STRIP;
	rep=RegisterClass(&wc);
	if (rep==0) return -1;

	//register class for our custom BUS
	wc.style		= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;	//allows double click 	  
	wc.lpfnWndProc	=(WNDPROC)VMSCTL_BusCallback;	//Adresse of the related callback
	wc.cbClsExtra	=0;									//0 per default.
	wc.cbWndExtra	=sizeof(void*);						//alloc some byte to store pointer inside window.
	wc.hInstance	=hinst; 	                		//application hinstance.
	wc.hIcon		=NULL;								//no Icon
	wc.hCursor		=LoadCursor(NULL,IDC_ARROW);		//handle on cursor mouse.
	wc.hbrBackground=NULL;								//no background
	wc.lpszMenuName	=NULL;    							//no menu.
	wc.lpszClassName=VMSCTL_CLASSNAME_BUS;
	rep=RegisterClass(&wc);
	if (rep==0) return -1;
	
	return 0;
}

void VMSCTL_EndLib(void)
{
	UnregisterClass(VMSCTL_CLASSNAME_STRIP,G_hinstance);
	UnregisterClass(VMSCTL_CLASSNAME_BUS,G_hinstance);
}

