#include "stdafx.h"
#include "Core.h"

ToolDrawingStrict::ToolDrawingStrict(FwCHAR *name, HCURSOR cursor, UINT id) : ToolDrawing(name,cursor,id)
{

}

ToolDrawingStrict::~ToolDrawingStrict()
{

}

void ToolDrawingStrict::capMouseDblClk(WPARAM wParam, LPARAM lParam, int button){ }

void ToolDrawingStrict::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button){ }
void ToolDrawingStrict::capMouseWheel(WPARAM wParam, LPARAM lParam){ }
void ToolDrawingStrict::capMouseDown(WPARAM wParam, LPARAM lParam, int button){ }
void ToolDrawingStrict::capMouseMove(WPARAM wParam, LPARAM lParam){ }
void ToolDrawingStrict::capMouseUp(WPARAM wParam, LPARAM lParam, int button){ }
	
void ToolDrawingStrict::capKeyDown(WPARAM wParam, LPARAM lParam){ }
void ToolDrawingStrict::capKeyUp(WPARAM wParam, LPARAM lParam){ }

void ToolDrawingStrict::beginPaint()
{
	Color color = this->core->getToolws()->getToolwCC()->getForeColor();

	this->brushColor = new SolidBrush(color);
	this->penColor = new Pen(color,(REAL)this->size);

	int x = 0;
	int y = 0;
	int width = this->chicore->getWorkspace()->getSelectedLayer()->getWidth();
	int height = this->chicore->getWorkspace()->getSelectedLayer()->getHeight();

	this->cliprect = Rect(x,y,width,height);

	if( this->id == ICC_FILBUC ){
		int x = this->chicore->getWorkspace()->getSelectedLayer()->getX();
		int y = this->chicore->getWorkspace()->getSelectedLayer()->getY();
		int w = this->chicore->getWorkspace()->getPxWidth();
		int h = this->chicore->getWorkspace()->getPxHeight();

		this->cliprect = Rect(min(x,0),min(y,0),w + abs(x),h + abs(y));
	}

	this->sceneAbove = this->chicore->getWorkspace()->renderAbove(this->cliprect);
	this->sceneBelow = this->chicore->getWorkspace()->renderBelow(this->cliprect);

	this->drawbmp = new Bitmap(this->cliprect.Width,this->cliprect.Height,Core::getPixelFormat());
	this->drawgfx = Graphics::FromImage(this->drawbmp);
	SetRect(&this->drawrect,0,0,0,0);

	if( this->isAA == true )
		this->drawgfx->SetSmoothingMode(SmoothingModeAntiAlias);

	this->setMouseCapture();
}

/*void ToolDrawingStrict::endPaint()
{
	this->setMouseRelease();

	this->chicore->getWorkspace()->getSelectedLayer()->applyPaint(this->drawbmp,this->cliprect);
	this->chicore->getWorkspace()->update();

	this->destroy();
}*/

POINT ToolDrawingStrict::getMousePos(LPARAM lParam)
{
	POINT mouse = Core::makePoint(lParam);

	Rect scene = this->chicore->getDrawer()->getRectScene();
	double zoom = this->chicore->getDrawer()->getZoom();

	mouse.x = (int)floor(mouse.x / zoom);
	mouse.y = (int)floor(mouse.y / zoom);

	scene.X /= zoom;
	scene.Y /= zoom;

	mouse.x -= scene.X;
	mouse.y -= scene.Y;

	return mouse;
}