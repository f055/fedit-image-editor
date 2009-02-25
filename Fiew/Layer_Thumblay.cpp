/*
Layer_Thumblay.cpp
Partially inherited from Fiew 2.0
Object responsible for displaying thumbnail view, ie. for opening a folder dialog.
It relies on a Cacher object, and is refreshed to represent current Cacher state.
*/

#include "stdafx.h"
#include "Core.h"
/*
Construct Thumblay as Overlay, like in Fiew 2.0
*/
Thumblay::Thumblay(ChildCore *core, Image *image) : Overlay(core,image)
{
	this->explorer = NULL;
	this->cacher = core->getCacher();

	this->lastCell = NULL;
	this->ticker = TICKER_OFF;
	this->picker = NULL;

	if( this->image != NULL )
		delete this->image;

	if( this->core->getCacher() != NULL ){
		this->lastCell = this->core->getCacher()->getThat();
		this->core->getCacher()->setFull(true);
	}
	
	this->image = new Bitmap(OVL_SIZE,OVL_SIZE,Core::getPixelFormat());
	this->update(true);
}
/*
Construct Thumblay as dialog, like in Fedit 1.0
*/
Thumblay::Thumblay(HWND hOwner, Core *core, FwCHAR *path) : Overlay(NULL,NULL)
{
	this->hOwner = hOwner;
	this->hardcore = core;

	this->lastCell = NULL;
	this->ticker = TICKER_OFF;
	this->picker = NULL;

	this->explorer = new Explorer(core);
	this->cacher = NULL;
	this->image = NULL;

	if( this->explorer->browse( new FwCHAR(path->toWCHAR()) ) == true ){
		this->hardcore->setExplorer(this->explorer);

		this->cacher = new Cacher(core);

		this->lastCell = this->cacher->getThat();
		this->cacher->setFull(true);

		this->image = new Bitmap(OVL_SIZE,OVL_SIZE,Core::getPixelFormat());
		this->update(true);
	}
	else {
		EndDialog(this->hOwner,NO);
		DestroyWindow(this->hOwner);
	}
}

Thumblay::~Thumblay()
{
	this->hardcore->setExplorer(NULL);

	if( this->cacher != NULL )
		delete this->cacher;
	if( this->explorer != NULL )
		delete this->explorer;
}
/*
Update the view of the state of thumbnail list
and eventually tick the ticker representing Cacher processing
*/
void Thumblay::update(bool init)
{
	this->subrender();
	if( this->core != NULL )
		this->invalidate(init);
	else
		InvalidateRect(this->hOwner,NULL,TRUE);

	if( this->cacher != NULL ){
		if( this->cacher->isRunning() == true ){
			this->ticker++;
			this->ticker = this->ticker % TICKER_STEPS;
			
			HWND owner = NULL;
			if( this->core != NULL )
				owner = this->core->getWindowHandle();
			else
				owner = this->hOwner;

			SetTimer(owner,TIMER_THB,THB_TOUT,NULL);

			return;
		}
	}
	if( this->ticker > TICKER_OFF ){
		this->ticker = TICKER_OFF;

		HWND owner = NULL;
			if( this->core != NULL )
				owner = this->core->getWindowHandle();
			else
				owner = this->hOwner;

		SetTimer(owner,TIMER_THB,THB_TOUT,NULL);
	}
}
/*
Overloaded method from Layer object
Opens the currently selected file on thumbnails list
in an editor MDI child window
*/
void Thumblay::hide()
{
	ChildCore *child = NULL;

	if( this->lastCell->getFile()->isArchived() == true ){
		FwCHAR *archpath = this->explorer->getArchivePath(); 
		FwCHAR *filepath = this->lastCell->getFile()->getFilePath();

		FwCHAR *file = new FwCHAR();
		file->getFilenameFrom(filepath);
		FwCHAR *diskpath = new FwCHAR();
		diskpath->getFolderFrom(archpath);

		diskpath->mergeWith(file->toWCHAR());
		delete file;

		child = new ChildCore(
			this->hardcore,
			(Bitmap *)this->lastCell->getImage(),
			diskpath
			);
		this->hardcore->neww(child);
	}
	else {
		this->hardcore->open( new FwCHAR(this->lastCell->getFile()->getFilePath()->toWCHAR()) );
	}
}
/*
Overloaded method from Layer object
Represents RMOUSEBUTTON click, closes the Thumblay
*/
void Thumblay::nextImage(int x, int y)
{
	if( this->core != NULL ){
		this->setCancel();
		this->hide();
	}
	else {
		EndDialog(this->hOwner,YES);
	}
}
/*
Overloaded method from Layer object
Represents LMOUSEBUTTON click, chooses a thumbnail
*/
void Thumblay::prevImage(int x, int y)
{
	if( x != FERROR && y != FERROR ){
		int newpick = this->getPicker(x,y);
		this->setPicker(newpick);
	}
}
void Thumblay::prevImageDblClk(int x, int y)
{
	if( x != FERROR && y != FERROR ){
		int newpick = this->getPicker(x,y);
		if( newpick == this->picker )
			this->hide();
		this->setPicker(newpick);
	}
}
/*
Sets the current thumbnail
*/
void Thumblay::setPicker(int newpick)
{
	if( this->cacher != NULL ){
		if( newpick != INT_MAX ){

			int i, diff = abs(newpick - this->picker);
			bool result = false;

			this->cacher->lockCache();

			for( i = 0; i < diff; i++ ){
				if( newpick < this->picker ){
					if( this->cacher->prev() == false )
						break;
					this->picker--;
					if( this->picker < -THB_COUNT )
						this->picker = -THB_COUNT + THB_ROW - 1;

					result = true;
				}
				else {
					if(	this->cacher->next() == false )
						break;
					this->picker++;
					if( this->picker > THB_COUNT )
						this->picker = THB_COUNT - THB_ROW + 1;

					result = true;
				}
			}
			this->lastCell = this->cacher->getThat();

			this->cacher->unlockCache();

			if( result == true ){
				this->subrender();

				if( this->core != NULL )
					this->invalidate(false);
				else
					InvalidateRect(this->hOwner,NULL,TRUE);
			}
		}
	}
}
/*
Gets the index of a thumbnail basing on mouse click position
*/
int Thumblay::getPicker(int x, int y)
{
	RECT lay;
	if( this->core != NULL )
		lay = this->getOverlayRect();
	else
		SetRect(&lay,0,0,OVL_SIZE,OVL_SIZE);

	int picker = INT_MAX;

	if( x > lay.left && x < lay.right &&
		y > lay.top && y < lay.bottom ){

		x -= (lay.left + OVL_MARGIN);
		y -= (lay.top + OVL_MARGIN);

		int col = (int)floor( (double)(x / (THB_SMSIZE + THB_SPACE)) );
		int row = (int)floor( (double)(y / (THB_SMSIZE + THB_SPACE)) );

		if( col < THB_ROW && row < THB_ROW )
			picker = col - THB_COUNT + THB_ROW * row;
	}
	return picker;
}

void Thumblay::scroll(int hor, int ver, bool invalidate)
{
	return;
}
void Thumblay::scrollSet(int x, int y, bool invalidate)
{
	return;
}
void Thumblay::scrollHor(int val)
{
	if( val < 0 )
		this->setPicker(this->picker + 1);
	else if( val > 0 )
		this->setPicker(this->picker - 1);
}
void Thumblay::scrollVer(int val)
{
	if( val < 0 )
		this->setPicker(this->picker + THB_ROW);
	else if( val > 0 )
		this->setPicker(this->picker - THB_ROW);
}

void Thumblay::zoomer(double val)
{
	return;
}
void Thumblay::zoomat(double val, bool invalidate)
{
	return;
}
void Thumblay::zoomend(bool invalidate)
{
	return;
}

void Thumblay::rotate(int val)
{
	return;
}
void Thumblay::rotateReset(bool novalid)
{
	return;
}

void Thumblay::setFitmode(int mode)
{
	return;
}
void Thumblay::unsetFitmode()
{
	return;
}
void Thumblay::setSidedraw()
{
	return;
}
void Thumblay::setSidemode(int mode)
{
	return;
}
/*
Subrender routing for filling the Layer image object with
thumbnails view scene
*/
void Thumblay::subrender()
{
	Graphics *gfx = Graphics::FromImage(this->image);
	gfx->Clear(CLR_WHITE);

	Cacher *cacher = this->cacher;
	if( cacher != NULL ){
		Image *thumb = NULL;
		Cell *cell = NULL;
		int i, x, y, mx, my, count;
		bool top, bot, left, right;

		right = false;
		left = false;
		top = false;
		bot = false;

		mx = OVL_MARGIN + 2 * (THB_SMSIZE + THB_SPACE);
		my = mx;

		x = y = OVL_MARGIN;
		count = 0;

		for( i = -THB_COUNT - this->picker; i <= THB_COUNT - this->picker; i++ ){
			cacher->lockCache();

			if( cacher->getCache() != NULL ){
				cell = cacher->getCache()->gettoThat(i);
				if( cell != NULL ){
					thumb = cell->getImageThumb();
					if( thumb != NULL ){
						if( i != 0 ){
							gfx->DrawImage(thumb,x,y,THB_SMSIZE,THB_SMSIZE);
							gfx->DrawRectangle(this->Pen_Border,
											   x,
											   y,
											   THB_SMSIZE,
											   THB_SMSIZE);
						}	
						else {
							mx = x + (int)((THB_SIZE - THB_SMSIZE)/4);
							my = y + (int)((THB_SIZE - THB_SMSIZE)/4);
						}
					}
					if( i == -THB_ROW )
						top = true;
					if( i == THB_ROW )
						bot = true;
					if( i == -1 && cacher->getCache()->isThatHead() == false )
						left = true;
					if( i == 1 && cacher->getCache()->isThatTail() == false )
						right = true;
				}
			}
			count++;
			x += THB_SMSIZE + THB_SPACE;
			if( count >= THB_ROW ){
				x = OVL_MARGIN;
				y += THB_SMSIZE + THB_SPACE;
				count = 0;
			}
			cacher->unlockCache();
		}
		int frame = 2;
		cacher->lockCache();
		if( cacher->getCache() != NULL ){
			cell = cacher->getThat();
			if( cell != NULL ){
				thumb = cell->getImageThumb();
				if( thumb != NULL ){
					gfx->FillRectangle(this->Brush_Back,
									   mx - THB_SIZE/4,
									   my - THB_SIZE/4,
									   THB_SIZE,
									   THB_SIZE);
					gfx->DrawImage(thumb,
								   mx - THB_SIZE/4,
								   my - THB_SIZE/4,
								   THB_SIZE,
								   THB_SIZE);
					gfx->DrawRectangle(this->Pen_Border,
									   mx - THB_SIZE/4,
									   my - THB_SIZE/4,
									   THB_SIZE,
									   THB_SIZE);
					gfx->DrawRectangle( this->Pen_DarkBorder,
										mx - frame - THB_SIZE/4,
										my - frame - THB_SIZE/4,
										THB_SIZE + 2*frame,
										THB_SIZE + 2*frame );
				}
			}
		}
		cacher->unlockCache();

		int size = 3;
		int width = 10;
		int ax = (int)(mx - frame - THB_SIZE/4);
		int ay = (int)(my - frame - THB_SIZE/4);
		int asize = THB_SIZE + 2*frame;
		Point arrow[3];

		if( top == true ){
			arrow[0].X = (int)(ax + (THB_SIZE/2) - width);
			arrow[0].Y = ay;
			arrow[1].X = (int)(ax + (THB_SIZE/2));
			arrow[1].Y = ay - width;
			arrow[2].X = (int)(ax + (THB_SIZE/2) + width);
			arrow[2].Y = ay;
			gfx->FillPolygon(this->Brush_DarkBack,arrow,size);
		}
		if( bot == true ){
			arrow[0].X = (int)(ax + (THB_SIZE/2) - width);
			arrow[0].Y = ay + asize;
			arrow[1].X = (int)(ax + (THB_SIZE/2));
			arrow[1].Y = ay + asize + width;
			arrow[2].X = (int)(ax + (THB_SIZE/2) + width);
			arrow[2].Y = ay + asize;
			gfx->FillPolygon(this->Brush_DarkBack,arrow,size);
		}
		if( left == true ){
			arrow[0].X = ax;
			arrow[0].Y = (int)(ay + (THB_SIZE/2) - width);
			arrow[1].X = ax - width;
			arrow[1].Y = (int)(ay + (THB_SIZE/2));
			arrow[2].X = ax;
			arrow[2].Y = (int)(ay + (THB_SIZE/2) + width);
			gfx->FillPolygon(this->Brush_DarkBack,arrow,size);
		}
		if( right == true ){
			arrow[0].X = ax + asize;
			arrow[0].Y = (int)(ay + (THB_SIZE/2) - width);
			arrow[1].X = ax + asize + width;
			arrow[1].Y = (int)(ay + (THB_SIZE/2));
			arrow[2].X = ax + asize;
			arrow[2].Y = (int)(ay + (THB_SIZE/2) + width);
			gfx->FillPolygon(this->Brush_DarkBack,arrow,size);
		}
	}
	if( this->ticker > TICKER_OFF ){
		int tsize = TICKER_SIZE;
		int tx,ty;
		if( this->ticker == 0 ){
			tx = OVL_SIZE - 2*TICKER_INDENT;
			ty = OVL_SIZE - 2*TICKER_INDENT;
		}
		if( this->ticker == 1 ){
			tx = OVL_SIZE - TICKER_INDENT;
			ty = OVL_SIZE - 2*TICKER_INDENT;
		}
		if( this->ticker == 2 ){
			tx = OVL_SIZE - TICKER_INDENT;
			ty = OVL_SIZE - TICKER_INDENT;
		}
		if( this->ticker == 3 ){
			tx = OVL_SIZE - 2*TICKER_INDENT;
			ty = OVL_SIZE - TICKER_INDENT;
		}
		gfx->FillRectangle(this->Brush_DarkBack,tx,ty,tsize,tsize);
	}
	delete gfx;
}

Cell *Thumblay::getLastCell()
{
	return this->lastCell;
}