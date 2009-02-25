/*
Inherited from Fiew 2.0
Displays a list of files in the current Cacher object. For future use.
*/

#include "stdafx.h"
#include "Core.h"

Listlay::Listlay(ChildCore *core, Image *image) : Overlay(core,image)
{
	this->lastCell = NULL;
	this->ticker = TICKER_OFF;

	if( this->image != NULL )
		delete this->image;

	if( this->core->getCacher() != NULL ){
		this->lastCell = this->core->getCacher()->getThat();
		this->core->getCacher()->setFull(true,false);
	}
	
	this->image = new Bitmap(OVL_SIZE,OVL_SIZE,Core::getPixelFormat());
	this->update(true);
}

Listlay::~Listlay()
{
}

void Listlay::update(bool init)
{
	this->subrender();
	this->invalidate(init);

	if( this->core->getCacher() != NULL ){
		if( this->core->getCacher()->isRunning() == true ){
			this->ticker++;
			this->ticker = this->ticker % TICKER_STEPS;
			SetTimer(this->core->getWindowHandle(),TIMER_THB,THB_TOUT,NULL);
			return;
		}
	}
	if( this->ticker > TICKER_OFF ){
		this->ticker = TICKER_OFF;
		SetTimer(this->core->getWindowHandle(),TIMER_THB,THB_TOUT,NULL);
	}
}

void Listlay::nextImage(int x, int y)
{
	this->setCancel();
	this->hide();
}
void Listlay::prevImage(int x, int y)
{
	this->hide();
}

void Listlay::scroll(int hor, int ver, bool invalidate)
{
	if( this->core->getCacher() != NULL ){
		bool result = false;
		HCURSOR last = NULL;
		if( this->core->getGui() != NULL )
			last = this->core->getGui()->setCursor(CURSOR_ARRWAIT);

		int limit, i = 0;
		if( ver < 0 ){
			if( (result = this->core->getCacher()->next()) == true ){
				limit = (int)(abs(ver) / LST_MOD);
				for( i = 0; i < limit; i++ )
					if( this->core->getCacher()->next() == false )
						break;
			}
		}
		else if( ver > 0 ){
			if( (result = this->core->getCacher()->prev()) == true ){
				limit = (int)(abs(ver) / LST_MOD);
				for( i = 0; i < limit; i++ )
					if( this->core->getCacher()->prev() == false )
						break;
			}
		}
		if( result == true ){
			this->subrender();
			if( this->core->getGui() != NULL )
				this->core->getGui()->updateText();

			this->invalidate(false);
		}
		if( this->core->getGui() != NULL )
			this->core->getGui()->setCursor(last);
	}
}
void Listlay::scrollSet(int x, int y, bool invalidate)
{
	return;
}
void Listlay::scrollHor(int val)
{
	return;
}
void Listlay::scrollVer(int val)
{
	if( this->core->getCacher() != NULL ){
		bool result = false;
		HCURSOR last = NULL;
		if( this->core->getGui() != NULL )
			last = this->core->getGui()->setCursor(CURSOR_ARRWAIT);

		if( val < 0 )
			result = this->core->getCacher()->next();

		else if( val > 0 )
			result = this->core->getCacher()->prev();

		if( result == true ){
			this->subrender();
			if( this->core->getGui() != NULL )
				this->core->getGui()->updateText();

			this->invalidate(false);
		}
		if( this->core->getGui() != NULL )
			this->core->getGui()->setCursor(last);
	}
}

void Listlay::zoomer(double val)
{
	return;
}
void Listlay::zoomat(double val, bool invalidate)
{
	return;
}
void Listlay::zoomend(bool invalidate)
{
	return;
}

void Listlay::rotate(int val)
{
	return;
}
void Listlay::rotateReset(bool novalid)
{
	return;
}

void Listlay::setFitmode(int mode)
{
	return;
}
void Listlay::unsetFitmode()
{
	return;
}
void Listlay::setSidedraw()
{
	return;
}
void Listlay::setSidemode(int mode)
{
	return;
}

void Listlay::subrender()
{
	Graphics *gfx = Graphics::FromImage(this->image);
	gfx->Clear(CLR_WHITE);

	Cacher *cacher = this->core->getCacher();
	if( cacher != NULL ){
		FwCHAR *string = NULL;
		FwCHAR *name = NULL;
		Cell *cell = NULL;
		int i, x, y;
		bool top, bot, archived = false;

		top = false;
		bot = false;

		x = LST_X;
		y = LST_Y;

		for( i = -CACHE_SIZE; i < CACHE_SIZE; i++ ){
			cacher->lockCache();

			if( cacher->getCache() != NULL ){
				cell = cacher->getCache()->gettoThat(i);
				if( cell != NULL ){
					if( cell->getFile() != NULL ){
						name = cell->getFile()->getFileName();
					}
					if( name != NULL ){
						if( i == 0 )
							gfx->FillRectangle(this->Brush_LiteBack,0,y+1,OVL_SIZE,FONTSIZE+2);
						
						string = name;
						if( cell->getFile()->isArchived() == true ){
							if( archived == false )
								archived = true;

							string = new FwCHAR(L"~");
							string->mergeWith(name);

							x = 3*LST_X;
						}
						else if( archived == true ){
							archived = false;
							x = LST_X;
						}
						gfx->DrawString(string->toWCHAR(),
										string->toLength(),
										this->Font_Default,
										PointF((REAL)x,(REAL)y),
										this->Brush_DarkBack);

						if( archived == true )
							delete string;
					}
					if( i == -1 && cacher->getCache()->isThatHead() == false )
						top = true;
					if( i == 1 && cacher->getCache()->isThatTail() == false )
						bot = true;
				}
			}
			cacher->unlockCache();

			y += (FONTSIZE + 2);
		}
		int size = 3;
		int width = 20;
		int ax = (int)(OVL_SIZE/2);
		int ay = LST_Y;
		int asize = OVL_SIZE - 2*LST_Y;
		Point arrow[3];

		if( top == true ){
			arrow[0].X = ax - width;
			arrow[0].Y = ay;
			arrow[1].X = ax;
			arrow[1].Y = ay - width;
			arrow[2].X = ax + width;
			arrow[2].Y = ay;
			gfx->FillPolygon(this->Brush_LiteBack,arrow,size);
		}
		if( bot == true ){
			arrow[0].X = ax - width;
			arrow[0].Y = ay + asize;
			arrow[1].X = ax;
			arrow[1].Y = ay + asize + width;
			arrow[2].X = ax + width;
			arrow[2].Y = ay + asize;
			gfx->FillPolygon(this->Brush_LiteBack,arrow,size);
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

Cell *Listlay::getLastCell()
{
	return this->lastCell;
}