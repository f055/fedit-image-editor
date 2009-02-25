/*
Drawer.cpp
Partially inherited from Fiew 2.0
One of the major object in Fedit, along with Layer it coordinates
the rendering of the scene of a MDI child window basing on the Workspace info
*/

#include "stdafx.h"
#include "Core.h"

Drawer::Drawer(ChildCore *core)
{
	this->core = core;

	this->layers = NULL;
	this->fulldraw = true;
	this->zoomdraw = true;

	this->scene = NULL;
	this->overlay = NULL;
	this->gridlay = NULL;
	this->snaplay = NULL;

	this->thumblay = NULL;
	this->listlay = NULL;

	this->visAbout = false;
	this->visManual = false;
	this->visThumbs = false;
	this->visList = false;

	this->isPreload = false;

	this->zoom = DRZOOMINIT;

	this->rectScene = Rect(0,0,0,0);
	this->rectInvalidate = Rect(0,0,0,0);

	this->bmpInvalidate = NULL;
	this->bmpScene = NULL;

	this->penSelection = new Pen(CLR_BLACK,1);
	this->penSelection->SetDashStyle(DashStyleDash);

	this->penDraw = new Pen(CLR_BLACK,1);
	this->penDraw->SetAlignment(PenAlignmentCenter);

	this->drawpath = NULL;
}

Drawer::~Drawer()
{
	delete this->layers;

	delete this->penSelection;
	delete this->penDraw;

	if( this->bmpInvalidate != NULL )
		delete this->bmpInvalidate;
	if( this->bmpScene != NULL )
		delete this->bmpScene;
}
/*
Main paint routine straight to MDI Child device context
*/
void Drawer::paint(HDC hdc)
{
	Graphics *gfx = Graphics::FromHDC(hdc);
	gfx->SetInterpolationMode(InterpolationModeNearestNeighbor);

	int count = this->layers->getCount();
	RECT client = this->getClientSize();

	Layer *lay = NULL;
	Bitmap *bmp = NULL;

	// full scene redraw
	if( count > 0 && this->fulldraw == true ){
		Bitmap *scene = new Bitmap(client.right,client.bottom,Core::getPixelFormat());
		Graphics *scenegfx = Graphics::FromImage(scene);
		scenegfx->SetInterpolationMode(InterpolationModeNearestNeighbor);

		// clear to background color
		scenegfx->Clear(CLR_FRAME_LIGHT);

		// go through all Layer objects (NOT Workspace Frames)
		this->layers->gotoHead();
		while( this->layers->next() == true ){
			lay = this->layers->getThat();

			// render transparency grid before the first Layer
			if( lay != NULL ){
				bmp = lay->render();
				this->renderBackground(scenegfx);
			}

			// draw the Layer image
			if( bmp != NULL ){
				scenegfx->DrawImage(
					bmp,
					Rect(lay->getX(),lay->getY(),
						 lay->getWidth(),lay->getHeight()),
					0,0,lay->getWidth(),lay->getHeight(),
					UnitPixel);
				delete bmp;
			}
			bmp = NULL;
			lay = NULL;
		};
		// render additional paths
		this->renderDraw(scene);
		// render selection paths
		this->renderSelection(scene);

		// draw the scene onto the MDI child window client
		gfx->DrawImage(
			scene,
			Rect(0,0,scene->GetWidth(),scene->GetHeight()),
			0,0,
			scene->GetWidth(),scene->GetHeight(),
			UnitPixel);

		delete scenegfx;
		
		if( this->bmpScene != NULL )
			delete this->bmpScene;
		this->bmpScene = scene;
	}
	// partial scene redraw
	else if( this->fulldraw == false ){
		bmp = this->bmpInvalidate;
		if( bmp != NULL ){
			if( this->modeInvalidate >= INVSCENE ){
				Bitmap *scene = this->bmpScene;

				// XOR draw
				if( this->modeInvalidate >= INVSCENEXOR ){
					scene = (Bitmap *)this->bmpScene->GetThumbnailImage(
					this->bmpScene->GetWidth(),
					this->bmpScene->GetHeight()
					);

					Layer::scaleInvert(
						scene,
						&this->rectInvalidate,
						bmp
						//Rect(0,0,this->rectInvalidate.Width,this->rectInvalidate.Height)
						);
				}
				gfx->DrawImage(
					scene,
					0,0,
					scene->GetWidth(),
					scene->GetHeight() );
			
				if( this->modeInvalidate >= INVSCENEXOR )
					delete scene;
			}
			else {
				gfx->DrawImage(
					bmp,
					this->rectInvalidate,
					0,0,
					bmp->GetWidth(),
					bmp->GetHeight(),
					UnitPixel
					);
			}
			delete bmp;
		}
		bmp = NULL;
		this->bmpInvalidate = NULL;
	}
	// if nothing, just clear the background
	else {
		gfx->Clear(CLR_FRAME_LIGHT);
	}
	delete gfx;

	this->fulldraw = true;
}
/*
Reinitialize Drawer object
*/
void Drawer::reset(Image *image)
{
	this->hideOverlay(false);

	if( this->layers != NULL )
		delete this->layers;
	this->layers = new List<Layer>();
	
	this->scene = new Layer(this->core,image);

	this->gridlay = new Gridlay(this->core);
	//this->snaplay = new Snaplay(this->core);
	this->overlay = NULL;
	this->thumblay = NULL;
	this->listlay = NULL;

	this->layers->add(this->gridlay);
	this->layers->add(this->scene);
	//this->layers->add(this->snaplay);

	this->zoom = (int)(this->scene->getZoom() * DRZOOMINIT);
	
	this->zoom = this->zoom - (this->zoom%DRZOOMLOSTEP);
	this->zoomSet((double)this->zoom / DRZOOMINIT);
}

Layer *Drawer::getTopmost()
{
	return this->layers->getTail();
}
Layer *Drawer::getScene()
{
	if( this->scene != NULL )
		if( this->scene->isContent() )
			return this->scene;
	return NULL;
}
Overlay *Drawer::getOverlay()
{
	if( this->overlay != NULL )
		if( this->overlay->isContent() )
			return (Overlay *)this->overlay;
	return NULL;
}
Thumblay *Drawer::getThumblay()
{
	if( this->visThumbs == true )
		return this->thumblay;
	return NULL;
}
Listlay *Drawer::getListlay()
{
	if( this->visList == true )
		return this->listlay;
	return NULL;
}

RECT Drawer::getClientSize()
{
	RECT client;
	client.left = client.right = 0;
	client.top = client.bottom = 0;

	if( this->scene != NULL )
		return this->scene->getClientSize();
	return client;
}

Rect Drawer::getRectScene()
{
	return this->rectScene;
}

Bitmap *Drawer::getBmpScene()
{
	return this->bmpScene;
}
/*
Retrive the client center in Workspace coordinates
*/
Point Drawer::getCenter()
{
	RECT client = this->getClientSize();
	
	double w = min(this->rectScene.Width,client.right) / this->getZoom();
	double h = min(this->rectScene.Height,client.bottom) / this->getZoom();

	double x = -min(this->rectScene.X,0) / this->getZoom();
	double y = -min(this->rectScene.Y,0) / this->getZoom();

	return Point((int)ceil(x + w/2),(int)ceil(y + h/2));
}

double Drawer::getZoom()
{
	double zoom = this->zoom / (double)DRZOOMINIT;
	return zoom;
}

void Drawer::updateOverlay()
{
	if( this->visThumbs == true )
		this->getThumblay()->update();
	if( this->visList == true )
		this->getListlay()->update();
}

void Drawer::hideOverlay(bool invalid)
{
	if( this->overlay != NULL ){
		if( invalid == false )
			this->overlay->setCancel();
		this->layers->remove(this->overlay);

		if( this->overlay->isCancel() == false ){
			if( this->visThumbs == true || this->visList == true ){
				//this->getScene()->loadContent(); // create new child with selected content!
				if( this->core->getCacher() != NULL )
					this->core->getCacher()->setFull(false);
				if( this->core->getGui() != NULL )
					this->core->getGui()->updateText();
			}
		}
		else {
			if( this->visThumbs == true ){
				if( this->core->getCacher() != NULL ){
					Cell *last = this->getThumblay()->getLastCell();
					this->core->getCacher()->gotoCell(last);
					this->core->getCacher()->setFull(false);
				}
			}
			if( this->visList == true ){
				if( this->core->getCacher() != NULL ){
					Cell *last = this->getListlay()->getLastCell();
					this->core->getCacher()->gotoCell(last);
					this->core->getCacher()->setFull(false);
				}
			}
		}
		delete this->overlay;
	}
	else
		invalid = false;

	this->overlay = NULL;
	this->thumblay = NULL;
	this->listlay = NULL;

	this->visAbout = false;
	this->visManual = false;
	this->visThumbs = false;
	this->visList = false;

	if( invalid == true ){
		if( this->core->getGui() != NULL ){
			this->core->getGui()->updateText();
			this->core->getGui()->setCursor(CURSOR_CLIENT);
		}
		this->invalidate();
	}
}

void Drawer::showAbout()
{
	if( this->visAbout == false ){
		this->hideOverlay(false);
		this->overlay =
			new Overlay(this->core,Core::getImageResource(IDR_ABOUT,RC_PNG));
		this->layers->add(this->overlay);
		this->visAbout = true;
	}
	else {
		this->hideOverlay(false);
		this->visAbout = false;
	}
	this->invalidate();
}
void Drawer::showManual()
{
	if( this->visManual == false ){
		/*this->hideOverlay(false);
		this->overlay =
			new Overlay(this->core,Core::getImageResource(IDR_MANUAL,RC_PNG));
		this->layers->add(this->overlay);
		this->visManual = true;*/
	}
	else {
		this->hideOverlay(false);
		this->visManual = false;
	}
	this->invalidate();
}
void Drawer::showThumbs()
{
	HCURSOR last = NULL;
	if( this->core->getGui() != NULL )
		last = this->core->getGui()->setCursor(CURSOR_WAIT);

	if( this->visThumbs == false ){
		this->hideOverlay(false);

		this->thumblay =
			new Thumblay(this->core,NULL);
		this->overlay = this->thumblay;

		this->layers->add(this->overlay);
		this->visThumbs = true;
	}
	else {
		this->hideOverlay(false);
		this->visThumbs = false;
	}
	this->invalidate();

	if( this->core->getGui() != NULL )
		this->core->getGui()->setCursor(last);	
}
void Drawer::showList()
{
	HCURSOR last = NULL;
	if( this->core->getGui() != NULL )
		last = this->core->getGui()->setCursor(CURSOR_WAIT);

	if( this->visList == false ){
		this->hideOverlay(false);

		this->listlay =
			new Listlay(this->core,NULL);
		this->overlay = this->listlay;

		this->layers->add(this->overlay);
		this->visList = true;
	}
	else {
		this->hideOverlay(false);
		this->visList = false;
	}
	this->invalidate();

	if( this->core->getGui() != NULL )
		this->core->getGui()->setCursor(last);	
}

bool Drawer::isAbout()
{
	return this->visAbout;
}
bool Drawer::isManual()
{
	return this->visManual;
}
bool Drawer::isThumbs()
{
	return this->visThumbs;
}
bool Drawer::isList()
{
	return this->visList;
}

void Drawer::setMenuheight(int val)
{
	this->layers->gotoHead();
	do {
		this->layers->getThat()->setMenuheight(val);
	} while( this->layers->next() );

	this->getTopmost()->invalidate();
}
/*
Full redraw invalidation
*/
void Drawer::invalidate(bool full)
{
	this->fulldraw = full;
	InvalidateRect(this->core->getWindowHandle(),NULL,FALSE);
}
/*
Partial redraw invalidation
bmp -	source bitmap
rect -	invalidate rectangle
mode -	addition mode
full -	force full redraw
*/
void Drawer::invalidate(Rect *rect, Bitmap *bmp, int mode, bool full)
{
	double zoom = this->getZoom();

	this->fulldraw = full;
	this->modeInvalidate = mode;

	this->rectInvalidate = Rect(
		(int)(rect->X * zoom) + max(this->rectScene.X,0),
		(int)(rect->Y * zoom) + max(this->rectScene.Y,0),
		(int)(rect->Width * zoom),
		(int)(rect->Height * zoom)
		);
	if( this->bmpInvalidate != NULL )
		delete this->bmpInvalidate;

	this->bmpInvalidate = new Bitmap(
		this->rectInvalidate.Width,
		this->rectInvalidate.Height,
		Core::getPixelFormat());

	Graphics *g = Graphics::FromImage(this->bmpInvalidate);
	this->setInterpolation(g,zoom);

	if( mode < INVSCENE ){
		this->renderBackground(
			g,
			Rect(this->rectInvalidate.X - max(this->rectScene.X,0),
				 this->rectInvalidate.Y - max(this->rectScene.Y,0),
				 this->rectInvalidate.Width,
				 this->rectInvalidate.Height)
			);
	}

	if( zoom > ZOOMINIT ){
		Bitmap *bigbmp = 
			new Bitmap(
				this->bmpInvalidate->GetWidth(),
				this->bmpInvalidate->GetHeight(),
				this->bmpInvalidate->GetPixelFormat());

		this->scene->scaleEnlarge(bmp,bigbmp,zoom);
		g->DrawImage(bigbmp,0,0,bigbmp->GetWidth(),bigbmp->GetHeight());
		delete bigbmp;
	}
	else {
		g->DrawImage(
			bmp,
			Rect(0,0,this->bmpInvalidate->GetWidth(),this->bmpInvalidate->GetHeight()),
			0,0,bmp->GetWidth(),bmp->GetHeight(),UnitPixel );
	}
	delete g;
	delete bmp;

	InvalidateRect(this->core->getWindowHandle(),NULL,FALSE);
}
/*
Zoom by value
*/
void Drawer::zoomer(double val)
{
	if( val > 0 ){
		if( this->zoom < DRZOOMINIT )
			this->zoom += DRZOOMLOSTEP;
		else if( this->zoom >= DRZOOMINIT )
			this->zoom += DRZOOMHISTEP;
	}
	else {
		if( this->zoom <= DRZOOMINIT && this->zoom > DRZOOMLOSTEP )
			this->zoom -= DRZOOMLOSTEP;
		else if( this->zoom > DRZOOMINIT )
			this->zoom -= DRZOOMHISTEP;
	}
	this->zoomSet((double)this->zoom / (double)DRZOOMINIT);
}
/*
Reset zoom
*/
void Drawer::zoomReset()
{
	this->zoomSet(DRZOOMINIT);
}
/*
Set zoom
*/
void Drawer::zoomSet(double val)
{
	this->zoom = (int)(val * DRZOOMINIT);

	if( this->zoom > DRZOOMINIT ){
		if( this->zoom % DRZOOMHISTEP != 0 )
			this->zoom += DRZOOMHISTEP - this->zoom % DRZOOMHISTEP;
	}
	else {
		if( this->zoom % DRZOOMLOSTEP != 0 )
			this->zoom -= this->zoom % DRZOOMLOSTEP;
	}
	this->zoom = max(this->zoom,DRZOOMLOSTEP);

	val = (double)this->zoom / (double)DRZOOMINIT;

	this->layers->gotoHead();
	while( this->layers->getThat() != NULL ){
		this->layers->getThat()->zoomat(val,false);
		if( this->layers->next() == false )
			break;
	}
	this->core->setText();
	this->invalidate();
}

void Drawer::scroll(int x, int y)
{
	this->layers->gotoHead();
	while( this->layers->getThat() != NULL ){
		this->layers->getThat()->scroll(x,y,false);
		if( this->layers->next() == false )
			break;
	}
	this->invalidate();
}

void Drawer::scrollSet(int x, int y)
{
	this->layers->gotoHead();
	while( this->layers->getThat() != NULL ){
		this->layers->getThat()->scrollSet(x,y,false);
		if( this->layers->next() == false )
			break;
	}
	this->invalidate();
}
/*
Translates the pixel x,y to Workspace coordinates
and sets that point as the center of current child client
*/
void Drawer::scrollPinpoint(int x, int y)
{
	double zoom = this->getZoom();
	RECT client = this->getClientSize();

	Point pin((int)(x * zoom),(int)(y * zoom));
	Point corner(pin.X - client.right/2,pin.Y - client.bottom/2);

	this->scene->locate();

	this->scrollSet(
		this->scene->getMaxrollHor() - corner.X,
		this->scene->getMaxrollVer() - corner.Y
		);
}
/*
Sets the additional drawing path
*/
void Drawer::setDrawpath(GraphicsPath *path, bool zoom, bool invalidate)
{
	if( this->drawpath != NULL )
		delete this->drawpath;
	this->drawpath = path;
	this->zoomdraw = zoom;

	if( invalidate == true )
		this->invalidate();
}
/*
Sets the path pen size
*/
void Drawer::setDrawpenSize(int size)
{
	this->penDraw->SetWidth((REAL)max(size,1));
}
/*
Sets the path pen style
*/
void Drawer::setDrawpenStyle(DashStyle style)
{
	this->penDraw->SetDashStyle(style);
}

void Drawer::setInterpolation(Graphics *g, double zoom)
{
	if( zoom < ZOOMINIT )
		g->SetInterpolationMode(InterpolationModeBilinear);
	else
		g->SetInterpolationMode(InterpolationModeNearestNeighbor);
}

void Drawer::setHQInterpolation(Graphics *g, double zoom)
{
	g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
}
/*
Renders the transparency background onto a specified Graphics with specified zoom
*/
bool Drawer::renderBackground(Gdiplus::Graphics *scenegfx, double zoom, bool supress)
{
	if( this->gridlay != NULL ){
		if( this->gridlay->isRendered() == true ){
			int x = this->gridlay->getX();
			int y = this->gridlay->getY();
			if( supress == true ){
				x = min(x,0);
				y = min(y,0);
			}

			scenegfx->DrawImage(
				this->gridlay->render(),
				Rect(x,y,(int)( this->gridlay->getWidth() / zoom),(int)( this->gridlay->getHeight() / zoom )),
				0,0,this->gridlay->getWidth(),this->gridlay->getHeight(),
				UnitPixel);
			return true;
		}
	}
	return false;
}
/*
Renders the transparency grid onto a Graphics object bounded with the specified rectangle
*/
bool Drawer::renderBackground(Gdiplus::Graphics *scenegfx, Rect rect)
{
	if( this->gridlay != NULL ){
		if( this->gridlay->isRendered() == true ){
			scenegfx->DrawImage(
				this->gridlay->render(),
				Rect(0,0,rect.Width,rect.Height),
				rect.X/* + this->gridlay->getX()*/,
				rect.Y/* + this->gridlay->getY()*/,
				rect.Width,
				rect.Height,
				UnitPixel);
			return true;
		}
	}
	return false;
}
/*
Preloads the transparency grid basing on the Layer position, size and scroll info
*/
void Drawer::preloadBackground(Layer *lay)
{
	if( this->scene != NULL ){
		this->gridlay->preload(
			this->scene->getClientSize(),
			this->scene->getScrollInfo(),
			this->scene->getX(),
			this->scene->getY(),
			this->scene->getWidth(),
			this->scene->getHeight()
			);

		this->rectScene = Rect(
			this->scene->getX(),
			this->scene->getY(),
			this->scene->getWidth(),
			this->scene->getHeight()
			);

		if( this->isPreload == false ){
			this->isPreload = true;
			this->core->getWorkspace()->updateToolws();
		}
	}
}
/*
Renders the selection path onto a specified bitmap
*/
void Drawer::renderSelection(Bitmap *scene)
{
	this->renderGraphicsPath(
		scene,
		this->core->getWorkspace()->getSelection(),
		this->penSelection,
		2,
		true,
		true
		);
}
/*
Renders an additional drawing path onto a specified bitmap
*/
void Drawer::renderDraw(Bitmap *scene)
{
	this->renderGraphicsPath(
		scene,
		this->drawpath,
		this->penDraw,
		NULL,
		this->zoomdraw
		);
}
/*
Renders a graphics path onto a bitmap
scene -		target bitmap
current -	graphics path
pen -		pen to use
hatch -		hatch brush mode
zoom -		use zoom flag
zoomround -	round zoom to DRZOOMINIT compatible values flag
*/
void Drawer::renderGraphicsPath(Bitmap *scene, GraphicsPath *current, Pen *pen, int hatch, bool zoom, bool zoomround)
{
	if( current == NULL )
		return;

	Rect srcRect(
		0,//max(this->rectScene.X,0),
		0,//max(this->rectScene.Y,0),
		scene->GetWidth(),//min(this->rectScene.Width,scene->GetWidth()),
		scene->GetHeight() );//min(this->rectScene.Height,scene->GetHeight()) );
	Rect bmpRect(
		0,
		0,
		srcRect.Width,
		srcRect.Height );

	Bitmap *selection = 
		new Bitmap(srcRect.Width,srcRect.Height,scene->GetPixelFormat());
	Graphics *gfx = Graphics::FromImage(selection);

	GraphicsPath *path = new GraphicsPath();
	path->AddPath(current,FALSE);

	int shx, shy;
	shx = this->rectScene.X;//min(this->rectScene.X,0);
	shy = this->rectScene.Y;//min(this->rectScene.Y,0);

	if( zoom == true ){
		Matrix zoomer;
		double z = this->getZoom();

		zoomer.Scale((REAL)z,(REAL)z);
		path->Transform(&zoomer);

		if( z > ZOOMINIT && zoomround == true ){
			if( this->rectScene.X < 0 )
				shx -= shx % (int)z;
			if( this->rectScene.Y < 0 )
				shy -= shy % (int)z;
		}
	}
	gfx->TranslateTransform((REAL)shx,(REAL)shy);
	gfx->DrawPath(pen,path);

	delete path;
	delete gfx;

	Layer::scaleInvert(
		scene,
		&srcRect,
		selection,
		&bmpRect,
		hatch
		);		

	delete selection;
}