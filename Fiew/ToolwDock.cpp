/*
ToolwDock.cpp
This object represents the docking window. It contains a content pane that
is filled by Tool objects independently of the ToolwDock itself.
This object contains usual docking properties.
*/

#include "stdafx.h"
#include "Core.h"

ToolwDock::ToolwDock(Core *core, Toolws *controler) : Toolw(core,controler)
{
	this->isDocked = false;
	this->isDragged = false;

	this->styleDocked = 
		WS_CHILD;
	this->styleUndocked = 
		WS_POPUP | WS_DLGFRAME;

	this->x = TWPOS_DOCK;
	this->y = TWPOS_DOCK;
	this->width = TW_DOCK_W;
	this->height = TW_DOCK_H;

	this->dockPlace = TOP;
	this->lastPlace = this->dockPlace;

	this->lastMouse.x = 0;
	this->lastMouse.y = 0;

	this->children = new List<HWND__>();
}

ToolwDock::~ToolwDock()
{
	this->destroy();
}

void ToolwDock::destroy()
{

}

HWND ToolwDock::initialize()
{
	this->hToolw = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			IDCL_TOOLW_DOCK,NULL,
			this->styleUndocked,
			this->x,this->y,this->width,this->height,
			this->core->getWindowHandle(),
			NULL,this->core->getInstance(),NULL);

	SetProp(this->hToolw,ATOM_THIS,this);
	SendMessage(this->hToolw,WM_CREATE,NULL,NULL);

	return this->hToolw;
}

void ToolwDock::undock(int place)
{
	RECT client = this->core->getClientSize();

	int width = client.right;// - this->x - TW_DOCK_UDKGRAB;
	if( place == MID )
		width = this->width;


	SetWindowLong(this->hToolw,GWL_STYLE,this->styleUndocked);
    SetParent(this->hToolw,NULL);

	this->setDock(this->x,this->y,width);

	this->dockPlace = place;
	this->isDocked = false;
}

void ToolwDock::dock(int place)
{
	RECT client = this->core->getClientSize();

	SetWindowLong(this->hToolw,GWL_STYLE,this->styleDocked);
	SetParent(this->hToolw,this->core->getWindowHandle());

	if( place == TOP )
		this->setDock(0,0,client.right);
	else if( place == BOT )
		this->setDock(0,client.bottom - TW_DOCK_H,client.right);

	this->dockPlace = place;
	this->isDocked = true;
}

void ToolwDock::toggleDock(int place)
{
	ShowWindow(this->hToolw,SW_HIDE);

	if( this->isDocked == true )
		this->undock(place);
	else
		this->dock(place);

	ShowWindow(this->hToolw,SW_SHOW);
	this->setSiblings();
}
/*
Submit a content to the dock
*/
void ToolwDock::addChild(HWND hWnd, HWND zorder, Tool *owner)
{
	this->setChildSize(hWnd);

	SetWindowPos(
		hWnd,
		zorder,
		0,0,0,0,
		SWP_NOMOVE | SWP_NOSIZE
		);
	SetProp(hWnd,ATOM_THIS,this);
	SetProp(hWnd,ATOM_OWNER,owner);

	this->children->add(hWnd);
}
/*
Set the content size
*/
void ToolwDock::setChildSize(HWND hWnd)
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	SetWindowPos(
		hWnd,
		hWnd,
		TW_DOCK_UDKGRAB + TW_CC_BUTW + TW_DOCK_LEFTMARGIN,
		TW_DOCK_TOPMARGIN,
		client.right - TW_DOCK_UDKGRAB - TW_DOCK_LEFTMARGIN,
		client.bottom - TW_DOCK_BOTMARGIN,
		SWP_FRAMECHANGED | SWP_NOZORDER
		);
}
void ToolwDock::setChildPos(HWND hWnd)
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	SetWindowPos(
		hWnd,
		hWnd,
		TW_DOCK_UDKGRAB + TW_CC_BUTW + TW_DOCK_LEFTMARGIN,
		TW_DOCK_TOPMARGIN + ((this->isDocked == true ) ? 2 : 0),
		NULL,
		NULL,
		SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE
		);
}
/*
Hide all other contents and show only the specified content
*/
void ToolwDock::setChild(HWND hWnd)
{
	bool result = false;

	this->children->gotoHead();
	ShowWindow(this->children->getThat(),SW_HIDE);
	while( this->children->next() == true ){
		if( this->children->getThat() == hWnd ){
			ShowWindow(this->children->getThat(),SW_SHOW);
			result = true;
		}
		else {
			ShowWindow(this->children->getThat(),SW_HIDE);
		}
	}
	if( result == false )
		ShowWindow(this->children->getHead(),SW_SHOW);

	InvalidateRect(this->hToolw,NULL,TRUE);
}
/*
Hide all contents
*/
void ToolwDock::hideChild()
{
	this->children->gotoHead();
	ShowWindow(this->children->getThat(),SW_SHOW);
	while( this->children->next() == true ){
		ShowWindow(this->children->getThat(),SW_HIDE);
	}
}

void ToolwDock::setSiblings()
{
	this->children->gotoHead();
	do {
		this->setChildPos(this->children->getThat());
	} while( this->children->next() == true );
	//SendMessage(this->core->getMdiclientHandle(),WM_SIZE,NULL,NULL);
	this->processFrame(this->core->getMdiclientHandle());
}

bool ToolwDock::addPos(int x, int y)
{
	if( x == 0 && y == 0 )
		return false;

	if( this->isDocked == false ){
		this->x += x;
		this->y += y;

		this->setDock();
		return true;
	}
	return false;
}

void ToolwDock::setPos(int x, int y)
{
	this->x = x;
	this->y = y;

	this->setDock();
}

void ToolwDock::setSize(int w, int h)
{
	this->width = w;
	this->height = h;

	this->setDock();
}

void ToolwDock::setDock(int x, int y, int w, int h)
{
	this->x = x;
	this->y = y;
	this->width = w;
	this->height = h;

	this->setDock();
}

void ToolwDock::setDock()
{
	SetWindowPos(
		this->hToolw,
		NULL,
		this->x,
		this->y,
		this->width,
		this->height,
		SWP_FRAMECHANGED | SWP_NOZORDER
		);
}

void ToolwDock::paint(HDC hdc)
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	Graphics *g = Graphics::FromHDC(hdc);

	if( this->isDocked == true || this->dockPlace == MID ){
		if( this->dockPlace == TOP || this->dockPlace == BOT ){
			DrawEdge(hdc,&client,EDGE_ETCHED,BF_TOP);
			if( this->dockPlace == TOP )
				DrawEdge(hdc,&client,EDGE_RAISED,BF_BOTTOM);
		}

		if( this->dockPlace != MID ){
			client.top += 3;
			client.bottom -= 3;
		}
		client.left += 2;
		client.right = 5;
		DrawEdge(hdc,&client,BDR_RAISEDINNER,BF_RECT);

		client.left += 5;
		client.right = TW_DOCK_DKGRAB;
		DrawEdge(hdc,&client,BDR_RAISEDINNER,BF_RECT);
	}
	else {
		Color cLgrd = CLR_CAPTION_ACTIV;
		Color cRgrd = CLR_CAPTION_GRAD;
		
		Rect rect = Rect(
			client.left,
			client.top,
			TW_DOCK_UDKGRAB,
			client.bottom);

		LinearGradientBrush *lgb =
			new LinearGradientBrush(rect,cRgrd,cLgrd,LinearGradientModeVertical);

		g->FillRectangle(lgb,rect);
		delete lgb;
	}
	Tool *tool = this->controler->getToolwCC()->getTool();
	if( tool != NULL ){
		Image *icon = Core::getImageResource(tool->getId(),RC_PNG);
		g->DrawImage(
			icon,
			TW_DOCK_UDKGRAB,
			5 + ((this->isDocked == true) ? 2 : 0),
			icon->GetWidth(),
			icon->GetHeight()
			);
		delete icon;
	}

	delete g;
}

int ToolwDock::getWidth()
{
	return this->width;
}

int ToolwDock::getHeight()
{
	return this->height;
}

int ToolwDock::getDockedHeight()
{
	if( this->isDocked == true )
		return this->height;
	return 0;
}

int ToolwDock::getDockState()
{
	if( this->isDocked == true )
		return this->dockPlace;
	return 0;
}

void ToolwDock::capMouseDown(POINTS p, UINT button)
{
	if( button == WM_LBUTTONDOWN ){
		RECT frame;
		GetWindowRect(this->core->getMdiclientHandle(),&frame);

		if( this->isDocked == true ){
			if( p.x >= 0 && p.x <= TW_DOCK_DKGRAB ){
				SetCapture(this->hToolw);

				this->isDragged = true;
				this->lastMouse = p;

				this->x = frame.left;
				this->y = frame.top - this->height;

				this->lastPlace = this->dockPlace;
				this->toggleDock(MID);
			}
		}
		else {
			if( p.x >= 0 && p.x <= TW_DOCK_UDKGRAB ){
				SetCapture(this->hToolw);

				this->isDragged = true;
				this->lastMouse = p;
			}
		}
	}
}

void ToolwDock::capMouseMove(POINTS p)
{
	if( this->isDragged == true ){
		RECT docker;
		GetWindowRect(this->hToolw,&docker);

		if( this->addPos(p.x - this->lastMouse.x,p.y - this->lastMouse.y) == true )
			this->core->redrawAll(&docker);
	}
}

void ToolwDock::capMouseUp(POINTS p, UINT button)
{
	if( this->isDragged == true ){
		ReleaseCapture();
		this->isDragged = false;

		RECT frame;
		GetWindowRect(this->core->getMdiclientHandle(),&frame);	

		int dragTx = abs(frame.left - this->x);
		int dragTy = abs(frame.top - this->y);
		int dragTyBot = abs(frame.bottom - this->y - this->height);

		if( dragTx < TW_DOCK_UDKGRAB &&
			dragTy < TW_DOCK_UDKGRAB ){
			this->toggleDock(TOP);
		}
		else if( dragTx < TW_DOCK_UDKGRAB &&
				 dragTyBot < TW_DOCK_UDKGRAB ){

			this->toggleDock(BOT);
		}
		else if( this->dockPlace == MID ){
			this->undock();
			ShowWindow(this->hToolw,SW_SHOW);
			this->core->redrawAll(NULL);
		}
		else {
			this->core->redrawAll(NULL);
		}
	}
}

int ToolwDock::processFrame(HWND hWnd)
{
	RECT client;
	GetClientRect(this->core->getWindowHandle(),&client);

	int dockState = this->getDockState();
	int dockHeight = this->getDockedHeight();

	int mdiY = dockHeight;
	int mdiH = dockHeight;
	if( dockState == BOT )
		mdiY = 0;

	SetWindowPos(
		hWnd,
		NULL,
		0,
		mdiY,
		client.right,
		client.bottom - mdiH,
		SWP_FRAMECHANGED | SWP_NOZORDER);

	if( dockState != 0 )
		this->setSize(client.right);
	if( dockState == BOT )
		this->setPos(0,client.bottom - mdiH);

	SendMessage(this->core->getMdiclientHandle(),WM_SIZING,NULL,NULL);

	return 0;
}

LRESULT CALLBACK ToolwDock::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwDock *that = (ToolwDock *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	PAINTSTRUCT ps;
	HWND hwnd;
	HDC hdc;

	switch(message)
	{
		case WM_CREATE:
			hwnd = that->core->CreateWindowExSubstituteFont(
				NULL,
				IDCL_STATIC,
				MESSAGE_NODOCK,
				WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
				0,0,0,0,
				that->hToolw,
				NULL,that->core->getInstance(),NULL);

			that->addChild(hwnd,HWND_BOTTOM);
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			that->capMouseDown(MAKEPOINTS(lParam),message);
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			that->capMouseUp(MAKEPOINTS(lParam),message);
			break;
		case WM_MOUSEMOVE:
			that->capMouseMove(MAKEPOINTS(lParam));
			break;
		case WM_PAINT:
			hdc = BeginPaint(hDlg,&ps);
			that->paint(hdc);
			EndPaint(hDlg,&ps);
			return DefWindowProc(hDlg,message,wParam,lParam);
		case WM_NCACTIVATE:
			return that->controler->overrideNCActivate(hDlg,wParam,lParam);
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Pass the WM_COMMANDS to the owner of the contents
*/
LRESULT CALLBACK ToolwDock::processTools(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwDock *that = (ToolwDock *)GetProp(hDlg,ATOM_THIS);
	Tool *owner = (Tool *)GetProp(hDlg,ATOM_OWNER);

	if( that == NULL || owner == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	switch(message)
	{
		case WM_CREATE:
			break;
		case WM_COMMAND:
			switch(HIWORD(wParam)){
				case CBN_SELCHANGE:
				case BN_CLICKED:
				case BN_DBLCLK:
				case EN_CHANGE:
					owner->notify(LOWORD(wParam));
					break;
			}
			break;
		case WM_DRAWITEM:
			Dialogs::paintButton( (LPDRAWITEMSTRUCT)lParam );
			break;
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}
