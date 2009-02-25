#include "stdafx.h"
#include "Core.h"

ToolLine::ToolLine(int mode) : ToolDrawingLinear(new FwCHAR(TOOL_LINE),NULL,mode)
{
	this->start.x = this->start.y = 0;

	this->prevRect = Rect(0,0,0,0);
	this->drawRect = Rect(0,0,0,0);

	this->rrcRound = 10;
	this->mode = mode;

	switch(mode){
		case ICC_SHAREC:
			delete this->name;
			this->name = new FwCHAR(TOOL_RECT);
			break;
		case ICC_SHACIR:
			delete this->name;
			this->name = new FwCHAR(TOOL_ELLI);
			break;
		case ICC_SHARRC:
			delete this->name;
			this->name = new FwCHAR(TOOL_RREC);
			break;
	}
}

ToolLine::~ToolLine()
{

}

void ToolLine::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isLayerReady() == false )
				return;

			this->isDraw = true;

			this->beginPaint();
			this->beginPath();

			this->setDrawrectDown(STRICTY);
			this->setDrawrect();

			this->start = this->mouse;
			this->startLinear = this->mouseLinear;

			this->drawpath->AddLine(
				this->startLinear.x,
				this->startLinear.y,
				this->startLinear.x,
				this->startLinear.y);
			break;
	}
}
void ToolLine::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	this->loadCursor();

	if( this->isDraw == true ){
		this->setDrawrectMove(STRICTY);
		this->setDrawrect();

		Point point1(
			this->startLinear.x,
			this->startLinear.y
			);
		Point point2(
			this->mouseLinear.x,
			this->mouseLinear.y
			);
		Rect rect(
			min(point1.X,point2.X),
			min(point1.Y,point2.Y),
			abs(point1.X - point2.X),
			abs(point1.Y - point2.Y)
			);

		this->drawpath->Reset();
		switch(this->mode){
			case ICC_SHALIN:
				this->drawpath->AddLine(point1,point2);
				break;
			case ICC_SHAREC:
				this->drawpath->AddRectangle(rect);
				break;
			case ICC_SHACIR:
				this->drawpath->AddEllipse(rect);
				break;
			case ICC_SHARRC:
				this->drawpath->AddEllipse(Rect(
					rect.X,rect.Y,this->rrcRound,this->rrcRound));
				this->drawpath->AddEllipse(Rect(
					rect.X + rect.Width - this->rrcRound,
					rect.Y,this->rrcRound,this->rrcRound));
				this->drawpath->AddEllipse(Rect(
					rect.X,rect.Y + rect.Height - this->rrcRound,
					this->rrcRound,this->rrcRound));
				this->drawpath->AddEllipse(Rect(
					rect.X + rect.Width - this->rrcRound,
					rect.Y + rect.Height - this->rrcRound,
					this->rrcRound,this->rrcRound));
				this->drawpath->AddRectangle(Rect(
					rect.X + this->rrcRound/2,rect.Y,
					rect.Width - this->rrcRound,rect.Height));
				this->drawpath->AddRectangle(Rect(
					rect.X,rect.Y + this->rrcRound/2,
					rect.Width,rect.Height - this->rrcRound));
				//this->drawpath->Flatten();
				this->drawpath->Outline();
				break;
		}

		this->updatePath();
	}
}
void ToolLine::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isDraw == true ){
				this->isDraw = false;

				this->drawLine();
				this->endPath();

				this->setDrawrect();
				this->setDrawrectUp(STRICTY);

				this->endPaint();
			}
			break;
	}
}

void ToolLine::setDrawrect()
{
	int margin = this->size + 1;

	Rect last = this->prevRect;
	RECT temp;
	SetRect(
		&temp,
		max( min(this->drawrect.left,this->drawrect.right) - margin, 0 ),
		max( min(this->drawrect.top,this->drawrect.bottom) - margin, 0 ),
		min( max(this->drawrect.left,this->drawrect.right) + margin,
			this->chicore->getWorkspace()->getPxWidth() ),
		min( max(this->drawrect.top,this->drawrect.bottom) + margin,
			this->chicore->getWorkspace()->getPxHeight() )
		);

	this->prevRect = Rect(
		temp.left,
		temp.top,
		temp.right - temp.left,
		temp.bottom - temp.top
		);

	Rect::Union(this->drawRect,last,this->prevRect);
}

Rect ToolLine::getDrawrect()
{
	return this->drawRect;
}

void ToolLine::drawLine()
{
	Matrix mx;
	mx.Translate(
		(REAL)( this->start.x - this->startLinear.x ),
		(REAL)( this->start.y - this->startLinear.y )
		);
	this->drawpath->Transform(&mx);

	this->drawgfx->DrawPath(this->penColor,this->drawpath);
}