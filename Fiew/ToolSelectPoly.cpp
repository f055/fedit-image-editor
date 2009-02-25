#include "stdafx.h"
#include "Core.h"

ToolSelectPoly::ToolSelectPoly(int mode) : ToolSelecting(new FwCHAR(TOOL_SELPOLY),NULL,mode)
{
	this->poly = NULL;

	this->mode = mode;
	switch(this->mode){
		case ICC_LASMOS:
			delete this->name;
			this->name = new FwCHAR(TOOL_SELMOS);
			break;
	}
}

ToolSelectPoly::~ToolSelectPoly()
{
	if( this->poly != NULL )
		delete this->poly;
}

void ToolSelectPoly::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->endPoly();
			break;
	}
}

void ToolSelectPoly::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isSelecting == false ){
				this->beginPaint();

				if( this->poly != NULL )
					delete this->poly;
				this->poly = new List<Point>();

				this->isMoving = this->getSelectionMoveState(this->mouse.x,this->mouse.y);

				if( this->isMoving == false ){
					this->isSelecting = true;
					this->setOldPath();

					this->setSelection(NULL);
				}
				this->poly->add( new Point(this->mouse.x,this->mouse.y) );
			}
			if( this->isSelecting == true ){
				if( this->isEnd() == true && poly->getCount() > 2 ){
					this->endPoly();
					return;
				}
			}
			this->poly->add( new Point(this->mouse.x,this->mouse.y) );

			break;
	}
}

void ToolSelectPoly::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->isMoving == true ){
		this->move(
			this->mouse.x - this->poly->getTail()->X,
			this->mouse.y - this->poly->getTail()->Y);

		this->poly->getTail()->X = this->mouse.x;
		this->poly->getTail()->Y = this->mouse.y;
	}
	else if( this->isSelecting == true ){
		if( this->mode == ICC_LASMOS ){
			this->poly->add(new Point(this->mouse.x,this->mouse.y));
		}
		else {
			this->poly->getTail()->X = this->mouse.x;
			this->poly->getTail()->Y = this->mouse.y;
		}

		this->setSelection(this->poly,false,false);
	}
}

void ToolSelectPoly::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isMoving == true ){
				this->endPaint();

				this->isMoving = false;

				if( this->mouse.x == this->poly->getHead()->X &&
					this->mouse.y == this->poly->getHead()->Y )
					this->setSelection(NULL,true);

				this->setUpdate(this->chicore->getWorkspace()->getSelection());

				delete this->poly;
				this->poly = NULL;
			}
			else if( this->isSelecting == true ){
				if( this->mode == ICC_LASMOS ){
					this->endPoly();
				}
				else {
					if( this->isEnd() == true && poly->getCount() > 2 ){
						this->endPoly();
					}
				}
			}
			break;
	}
}

void ToolSelectPoly::endPoly()
{
	this->endPaint();

	this->isSelecting = false;

	if( this->poly != NULL ){
		if( this->poly->getCount() > 3 ){
			if( this->mode == ICC_LASMOS ){
				delete this->poly->removeTail();
				this->setSelection(this->poly,true,true,true);
			}
			else {
				GraphicsPath *path = new GraphicsPath();
				path->AddLines(this->poly->toArray(),this->poly->getCount());
				path->Outline();
				this->setSelection(path,true,this->isCtrl);
			}
		}
	}
	else {
		this->setSelection(NULL,true);
	}

	this->resetOldPath();

	delete this->poly;
	this->poly = NULL;
}

bool ToolSelectPoly::isEnd()
{
	int margin = 5;
	Point *head = this->poly->getHead();

	if( this->mouse.x > head->X - margin &&
		this->mouse.x < head->X + margin &&
		this->mouse.y > head->Y - margin &&
		this->mouse.y < head->Y + margin )
		return true;

	return false;
}