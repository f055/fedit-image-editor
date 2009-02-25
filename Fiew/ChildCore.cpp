/*
ChildCore.cpp
The ChildCore object is the MDI Child window, it contains all neccessary elemnts
to control the presentation of current work.
*/

#include "stdafx.h"
#include "Core.h"
/*
Contructor from a filename
*/
ChildCore::ChildCore(Core *core,
					 FwCHAR *fpath,
					 int type)
{
	Image *temp = new Image(fpath->toWCHAR());

	this->core = core;
	this->windowHandle = NULL;

	this->filepath = fpath;
	this->title = new FwCHAR();
	this->title->getFilenameFrom(this->filepath);

	this->workspace = NULL;
	this->drawer = NULL;

	bool result = false;
	switch(type){
		case TYPE_FED:
			this->workspace = new Workspace(this);
			result = this->workspace->load(this->filepath->toWCHAR());
			break;
		default:
			Bitmap *source = new Bitmap(temp->GetWidth(),temp->GetHeight(),Core::getPixelFormat());

			Graphics *gfx = Graphics::FromImage(source);
			gfx->DrawImage(temp,0,0,temp->GetWidth(),temp->GetHeight());
			delete gfx;
			delete temp;

			this->workspace = new Workspace(this,source);

			result = true;
			break;
	}
	this->constructed = result;

	if( result == true )
		this->reset();
}
/*
Constructor from a bitmap
*/
ChildCore::ChildCore(Core *core, Bitmap *temp, FwCHAR *title)
{
	this->core = core;
	this->windowHandle = NULL;

	if( title != NULL ){
		this->filepath = new FwCHAR(title->toWCHAR());
		this->title = new FwCHAR();
		this->title->getFilenameFrom(this->filepath);
	}
	else {
		this->filepath = new FwCHAR(UNTITLED);
		this->title = new FwCHAR();
	}
	this->title->getFilenameFrom(this->filepath);

	Bitmap *source = new Bitmap(temp->GetWidth(),temp->GetHeight(),Core::getPixelFormat());

	Graphics *gfx = Graphics::FromImage(source);
	gfx->DrawImage(temp,0,0,temp->GetWidth(),temp->GetHeight());
	delete gfx;
	delete temp;

	this->workspace = new Workspace(this,source);

	this->reset();
}
/*
Contructor from a new project
*/
ChildCore::ChildCore(Core *core, 
					 FwCHAR *fname,
					 float width, float height, int sunit, 
					 float res, int runit, 
					 int dmode, int dunit, 
					 int bkgnd)
{
	this->core = core;
	this->windowHandle = NULL;

	this->filepath = new FwCHAR(fname->toWCHAR());
	this->title = fname;

	this->workspace = new Workspace(this,width,height,sunit,res,runit,dmode,dunit,bkgnd);

	this->reset();
}
/*
Reset variables
*/
void ChildCore::reset()
{
	this->constructed = true;
	this->initialized = false;
	this->repaint = true;

	this->isTracked = false;

	this->toolCurrent = NULL;

	this->drawer = new Drawer(this);
}

ChildCore::~ChildCore()
{
	this->destroy();
}

void ChildCore::destroy()
{
	SetProp(this->windowHandle,ATOM_THIS,NULL);

	if( this->filepath != NULL )
		delete this->filepath;
	if( this->title != NULL )
		delete this->title;
	if( this->workspace != NULL )
		delete this->workspace;
	if( this->drawer != NULL )
		delete this->drawer;

	this->toolCurrent = NULL;
}

bool ChildCore::isInitialized()
{
	return this->initialized;
}

bool ChildCore::isConstructed()
{
	return this->constructed;
}
/*
Initializates the child window
*/
bool ChildCore::initialize()
{
	int chiposition = CHIPADDING + CHIPADMULTI * Core::tickChicounter();

	// locate child
	RECT client, screen;
	SetRect(&client,0,0,0,0);
	GetClientRect(this->core->getMdiclientHandle(),&screen);

	client.right = min(this->workspace->getPxWidth(),screen.right - MINCHISIZE);
	client.bottom = min(this->workspace->getPxHeight(),screen.bottom - MINCHISIZE);

	// create child window
	this->windowHandle = 
		CreateMDIWindow(IDCL_MDICHILD,
						NULL,
						WS_VSCROLL | WS_HSCROLL,// | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
						chiposition,chiposition,
						client.right,client.bottom,
						this->core->getMdiclientHandle(),
						this->core->getInstance(),
						NULL);

	if( this->windowHandle != NULL ){
		// assign its object to the window
		SetProp(this->windowHandle,ATOM_THIS,this);

		EnableScrollBar(this->windowHandle,SB_BOTH,ESB_DISABLE_BOTH);

		// resize window in respect to the opened image
		AdjustWindowRectEx(&client,
						   GetWindowLongA(this->windowHandle,GWL_STYLE),
						   FALSE,GetWindowLongA(this->windowHandle,GWL_EXSTYLE));
		client.right -= min(client.left,0) - GetSystemMetrics(SM_CXVSCROLL);
		client.bottom -= min(client.top,0) - GetSystemMetrics(SM_CYHSCROLL);
		MoveWindow(this->windowHandle,
				   chiposition,chiposition,
				   max(client.right,MINCHISIZE),
				   max(client.bottom,MINCHISIZE),
				   TRUE);
		
		// initialize Workspace
		this->workspace->initialize();
		// set the current Tool
		this->setTool( this->core->getToolws()->getToolwCC()->getTool() );

		// enable Tool Windows
		this->core->getToolws()->enableToolwBoxes();

		// set child window text
		this->setText();

		// update application main menu
		this->core->getGui()->updateMenu();

		this->initialized = true;
	}
	return this->initialized;
}

void ChildCore::invalidate(bool repaint)
{
	this->repaint = repaint;
	InvalidateRect(this->windowHandle,NULL,TRUE);
}
/*
Save this child's file or Workspace to a defined destination file
*/
bool ChildCore::save(WCHAR *filepath, int type)
{
	Status result = GenericError;
	this->core->getGui()->setCursor(CURSOR_WAIT);

	switch(type){
		case TYPE_FED:
			// Fedit Document, save the Workspace
			result = (this->workspace->save(filepath) == true) ? Ok : GenericError;
			break;
		default:
			// Gdiplus compatible document, save with Gdiplus
			Bitmap *image = this->workspace->render();

			CLSID encoderClsid;
			EncoderParameters encoderParameters;
			ULONG quality = 100;

			encoderParameters.Count = 1;
			encoderParameters.Count = 1;
			encoderParameters.Parameter[0].Guid = EncoderQuality;
			encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
			encoderParameters.Parameter[0].NumberOfValues = 1;
			encoderParameters.Parameter[0].Value = &quality;

			switch(type){
				case TYPE_JPG:
					Core::getEncoder(ENCJPG,&encoderClsid);
					break;
				case TYPE_PNG:
					Core::getEncoder(ENCPNG,&encoderClsid);
					break;
				case TYPE_GIF:
					Core::getEncoder(ENCGIF,&encoderClsid);
					break;
				case TYPE_TIFF:
					Core::getEncoder(ENCTIF,&encoderClsid);
					break;
				case TYPE_BITMAP:
				case TYPE_ICO:
					Core::getEncoder(ENCBMP,&encoderClsid);
					break;
			}
			result = image->Save(
				filepath,
				&encoderClsid,
				&encoderParameters
				);
			delete image;
			break;
	}
	this->core->getGui()->setCursor();

	if( result == Ok ){
		// prepare new window texts
		if( this->filepath != NULL && this->filepath->toWCHAR() != filepath ){
			delete this->filepath;
			this->filepath = new FwCHAR(filepath);
		}

		if( this->title != NULL )
			delete this->title;
		this->title = new FwCHAR();
		this->title->getFilenameFrom(this->filepath);

		// set the new window titles
		this->setText();

		return true;
	}

	this->core->messageBox_Error(MESSAGE_CANNOTSAVE);

	return false;
}
/*
Set child window text
*/
void ChildCore::setText()
{
	FwCHAR *windowTitle = new FwCHAR(this->title->toWCHAR());

	windowTitle->mergeWith(L" (");

	FwCHAR *w = new FwCHAR(this->workspace->getPxWidth());
	FwCHAR *h = new FwCHAR(this->workspace->getPxHeight());
	windowTitle->mergeWith(w);
	windowTitle->mergeWith(L"x");
	windowTitle->mergeWith(h);
	windowTitle->mergeWith(L" @ ");

	FwCHAR *z = new FwCHAR( (int)(this->drawer->getZoom() * DRZOOMINIT) );
	windowTitle->mergeWith(z);
	windowTitle->mergeWith(L"%)");

	SetWindowText(this->windowHandle,windowTitle->toWCHAR());

	delete windowTitle;
	delete w;
	delete h;
	delete z;
}
/*
Scroll child contents by x,y
*/
void ChildCore::addSkew(int x, int y)
{
	this->drawer->scroll(x,y);
}
/*
Set scroll of child contents to x,y
*/
void ChildCore::setSkew(int x, int y)
{
	this->drawer->scrollSet(x,y);
}
/*
Place the point x,y of child contents at the center of child's client area
*/
void ChildCore::pinSkew(int x, int y)
{
	this->drawer->scrollPinpoint(x,y);
}

void ChildCore::addZoom(double zoom)
{
	this->drawer->zoomer(zoom);
}
void ChildCore::setZoom(double zoom)
{
	this->drawer->zoomSet(zoom);
}

void ChildCore::setScrollBar(SCROLLINFO hor, SCROLLINFO ver)
{
	this->maxrollHor = (hor.nMax - hor.nPage)/2;
	this->maxrollVer = (ver.nMax - ver.nPage)/2;

	if( hor.nMin == 0 && hor.nMax == 0 ){
		//ver.fMask = SIF_DISABLENOSCROLL;
		//SetScrollInfo(this->windowHandle,SB_HORZ,&hor,TRUE);
		EnableScrollBar(this->windowHandle,SB_HORZ,ESB_DISABLE_BOTH);
	}
	else {
		hor.fMask = SIF_ALL;
		SetScrollInfo(this->windowHandle,SB_HORZ,&hor,TRUE);
		EnableScrollBar(this->windowHandle,SB_HORZ,ESB_ENABLE_BOTH);
	}

	if( ver.nMin == 0 && ver.nMax == 0 ){
		//ver.fMask = SIF_DISABLENOSCROLL;
		//SetScrollInfo(this->windowHandle,SB_VERT,&ver,TRUE);
		EnableScrollBar(this->windowHandle,SB_VERT,ESB_DISABLE_BOTH);
	}
	else {
		ver.fMask = SIF_ALL;
		SetScrollInfo(this->windowHandle,SB_VERT,&ver,TRUE);
		EnableScrollBar(this->windowHandle,SB_VERT,ESB_ENABLE_BOTH);
	}
}

void ChildCore::setTool(Tool *tool)
{
	if( this->toolCurrent == tool )
		return;

	if( this->toolCurrent != NULL)
		this->toolCurrent->deactivate();

	this->toolCurrent = tool;

	if( this->toolCurrent != NULL )
		this->toolCurrent->activate();
}
/*
Pass the double click message to the currently selected Tool
*/
void ChildCore::sendToolDblClk(UINT message, WPARAM wParam, LPARAM lParam, int itemId)
{
	if( this->toolCurrent != NULL )
		this->toolCurrent->processMessages(
			this->core,this,this->windowHandle,message,wParam,lParam,NONCLIENT);
}

HWND ChildCore::getWindowHandle()
{
	return this->windowHandle;
}

Interface *ChildCore::getGui()
{
	return NULL;
}

Explorer *ChildCore::getExplorer()
{
	return NULL;
}

Cacher *ChildCore::getCacher()
{
	return NULL;
}

Drawer *ChildCore::getDrawer()
{
	return this->drawer;
}

Workspace *ChildCore::getWorkspace()
{
	return this->workspace;
}

FwCHAR *ChildCore::getFilepath()
{
	return this->filepath;
}
/*
Process child messages
*/
LRESULT CALLBACK ChildCore::processMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ChildCore *that = (ChildCore *)GetProp(hWnd,ATOM_THIS);
	if( that == NULL )
		return DefMDIChildProc(hWnd,message,wParam,lParam);
	
	int dlgresult = NULL;
	PAINTSTRUCT ps;
	SCROLLINFO sih, siv;
	LRESULT result;
	HDC hDc;

	ChildCore *child;

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = hWnd;

	switch (message)
	{
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
		case WM_KEYDOWN:
		case WM_KEYUP:
			// if interface blocked mouse buttons - return
			if( that->core->getGui()->mBB == true )
				return DefMDIChildProc(hWnd,message,wParam,lParam);
			// pass the interface messages to the currently selected Tool
			if( that->toolCurrent != NULL ){
				result = that->toolCurrent->processMessages(
					that->core,that,hWnd,message,wParam,lParam,CLIENT);
				if( message == WM_MOUSEMOVE ){
					that->core->getGui()->setCursor(that->toolCurrent->getCursor());

					// track mouse for the leaving of client area
					if( that->isTracked == false )
						TrackMouseEvent(&tme);
				}
				return result;
			}
			else {
				return DefMDIChildProc(hWnd,message,wParam,lParam);
			}
			if( message == WM_KEYDOWN || message == WM_KEYUP )
				return DefMDIChildProc(hWnd,message,wParam,lParam);

			return DefMDIChildProc(hWnd,message,wParam,lParam);
		case WM_MOUSELEAVE:
			that->core->getGui()->setCursor();
			that->isTracked = false;
			break;

		case WM_VSCROLL:
		case WM_HSCROLL:
			// process window scrolls
			sih.cbSize = sizeof(sih);
			sih.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
			GetScrollInfo(hWnd,SB_HORZ,&sih);

			siv.cbSize = sizeof(siv);
			siv.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
			GetScrollInfo(hWnd,SB_VERT,&siv);

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
						that->setSkew(that->maxrollHor - HIWORD(wParam),that->maxrollVer - siv.nPos);
					else
						that->setSkew(that->maxrollHor - sih.nPos,that->maxrollVer - HIWORD(wParam));
					break;
			}
			break;

		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
			if( that->repaint == true ){
				hDc = BeginPaint(hWnd,&ps);
				that->drawer->paint(hDc);
				EndPaint(hWnd,&ps);
			}
			else {
				that->repaint = true;
			}
			break;
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
			// make any controls on child client transparent
			hDc = (HDC)wParam;
			
			SetBkMode(hDc,TRANSPARENT);
			return (LRESULT)GetStockObject(NULL_BRUSH);
		case WM_SIZE:
			// invalidate the contents upon resize
			that->drawer->invalidate();
			return DefMDIChildProc(hWnd,message,wParam,lParam);
		case WM_MDIACTIVATE:
			// update the interface upon activation of child window
			result = DefMDIChildProc(hWnd,message,wParam,lParam);
			if( (HWND)lParam == hWnd )
				that->workspace->updateToolws();
			return result;
		case WM_CLOSE:
			// process child close - prompt for save, reload interface
			dlgresult = IDNO;
			if( that->getWorkspace()->getHistory()->getHistoryElems()->getCount() > 1 )
				dlgresult = that->core->messageBox_Prompt(MESSAGE_SAVE);

			if( dlgresult == IDCANCEL )
				return 1;
			if( dlgresult == IDYES )
				if( that->core->getGui()->saveFile() == false )
					return 1;

			child = that->core->getChildren()->remove(that);
			that->core->getToolws()->getToolwBoxLayers()->load();
			that->core->getToolws()->getToolwBoxInfo()->disable();
			that->core->getToolws()->getToolwBoxHistory()->load();

			return DefMDIChildProc(hWnd,message,wParam,lParam);
		case WM_DESTROY:
			if( that->core->getChildren()->getCount() <= 0 ){
				that->core->getToolws()->getToolwDock()->hideChild();
				that->core->getGui()->updateMenu();
			}
			delete that;
		default:
			return DefMDIChildProc(hWnd,message,wParam,lParam);
	}
	return 0;
}