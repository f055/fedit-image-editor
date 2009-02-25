#include "stdafx.h"
#include "Core.h"

ToolSelectRect::ToolSelectRect(int mode) : ToolSelecting(new FwCHAR(TOOL_SELRECT),NULL,ICC_SELREC)
{
	SetRect(&this->selection,0,0,0,0);
	this->mode = mode;

	switch(this->mode){
		case SELCIRC:
			delete this->name;
			this->name = new FwCHAR(TOOL_SELCIRC);
			this->id = ICC_SELCIR;
			break;
		case SELHOR:
			delete this->name;
			this->name = new FwCHAR(TOOL_SELHOR);
			this->id = ICC_SELHOR;
			break;
		case SELVER:
			delete this->name;
			this->name = new FwCHAR(TOOL_SELVER);
			this->id = ICC_SELVER;
			break;
	}
}

ToolSelectRect::~ToolSelectRect()
{

}

void ToolSelectRect::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->beginPaint();

			this->isMoving = this->getSelectionMoveState(this->mouse.x,this->mouse.y);

			this->setOldPath();
			if( this->isMoving == false ){
				this->isSelecting = true;
				

				this->setSelection(NULL);
			}

			SetRect(
				&this->selection,
				this->mouse.x,
				this->mouse.y,
				this->mouse.x,
				this->mouse.y
				);
			break;
	}
}

void ToolSelectRect::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->isMoving == true ){
		this->move(
			this->mouse.x - this->selection.left,
			this->mouse.y - this->selection.top);

		this->selection.left = this->mouse.x;
		this->selection.top = this->mouse.y;
	}
	else if( this->isSelecting == true ){
		this->selection.right = this->mouse.x;
		this->selection.bottom = this->mouse.y;

		if( this->isShift == true ){
			int dx = this->selection.right - this->selection.left;
			int dy = this->selection.bottom - this->selection.top;

			int ds = min(abs(dx),abs(dy));

			if( dx >= 0 && dy >= 0 ){
				this->selection.right = this->selection.left + ds;
				this->selection.bottom = this->selection.top + ds;
			}
			else if( dx >= 0 && dy < 0 ){
				this->selection.right = this->selection.left + ds;
				this->selection.bottom = this->selection.top - ds;
			}
			else if( dx < 0 && dy >= 0 ){
				this->selection.right = this->selection.left - ds;
				this->selection.bottom = this->selection.top + ds;
			}
			else if( dx < 0 && dy < 0 ){
				this->selection.right = this->selection.left - ds;
				this->selection.bottom = this->selection.top - ds;
			}
		}

		this->setSelection(this->selection,this->mode);
	}
}

void ToolSelectRect::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->endPaint();

			if( this->isMoving == true ){
				this->isMoving = false;

				if( this->mouse.x == this->selection.right &&
					this->mouse.y == this->selection.bottom ){
					this->setSelection(NULL,true);
					this->resetOldPath();
				}
				else {
					this->resetOldPath(false);
					this->setUpdate(this->chicore->getWorkspace()->getSelection());
				}
			}
			else if( this->isSelecting == true ){
				this->isSelecting = false;

				if( this->mouse.x == this->selection.left &&
					this->mouse.y == this->selection.top )
					this->setSelection(NULL,true);
				else
					this->setSelection(this->selection,this->mode,true);

				this->resetOldPath();
			}
			break;
	}
}

void ToolSelectRect::setSelectAll()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		this->chicore = child;
		this->setOldPath();

		RECT rect;
		SetRect(
			&rect,
			0,0,
			child->getWorkspace()->getPxWidth(),
			child->getWorkspace()->getPxHeight()
			);
		this->setSelection(rect,true);

		this->resetOldPath();
	}
}
void ToolSelectRect::deselect()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		this->chicore = child;
		this->setOldPath();

		this->setSelection(NULL,true);
		this->chicore->getDrawer()->setDrawpath(NULL);

		if( this->core->getToolws()->getToolwCC()->getTool() ==
			this->core->getToolws()->getToolwCC()->toolCrop ){
			((ToolCrop *)this->core->getToolws()->getToolwCC()->toolCrop)->finalize(false,false);
		}

		this->resetOldPath();
	}
}
void ToolSelectRect::setSelectInverse()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		this->chicore = child;

		this->setOldPath();
		if( this->oldPath != NULL ){
			GraphicsPath *path = new GraphicsPath(FillModeWinding);
			path->AddRectangle(Rect(
				0,0,
				child->getWorkspace()->getPxWidth(),
				child->getWorkspace()->getPxHeight())
			);
			this->oldPath->SetFillMode(FillModeWinding);

			path->AddPath(this->oldPath,TRUE);
			path->Flatten();
			path->Outline();

			this->setSelection(path,true);

			this->resetOldPath();
		}
	}
}