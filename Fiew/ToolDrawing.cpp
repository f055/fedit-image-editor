#include "stdafx.h"
#include "Core.h"

ToolDrawing::ToolDrawing(FwCHAR *name, HCURSOR cursor, UINT id) : Tool(name,
																	   ((cursor == NULL) ? LoadCursor(NULL,IDC_CROSS) : cursor),
																	   id)
{
	this->brushColor = NULL;
	this->penColor = NULL;

	this->size = 1;
	this->rectmargin = 0;

	this->isDraw = false;
	this->isAA = false;

	this->drawmouse.x = this->drawmouse.y = 0;
	SetRect(&this->drawrect,0,0,0,0);
	this->cliprect = Rect(0,0,0,0);

	this->drawgfx = NULL;
	this->drawbmp = NULL;
	this->sceneAbove = NULL;
	this->sceneBelow = NULL;
	this->sceneBetween = NULL;

	this->fillDock();
}

ToolDrawing::~ToolDrawing()
{
	this->destroy();
}

void ToolDrawing::destroy()
{
	if( this->brushColor != NULL )
		delete this->brushColor;
	this->brushColor = NULL;
	if( this->penColor != NULL )
		delete this->penColor;
	this->penColor = NULL;

	if( this->drawgfx != NULL )
		delete this->drawgfx;
	this->drawgfx = NULL;
	if( this->drawbmp != NULL )
		delete this->drawbmp;
	this->drawbmp = NULL;
	if( this->sceneAbove != NULL )
		delete this->sceneAbove;
	this->sceneAbove = NULL;
	if( this->sceneBelow != NULL )
		delete this->sceneBelow;
	this->sceneBelow = NULL;
	if( this->sceneBetween != NULL )
		delete this->sceneBetween;
	this->sceneBetween = NULL;
}

void ToolDrawing::fillDock()
{
	this->dlgSize = this->core->CreateWindowExSubstituteFont(
		WS_EX_CLIENTEDGE,
		IDCL_EDIT,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
		0,0,30,20,
		this->hdocktool,
		(HMENU)111,this->core->getInstance(),NULL
		);
	Dialogs::limitInputInt(this->dlgSize,1,200);
	SetDlgItemInt(this->hdocktool,GetDlgCtrlID(this->dlgSize),this->size,FALSE);

	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		(this->id == ICC_FILBUC) ? DOCKDLG_PXTOLER : DOCKDLG_SIZE,
		WS_CHILD | WS_VISIBLE | SS_SIMPLE,
		33,3,60,20,
		this->hdocktool,
		NULL,this->core->getInstance(),NULL
		);
	this->dlgSlider = CreateWindowEx(
		NULL,
		IDCL_SLIDER,
		NULL,
		WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_NOTICKS,
		100,2,120,20,
		this->hdocktool,
		(HMENU)333,this->core->getInstance(),NULL
		);
	Dialogs::limitSliderInt(this->dlgSize,this->dlgSlider,1,200,this->size);

	if( this->id != ICC_ERSNOR ){
		this->dlgAA = this->core->CreateWindowExSubstituteFont(
			NULL,
			IDCL_BUTTON,
			DOCKDLG_AA,
			WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			230,0,105,20,
			this->hdocktool,
			(HMENU)222,this->core->getInstance(),NULL
			);
		if( this->isAA == true )
			CheckDlgButton(this->hdocktool,GetDlgCtrlID(this->dlgAA),BST_CHECKED);
		else
			CheckDlgButton(this->hdocktool,GetDlgCtrlID(this->dlgAA),BST_UNCHECKED);
	}
}

void ToolDrawing::notify(int id)
{
	this->size = GetDlgItemInt(this->hdocktool,GetDlgCtrlID(this->dlgSize),NULL,FALSE);
	this->isAA = (bool)IsDlgButtonChecked(this->hdocktool,GetDlgCtrlID(this->dlgAA));
	if( this->size != SendMessage(this->dlgSlider,TBM_GETPOS,NULL,NULL) )
		SendMessage(this->dlgSlider,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)this->size);
}

void ToolDrawing::activate()
{
	this->activateDock();

	SetRect(&this->drawrect,0,0,0,0);
	this->cliprect = Rect(0,0,0,0);

	this->rectmargin = size + 1;
	this->isDraw = false;

	this->notify(NULL);
}
void ToolDrawing::deactivate()
{
	this->destroy();
}

void ToolDrawing::beginPaint()
{
	Color color = this->core->getToolws()->getToolwCC()->getForeColor();

	this->brushColor = new SolidBrush(color);
	this->penColor = new Pen(color,(REAL)this->size);

	double zoom = this->chicore->getDrawer()->getZoom();
	RECT client = this->chicore->getDrawer()->getClientSize();
	Rect scene = this->chicore->getDrawer()->getRectScene();

	if( zoom > ZOOMINIT ){
		client.right += client.right % (int)(zoom);
		client.bottom += client.bottom % (int)(zoom);

		if( scene.X < 0 )
			scene.X -= scene.X % (int)(zoom);
		if( scene.Y < 0 )
			scene.Y -= scene.Y % (int)(zoom);
	}
	int spw = this->chicore->getWorkspace()->getPxWidth();
	int sph = this->chicore->getWorkspace()->getPxHeight();

	int x = (int)abs(min(scene.X / zoom,0));
	int y = (int)abs(min(scene.Y / zoom,0));
	int width = (int)(min(client.right,spw * zoom) / zoom);
	int height = (int)(min(client.bottom,sph * zoom) / zoom);

	this->cliprect = Rect(x,y,width,height);

	this->sceneAbove = this->chicore->getWorkspace()->renderAbove(this->cliprect);
	this->sceneBelow = this->chicore->getWorkspace()->renderBelow(this->cliprect);
	if( this->id == ICC_ERSNOR ){
		this->sceneBetween = this->chicore->getWorkspace()->renderBelow(this->cliprect,(this->id == ICC_ERSNOR));
	}

	this->drawbmp = new Bitmap(this->cliprect.Width,this->cliprect.Height,Core::getPixelFormat());
	this->drawgfx = Graphics::FromImage(this->drawbmp);

	if( this->chicore->getWorkspace()->getSelection() != NULL ){
		GraphicsPath *path = this->chicore->getWorkspace()->getSelection()->Clone();

		Matrix mx;
		mx.Translate((REAL)-this->cliprect.X,(REAL)-this->cliprect.Y);
		path->Transform(&mx);
		path->Outline();

		this->drawgfx->SetClip(path);
		delete path;
	}

	SetRect(&this->drawrect,0,0,0,0);

	if( this->isAA == true )
		this->drawgfx->SetSmoothingMode(SmoothingModeAntiAlias);

	this->setMouseCapture();
}

void ToolDrawing::endPaint()
{
	this->setMouseRelease();

	Rect drawRect = this->getSrcrect(this->size + 2);

	this->chicore->getWorkspace()->getSelectedLayer()->applyPaint(this->drawbmp,this->cliprect,drawRect,this);

	this->core->getToolws()->getToolwBoxLayers()->refreshCurrent();
	this->chicore->getWorkspace()->update();

	this->destroy();
}

void ToolDrawing::update(Rect rect, int mode)
{
	Bitmap *temp = new Bitmap(rect.Width,rect.Height,Core::getPixelFormat());
	Graphics *g = Graphics::FromImage(temp);

	if( mode < INVSCENE ){
		g->DrawImage(
			this->sceneBelow,
			Rect(0,0,rect.Width,rect.Height),
			rect.X,rect.Y,
			rect.Width,rect.Height,
			UnitPixel
			);
	}
	g->DrawImage(
		this->drawbmp,
		Rect(0,0,rect.Width,rect.Height),
		rect.X,rect.Y,
		rect.Width,rect.Height,
		UnitPixel,
		this->chicore->getWorkspace()->getSelectedLayer()->getIatt()
		);
	if( mode < INVSCENE ){
		g->DrawImage(
			this->sceneAbove,
			Rect(0,0,rect.Width,rect.Height),
			rect.X,rect.Y,
			rect.Width,rect.Height,
			UnitPixel
			);
	}

	delete g;
	this->chicore->getDrawer()->invalidate(&rect,temp,mode);
}

void ToolDrawing::capMouseDblClk(WPARAM wParam, LPARAM lParam, int button){ }

void ToolDrawing::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button){ }
void ToolDrawing::capMouseWheel(WPARAM wParam, LPARAM lParam){ }
void ToolDrawing::capMouseDown(WPARAM wParam, LPARAM lParam, int button){ }
void ToolDrawing::capMouseMove(WPARAM wParam, LPARAM lParam){ }
void ToolDrawing::capMouseUp(WPARAM wParam, LPARAM lParam, int button){ }
	
void ToolDrawing::capKeyDown(WPARAM wParam, LPARAM lParam){ }
void ToolDrawing::capKeyUp(WPARAM wParam, LPARAM lParam){ }

Rect ToolDrawing::getDrawrect()
{
	return Rect(
		min(this->drawrect.left,this->drawrect.right),
		min(this->drawrect.top,this->drawrect.bottom),
		max(this->drawrect.left,this->drawrect.right),
		max(this->drawrect.top,this->drawrect.bottom)
		);
}

Rect ToolDrawing::getSrcrect(int margin)
{
	SetRect(
		&this->srcrect,
		min(this->srcrect.left,this->srcrect.right),
		min(this->srcrect.top,this->srcrect.bottom),
		max(this->srcrect.left,this->srcrect.right),
		max(this->srcrect.top,this->srcrect.bottom)
		);
	return Rect(
		this->srcrect.left - margin,
		this->srcrect.top - margin,
		this->srcrect.right - this->srcrect.left + 2 * margin,
		this->srcrect.bottom - this->srcrect.top + 2 * margin);
}

void ToolDrawing::setDrawrectDown(int mode)
{
	switch(mode){
		case STRICTY:
		case INFLATE:
			SetRect(
				&this->drawrect,
				this->mouse.x,
				this->mouse.y,
				this->mouse.x,
				this->mouse.y
				);
			SetRect(
				&this->srcrect,
				this->mouse.x,
				this->mouse.y,
				this->mouse.x,
				this->mouse.y
				);
			break;
	}
}

void ToolDrawing::setDrawrectMove(int mode)
{
	switch(mode){
		case INFLATE:
			SetRect(
				&this->drawrect,
				min(this->drawrect.left,this->mouse.x),
				min(this->drawrect.top,this->mouse.y),
				max(this->drawrect.right,this->mouse.x),
				max(this->drawrect.bottom,this->mouse.y)
				);
			SetRect(
				&this->srcrect,
				min(this->srcrect.left,this->mouse.x),
				min(this->srcrect.top,this->mouse.y),
				max(this->srcrect.right,this->mouse.x),
				max(this->srcrect.bottom,this->mouse.y)
				);
			break;
		case STRICTY:
			SetRect(
				&this->drawrect,
				this->drawrect.left,
				this->drawrect.top,
				this->mouse.x,
				this->mouse.y
				);
			SetRect(
				&this->srcrect,
				this->srcrect.left,
				this->srcrect.top,
				this->mouse.x,
				this->mouse.y
				);
			break;
	}
}

void ToolDrawing::setDrawrectUp(int mode)
{
	switch(mode){
		case INFLATE:
			SetRect(
				&this->drawrect,
				min(this->drawrect.left,this->mouse.x),
				min(this->drawrect.top,this->mouse.y),
				max(this->drawrect.right,this->mouse.x),
				max(this->drawrect.bottom,this->mouse.y)
				);
			SetRect(
				&this->srcrect,
				min(this->srcrect.left,this->mouse.x),
				min(this->srcrect.top,this->mouse.y),
				max(this->srcrect.right,this->mouse.x),
				max(this->srcrect.bottom,this->mouse.y)
				);
			break;
		case STRICTY:
			SetRect(
				&this->drawrect,
				this->drawrect.left,
				this->drawrect.top,
				this->mouse.x,
				this->mouse.y
				);
			SetRect(
				&this->srcrect,
				this->drawrect.left,
				this->drawrect.top,
				this->mouse.x,
				this->mouse.y
				);
			break;
	}
}

POINT ToolDrawing::getMousePos(LPARAM lParam)
{
	POINT mouse = Core::makePoint(lParam);

	Rect scene = this->chicore->getDrawer()->getRectScene();
	double zoom = this->chicore->getDrawer()->getZoom();

	mouse.x -= max(scene.X,0);
	mouse.y -= max(scene.Y,0);

	mouse.x = (int)floor(mouse.x / zoom);
	mouse.y = (int)floor(mouse.y / zoom);

	//scene.X = floor(scene.X / zoom);
	//scene.Y = floor(scene.Y / zoom);

	//mouse.x -= max(scene.X,0);
	//mouse.y -= max(scene.Y,0);

	return mouse;
}

void ToolDrawing::updateInterface(WPARAM wParam, LPARAM lParam)
{
	if( !(wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON )) ){
		this->updateToolws(wParam,lParam);
	}		
}