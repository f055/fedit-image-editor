/*
Frame.cpp
Frame is the object that represents the layer in the editing process.
It contains various info about the state of the layer and the Bitmap
or Path data
*/

#include "stdafx.h"
#include "Core.h"
/*
Constructor for new project
*/
Frame::Frame(Workspace *wspc, int width, int height)
{
	this->workspace = wspc;

	this->image = new Bitmap(width,height,Core::getPixelFormat());
	this->initialize();
	this->isEmpty = true;
}
/*
Contructor for Bitmap at location x,y
*/
Frame::Frame(Workspace *wspc, Bitmap *source, int x, int y)
{
	this->workspace = wspc;

	this->image = source;
	this->initialize();

	this->x = x;
	this->y = y;
}
/*
Contructors for images
*/
Frame::Frame(Workspace *wspc, Bitmap *source)
{
	this->workspace = wspc;

	this->image = source;
	this->initialize();
}
Frame::Frame(Workspace *wspc, Image *source)
{
	this->workspace = wspc;

	this->image = (Bitmap *)source;
	this->initialize();
}
/*
Empty named layer or FrameText
*/
Frame::Frame(Workspace *wspc, FwCHAR *name)
{
	this->workspace = wspc;

	this->image = new Bitmap(1,1,Core::getPixelFormat());

	this->initialize(name);
}
/*
Clone Frame object
*/
Frame *Frame::clone()
{
	Frame *clone = new Frame(
		this->workspace,
		this->image->Clone(0,0,this->image->GetWidth(),this->image->GetHeight(),this->image->GetPixelFormat()),
		this->x,
		this->y
		);
	clone->setName( new FwCHAR(this->name->toWCHAR()) );
	clone->setOpacity(this->opacity);
	clone->toggleEmpty(this->isEmpty);
	if( this->isVisible == false )
		clone->toggleVisibility();
	if( this->isLocked == true )
		clone->toggleLock();

	return clone;
}
/*
Initialize Frame object
*/
void Frame::initialize(FwCHAR *text)
{
	this->name = NULL;
	this->thumb = NULL;

	if( text == NULL ){
		this->name = new FwCHAR(FRAME_NAME);
		this->name->mergeWith(L" ");

		FwCHAR *count = new FwCHAR(this->workspace->getLayers()->getCount() + 1);
		this->name->mergeWith(count);
		delete count;
	}
	else {
		this->name = new FwCHAR(text->toWCHAR());
	}

	this->gfx = Graphics::FromImage(this->image);

	this->iatt = new ImageAttributes();

	this->x = NULL;
	this->y = NULL;

	this->opacity = MAXOPACITY;
	this->isLocked = false;
	this->isVisible = true;
	this->isEmpty = false;
}

Frame::~Frame()
{
	if( this->name != NULL )
		delete this->name;
	if( this->thumb != NULL )
		delete this->thumb;

	if( this->image != NULL )
		delete this->image;
	if( this->gfx != NULL )
		delete this->gfx;

	if( this->iatt != NULL )
		delete this->iatt;
}
/*
Applies a Bitmap onto the layer in a specified location
scrBmp		- source bitmap
trgtRect	- target rectangle
srcRect		- source rectangle
executor	- Tool that called applyPaint()
effect		- is applyPaint() called by a filter
toolId		- Tool ID number
historical	- is applyPaint() called by a HistoricalElement
*/
void Frame::applyPaint(Bitmap *srcBmp, Rect trgtRect, Rect srcRect, Tool *executor, bool effect, int toolId, bool historical)
{
	if( historical == false ){
		if( ((this->isLocked == true || this->isVisible == false) && effect == false) ||
			((this->isLocked == true && effect == true)) )
			return;
	}

	Rect imgRect(
		this->x,
		this->y,
		this->getWidth(),
		this->getHeight()
		);
	if( this->isEmpty == true ){
		this->isEmpty = false;
		//imgRect = srcRect;
	}
	
	Rect unionRect;
	Rect::Union(unionRect,imgRect,trgtRect);

	if( unionRect.Width > imgRect.Width || unionRect.Height > imgRect.Height ){
		Bitmap *temp = new Bitmap(unionRect.Width,unionRect.Height,Core::getPixelFormat());

		delete this->gfx;
		this->gfx = Graphics::FromImage(temp);

		this->gfx->DrawImage(
			this->image,
			Rect(abs(unionRect.X - this->x),
				 abs(unionRect.Y - this->y),
				 imgRect.Width,
				 imgRect.Height),
			0,0,
			imgRect.Width,
			imgRect.Height,
			UnitPixel
			);
		this->x = unionRect.X;
		this->y = unionRect.Y;

		delete this->image;
		this->image = temp;
	}

	Rect target(
		trgtRect.X - this->x + srcRect.X,
		trgtRect.Y - this->y + srcRect.Y,
		srcRect.Width,
		srcRect.Height);

	/* History Before */
	HEToolDrawing *he = new HEToolDrawing(this->workspace,this,executor);

	Bitmap *before = new Bitmap(target.Width,target.Height,this->image->GetPixelFormat());
	Bitmap *after = new Bitmap(target.Width,target.Height,this->image->GetPixelFormat());

	Graphics *gbx = Graphics::FromImage(before);
	Graphics *gax = Graphics::FromImage(after);

	gbx->DrawImage(
		this->image,
		Rect(0,0,target.Width,target.Height),
		target.X,target.Y,
		target.Width,target.Height,
		UnitPixel
		);
	/* *** */

	if( effect == true ){
		if( this->workspace->getSelection() != NULL )
			this->gfx->SetClip(this->workspace->getSelection());
		this->gfx->Clear(NULL);
	}

	this->gfx->DrawImage(
		srcBmp,
		target,
		srcRect.X,srcRect.Y,
		srcRect.Width,
		srcRect.Height,
		UnitPixel
		);

	/* History After */
	gax->DrawImage(
		this->image,
		Rect(0,0,target.Width,target.Height),
		target.X,target.Y,
		target.Width,target.Height,
		UnitPixel
		);

	he->set(before,after,target);
	if( effect == true ){
		this->gfx->ResetClip();
		he->setToolId(toolId);
	}

	delete gbx;
	delete gax;
	/* *** */

	this->workspace->getHistory()->add(he);

	this->resetThumb();
}
/*
Clears the bitmap data from the selected Path.
Parameter analogous to applyPaint()
*/
void Frame::applyClear(GraphicsPath *clrPath, Tool *executor, bool historical)
{
	if( historical == false )
		if( this->isLocked == true || this->isVisible == false )
			return;

	Rect target;
	clrPath->GetBounds(&target);

	/* History Before */
	HEToolDrawing *he = new HEToolDrawing(this->workspace,this,executor);

	Bitmap *before = new Bitmap(target.Width,target.Height,this->image->GetPixelFormat());
	Bitmap *after = new Bitmap(target.Width,target.Height,this->image->GetPixelFormat());

	Graphics *gbx = Graphics::FromImage(before);
	Graphics *gax = Graphics::FromImage(after);

	gbx->DrawImage(
		this->image,
		Rect(0,0,target.Width,target.Height),
		target.X,target.Y,
		target.Width,target.Height,
		UnitPixel
		);
	/* *** */

	this->gfx->SetClip(clrPath);
	this->gfx->Clear(NULL);
	this->gfx->ResetClip();

	/* History After */
	gax->DrawImage(
		this->image,
		Rect(0,0,target.Width,target.Height),
		target.X,target.Y,
		target.Width,target.Height,
		UnitPixel
		);

	he->set(before,after,target);

	delete gbx;
	delete gax;
	/* *** */

	this->workspace->getHistory()->add(he);

	this->resetThumb();
}
/*
Applies a bitmap that has been filterd by ToolFilter.
Instead of drawing over, it clears the target rectangle
and paint the effect bitmap there.
Parameters analogous to applyPaint()
*/
void Frame::applyEffect(Bitmap *effect, Rect *trgtRect, Tool *executor, int toolId, bool historical)
{
	int iw = this->image->GetWidth();
	int ih = this->image->GetHeight();

	int ew = effect->GetWidth();
	int eh = effect->GetHeight();

	Rect target(this->x + (iw - ew) / 2,this->y + (ih - eh) / 2,ew,eh);
	Rect source(0,0,ew,eh);

	if( trgtRect == NULL ){
		trgtRect = &target;
	}
	else {
		trgtRect->X += this->x;
		trgtRect->Y += this->y;
	}

	this->applyPaint(effect,*trgtRect,source,executor,true,toolId);		
}
/*
Resize Frame size along with bitmap resize
*/
void Frame::resizeFrame(double wFactor, double hFactor, InterpolationMode mode)
{
	Bitmap *resized = new Bitmap(
		(int)(this->image->GetWidth() * wFactor),
		(int)(this->image->GetHeight() * hFactor),
		this->image->GetPixelFormat() );

	Graphics *gfx = Graphics::FromImage(resized);

	if( mode == InterpolationModeInvalid ){
		Drawer::setHQInterpolation(gfx);
	}
	/*else if( mode == InterpolationModeNearestNeighbor &&
		wFactor == hFactor &&
		wFactor > ZOOMINIT ){

		Layer::scaleEnlarge(this->image,resized,wFactor);
	}*/
	else { 
		gfx->SetInterpolationMode(mode);

		gfx->DrawImage(
			this->image,
			Rect(0,0,resized->GetWidth(),resized->GetHeight()),
			0,0,this->getWidth(),this->getHeight(),UnitPixel
			);
	}
	Drawer::setInterpolation(gfx);

	delete this->gfx;
	delete this->image;

	this->gfx = gfx;
	this->image = resized;

	this->x = (int)(this->x * wFactor);
	this->y = (int)(this->y * hFactor);

	this->resetThumb();
}
/*
Move Frame by x,y. May be forced by HistoricalElement
*/
void Frame::addXY(int x, int y, bool force)
{
	if( (this->isLocked == true || this->isVisible == false) && force == false )
		return;

	this->x += x;
	this->y += y;
}

/*
Set Frame position. May be forced by HistoricalElement
*/
void Frame::setXY(int x, int y, bool force)
{
	if( (this->isLocked == true || this->isVisible == false) && force == false )
		return;

	this->x = x;
	this->y = y;
}

int Frame::getX()
{
	return this->x;
}
int Frame::getY()
{
	return this->y;
}
int Frame::getWidth()
{
	return this->image->GetWidth();
}
int Frame::getHeight()
{
	return this->image->GetHeight();
}

Bitmap *Frame::getRender()
{
	if( this->isVisible == true )
		return this->image;

	return NULL;
}

Graphics *Frame::getGraphics()
{
	return this->gfx;
}
/*
Create Frame thumbnail
*/
Bitmap *Frame::getThumb(int w, int h)
{
	if( this->thumb != NULL )
		delete this->thumb;
		//return this->thumb;

	Bitmap *thumb = new Bitmap(w,h,this->image->GetPixelFormat());

	int iwidth = this->image->GetWidth();
	int iheight = this->image->GetHeight();

	if( iwidth > w || iheight > h ){
		double dx = (double)w / (double)iwidth;
		double dy = (double)h / (double)iheight;

		iwidth = (int)(min(dx,dy) * iwidth);
		iheight = (int)(min(dx,dy) * iheight);
	}
	int x = (int)((w - iwidth) / 2);
	int y = (int)((h - iheight) / 2);

	Graphics *g = Graphics::FromImage(thumb);
	SolidBrush *b = new SolidBrush(CLR_FRAME_LIGHT);
	int jump = GRDBGBLOCK / 2;
	int count = 0;

	g->Clear(CLR_WHITE);
	for( int x = 0; x < w; x += jump )
		for( int y = 0; y < h; y += jump )
			if( (count++) % 2 == 0 )
				g->FillRectangle(b,Rect(x,y,jump,jump));

	g->DrawImage(
		this->image,
		Rect(x,y,iwidth,iheight),
		0,0,
		this->image->GetWidth(),
		this->image->GetHeight(),
		UnitPixel
		);

	delete g;

	this->thumb = thumb;

	return thumb;
}

void Frame::resetThumb()
{
	if( this->thumb != NULL )
		delete this->thumb;
	this->thumb = NULL;
}
/*
Set Frame transparency
*/
void Frame::setOpacity(int value)
{
	this->opacity = min( max(value,0) , MAXOPACITY );

	if( this->opacity < MAXOPACITY ){

		ColorMatrix matrix = {0.0f};
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = this->opacity / 100.0f;
		matrix.m[4][4] = 1.0f;

		this->iatt->SetColorMatrix(
			&matrix,
			ColorMatrixFlagsDefault,
			ColorAdjustTypeBitmap);
	}
	else {
		this->iatt->ClearColorMatrix(ColorAdjustTypeBitmap);
	}
}

int Frame::getOpacity()
{
	return this->opacity;
}

void Frame::setLock(bool val)
{
	this->isLocked = val;
}

void Frame::toggleLock()
{
	this->isLocked = !this->isLocked;
}

void Frame::toggleEmpty(bool val)
{
	this->isEmpty = val;
}

void Frame::setVisibility(bool val)
{
	this->isVisible = val;
}

void Frame::toggleVisibility()
{
	this->isVisible = !this->isVisible;
}

bool Frame::getIsLocked()
{
	return this->isLocked;
}

bool Frame::getIsVisible()
{
	return this->isVisible;
}

bool Frame::getIsEmpty()
{
	return this->isEmpty;
}

bool Frame::isFrameReady()
{
	if( this->isLocked == true ||
		this->isVisible == false )
		return false;

	return true;
}

void Frame::setName(FwCHAR *name)
{
	if( name != NULL ){
		if( name->toLength() > 0 ){
			if( this->name != NULL )
				delete this->name;
			this->name = name;
		}
		else {
			delete name;
		}
	}
}

FwCHAR *Frame::getName()
{
	return this->name;
}

ImageAttributes *Frame::getIatt()
{
	return this->iatt;
}

int Frame::getType()
{
	return FRM_NULL;
}
/*
Used for saving to Fedit Document. Writes Frame image data to IStream object
*/
bool Frame::setStream(IStream *stream, CLSID *encoder)
{
	if( stream != NULL )
		if( this->image->Save(stream,encoder) == Ok )
			return true;
	return false;
}
/*
Converts Frame image data to IStream object
*/
IStream *Frame::getStream(int &len, CLSID *encoder)
{
	IStream *stream = NULL;
	HGLOBAL buffer = NULL;

	if( (buffer = GlobalAlloc(GPTR,1)) != NULL ){
		if( CreateStreamOnHGlobal(buffer,true,(LPSTREAM*)&stream) == S_OK ){
			if( this->image->Save(stream,encoder) != Ok ){
				stream->Release();
				stream = NULL;
			}
		}
	}
	return stream;
}

Workspace *Frame::getWorkspace()
{
	return this->workspace;
}