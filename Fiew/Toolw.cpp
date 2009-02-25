/*
Toolw.cpp
This object represents the interface (in object-oriented programming sense)
of the Tool Windows classes. Contains basic methods and variables.
*/

#include "stdafx.h"
#include "Core.h"

Toolw::Toolw(Core *core, Toolws *controler)
{
	this->core = core;
	this->controler = controler;

	this->hToolw = NULL;
}

Toolw::~Toolw()
{
	this->destroy();
}

void Toolw::destroy()
{

}

HWND Toolw::initialize()
{
	return NULL;
}

HWND Toolw::getWindowHandle()
{
	return this->hToolw;
}

Core *Toolw::getCore()
{
	return this->core;
}

Toolws *Toolw::getControler()
{
	return this->controler;
}

void Toolw::globalSubclass(HWND hDlg)
{
	WNDPROC proc = (WNDPROC)GetWindowLong(hDlg,GWL_WNDPROC);
	SetProp(hDlg,ATOM_PROC,(HANDLE)proc);

	SetWindowLong(hDlg,GWL_WNDPROC,(LONG)Toolw::globalProc);
}

LRESULT CALLBACK Toolw::globalProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	if( proc == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	switch(message){
		case WM_ERASEBKGND:
			return 1;
		case WM_MOUSEMOVE:
			Core::self->getGui()->setCursor(IDC_HAND);
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
}

void Toolw::tooltipSubclass(HWND hDlg, FwCHAR *tip)
{
	WNDPROC proc = (WNDPROC)GetWindowLong(hDlg,GWL_WNDPROC);
	SetProp(hDlg,ATOM_TTPROC,(HANDLE)proc);
	SetProp(hDlg,ATOM_TTL,(HANDLE)tip);

	SetWindowLong(hDlg,GWL_WNDPROC,(LONG)Toolw::tooltipProc);
}

LRESULT CALLBACK Toolw::tooltipProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_TTPROC);
	if( proc == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	FwCHAR *tip = (FwCHAR *)GetProp(hDlg,ATOM_TTL);
	HWND hTip = (HWND)GetProp(hDlg,ATOM_CHILD);
	BOOL track = (BOOL)GetProp(hDlg,ATOM_BOOL);

	RECT client, screen;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = hDlg;

	switch(message){
		case WM_MOUSEMOVE:
			KillTimer(hDlg,TIMER_TIP);
			if( track == FALSE ){
				TrackMouseEvent(&tme);
				SetProp(hDlg,ATOM_BOOL,(HANDLE)TRUE);
			}
			if( hTip == NULL )
				SetTimer(hDlg,TIMER_TIP,TIPTOUT,NULL);
			break;
		case WM_MOUSELEAVE:
			KillTimer(hDlg,TIMER_TIP);
			SetProp(hDlg,ATOM_BOOL,(HANDLE)FALSE);
			if( hTip != NULL ){
				SetProp(hDlg,ATOM_CHILD,NULL);
				DestroyWindow(hTip);
			}
			break;
		case WM_TIMER:
			switch(wParam){
				case TIMER_TIP:
					KillTimer(hDlg,TIMER_TIP);

					if( tip != NULL ){
						SetRect(&screen,0,0,GetDeviceCaps(GetDC(NULL),HORZRES),GetDeviceCaps(GetDC(NULL),VERTRES));
						GetWindowRect(hDlg,&client);

						int width = tip->toLength() * (FONTSIZE_STATIC - 1);
						int height = 2 * FONTSIZE_STATIC;

						client.left += 5;
						client.right = client.left + width;

						client.top = client.bottom + 5;
						client.bottom = client.top + height;

						client.left += min(screen.right - client.right,0);
						client.top += min(screen.bottom - client.bottom,0);

						hTip = CreateWindowEx(
							WS_EX_TOPMOST,
							IDCL_FAKETOOLTIP,
							NULL,
							WS_POPUP | WS_BORDER,
							client.left,
							client.top,
							width,
							height,
							hDlg,
							NULL,
							Core::self->getInstance(),
							tip
							);
						SetProp(hDlg,ATOM_CHILD,hTip);
					}
					return 1;
			}
			break;
	}
	return CallWindowProc(proc,hDlg,message,wParam,lParam);
}

LRESULT CALLBACK Toolw::tooltipLabelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT client;
	FwCHAR *tip = NULL;
	CREATESTRUCT *cs;

	switch(message){
		case WM_CREATE:
			Core::self->getToolws()->getToolwset()->add(hDlg);

			cs = (LPCREATESTRUCT)lParam;
			tip = (FwCHAR *)cs->lpCreateParams;

			GetClientRect(hDlg,&client);

			if( tip != NULL ){
				Core::self->CreateWindowExSubstituteFont(
					NULL,
					IDCL_STATIC,
					tip->toWCHAR(),
					WS_CHILD | WS_VISIBLE | SS_CENTER,
					client.left,
					client.top,
					client.right,
					client.bottom,
					hDlg,
					NULL,
					Core::self->getInstance(),
					NULL
					);

				SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				ShowWindow(hDlg,SW_SHOW);
			}
			else {
				DestroyWindow(hDlg);
			}
			break;
		case WM_CTLCOLORSTATIC:
			SetBkMode((HDC)wParam,TRANSPARENT);
			SetDCBrushColor((HDC)wParam,GetSysColor(COLOR_INFOBK));
			SetTextColor((HDC)wParam,GetSysColor(COLOR_INFOTEXT));
			return (LRESULT)GetStockObject(DC_BRUSH);
		case WM_NCACTIVATE:
			return Core::self->getToolws()->overrideNCActivate(hDlg,wParam,lParam);
		case WM_ENABLE:
			return Core::self->getToolws()->overrideEnable(hDlg,wParam,lParam);
		case WM_DESTROY:
			Core::self->getToolws()->getToolwset()->remove(hDlg);
			break;
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}