/*
Tool.cpp
One of the major objects in Fedit. Tool class is the base class for all
tools that can be choosed from the Tool Control Center and some that can
be choosed from the application main menu.

All of the definitions of Tools are defined in Tool.h - to make reading
clearer all important info about different Tools methods are described there!
*/

#include "stdafx.h"
#include "Core.h"

Tool::Tool(FwCHAR *name, HCURSOR cursor, UINT id)
{
	this->core = Core::self;
	this->chicore = NULL;
	this->name = name;

	this->cursor = cursor;
	this->cursorBackup = cursor;

	this->id = id;

	this->mouse.x = this->mouse.y = 0;

	this->hdocktool = CreateWindowEx(
		NULL,
		IDCL_TOOLW_DKTL,
		name->toWCHAR(),
		WS_CHILD,
		0,0,0,0,
		this->core->getToolws()->getToolwDock()->getWindowHandle(),
		NULL,this->core->getInstance(),NULL
		);
	this->submitDock();
}

Tool::~Tool()
{
	if( this->name != NULL )
		delete this->name;
}

UINT Tool::getId()
{
	return this->id;
}

FwCHAR *Tool::getName()
{
	return this->name;
}

HCURSOR Tool::getCursor()
{
	return this->cursor;
}
void Tool::activate(){ this->activateDock(); }
void Tool::deactivate(){ }
void Tool::notify(int id){ }
void Tool::activateDock()
{
	this->core->getToolws()->getToolwDock()->setChild(this->hdocktool);
}
void Tool::submitDock()
{
	this->core->getToolws()->getToolwDock()->addChild(this->hdocktool,HWND_TOP,this);
}
void Tool::fillDock(){ }
void Tool::updateDock(){ }

void Tool::capKeyDown(WPARAM wParam, LPARAM lParam){ }
void Tool::capKeyUp(WPARAM wParam, LPARAM lParam){ }

void Tool::capMouseDblClk(WPARAM wParam, LPARAM lParam, int button){ }

void Tool::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button){ }
void Tool::capMouseDown(WPARAM wParam, LPARAM lParam, int button){ }
void Tool::capMouseMove(WPARAM wParam, LPARAM lParam){ }
void Tool::capMouseUp(WPARAM wParam, LPARAM lParam, int button){ }
void Tool::capMouseWheel(WPARAM wParam, LPARAM lParam){ }

LRESULT Tool::processMessages(Core *core,
							  ChildCore *chicore,
							  HWND hWnd,
							  UINT message,
							  WPARAM wParam,
							  LPARAM lParam,
							  int area)
{
	this->core = core;
	this->chicore = chicore;

	if( area == CLIENT ){
		switch(message){
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
				this->mouse = this->getMousePos(lParam);
				this->getKeyStatus(wParam);

				this->updateInterface(wParam,lParam);					
				break;
		}
	}
	switch(message){
		case WM_KEYDOWN:
			this->capKeyDown(wParam,lParam);
			break;
		case WM_KEYUP:
			this->capKeyUp(wParam,lParam);
			break;
		case WM_LBUTTONDBLCLK:
			if( area == CLIENT )
				this->capMouseClientDlbClk(wParam,lParam,LEFT);
			else
				this->capMouseDblClk(wParam,lParam,LEFT);
			break;
		case WM_MBUTTONDBLCLK:
			if( area == CLIENT )
				this->capMouseClientDlbClk(wParam,lParam,MID);
			else
				this->capMouseDblClk(wParam,lParam,MID);
			break;
		case WM_RBUTTONDBLCLK:
			if( area == CLIENT )
				this->capMouseClientDlbClk(wParam,lParam,RIGHT);
			else
				this->capMouseDblClk(wParam,lParam,RIGHT);
			break;
		case WM_LBUTTONDOWN:
			this->capMouseDown(wParam,lParam,LEFT);
			break;
		case WM_LBUTTONUP:
			this->capMouseUp(wParam,lParam,LEFT);
			break;
		case WM_MBUTTONDOWN:
			this->capMouseDown(wParam,lParam,MID);
			break;
		case WM_MBUTTONUP:
			this->capMouseUp(wParam,lParam,MID);
			break;
		case WM_RBUTTONDOWN:
			this->capMouseDown(wParam,lParam,RIGHT);
			break;
		case WM_RBUTTONUP:
			this->capMouseUp(wParam,lParam,RIGHT);
			break;
		case WM_MOUSEWHEEL:
			this->capMouseWheel(wParam,lParam);
			break;
		case WM_MOUSEMOVE:
			this->capMouseMove(wParam,lParam);
			break;
	}
	return 0;
}

void Tool::updateInterface(WPARAM wParam, LPARAM lParam)
{
	this->updateToolws(wParam,lParam);
}
void Tool::updateToolws(WPARAM wParam, LPARAM lParam)
{
	POINT pw = Tool::getWorkspaceMousePos(this->chicore,lParam);	
	this->core->getToolws()->getToolwBoxInfo()->loadPos(Point(pw.x,pw.y));

	POINT ps = Core::makePoint(lParam);
	Color color;
	((ToolSampleColor *)this->core->getToolws()->getToolwCC()->toolSampleColor)->getPixel(
		ps,this->chicore->getDrawer()->getBmpScene(),&color);
	this->core->getToolws()->getToolwBoxInfo()->loadColor(color);
}
POINT Tool::getMousePos(LPARAM lParam)
{
	return Core::makePoint(lParam);
}
void Tool::getKeyStatus(WPARAM wParam)
{
	SHORT state;

	state = GetKeyState(VK_SHIFT);
	this->isShift = ( state < 0 );

	state = GetKeyState(VK_CONTROL);
	this->isCtrl = ( state < 0 );

	state = GetKeyState(VK_MENU);
	this->isAlt = ( state < 0 );
}

void Tool::setMouseCapture()
{
	SetCapture(this->chicore->getWindowHandle());
}

void Tool::setMouseRelease()
{
	ReleaseCapture();
}
POINT Tool::getWorkspaceMousePos(ChildCore *chicore, LPARAM lParam)
{
	POINT mouse = Core::makePoint(lParam);

	Rect scene = chicore->getDrawer()->getRectScene();
	double zoom = chicore->getDrawer()->getZoom();

	mouse.x = (int)floor(mouse.x / zoom);
	mouse.y = (int)floor(mouse.y / zoom);

	scene.X /= zoom;
	scene.Y /= zoom;

	mouse.x -= scene.X;
	mouse.y -= scene.Y;

	return mouse;
}
HCURSOR Tool::createToolCursor(int hotXspot, int hotYspot, int toolId)
{
	HCURSOR cursor = NULL;
	HBITMAP hbmp = NULL;
	HBITMAP hmsk = NULL;
	Bitmap *bmp = (Bitmap *)Core::getImageResource(toolId,RC_PNG);

	/*
	Bitmap *msk = bmp->Clone(0,0,bmp->GetWidth(),bmp->GetHeight(),bmp->GetPixelFormat());

	Color color;
	Color pick(255,255,255);
	for( UINT i = 0; i < msk->GetWidth(); i++ ){
		for( UINT j = 0; j < msk->GetHeight(); j++ ){
			msk->GetPixel(i,j,&color);
			if( color.ToCOLORREF() == pick.ToCOLORREF() )
				msk->SetPixel(i,j,NULL);
		}
	}
	msk->GetHBITMAP(NULL,&hmsk);
	*/

	bmp->GetHBITMAP(NULL,&hbmp);

	ICONINFO ii;
	ii.fIcon = FALSE;
	ii.hbmColor = hbmp;
	ii.hbmMask = hbmp;
	ii.xHotspot = hotXspot;
	ii.yHotspot = hotYspot;

	cursor = (HCURSOR)CreateIconIndirect(&ii);

	delete bmp;
	DeleteObject(hbmp);

	/*
	delete msk;
	DeleteObject(hmsk);
	*/

	if( cursor == NULL )
		cursor = LoadCursor(NULL,IDC_ARROW);

	return cursor;
}
bool Tool::isLayerReady()
{
	if( this->chicore->getWorkspace()->getSelectedLayer()->isFrameReady() == false || 
		this->chicore->getWorkspace()->getSelectedLayer()->getType() == FRM_TEXT )
		return false;

	return true;
}

bool Tool::loadCursor()
{
	bool result = this->isLayerReady();

	if( result == false )
		this->cursor = LoadCursor(NULL,IDC_NO);
	else
		this->cursor = this->cursorBackup;

	return result;
}
/*
Fake Tools to store names in the form acceptable by HistoryElements
*/
ToolMerge::ToolMerge() : Tool(new FwCHAR(TOOL_MERGE),NULL,NULL){ }
ToolMerge::~ToolMerge(){ }

ToolRaster::ToolRaster() : Tool(new FwCHAR(TOOL_RASTER),NULL,ICC_TEX){ }
ToolRaster::~ToolRaster(){ }