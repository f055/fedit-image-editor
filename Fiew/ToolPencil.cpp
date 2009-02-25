#include "stdafx.h"
#include "Core.h"

ToolPencil::ToolPencil(int mode) : ToolDrawing(
									(mode == PENPEN) ? (new FwCHAR(TOOL_PENCIL)) : (new FwCHAR(TOOL_ERASER)),
									NULL,
									(mode == PENPEN) ? ICC_DRAPEN : ICC_ERSNOR
									)
{
	this->curve = NULL;
	this->counter = NULL;

	this->mode = mode;

	this->cursor = Tool::createToolCursor(5,17,(mode == PENPEN) ? ICC_DRAPEN : ICC_ERSNOR);
	this->cursorBackup = this->cursor;
}

ToolPencil::~ToolPencil()
{
	if( this->curve != NULL )
		delete this->curve;

	DestroyIcon(this->cursorBackup);
}

void ToolPencil::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isLayerReady() == false )
				return;

			this->isDraw = true;
			this->counter = 0;

			this->beginPaint();
			this->penColor->SetLineCap(LineCapRound,LineCapRound,DashCapFlat);
			this->penColor->SetLineJoin(LineJoinRound);

			Rect rect(
				this->mouse.x - (int)(this->size/2.0),
				this->mouse.y - (int)(this->size/2.0),
				this->size,
				this->size
				);

			switch(this->mode){
				case PENPEN:
					if( this->size > 1 )
						this->drawgfx->FillEllipse(this->brushColor,rect);
					else
						this->drawgfx->FillRectangle(this->brushColor,rect);
					break;
				case PENRUB:
					GraphicsPath *path = new GraphicsPath();
					if( this->size > 1 )
						path->AddEllipse(rect);
					else
						path->AddRectangle(rect);
					this->drawgfx->SetClip(path);
					this->drawgfx->DrawImage(this->sceneBetween,0,0);
					this->drawgfx->DrawImage(this->sceneAbove,0,0);
					this->drawgfx->ResetClip();
					delete path;
					break;
			}

			this->setDrawrectDown();

			this->curve = new List<PointF>();
			this->curve->add( new PointF((REAL)this->mouse.x,(REAL)this->mouse.y) );

			this->update(this->getDrawrect());
			break;
	}
}

void ToolPencil::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	this->loadCursor();

	if( this->isDraw == true ){
		this->curve->add( new PointF((REAL)this->mouse.x,(REAL)this->mouse.y) );

		int last = 3;
		this->counter = (this->counter + 1) % 5;

		PointF *ptfar = this->curve->toArray(last);

		switch(this->mode){
			case PENPEN:
				this->drawgfx->DrawLines(
					this->penColor,
					ptfar,
					last
					);
				break;
			case PENRUB:
				GraphicsPath *path = new GraphicsPath();
				path->AddLines(ptfar,last);
				path->Widen(this->penColor);
				this->drawgfx->SetClip(path);
				this->drawgfx->DrawImage(this->sceneBetween,0,0);
				this->drawgfx->DrawImage(this->sceneAbove,0,0);
				this->drawgfx->ResetClip();
				delete path;
				break;
		}
		delete [] ptfar;

		this->setDrawrectMove();

		this->update(this->getDrawrect());

		if( this->counter == 0 )
			SetRect(
				&this->drawrect,
				this->mouse.x,
				this->mouse.y,
				this->mouse.x,
				this->mouse.y);
	}
}

void ToolPencil::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isDraw == true ){
				this->isDraw = false;
				/*this->curve->add( new PointF(this->mouse.x,this->mouse.y) );*/
				/*this->drawgfx->DrawLines(
					this->penColor,
					this->curve->toArray(),
					this->curve->getCount()
					);*/
				this->setDrawrectUp();

				switch(this->mode){
					case PENPEN:
						this->endPaint();
						break;
					case PENRUB:
						Rect rect(
							this->mouse.x - (int)(this->size/2.0),
							this->mouse.y - (int)(this->size/2.0),
							this->size,
							this->size
							);

						this->setMouseRelease();						

						GraphicsPath *path = new GraphicsPath();

						if( this->curve->getCount() > 1 ){
							PointF *ptfar = this->curve->toArray();
							path->AddLines(ptfar,this->curve->getCount());
							path->Widen(this->penColor);
							delete [] ptfar;
						}
						if( this->size > 1 )
							path->AddEllipse(rect);
						else
							path->AddRectangle(rect);

						Matrix mx;
						mx.Translate((REAL)this->cliprect.X,(REAL)this->cliprect.Y);
						path->Transform(&mx);
						
						this->chicore->getWorkspace()->getSelectedLayer()->applyClear(path,this);
						
						delete path;

						this->chicore->getWorkspace()->updateToolws();
						this->chicore->getWorkspace()->update();
						this->destroy();
						
						break;
				}
				delete this->curve;
				this->curve = NULL;
			}
			break;
	}
}

Rect ToolPencil::getDrawrect()
{
	int margin = this->size + 1;

	RECT rect;
	SetRect(
		&rect,
		max( min(this->drawrect.left,this->drawrect.right) - margin, 0 ),
		max( min(this->drawrect.top,this->drawrect.bottom) - margin, 0 ),
		min( max(this->drawrect.left,this->drawrect.right) + margin,
			this->chicore->getWorkspace()->getPxWidth() ),
		min( max(this->drawrect.top,this->drawrect.bottom) + margin,
			this->chicore->getWorkspace()->getPxHeight() )
		);

	return Rect(
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top
		);
}