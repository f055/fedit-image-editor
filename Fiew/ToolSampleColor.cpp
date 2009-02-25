#include "stdafx.h"
#include "Core.h"

ToolSampleColor::ToolSampleColor() : Tool(new FwCHAR(TOOL_SMPCOL),NULL,ICC_SMPCOL)
{
	this->isMoving = false;
	this->button = NULL;

	this->cursor = Tool::createToolCursor(4,17,ICC_SMPCOL);
	this->cursorBackup = this->cursor;
}

ToolSampleColor::~ToolSampleColor()
{
	DestroyIcon(this->cursorBackup);
}

void ToolSampleColor::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	if( this->isMoving == false ){
		this->setMouseCapture();

		this->isMoving = true;
		this->button = button;

		this->pickColor();
	}
}

void ToolSampleColor::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->isMoving == true ){
		this->pickColor();
	}
}

void ToolSampleColor::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	if( this->isMoving == true ){
		this->setMouseRelease();

		this->isMoving = false;

		this->pickColor();
	}
}

bool ToolSampleColor::getPixel(POINT pixel, Bitmap *scene, Color *color)
{
	if( scene == NULL )
		return false;

	int x = min( max(pixel.x,0), (int)scene->GetWidth() - 1);
	int y = min( max(pixel.y,0), (int)scene->GetHeight() - 1);

	scene->GetPixel(x,y,color);

	return true;
}

void ToolSampleColor::pickColor()
{
	Color color;
	if( 
		this->getPixel(
		this->mouse,
		this->chicore->getDrawer()->getBmpScene(),
		&color) == false 
		)
		return;

	switch(this->button){
		case LEFT:
			this->core->getToolws()->getToolwCC()->setForeColor(color);
			break;
		case RIGHT:
			this->core->getToolws()->getToolwCC()->setBackColor(color);
			break;
	}
}