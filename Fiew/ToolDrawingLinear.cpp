#include "stdafx.h"
#include "Core.h"

ToolDrawingLinear::ToolDrawingLinear(FwCHAR *name, HCURSOR cursor, UINT id) : ToolDrawing(name,cursor,id)
{
	this->drawpath = NULL;

	this->mouseLinear.x = this->mouseLinear.y = 0;
	this->startLinear.x = this->startLinear.y = 0;
}

ToolDrawingLinear::~ToolDrawingLinear()
{
	if( this->drawpath != NULL )
		delete this->drawpath;
}

void ToolDrawingLinear::capMouseDblClk(WPARAM wParam, LPARAM lParam, int button){ }

void ToolDrawingLinear::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button){ }
void ToolDrawingLinear::capMouseWheel(WPARAM wParam, LPARAM lParam){ }
void ToolDrawingLinear::capMouseDown(WPARAM wParam, LPARAM lParam, int button){ }
void ToolDrawingLinear::capMouseMove(WPARAM wParam, LPARAM lParam){ }
void ToolDrawingLinear::capMouseUp(WPARAM wParam, LPARAM lParam, int button){ }
	
void ToolDrawingLinear::capKeyDown(WPARAM wParam, LPARAM lParam){ }
void ToolDrawingLinear::capKeyUp(WPARAM wParam, LPARAM lParam){ }

Rect ToolDrawingLinear::getDrawrect(){ return Rect(0,0,0,0); }

POINT ToolDrawingLinear::getMousePos(LPARAM lParam)
{
	POINT mouse = Core::makePoint(lParam);

	Rect scene = this->chicore->getDrawer()->getRectScene();
	double zoom = this->chicore->getDrawer()->getZoom();

	mouse.x = (int)floor(mouse.x / zoom);
	mouse.y = (int)floor(mouse.y / zoom);

	this->mouseLinear.x = mouse.x;
	this->mouseLinear.y = mouse.y;

	scene.X /= zoom;
	scene.Y /= zoom;

	mouse.x -= max(scene.X,0);
	mouse.y -= max(scene.Y,0);

	this->mouseLinear.x -= scene.X;
	this->mouseLinear.y -= scene.Y;

	return mouse;
}

void ToolDrawingLinear::beginPath()
{
	if( this->drawpath != NULL )
		delete this->drawpath;
	this->drawpath = new GraphicsPath();

	this->chicore->getDrawer()->setDrawpenSize(this->size);
}

void ToolDrawingLinear::updatePath()
{
	GraphicsPath *temp = new GraphicsPath();
	temp->AddPath(this->drawpath,FALSE);

	this->chicore->getDrawer()->setDrawpath(temp);
}

void ToolDrawingLinear::endPath()
{
	if( this->drawpath != NULL )
		delete this->drawpath;
	this->drawpath = NULL;

	this->chicore->getDrawer()->setDrawpath(NULL);
}