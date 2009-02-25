/*
ToolwBoxScroll.cpp
This object represents a scrollable content pane. It contains another
content pane inside itself that can be larger than ToolwBoxScroll and
ToolwBoxScroll displays scroll bars and enables scrolling through the
contents of the inside.
This content pane is autonomous and can be used anywhere.
*/

#include "stdafx.h"
#include "Core.h"

ToolwBoxScroll::ToolwBoxScroll(HWND hOwner, int x, int y, int w, int h, bool hor, bool ver)
{
	this->scrollHor = this->scrollVer = NULL;
	this->scrollHorMax = this->scrollVerMax = NULL;

	DWORD style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
	if( (this->isScrollHor = hor) == true )
		style = style | WS_HSCROLL;
	if( (this->isScrollVer = ver) == true )
		style = style | WS_VSCROLL;

	this->hContent = NULL;
	this->hOldContent = NULL;

	this->hToolw = CreateWindowEx(
		NULL,
		IDCL_TOOLW_BXSCRL,
		NULL,
		style,
		x,y,w,h,
		hOwner,
		NULL,
		Core::self->getInstance(),
		NULL
		);
	//EnableScrollBar(this->hToolw,SB_HORZ,ESB_DISABLE_BOTH);
	//EnableScrollBar(this->hToolw,SB_VERT,ESB_DISABLE_BOTH);

	SetProp(this->hToolw,ATOM_THIS,this);

	this->locate();
}

ToolwBoxScroll::~ToolwBoxScroll()
{

}
/*
Create new content pane that will be scrolled
owner -		parent control of the inside content
w -			width of the content
h -			height of the content
hs -		horizontal scroll initial position
vs -		vertical scroll initial position
hss -		show horizontal scroll flag
vss -		show vertical scroll flag
*/
HWND ToolwBoxScroll::createContent(HANDLE owner, int w, int h, int hs, int vs, bool hss, bool vss)
{
	RECT client = this->getBoxRect();

	if( w == NULL )
		w = client.right;
	if( h == NULL )
		h = client.bottom;

	this->hContent = CreateWindowEx(
		NULL,
		IDCL_TOOLW_BXSCRLCNT,
		NULL,
		WS_CHILD | /*WS_VISIBLE | */WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0,0,w,h,
		this->hToolw,
		NULL,
		Core::self->getInstance(),
		NULL
		);
	SetProp(this->hContent,ATOM_OWNER,owner);

	this->locate(hs,vs,hss,vss);

	return this->hContent;
}
/*
Destroy the scrollable content
*/
HWND ToolwBoxScroll::clearContent()
{
	if( this->hContent != NULL )
		DestroyWindow(this->hContent);
	this->hContent = NULL;

	this->locate();

	return this->hContent;
}
/*
Disconnect the current content from main content handle
*/
HWND ToolwBoxScroll::unplugContent()
{
	if( this->hOldContent != NULL )
		DestroyWindow(this->hOldContent);

	this->hOldContent = this->hContent;
	this->hContent = NULL;

	return this->hContent;
}
/*
Connect and show new content, destroy old content
*/
HWND ToolwBoxScroll::plugContent()
{
	ShowWindow(this->hContent,SW_SHOW);

	if( this->hOldContent != NULL )
		DestroyWindow(this->hOldContent);
	this->hOldContent = NULL;

	return this->hContent;
}

HWND ToolwBoxScroll::getWindowHandle()
{
	return this->hToolw;
}

SCROLLINFO ToolwBoxScroll::getScrollHor()
{
	RECT client = this->getBoxRect();
	SCROLLINFO info;

	info.cbSize = sizeof(info);
	info.nMin = 0;
	info.nMax = 2 * this->scrollHorMax;
	if( info.nMax != 0 )
		info.nMax += client.right;
	info.nPos = abs(this->scrollHor - this->scrollHorMax);
	info.nPage = client.right;

	return info;
}

SCROLLINFO ToolwBoxScroll::getScrollVer()
{
	RECT client = this->getBoxRect();
	SCROLLINFO info;

	info.cbSize = sizeof(info);
	info.nMin = 0;
	info.nMax = 2 * this->scrollVerMax;
	if( info.nMax != 0 )
		info.nMax += client.bottom;
	info.nPos = abs(this->scrollVer - this->scrollVerMax);
	info.nPage = client.bottom;

	return info;
}

void ToolwBoxScroll::setScrollBar()
{
	SCROLLINFO hor = this->getScrollHor();
	SCROLLINFO ver = this->getScrollVer();

	this->rollHor = (hor.nMax - hor.nPage)/2;
	this->rollVer = (ver.nMax - ver.nPage)/2;

	if( this->isScrollHor == true ){
		if( hor.nMin == 0 && hor.nMax == 0 ){
			ver.fMask = SIF_DISABLENOSCROLL;
			SetScrollInfo(this->hToolw,SB_HORZ,&hor,TRUE);
			EnableScrollBar(this->hToolw,SB_HORZ,ESB_DISABLE_BOTH);
		}
		else {
			hor.fMask = SIF_ALL;
			SetScrollInfo(this->hToolw,SB_HORZ,&hor,TRUE);
			EnableScrollBar(this->hToolw,SB_HORZ,ESB_ENABLE_BOTH);
		}
	}

	if( this->isScrollVer == true ){
		if( ver.nMin == 0 && ver.nMax == 0 ){
			ver.fMask = SIF_DISABLENOSCROLL;
			SetScrollInfo(this->hToolw,SB_VERT,&ver,TRUE);
			EnableScrollBar(this->hToolw,SB_VERT,ESB_DISABLE_BOTH);
		}
		else {
			ver.fMask = SIF_ALL;
			SetScrollInfo(this->hToolw,SB_VERT,&ver,TRUE);
			EnableScrollBar(this->hToolw,SB_VERT,ESB_ENABLE_BOTH);
		}
	}
}

void ToolwBoxScroll::setContentPos()
{
	SetWindowPos(
		this->hContent,
		this->hContent,
		this->scrollHor - this->scrollHorMax,
		this->scrollVer - this->scrollVerMax,
		NULL,NULL,
		SWP_FRAMECHANGED |
		SWP_NOZORDER |
		SWP_NOSIZE
		);
}

void ToolwBoxScroll::addSkew(int x, int y)
{
	this->scrollHor += x;
	this->scrollVer += y;
	this->boundRoll();
	this->setContentPos();
	this->setScrollBar();
}

void ToolwBoxScroll::setSkew(int x, int y)
{
	this->scrollHor = x;
	this->scrollVer = y;
	this->boundRoll();
	this->setContentPos();
	this->setScrollBar();
}

void ToolwBoxScroll::locate(int hs, int vs, bool hss, bool vss)
{
	RECT content = this->getContentRect();
	RECT client = this->getBoxRect();

	double hodiff = (client.right - content.right) / 2.0;
	double vediff = (client.bottom - content.bottom) / 2.0;

	this->scrollHorMax = (int)abs( min(floor(hodiff),0) );
	this->scrollVerMax = (int)abs( min(floor(vediff),0) );

	if( hss == true )
		this->scrollHor = this->scrollHorMax + hs;
	if( vss == true )
		this->scrollVer = this->scrollVerMax + vs;

	this->boundRoll();
	this->setContentPos();
	this->setScrollBar();
}

void ToolwBoxScroll::boundRoll()
{
	this->scrollHor = min(this->scrollHor,this->scrollHorMax);
	this->scrollHor = max(this->scrollHor,-this->scrollHorMax);

	this->scrollVer = min(this->scrollVer,this->scrollVerMax);
	this->scrollVer = max(this->scrollVer,-this->scrollVerMax);
}

void ToolwBoxScroll::resizeContent()
{
	RECT client = this->getBoxRect();
	RECT content = this->getContentRect();

	if( this->isScrollHor == false )
		content.right = client.right;
	if( this->isScrollVer == false )
		content.bottom = client.bottom;

	SetWindowPos(
		this->hContent,
		this->hContent,
		NULL,NULL,
		content.right,content.bottom,
		SWP_FRAMECHANGED |
		SWP_NOZORDER |
		SWP_NOMOVE
		);

	this->locate();
}

RECT ToolwBoxScroll::getContentRect()
{
	RECT content;
	if( this->hContent != NULL ){
		GetWindowRect(this->hContent,&content);

		content.right -= content.left;
		content.bottom -= content.top;

		content.left = 0;
		content.top = 0;
	}
	else {
		SetRect(&content,0,0,0,0);
	}
	return content;
}

RECT ToolwBoxScroll::getBoxRect()
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	return client;
}

int ToolwBoxScroll::getScrollHorMax()
{
	return this->scrollHorMax;
}

int ToolwBoxScroll::getScrollVerMax()
{
	return this->scrollVerMax;
}


LRESULT CALLBACK ToolwBoxScroll::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwBoxScroll *that = (ToolwBoxScroll *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	SCROLLINFO sih, siv;

	switch(message){
		case WM_SIZE:
			that->resizeContent();
			break;
		case WM_VSCROLL:
		case WM_HSCROLL:
			sih.cbSize = sizeof(sih);
			sih.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
			GetScrollInfo(hDlg,SB_HORZ,&sih);

			siv.cbSize = sizeof(siv);
			siv.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
			GetScrollInfo(hDlg,SB_VERT,&siv);

			switch (LOWORD(wParam)) 
			{ 
				case SB_PAGEUP:
					if( message == WM_HSCROLL )
						that->addSkew(sih.nPage,0);
					else
						that->addSkew(0,siv.nPage);
					break; 
				case SB_PAGEDOWN: 
					if( message == WM_HSCROLL )
						that->addSkew(-(int)sih.nPage,0);
					else
						that->addSkew(0,-(int)siv.nPage);
					break;
				case SB_LINEUP: 
					if( message == WM_HSCROLL )
						that->addSkew(SCROLLSTEP,0);
					else
						that->addSkew(0,SCROLLSTEP);
					break;
				case SB_LINEDOWN: 
					if( message == WM_HSCROLL )
						that->addSkew(-SCROLLSTEP,0);
					else
						that->addSkew(0,-SCROLLSTEP);
					break;
				case SB_THUMBTRACK:
					if( message == WM_HSCROLL )
						that->setSkew(that->rollHor - HIWORD(wParam),that->rollVer - siv.nPos);
					else
						that->setSkew(that->rollHor - sih.nPos,that->rollVer - HIWORD(wParam));
					break;
			}
			break;
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Pass any messages received by the scrollable content to the parent of that content
*/
LRESULT CALLBACK ToolwBoxScroll::processContents(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwBoxContent *tbc = (ToolwBoxContent *)GetProp(hDlg,ATOM_OWNER);
	if( tbc == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);
	else
		return tbc->processMessages(hDlg,message,wParam,lParam);

	return 0;
}