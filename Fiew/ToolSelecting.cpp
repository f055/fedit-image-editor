#include "stdafx.h"
#include "Core.h"

ToolSelecting::ToolSelecting(FwCHAR *name, HCURSOR cursor, UINT id) : Tool(name,
																		   ((cursor == NULL) ? LoadCursor(NULL,IDC_CROSS) : cursor),
																		   id)
{
	this->rectmargin = 2;

	this->isSelecting = false;
	this->isMoving = false;
	this->isAA = false;

	this->currentPath = NULL;
	this->oldPath = NULL;
}

ToolSelecting::~ToolSelecting()
{

}

void ToolSelecting::activate(){ this->activateDock(); }
void ToolSelecting::deactivate(){ }

void ToolSelecting::beginPaint()
{
	SetCapture(this->chicore->getWindowHandle());
}

void ToolSelecting::endPaint()
{
	ReleaseCapture();
}

void ToolSelecting::capMouseDblClk(WPARAM wParam, LPARAM lParam, int button){ }

void ToolSelecting::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button){ }
void ToolSelecting::capMouseWheel(WPARAM wParam, LPARAM lParam){ }
void ToolSelecting::capMouseDown(WPARAM wParam, LPARAM lParam, int button){ }
void ToolSelecting::capMouseMove(WPARAM wParam, LPARAM lParam){ }
void ToolSelecting::capMouseUp(WPARAM wParam, LPARAM lParam, int button){ }
	
void ToolSelecting::capKeyDown(WPARAM wParam, LPARAM lParam)
{
	int dx, dy;

	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			if( this->isMoving == false && this->isSelecting == false ){
				this->isAA = true;
				this->currentPath = this->chicore->getWorkspace()->getSelection();

				if( this->currentPath != NULL ){
					dx = dy = 0;
					switch(wParam){
						case VK_UP:
							dy = -1;
							break;
						case VK_DOWN:
							dy = 1;
							break;
						case VK_LEFT:
							dx = -1;
							break;
						case VK_RIGHT:
							dx = 1;
							break;
					}
					this->move(dx,dy);
				}
			}
			break;
	}
}
void ToolSelecting::capKeyUp(WPARAM wParam, LPARAM lParam)
{
	int dx, dy;

	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			if( this->isMoving == false && this->isSelecting == false && this->isAA == false ){
				this->currentPath = this->chicore->getWorkspace()->getSelection();

				if( this->currentPath != NULL ){
					dx = dy = 0;
					switch(wParam){
						case VK_UP:
							dy = -1;
							break;
						case VK_DOWN:
							dy = 1;
							break;
						case VK_LEFT:
							dx = -1;
							break;
						case VK_RIGHT:
							dx = 1;
							break;
					}
					this->move(dx,dy);
				}
			}
			this->isAA = false;
			break;
		case VK_DELETE:
			this->core->getGui()->clear();
			break;
	}
}

POINT ToolSelecting::getMousePos(LPARAM lParam)
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

	/*
	mouse.x = max(mouse.x,0);
	mouse.y = max(mouse.y,0);
	mouse.x = min(mouse.x,this->chicore->getWorkspace()->getPxWidth() - 1);
	mouse.y = min(mouse.y,this->chicore->getWorkspace()->getPxHeight() - 1);
	*/

	return mouse;
}

void ToolSelecting::setOldPath()
{
	this->oldPath = NULL;
	
	if( this->chicore->getWorkspace()->getSelection() != NULL ){
		this->oldPath = this->chicore->getWorkspace()->getSelection()->Clone();
	}
}
void ToolSelecting::resetOldPath(bool final)
{
	if( final == true && (this->oldPath != NULL || this->chicore->getWorkspace()->getSelection() != NULL) ){
		HEToolSelecting *he = new HEToolSelecting(
			this->chicore->getWorkspace(),
			this->chicore->getWorkspace()->getSelectedLayer(),
			this
			);
		he->set(this->oldPath,this->chicore->getWorkspace()->getSelection());

		this->chicore->getWorkspace()->getHistory()->add(he);
	}
	if( this->oldPath != NULL )
		delete this->oldPath;
	this->oldPath = NULL;
}

void ToolSelecting::setUpdate(GraphicsPath *path)
{
	((HEToolSelecting *)this->chicore->getWorkspace()->getHistory()->getHistoryElems()->getTail())->setUpdate(path);
}

bool ToolSelecting::getSelectionMoveState(int x, int y)
{
	this->currentPath = this->chicore->getWorkspace()->getSelection();

	if( this->currentPath != NULL && this->isCtrl != true )
		return (bool)this->currentPath->IsVisible( Point(x,y) );
	
	return false;
}

void ToolSelecting::setSelection(GraphicsPath *path, bool final, bool shiftJoin)
{
	if( this->oldPath != NULL && shiftJoin == true ){
		if( path != NULL ){
			path->AddPath(this->oldPath,TRUE);
		}
		else {
			path = this->oldPath->Clone();
		}
		//path->SetFillMode(FillModeWinding);
	}
	//else if( this->isCtrl == false && path == NULL )
	//	this->resetOldPath(false);

	if( final == true && path != NULL ){
		path->Flatten();
		path->Outline();
	}
	this->core->getActiveChild()->getWorkspace()->setSelection(path);
	this->update();
}

void ToolSelecting::setSelection(List<Point> *poly, bool close, bool clip, bool final)
{
	GraphicsPath *path = new GraphicsPath();
	if( close == true ){
		path->AddPolygon(poly->toArray(),poly->getCount());
		path->Flatten();
		path->Outline();
	}
	else {
		path->AddLines(poly->toArray(),poly->getCount());
	}

	this->setSelection(path,final,this->isCtrl);
}

void ToolSelecting::setSelection(RECT rect, int mode, bool final)
{
	GraphicsPath *path = new GraphicsPath();

	if( mode == SELRECT || mode == SELCIRC ){
		SetRect(
			&rect,
			max( min(rect.left,rect.right), 0 ),
			max( min(rect.top,rect.bottom), 0 ),
			min( max(rect.left,rect.right),
				this->core->getActiveChild()->getWorkspace()->getPxWidth() ),
			min( max(rect.top,rect.bottom),
				this->core->getActiveChild()->getWorkspace()->getPxHeight() )
			);
	}
	else if( mode == SELHOR ){
		SetRect(
			&rect,
			0,
			min( max(rect.top,0),
				this->core->getActiveChild()->getWorkspace()->getPxHeight() - 2 ),
			this->chicore->getWorkspace()->getPxWidth(),
			min( max(rect.top,0),
				this->core->getActiveChild()->getWorkspace()->getPxHeight() - 1 ) + 1
			);
	}
	else if( mode == SELVER ){
		SetRect(
			&rect,
			min( max(rect.left,0),
				this->core->getActiveChild()->getWorkspace()->getPxWidth() - 2 ),
			0,
			min( max(rect.left,0),
				this->core->getActiveChild()->getWorkspace()->getPxWidth() - 1 ) + 1,
			this->core->getActiveChild()->getWorkspace()->getPxHeight()
			);
	}
	if( mode == SELRECT ||
		mode == SELHOR  ||
		mode == SELVER  ){

		path->AddRectangle(
			Rect(rect.left,
				 rect.top,
				 rect.right - rect.left,
				 rect.bottom - rect.top) );
	}
	else if( mode == SELCIRC ){
		path->AddEllipse(
			Rect(rect.left,
				 rect.top,
				 rect.right - rect.left,
				 rect.bottom - rect.top) );
	}
	path->Flatten();
	path->Outline();

	this->setSelection(path,final,this->isCtrl);
}

void ToolSelecting::move(int x, int y)
{
	Matrix mx;
	mx.Translate((REAL)x,(REAL)y);

	this->currentPath->Transform(&mx);
	this->update();
}

void ToolSelecting::update()
{
	this->core->getActiveChild()->getDrawer()->invalidate();
}