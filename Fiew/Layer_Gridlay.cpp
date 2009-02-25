/*
Layer_Gridlay.cpp
Inherits from Layer. This object is responsible for drawing
the transparency grid in the scene presented on the MDI child window
*/

#include "stdafx.h"
#include "Core.h"

Gridlay::Gridlay(ChildCore *core) : Layer(core,NULL)
{
	this->image = NULL;
	this->gridblock = NULL;
	this->pattern = NULL;
	this->x = 0;
	this->y = 0;
	this->oldx = 0;
	this->oldy = 0;
	this->width = 0;
	this->height = 0;

	this->rendered = false;

	SolidBrush *b = new SolidBrush(CLR_FRAME_LIGHT);

	this->image = new Bitmap(2 * GRDBGBLOCK,2 * GRDBGBLOCK,Core::getPixelFormat());
	Graphics *g = Graphics::FromImage(this->image);

	g->SetInterpolationMode(InterpolationModeNearestNeighbor);
	g->Clear(CLR_WHITE);
	g->FillRectangle(b,GRDBGBLOCK,0,GRDBGBLOCK,GRDBGBLOCK);
	g->FillRectangle(b,0,GRDBGBLOCK,GRDBGBLOCK,GRDBGBLOCK);

	delete b;
	delete g;

	g = Graphics::FromHWND(this->core->getWindowHandle());
	this->pattern = new CachedBitmap((Bitmap *)this->image,g);
	delete g;
}

Gridlay::~Gridlay()
{
	if( this->pattern != NULL )
		delete this->pattern;
	if( this->gridblock != NULL )
		delete this->gridblock;
}

Bitmap *Gridlay::render()
{
	return this->gridblock;
}

bool Gridlay::isRendered()
{
	return this->rendered;
}
/*
Preload transparency grid basing on the size of MDI child client area
*/
void Gridlay::preload(RECT client, RECT scrollInfo, int x, int y, int width, int height)
{
	this->rendered = true;

	int multi = 2;
	double zoom = this->core->getDrawer()->getZoom();

	if( scrollInfo.left < 0 ){
		this->x = - abs(x % (multi * GRDBGBLOCK));
		if( zoom > ZOOMINIT )
			this->x -= this->x % (int)zoom;
	}
	else
		this->x = x;
	if( scrollInfo.top < 0 ){
		this->y = - abs(y % (multi * GRDBGBLOCK));
		if( zoom > ZOOMINIT )
			this->y -= this->y % (int)zoom;
	}
	else
		this->y = y;	

	width = min(client.right, width);
	height = min(client.bottom, height);

	if( this->width != width || this->height != height ||
		this->oldx != this->x || this->oldy != this->y ){
		if( this->gridblock != NULL )
			delete this->gridblock;
		this->gridblock = NULL;

		this->width = width;
		this->height = height;

		this->gridblock = new Bitmap(this->width,
									 this->height,
									 Core::getPixelFormat());

		Graphics *g = Graphics::FromImage(this->gridblock);
		g->SetInterpolationMode(InterpolationModeNearestNeighbor);

		for( int i = min(this->y,0); i < this->height + (multi * GRDBGBLOCK); i += multi * GRDBGBLOCK )
			for( int j = min(this->x,0); j < this->width + (multi * GRDBGBLOCK); j += multi * GRDBGBLOCK )
				g->DrawCachedBitmap(this->pattern,j,i);

		delete g;

		this->oldx = this->x;
		this->oldy = this->y;
	}
	this->x = max(this->x,0);
	this->y = max(this->y,0);
}

void Gridlay::scroll(int hor, int ver, bool invalidate){ }
void Gridlay::scrollSet(int x, int y, bool invalidate){ }
void Gridlay::scrollHor(int val){ }
void Gridlay::scrollVer(int val){ }

void Gridlay::zoomer(double val){ }
void Gridlay::zoomat(double val, bool invalidate){ }
void Gridlay::zoomend(bool invalidate){ }