/*
Layer_Snaplay.cpp
Object responsible for displaying snapping grid. For future use.
*/

#include "stdafx.h"
#include "Core.h"

Snaplay::Snaplay(ChildCore *core, Image *image) : Layer(core,NULL)
{
	this->image = NULL;
	this->x = 0;
	this->y = 0;
	this->oldZoom = 1;
	this->oldWidth = 0;
	this->oldHeight = 0;
	this->width = 0;
	this->height = 0;

	Bitmap *texture = new Bitmap(2,2);
	texture->SetPixel(0,0,CLR_WHITE);
	texture->SetPixel(1,0,CLR_BLACK);
	texture->SetPixel(0,1,CLR_BLACK);
	texture->SetPixel(1,1,CLR_WHITE);

	TextureBrush *tb = new TextureBrush(texture);
	this->penPattern = new Pen(tb,1);
	this->penSolid = new Pen(CLR_BLACK,1);

	if( image != NULL )
		this->image = new Bitmap(image->GetWidth(),image->GetHeight(),Core::getPixelFormat());

	delete texture;
	delete tb;
}

Snaplay::~Snaplay()
{
	if( this->penPattern != NULL )
		delete this->penPattern;
}

Bitmap *Snaplay::render()
{
	return NULL;
}

void Snaplay::preload()
{
	
}