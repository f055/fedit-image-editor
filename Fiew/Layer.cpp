/*
Layer.cpp
Partially inherited from Fiew 2.0
One of the major Fedit components, responsible for rendering the scene
of MDI child window. It performs scrolling, zooming, and several other
function that were used in Fiew 2.0
*/

#include "stdafx.h"
#include "Core.h"

Layer::Layer(ChildCore *core, Image *image)
{
	this->core = core;

	this->image = NULL;
	this->scali = NULL;

	this->fulldraw = true;
	this->sidedraw = false;
	this->zoomdraw = false;
	this->fullscreen = false;
	this->cancel = false;

	this->dimension = NULL;
	this->frameCount = 0;
	this->frameThat = 0;

	this->Brush_Back = new SolidBrush(CLR_WHITE);
	this->Brush_DarkBack = new SolidBrush(CLR_DIRK);
	this->Brush_LiteBack = new SolidBrush(CLR_LITE);

	this->Pen_Border = new Pen(CLR_FRAME_LIGHT,1);
	this->Pen_DarkBorder = new Pen(CLR_BLACK,1);

	this->FontSize = FONTSIZE;

	this->FontFamily_Arial = new FontFamily(FONT,NULL);

	this->Font_Default = 
		new Font(this->FontFamily_Arial,(REAL)this->FontSize,FontStyleRegular,UnitPixel);

	this->mut_image = NULL;
	this->mut_animloop = NULL;
	this->mut_terminator = NULL;

	this->thrd_anim = NULL;

	this->offset = NULL;
	this->menuheight = NULL;

	this->fitmode = FITSCREENOV;
	this->sidemode = NULL;
	this->type = TYPE_NONE;

	this->zoom = ZOOMINIT;
	this->rot = ROT_0;
	this->gifdir = RIGHT;

	if( core != NULL ){
		this->init(image);
		this->locate(TOP);
	}
}

Layer::~Layer()
{
	this->reset();

	delete this->Brush_Back;
	delete this->Brush_DarkBack;
	delete this->Brush_LiteBack;

	delete this->Pen_Border;
	delete this->Pen_DarkBorder;

	delete this->FontFamily_Arial;

	delete this->Font_Default;
}

void Layer::init(Image *source)
{
	if( source != NULL ){
		this->loadContent(source,DEFAULT);
		this->unsetFitmode();
	}
}

void Layer::reset()
{
	if( WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_terminator);
		WaitForSingleObject(this->mut_animloop,INFINITE);
		ReleaseMutex(this->mut_animloop);
	}
	if( this->mut_image != NULL )
		CloseHandle(this->mut_image);
	this->mut_image = NULL;

	if( this->mut_animloop != NULL )
		CloseHandle(this->mut_animloop);
	this->mut_animloop = NULL;

	if( this->mut_terminator != NULL )
		CloseHandle(this->mut_terminator);
	this->mut_terminator = NULL;

	if( this->thrd_anim != NULL )
		CloseHandle(this->thrd_anim);
	this->thrd_anim = NULL;

	if( this->image != NULL )
		delete this->image;
	this->image = NULL;

	if( this->scali != NULL )
		delete this->scali;
	this->scali = NULL;

	if( this->dimension != NULL )
		delete this->dimension;
	this->dimension = NULL;
	this->frameCount = 0;
	this->frameThat = 0;

	this->gifdir = RIGHT;
}
/*
Reposition the current image
*/
void Layer::repos()
{
	this->sidemode = NULL;
	this->rotateReset(true);
	this->setFitmode(NULL);
}

void Layer::loadContent(Image *source, int init)
{
	this->reset();

	WaitForSingleObject(this->mut_image,INFINITE);
	this->image = source;
	ReleaseMutex(this->mut_image);

	this->afterLoadContent(init);
}

void Layer::afterLoadContent(int init)
{
	int mode = init;
	if( this->sidedraw == false && mode != UPDATE)
		mode = TOP;

	this->locate(mode);
	this->setFitmode();

	if( this->core->getGui() != NULL && init != DEFAULT && init != UPDATE )
		this->core->getGui()->update();
}

void Layer::nextImage(int x, int y)
{

}
void Layer::prevImage(int x, int y)
{

}
/*
Methods for animated gif viewing, for future use
*/
bool Layer::nextFrame(bool back)
{
	if( this->frameCount < 2 )
		return true;
	if( back == false &&
		WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT )
		return true;

	bool result = false;

	this->frameThat++;
	if( this->frameThat >= this->frameCount )
		this->frameThat = 0;
	if( this->frameThat == 0 )
		result = true;

	this->image->SelectActiveFrame(this->dimension,this->frameThat);

	return result;
}
bool Layer::prevFrame(bool back)
{
	if( this->frameCount < 2 )
		return true;
	if( back == false &&
		WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT )
		return true;

	bool result = false;

	if( this->frameThat == 0 )
		result = true;
	if( this->frameThat == 0 )
		this->frameThat = this->frameCount;

	this->frameThat--;
	this->image->SelectActiveFrame(this->dimension,this->frameThat);

	return result;
}
/*
Rendering routine which can either redraw everythin or just a part
of current image.
*/
Bitmap *Layer::render()
{
	this->locate();
	this->core->getDrawer()->preloadBackground(this);

	WaitForSingleObject(this->mut_image,INFINITE);

	int frame = 1;//FRAME;
	Bitmap *scene = NULL;
	if( this->fulldraw == true )
		scene = new Bitmap(this->cwidth,this->cheight,Core::getPixelFormat());
	else
		scene = new Bitmap(this->width,this->height,Core::getPixelFormat());

	Graphics *tmpgfx = Graphics::FromImage(scene);
	tmpgfx->SetInterpolationMode(InterpolationModeNearestNeighbor);

	Image *tmp = NULL;

	if( this->fulldraw == true ){
		if( this->image != NULL ){
			if( this->scali == NULL )
				tmp = this->image;
			else
				tmp = this->scali;

			tmpgfx->DrawRectangle(this->Pen_DarkBorder,
								  this->x - frame,
								  this->y - frame,
								  this->width + frame,
								  this->height + frame);

			if( this->zoom > ZOOMINIT ){
				Bitmap *bigbmp = 
					new Bitmap(scene->GetWidth(),scene->GetHeight(),scene->GetPixelFormat());
				this->scaleEnlarge(
					(Bitmap *)tmp,
					Rect(
						this->x,
						this->y,
						this->width,
						this->height
						),
					bigbmp,
					this->zoom
					);
				tmpgfx->DrawImage(
					bigbmp,
					0,0,
					bigbmp->GetWidth(),
					bigbmp->GetHeight()
					);
				delete bigbmp;
			}
			else {
				tmpgfx->DrawImage(
					tmp,
					Rect(
						this->x,
						this->y,
						this->width,
						this->height
						),
					0,0,
					tmp->GetWidth(),
					tmp->GetHeight(),
					UnitPixel
					);
			}
		}
		this->x = 0;
		this->y = 0;
		this->width = scene->GetWidth();
		this->height = scene->GetHeight();
	}
	else {
		if( this->scali == NULL )
				tmp = this->image;
			else
				tmp = this->scali;

		tmpgfx->DrawImage(tmp,
						  Rect(0,0,this->width,this->height),
						  0,0,tmp->GetWidth(),tmp->GetHeight(),
						  UnitPixel);
	}
	ReleaseMutex(this->mut_image);

	this->fulldraw = true;
	this->animate();

	delete tmpgfx;

	return scene;
}

void Layer::scroll(int hor, int ver, bool invalidate)
{
	int invalid = 2;
	if( (this->rollHor == this->maxrollHor && hor > 0) ||
		(this->rollHor == -this->maxrollHor && hor < 0 ) ||	hor == 0 )
		invalid--;
	if( (((this->rollVer == this->maxrollVer && ver > 0) ||
		(this->rollVer == this->minrollVer && ver < 0)) && 
		 this->sidedraw == false) || ver == 0 )
		invalid--;

	if( invalid > 0 ){
		this->rollHor += hor;
		this->rollVer += ver;
		this->boundRoll();
		if( invalidate == true )
			this->invalidate();
	}
}
void Layer::scrollSet(int x, int y, bool invalidate)
{
	this->rollHor = x;
	this->rollVer = y;
	this->boundRoll();
	if( invalidate == true )
		this->invalidate();
}
void Layer::scrollHor(int val)
{
	if( (this->rollHor == this->maxrollHor && val > 0) ||
		(this->rollHor == -this->maxrollHor && val < 0 ) )
		return;

	this->rollHor += val;
	this->boundRoll();
	this->invalidate();
}
void Layer::scrollVer(int val)
{
	if( ((this->rollVer == this->maxrollVer && val > 0) ||
		(this->rollVer == this->minrollVer && val < 0)) && this->sidedraw == false )
		return;

	this->rollVer += val;
	this->boundRoll();
	this->invalidate();
}
/*
Zoom by value
*/
void Layer::zoomer(double val)
{
	if( this->zoom <= ZOOMLOSTEP && val < 0 )
		return;
	if( val == NULL )
		return;

	this->zoombegin();
	this->unsetFitmode();

	this->zoom += val;

	this->boundZoom();
	this->invalidate(); // caution! without zoomend()
}
/*
Set zoom to a value
*/
void Layer::zoomat(double val, bool invalidate)
{
	this->zoombegin();
	this->unsetFitmode();

	this->zoom = val;
	this->boundZoom();
	this->zoomend(invalidate);
}
void Layer::zoombegin()
{
	this->zoomdraw = true;
}
void Layer::zoomend(bool invalidate)
{
	if( this->scali != NULL )
		delete this->scali;
	this->scali = NULL;

	//if( this->zoom < ZOOMINIT/2 )
		this->scali = this->scale(this->image);

	this->zoomdraw = false;
	if( invalidate == true )
		this->invalidate();
}
/*
Image rotation, for future use
*/
void Layer::rotate(Image *image)
{
	if( image == NULL )
		return;

	switch(this->rot){
		case 1:
			image->RotateFlip(Rotate90FlipNone);
			break;
		case 2:
			image->RotateFlip(Rotate180FlipNone);
			break;
		case 3:
			image->RotateFlip(Rotate270FlipNone);
			break;
	}
}
void Layer::rotate(int val)
{
	if( val == NULL )
		return;

	bool rotati = true;
	if( WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT )
		rotati = false;

	this->rot = (this->rot + val) % 4;
	switch(val){
		case 1:
			if( rotati == true )
				this->image->RotateFlip(Rotate90FlipNone);
			break;
		case 2:
			if( rotati == true )
				this->image->RotateFlip(Rotate180FlipNone);
			break;
		case 3:
			if( rotati == true )
				this->image->RotateFlip(Rotate270FlipNone);
			break;
	}
	this->setFitmode();
}
void Layer::rotateSet()
{
	bool rotati = true;
	if( WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT )
		rotati = false;

	switch(this->rot){
		case 1:
			if( rotati == true )
				this->image->RotateFlip(Rotate90FlipNone);
			break;
		case 2:
			if( rotati == true )
				this->image->RotateFlip(Rotate180FlipNone);
			break;
		case 3:
			if( rotati == true )
				this->image->RotateFlip(Rotate270FlipNone);
			break;
	}
}
void Layer::rotateReset(bool novalid)
{
	bool rotati = true;
	if( WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT )
		rotati = false;

	switch(this->rot){
		case 1:
			if( rotati == true )
				this->image->RotateFlip(Rotate270FlipNone);
			break;
		case 2:
			if( rotati == true )
				this->image->RotateFlip(Rotate180FlipNone);
			break;
		case 3:
			if( rotati == true )
				this->image->RotateFlip(Rotate90FlipNone);
			break;
	}
	this->rot = 0;
	if( novalid == false )
		this->zoomend();
}

int Layer::getX()
{
	return this->x;
}
int Layer::getY()
{
	return this->y;
}
int Layer::getWidth()
{
	return this->width;
}
int Layer::getHeight()
{
	return this->height;
}
int Layer::getMaxrollHor()
{
	if( this->image == NULL )
		return 0;

	RECT client;
	GetClientRect(this->core->getWindowHandle(),&client);

	WaitForSingleObject(this->mut_image,INFINITE);
	int width = (int)(this->zoom * this->image->GetWidth());
	ReleaseMutex(this->mut_image);

	return abs( min((int)((client.right - width)/2),0) );
}
int Layer::getMaxrollVer()
{
	if( this->image == NULL )
		return 0;

	RECT client;
	GetClientRect(this->core->getWindowHandle(),&client);

	WaitForSingleObject(this->mut_image,INFINITE);
	int height = (int)(this->zoom * this->image->GetHeight());
	ReleaseMutex(this->mut_image);

	return abs( min((int)((client.bottom - height)/2),0) );
}

File *Layer::getFile()
{
	return NULL;
}

Image *Layer::getImage()
{
	return this->image;
}

int Layer::getImageWidth()
{
	WaitForSingleObject(this->mut_image,INFINITE);
	int width = this->image->GetWidth();
	ReleaseMutex(this->mut_image);
	return width;
}
int Layer::getImageHeight()
{
	WaitForSingleObject(this->mut_image,INFINITE);
	int height = this->image->GetHeight();
	ReleaseMutex(this->mut_image);
	return height;
}
double Layer::getZoom()
{
	return this->zoom;
}
double Layer::getZoom(int width, int height)
{
	if( this->fitmode == NULL )
		return this->zoom;

	RECT client = this->getClientSize();
	return this->getZoom(width,height,client.right,client.bottom);
}
/*
Calculate zoom of current image basing on fit mode
*/
double Layer::getZoom(int width, int height, int cwidth, int cheight)
{
	if( this->fitmode == NULL )
		return this->zoom;

	//width += 2 * FRAME;
	//height += 2 * FRAME;

	double dx, dy;
	if( this->fitmode == FITSCREEN || this->fitmode == FITSCREENOV ||
		this->fitmode == FITWIDTH || this->fitmode == FITHEIGHT ){
		dx = (double)cwidth / (double)width;
		dy = (double)cheight / (double)height;
	}
	else if( this->fitmode == FITNUMPAD ){
		dx = (double)(3 * cwidth) / (double)width;
		dy = (double)(3 * cheight) / (double)height;
		dx += ZOOMINIT;
		dy += ZOOMINIT;
	}
	if( this->fitmode == FITSCREENOV )
		return min(ZOOMINIT,min(dx,dy));
	if( this->fitmode == FITSCREEN || this->fitmode == FITNUMPAD )
		return min(dx,dy);

	if( this->fitmode == FITWIDTH )
		return dy;
	if( this->fitmode == FITHEIGHT )
		return dx;

	return this->zoom;
}
bool Layer::getSidedraw()
{
	return this->sidedraw;
}
int Layer::getFitmode()
{
	return this->fitmode;
}

void Layer::setFitmode(int mode)
{
	if( this->fitmode == mode || mode == NULL ){
		this->fitmode = NULL;
		this->zoom = ZOOMINIT;
		this->zoomend();
		return;
	}
	else if( mode != DEFAULT ){
		this->fitmode = mode;
		this->sidedraw = false;
	}

	WaitForSingleObject(this->mut_image,INFINITE);
	if( this->image != NULL )
		this->zoom = this->getZoom(this->image->GetWidth(),this->image->GetHeight());
	ReleaseMutex(this->mut_image);

	this->zoomend();
}
void Layer::unsetFitmode()
{
	if( this->fitmode != NULL ){
		this->fitmode = NULL;
		if( this->core->getGui() != NULL )
			this->core->getGui()->updateMenu();
	}
}

void Layer::setSidemode(int mode)
{
	if( this->sidemode == mode )
		this->sidemode = NULL;
	else
		this->sidemode = mode;

	if( this->sidedraw == false && this->getMaxrollHor() == 0 )
		return;

	if( this->sidemode == LEFT )
		this->rollHor = this->getMaxrollHor();
	else if( this->sidemode == RIGHT )
		this->rollHor = -this->getMaxrollHor();

	this->invalidate();
}
int Layer::getSidemode()
{
	return this->sidemode;
}

void Layer::setSidedraw()
{
	if( this->sidedraw == false ){
		this->sidedraw = true;
		if( this->fitmode != NULL )
			this->setFitmode(NULL);
	}
	else
		this->sidedraw = false;

	this->loadContent(NULL);
}
void Layer::setMenuheight(int val)
{
	this->menuheight = val;
}
void Layer::setGifDir(int dir)
{
	WaitForSingleObject(this->mut_image,INFINITE);
	if( dir == DEFAULT ){
		if( this->gifdir == RIGHT )
			this->gifdir = LEFT;
		else
			this->gifdir = RIGHT;
	}
	else {
		if( this->gifdir != RIGHT && this->gifdir != LEFT )
			this->gifdir = RIGHT;
		else
			this->gifdir = dir;
	}
	ReleaseMutex(this->mut_image);
}
void Layer::setCancel(bool set)
{
	this->cancel = set;
}

bool Layer::setWall()
{
	bool result = false;

	/*if( this->image != NULL && this->cell != NULL ){
		CLSID bmpsid;
		Core::getEncoder(L"image/bmp", &bmpsid);
		FwCHAR *path = NULL;

		if( this->cell->getFile()->isArchived() == true ){
			if( this->core->getExplorer() != NULL ){
				path = new FwCHAR();
				path->getFolderFrom( this->core->getExplorer()->getArchivePath() );
				path->mergeWith( this->cell->getFile()->getFileName() );
			}
			else
				path = new FwCHAR( this->cell->getFile()->getFileName()->toWCHAR() );
		}
		else {
			path = new FwCHAR( this->cell->getFile()->getFilePath()->toWCHAR() );
		}
		if( this->cell->getFile()->getType() != TYPE_BITMAP )
			path->mergeWith(L".bmp");

		this->image->Save(path->toWCHAR(),&bmpsid,NULL);

		result = SystemParametersInfo(SPI_SETDESKWALLPAPER,0,path->toWCHAR(),
									  SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE );
		delete path;
	}*/
	return result;
}

bool Layer::isCancel()
{
	return this->cancel;
}
bool Layer::isContent()
{
	if( this->image != NULL )
		return true;
	return false;
}
/*
Locator routine that sets the position of the current image
and calculates scroll variables basing on the current client size
*/
void Layer::locate(int init)
{
	RECT client = this->getClientSize();
	this->cwidth = client.right;
	this->cheight = client.bottom;

	if( this->image == NULL ){
		this->width = this->cwidth;
		this->height = this->cheight;
	}
	else {
		WaitForSingleObject(this->mut_image,INFINITE);
		this->zoom = this->getZoom(this->image->GetWidth(),this->image->GetHeight(),
								   this->cwidth,this->cheight);

		this->width = (int)(this->zoom * this->image->GetWidth());
		this->height = (int)(this->zoom * this->image->GetHeight());
		ReleaseMutex(this->mut_image);

		if( WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT ){
			if( this->rot == ROT_90 || this->rot == ROT_270 ){
				int tmp = this->width;
				this->width = this->height;
				this->height = tmp;
			}
		}
		else
			ReleaseMutex(this->mut_animloop);
	}
	double hodiff = (this->cwidth - this->width) / 2.0;
	double vediff = (this->cheight - this->height) / 2.0;

	this->x = (int)ceil(hodiff);
	this->y = (int)ceil(vediff);

	this->maxrollHor = (int)abs( min(floor(hodiff),0) );
	this->maxrollVer = (int)abs( min(floor(vediff),0) );

	this->minrollVer = -this->maxrollVer;
	this->offrollVer = 0;//max(this->y,0);

	if( init != NULL && init != UPDATE ){
		this->rollHor = 0;
		if( this->sidemode == LEFT )
			this->rollHor = this->maxrollHor;
		else if( this->sidemode == RIGHT )
			this->rollHor = -this->maxrollHor;

		if( init == TOP )
			this->rollVer = this->maxrollVer + this->offset;
		else if( init == BOT )
			this->rollVer = this->minrollVer + this->offset;
		this->offset = 0;
	}
	/*if( this->image != NULL && this->sidedraw == true ){
		if( this->prev != NULL )
			if( this->y > 0 )
				this->maxrollVer = this->height;
			else
				this->maxrollVer = -this->y + this->cheight + MARGIN;

		if( this->next != NULL )
			if( this->y > 0 )
				this->minrollVer = -this->height;
			else
				this->minrollVer = this->y - this->cheight - MARGIN;
	}*/
	this->boundRoll();

	this->x += this->rollHor;
	this->y += this->rollVer - this->menuheight;

	if( this->x == 1 )
		this->x = 0;
	if( this->y == 1 )
		this->y = 0;

	this->core->setScrollBar( this->getScrollHor(),this->getScrollVer() );
}

void Layer::boundRoll()
{
	this->rollHor = min(this->rollHor,this->maxrollHor);
	this->rollHor = max(this->rollHor,-this->maxrollHor);

	this->rollVer = min(this->rollVer,this->maxrollVer);
	this->rollVer = max(this->rollVer,-this->maxrollVer);

	/*if( this->rollVer > this->maxrollVer ){
		this->offset = 0;//this->rollVer - this->maxrollVer;
		this->rollVer = min(this->rollVer,0);
		//if( this->image != NULL && this->sidedraw == true )
		//	this->prevContent();
	}
	if( this->rollVer < this->minrollVer ){
		this->offset = 0;//this->rollVer - this->minrollVer;
		this->rollVer = max(this->rollVer,this->minrollVer);
		//if( this->image != NULL && this->sidedraw == true )
		//	this->nextContent();
	}*/
}

void Layer::boundZoom()
{
	this->zoom = max(this->zoom,ZOOMLOSTEP);
}

RECT Layer::getClientSize()
{
	RECT client;
	client.top = 0;
	client.left = 0;
	client.right = 0;
	client.bottom = 0;

	this->fullscreen = false;
	if( this->core->getGui() != NULL )
		this->fullscreen = this->core->getGui()->isFullscreen();

	if( this->fullscreen == true ){
		client.right = GetDeviceCaps(GetDC(NULL),HORZRES);
		client.bottom = GetDeviceCaps(GetDC(NULL),VERTRES);
	}
	else if( this->core->getWindowHandle() != NULL )
		GetClientRect(this->core->getWindowHandle(),&client);

	return client;
}

SCROLLINFO Layer::getScrollHor()
{
	RECT client = this->getClientSize();
	SCROLLINFO info;


	info.cbSize = sizeof(info);
	info.nMin = 0;
	info.nMax = 2 * this->maxrollHor;
	if( info.nMax != 0 )
		info.nMax += client.right;
	info.nPos = abs(this->rollHor - this->maxrollHor);
	info.nPage = client.right;

	return info;
}

SCROLLINFO Layer::getScrollVer()
{
	RECT client = this->getClientSize();
	SCROLLINFO info;

	info.cbSize = sizeof(info);
	info.nMin = 0;
	info.nMax = 2 * this->maxrollVer;
	if( info.nMax != 0 )
		info.nMax += client.bottom;
	info.nPos = abs(this->rollVer - this->maxrollVer);
	info.nPage = client.bottom;

	return info;
}

RECT Layer::getScrollInfo()
{
	RECT info;

	SetRect(
		&info,
		this->rollHor - this->maxrollHor,
		this->rollVer - this->maxrollVer,
		this->maxrollHor + this->rollHor,
		this->maxrollVer + this->rollVer
		);
	return info;
}

void Layer::invalidate(bool full)
{
	this->fulldraw = full;
	this->core->getDrawer()->invalidate(full);
}

Image *Layer::scale(Image *source)
{
	if( source == NULL )
		return NULL;

	double zoom = this->zoom;
	if( source != this->image )
		zoom = this->getZoom(source->GetWidth(),source->GetHeight());

	int width = (int)(zoom * source->GetWidth());
	int height = (int)(zoom * source->GetHeight());


	Bitmap *bmp = NULL;
	
	if( zoom < ZOOMINIT ){
		bmp = new Bitmap(width,height,Core::getPixelFormat()); 
		Graphics *gfx = Graphics::FromImage(bmp);
		Drawer::setInterpolation(gfx,this->zoom);
		gfx->DrawImage(source,0,0,width,height);
		delete gfx;
	}
	return bmp;
}
/*
Image pixel-precise scaling when zoom greater that 1 with additional
inversion of color using bits operations
*/
void Layer::scaleInvert(Bitmap *src, Rect *srcRect, Bitmap *bmp, Rect *bmpRect, int hatch, double zoom)
{
	UINT *src0, *bmp0, x, y;
	BitmapData srcData, bmpData;
	int z = (int)zoom;

	Rect srcRectangle(
		srcRect->X,
		srcRect->Y,
		min(srcRect->Width,(int)src->GetWidth() - srcRect->X),
		min(srcRect->Height,(int)src->GetHeight() - srcRect->Y)
		);
	Rect bmpRectangle(
		0,0,
		min(srcRect->Width,(int)bmp->GetWidth()),
		min(srcRect->Height,(int)bmp->GetHeight())
		);
	if( bmpRect != NULL ){
		bmpRectangle = Rect(
			bmpRect->X,
			bmpRect->Y,
			bmpRect->Width,
			bmpRect->Height
			);
	}

	src->LockBits(
			&srcRectangle,
			ImageLockModeRead,
			src->GetPixelFormat(),
			&srcData
			);
	bmp->LockBits(
			&bmpRectangle,
			ImageLockModeWrite,
			bmp->GetPixelFormat(),
			&bmpData
			);
	src0 = (UINT *)srcData.Scan0;
	bmp0 = (UINT *)bmpData.Scan0;

		for( y = 0; y < srcRectangle.Height; y++ ){
			for( x = 0; x < srcRectangle.Width; x++ ){
				if( x >= 0 && x < bmpRectangle.Width && 
					y >= 0 && y < bmpRectangle.Height ){
						if( bmp0[y * bmpData.Stride / 4 + x] != NULL ){
							src0[y * srcData.Stride / 4 + x] =
								Core::RGBtoUINT(
									Core::invClr( 
										Core::UINTtoRGB( src0[y * srcData.Stride / 4 + x] )
									)
								);
						}
				}
			}
		}

	src->UnlockBits(&srcData);
	bmp->UnlockBits(&bmpData);
}
/*
Pixel-precise enlarge using bits operations, also copies source bitmap to destination
*/
void Layer::scaleEnlarge(Bitmap *src, Bitmap *bmp, double zoom)
{
	UINT *src0, *bmp0, x, y, i, j;
	BitmapData srcData, bmpData;
	int xx, yy, z = (int)zoom;

	Rect srcRect(0,0,src->GetWidth(),src->GetHeight());
	Rect bmpRect(0,0,bmp->GetWidth(),bmp->GetHeight());

	src->LockBits(
			&srcRect,
			ImageLockModeRead,
			src->GetPixelFormat(),
			&srcData
			);
	bmp->LockBits(
			&bmpRect,
			ImageLockModeWrite,
			bmp->GetPixelFormat(),
			&bmpData
			);
	src0 = (UINT *)srcData.Scan0;
	bmp0 = (UINT *)bmpData.Scan0;

	if( zoom > ZOOMINIT ){
		for( y = 0; y < srcRect.Height; y++ ){
			for( x = 0; x < srcRect.Width; x++ ){
				for( i = 0; i < z; i++ ){
					for( j = 0; j < z; j++ ){
						xx = z * x + i;
						yy = z * y + j;
						if( xx >= 0 && xx < bmpRect.Width && 
							yy >= 0 && yy < bmpRect.Height ){
							bmp0[yy * bmpData.Stride / 4 + xx] =
								src0[y * srcData.Stride / 4 + x];
						}
					}
				}
			}
		}
	}
	else if( zoom < ZOOMINIT ){
		UINT sum = 0;
		z = (int)((1 - zoom) * 10);

		for( y = 0; y < bmpRect.Height; y++ ){
			for( x = 0; x < bmpRect.Width; x++ ){
				sum = 0;
				for( i = 0; i < z; i++ ){
					for( j = 0; j < z; j++ ){
						xx = z * x + i;
						yy = z * y + j;
						if( xx >= 0 && xx < srcRect.Width && 
							yy >= 0 && yy < srcRect.Height ){
							
							sum += src0[yy * srcData.Stride / 4 + xx];
						}
					}
				}
				sum = sum / (z & 0xff);

				bmp0[y * bmpData.Stride / 4 + x] = sum;
			}
		}
	}
	else {
		for( y = 0; y < srcRect.Height; y++ ){
			for( x = 0; x < srcRect.Width; x++ ){
				if( x >= 0 && x < bmpRect.Width && 
					y >= 0 && y < bmpRect.Height ){
					bmp0[y * bmpData.Stride / 4 + x] =
						src0[y * srcData.Stride / 4 + x];
				}
			}
		}
	}
	src->UnlockBits(&srcData);
	bmp->UnlockBits(&bmpData);
}
/*
Pixel-precise enlarge using bits operations,
also copies source bitmap rectangle to destination
*/
void Layer::scaleEnlarge(Bitmap *src, Rect srcRect, Bitmap *bmp, double zoom)
{
	UINT *src0, *bmp0, x, y, i, j;
	BitmapData srcData, bmpData;
	int xx, yy, z = (int)zoom;

	int bmpw = bmp->GetWidth();
	int bmph = bmp->GetHeight();
	int bmpx = max(srcRect.X,0);
	int bmpy = max(srcRect.Y,0);

	Rect bmpRect(
		bmpx,
		bmpy,
		min(bmpw - bmpx,srcRect.Width),
		min(bmph - bmpy,srcRect.Height)
		);

	srcRect.X = (int)( abs(min(srcRect.X,0)) / zoom );
	srcRect.Y = (int)( abs(min(srcRect.Y,0)) / zoom );
	srcRect.Width = (int)( min(srcRect.Width / zoom,ceil(bmpw / zoom)) );
	srcRect.Height = (int)( min(srcRect.Height / zoom,ceil(bmph / zoom)) );

	src->LockBits(
			&srcRect,
			ImageLockModeRead,
			src->GetPixelFormat(),
			&srcData
			);
	bmp->LockBits(
			&bmpRect,
			ImageLockModeWrite,
			bmp->GetPixelFormat(),
			&bmpData
			);

	src0 = (UINT *)srcData.Scan0;
	bmp0 = (UINT *)bmpData.Scan0;

	for( y = 0; y < srcRect.Height; y++ ){
		for( x = 0; x < srcRect.Width; x++ ){
			for( i = 0; i < z; i++ ){
				for( j = 0; j < z; j++ ){
					xx = z * x + i;
					yy = z * y + j;
					if( xx >= 0 && xx < bmpRect.Width && 
						yy >= 0 && yy < bmpRect.Height ){
						bmp0[yy * bmpData.Stride / 4 + xx] =
							src0[y * srcData.Stride / 4 + x];
					}
				}
			}
		}
	}
	src->UnlockBits(&srcData);
	bmp->UnlockBits(&bmpData);
}
/*
Gif animation. For future use.
*/
void Layer::animate()
{
	/*if( this->isTopmost() == false )
		return;
	if( WaitForSingleObject(this->mut_animloop,0) == WAIT_TIMEOUT )
		return;
	else
		ReleaseMutex(this->mut_animloop);

	if( this->cell == NULL )
		return;
	if( this->cell->getFile() == NULL )
		return;
	this->type = this->cell->getFile()->getType();
	if( this->type == TYPE_TIFFINTEL || this->type == TYPE_TIFFMOTOROLA ||
		this->type == TYPE_GIF87a || this->type == TYPE_GIF89a ){

		UINT dCount = this->image->GetFrameDimensionsCount();
		GUID *dList = new GUID[dCount];
		this->image->GetFrameDimensionsList(dList,dCount);
		this->dimension = &dList[0];
		this->frameCount = this->image->GetFrameCount(dimension);

		for( UINT i = 1; i < dCount; i++ )
			delete &dList[i];
		if( this->frameCount < 2 ){
			if( this->dimension != NULL )
				delete this->dimension;
			this->dimension = NULL;
			return;
		}
	}
	else
		return;
	if( this->type != TYPE_GIF87a && this->type != TYPE_GIF89a )
		return;	

	this->mut_image = CreateMutex(NULL,false,NULL);
	this->mut_animloop = CreateMutex(NULL,false,NULL);
	this->mut_terminator = CreateMutex(NULL,true,NULL);

	this->thrd_anim = CreateThread(NULL,NULL,
								  (LPTHREAD_START_ROUTINE)Layer::anim,
								   this,NULL,NULL);*/
}

DWORD WINAPI Layer::anim(LPVOID param)
{
	Layer *that = (Layer *)param;

	WaitForSingleObject(that->mut_animloop,INFINITE);

	WaitForSingleObject(that->mut_image,INFINITE);
	if( that->image == NULL ){
		ReleaseMutex(that->mut_image);
		return 0;
	}
	int propertySize = that->image->GetPropertyItemSize(PropertyTagFrameDelay);
	PropertyItem *frameDelay = (PropertyItem*) malloc(propertySize);
	that->image->GetPropertyItem(PropertyTagFrameDelay,propertySize,frameDelay);

	ReleaseMutex(that->mut_image);

	long *delays = (long *)frameDelay->value;
	bool mode = false;

	while( true ){
		if( that->isTopmost() == false )
			break;
		if( WaitForSingleObject(that->mut_terminator,0) != WAIT_TIMEOUT ){
			ReleaseMutex(that->mut_terminator);
			ReleaseMutex(that->mut_animloop);
			return 0;
		}
		Sleep( max((delays[that->frameThat] * 100),MINDELAY) );

		WaitForSingleObject(that->mut_image,INFINITE);

		if( that->gifdir == RIGHT )
			mode = that->nextFrame();
		else if( that->gifdir == LEFT )
			mode = that->prevFrame();
		ReleaseMutex(that->mut_image);

		that->invalidate(mode);
	}
	if( frameDelay != NULL )
		delete frameDelay;	

	ReleaseMutex(that->mut_animloop);
	return 0;
}

bool Layer::isTopmost()
{
	if( this->core->getDrawer()->getTopmost() == this )
		return true;
	return false;
}