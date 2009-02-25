/*
Layer_Overlay.cpp
Inherited from Fiew 2.0
Defines generall overlay object that can be displayed over the current
MDI child scene with an additional alpha blended white area for improved look
*/

#include "stdafx.h"
#include "Core.h"

Overlay::Overlay(ChildCore *core, Image *image) : Layer(core,image)
{
	this->fitmode = NULL;
}

Overlay::~Overlay()
{
}

void Overlay::nextImage(int x, int y)
{
	this->hide();
}
void Overlay::prevImage(int x, int y)
{
	this->hide();
}

Bitmap *Overlay::render()
{
	if( this->core != NULL ){
		this->locate();

		int frame = FRAME;
		Bitmap *scene = NULL;
		if( this->fulldraw == true )
			scene = new Bitmap(this->cwidth,this->cheight,Core::getPixelFormat());
		else
			scene = new Bitmap(this->width,this->height,Core::getPixelFormat());

		Graphics *tmpgfx = Graphics::FromImage(scene);

		if( this->fulldraw == true ){
			tmpgfx->Clear(CLR_WHITE_ALPHA);

			if( this->image != NULL ){
				tmpgfx->DrawRectangle(this->Pen_Border,
									  this->x - frame,
									  this->y - frame,
									  this->width + frame + 1,
									  this->height + frame + 1);

				tmpgfx->DrawImage(this->image,this->x,this->y,this->width,this->height);
			}
			this->x = 0;
			this->y = 0;
			this->width = scene->GetWidth();
			this->height = scene->GetHeight();
		}
		else {
			tmpgfx->DrawImage(this->image,0,0,this->width,this->height);
		}
		this->fulldraw = true;
		delete tmpgfx;

		return scene;
	}
	else {
		return (Bitmap *)this->image;
	}
}

void Overlay::zoomer(double val)
{
	return;
}
void Overlay::zoomat(double val, bool invalidate)
{
	return;
}
void Overlay::zoomend(bool invalidate)
{
	return;
}

void Overlay::rotate(int val)
{
	return;
}
void Overlay::rotateReset(bool novalid)
{
	return;
}

void Overlay::setFitmode(int mode)
{
	return;
}
void Overlay::unsetFitmode()
{
	return;
}
void Overlay::setSidedraw()
{
	return;
}
void Overlay::setSidemode(int mode)
{
	return;
}

void Overlay::hide()
{
	SetTimer(this->core->getWindowHandle(),TIMER_OVL,OVL_HIDEOUT,NULL);
}

RECT Overlay::getOverlayRect()
{
	RECT client = this->getClientSize();

	RECT overlay;

	overlay.left = (int)((client.right - OVL_SIZE)/2);
	overlay.top = (int)((client.bottom - OVL_SIZE)/2);
	overlay.right = overlay.left + OVL_SIZE;
	overlay.bottom = overlay.top + OVL_SIZE;

	return overlay;
}