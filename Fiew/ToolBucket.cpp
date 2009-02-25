#include "stdafx.h"
#include "Core.h"

ToolBucket::ToolBucket() : ToolDrawingStrict(new FwCHAR(TOOL_FILBUC),NULL,ICC_FILBUC)
{
	this->isPreselect = false;
	this->sourceBmp = NULL;

	this->cursor = Tool::createToolCursor(19,15,ICC_FILBUC);
	this->cursorBackup = this->cursor;
}

ToolBucket::~ToolBucket()
{
	if( this->sourceBmp != NULL )
		delete this->sourceBmp;

	DestroyIcon(this->cursorBackup);
}

void ToolBucket::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isLayerReady() == false )
				return;

			if( this->chicore->getWorkspace()->getSelection() == NULL ){
				this->setMouseCapture();

				this->sourceBmp = this->chicore->getWorkspace()->getSelectedLayerProjection(true);

				this->callColor(&this->colorPick);
				this->callMagic(wParam,lParam);

				this->isDraw = true;
			}
			else {
				this->isPreselect = true;
			}
			break;
	}
}

void ToolBucket::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	this->loadCursor();

	if( this->isDraw == true ){
		Color temp;
		this->callColor(&temp);

		if( temp.GetValue() != this->colorPick.GetValue() ){
			this->colorPick = temp;
			this->callMagic(wParam,lParam);
		}
	}
}

void ToolBucket::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->setMouseRelease();

			this->isDraw = false;

			if( this->chicore->getWorkspace()->getSelection() != NULL ){
				if( this->chicore->getWorkspace()->getSelection()->IsVisible(
					this->mouse.x,this->mouse.y) ){

					if( this->sourceBmp != NULL )
						delete this->sourceBmp;
					this->sourceBmp = NULL;

					this->beginPaint();

					this->drawgfx->FillPath(
						this->brushColor,
						this->chicore->getWorkspace()->getSelection()
						);

					Rect bounds;
					this->chicore->getWorkspace()->getSelection()->GetBounds(&bounds);
					SetRect(
						&this->srcrect,
						bounds.X,
						bounds.Y,
						bounds.X + bounds.Width,
						bounds.Y + bounds.Height
						);
					this->endPaint();
				}
			}

			if( this->isPreselect == false )
				this->chicore->getWorkspace()->setSelection(NULL);
			this->isPreselect = false;

			break;
	}
}

void ToolBucket::callColor(Color *color)
{
	this->chicore->getWorkspace()->getSelectedLayer()->getRender()->GetPixel(
		this->mouse.x,
		this->mouse.y,
		color);
}

void ToolBucket::callMagic(WPARAM wParam, LPARAM lParam)
{
	this->chicore->getWorkspace()->setSelection( ToolSelectWand::doMagic(
		this->sourceBmp,
		NULL,
		this->mouse.x,
		this->mouse.y,
		0,
		0,
		this->size	)
		);
	this->chicore->getDrawer()->invalidate();
}