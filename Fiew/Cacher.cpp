/*
Cacher.cpp
Inherited from Fiew 2.0
Cacher browses through Explorer filepath data and loads
raw image data (NOT as Gdiplus objects) into memory.
It uses multithreading and can manage strictly defined
cache size ie. only 10 images before and after the current
image.
Cell object that is used for Cacher data list is responsible
for retriving Gdiplus images and thumbnails from raw image
data.
*/

#include "stdafx.h"
#include "Core.h"

Cell::Cell(File *file, byte *data)
{
	this->file = file;
	this->thumb = NULL;
	this->stream = NULL;

	this->loadStream(data,this->file->getSize());
}

Cell::Cell(File *file)
{
	this->file = file;
	this->thumb = NULL;
	this->stream = NULL;

#ifdef DISKCACHING
	HANDLE hFile = 
		CreateFile(this->file->getFilePath()->toWCHAR(),
				   GENERIC_READ,
				   FILE_SHARE_READ,
				   NULL,
				   OPEN_EXISTING,
				   FILE_ATTRIBUTE_NORMAL,
				   NULL);

	if( hFile != INVALID_HANDLE_VALUE ){
		byte *buffer = new byte[this->file->getSize()];
		BOOL result = 0;
		DWORD read = 0;

		result = ReadFile(hFile,buffer,this->file->getSize(),&read,NULL);
		if( result && read == this->file->getSize() )
			this->loadStream(buffer,read);

		delete [] buffer;
	}
	CloseHandle(hFile);
#endif
}

Cell::~Cell()
{
	if( this->thumb != NULL )
		delete this->thumb;

	if( this->stream != NULL )
		this->stream->Release();
}

bool Cell::loadStream(byte *data, DWORD len)
{
#ifndef DISKCACHING
	if( this->file->isArchived() == false )
		return false;
#endif

	if( data == NULL || len < 1 )
		return false;
	if( this->stream != NULL )
		this->stream->Release();

	HGLOBAL buffer = NULL;

	if( (buffer = GlobalAlloc(GPTR,len)) != NULL )
		if( CreateStreamOnHGlobal(buffer,true,(LPSTREAM*)&this->stream) == S_OK )
			if( this->stream->Write((void *)data,len,NULL) == S_OK )
				return true;

	if( this->stream != NULL )
		this->stream->Release();
	this->stream = NULL;

	return false;
}

Cell *Cell::loadThumb(int load)
{
	if( load == YES )
		this->getImageThumb();

	return this;
}

File *Cell::getFile()
{
	return this->file;
}

Image *Cell::getImage()
{
#ifndef DISKCACHING
	if( this->file->isArchived() == false )
		return Image::FromFile(this->file->getFilePath()->toWCHAR());
#endif

	if( this->stream != NULL )
		return Image::FromStream(this->stream);
	return NULL;
}

Bitmap *Cell::getImageThumb(int width, int height)
{
	Image *img = this->getImage();

	if( img != NULL ){
		Bitmap *thumb = new Bitmap(width,height,Core::getPixelFormat());
		Graphics *gfx = Graphics::FromImage(thumb);

		gfx->DrawImage(img,0,0,width,height);

		delete gfx;
		delete img;

		return thumb;
	}
	return NULL;
}
Bitmap *Cell::getImageThumb()
{
	if( this->thumb == NULL ){
		Image *img = this->getImage();

		if( img != NULL ){
			this->thumb = new Bitmap(THB_SIZE,THB_SIZE,Core::getPixelFormat());

			int frame = 1;
			Graphics *gfx = Graphics::FromImage(this->thumb);

			int iwidth = img->GetWidth();
			int iheight = img->GetHeight();
			int thbSize = THB_SIZE - 4*frame;

			if( iwidth > thbSize || iheight > thbSize ){
				double dx = (double)thbSize / (double)iwidth;
				double dy = (double)thbSize / (double)iheight;

				iwidth = (int)(min(dx,dy) * iwidth);
				iheight = (int)(min(dx,dy) * iheight);
			}
			int x = max( (int)((thbSize - iwidth) / 2), 2*frame );
			int y = max( (int)((thbSize - iheight) / 2), 2*frame );

			gfx->DrawImage(img,x,y,iwidth,iheight);

			delete gfx;
			delete img;
		}		
	}
	return this->thumb;
}

IStream *Cell::getStream()
{
	return this->stream;
}

bool Cell::isLoaded()
{
#ifndef DISKCACHING
	if( this->file->isArchived() == false )
		return true;
#endif

	if( this->stream != NULL )
		return true;
	return false;
}

/* *** */

Cacher::Cacher(Core *core)
{
	this->mut_cache = NULL;
	this->mut_bool = CreateMutex(NULL,false,NULL);

	this->mut_initloop = NULL;
	this->mut_initterminator = NULL;

	this->mut_nextloop = NULL;
	this->mut_nextstep = NULL;
	this->mut_nextterminator = NULL;
	this->mut_prevloop = NULL;
	this->mut_prevstep = NULL;
	this->mut_prevterminator = NULL;

	this->thrd_init = NULL;
	this->thrd_next = NULL;
	this->thrd_prev = NULL;

	this->sem_nexts = NULL;
	this->sem_prevs = NULL;
	this->sem_nextlock = NULL;
	this->sem_prevlock = NULL;

	this->cache = new List<Cell>();
	this->core = core;
	this->full = NO;

	this->source = this->core->getExplorer()->getRoot();
	this->init();
}

Cacher::~Cacher()
{
	if( WaitForSingleObject(this->mut_initloop,0) == WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_initterminator);
		WaitForSingleObject(this->mut_initloop,INFINITE);
		ReleaseMutex(this->mut_initloop);
	}
	if( WaitForSingleObject(this->mut_nextloop,0) == WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_nextterminator);
		ReleaseSemaphore(this->sem_nextlock,1,NULL);
		WaitForSingleObject(this->mut_nextloop,INFINITE);
		ReleaseMutex(this->mut_nextloop);
	}
	if( WaitForSingleObject(this->mut_prevloop,0) == WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_prevterminator);
		ReleaseSemaphore(this->sem_prevlock,1,NULL);
		WaitForSingleObject(this->mut_prevloop,INFINITE);
		ReleaseMutex(this->mut_prevloop);
	}

	if( this->mut_cache != NULL )
		CloseHandle(this->mut_cache);
	if( this->mut_bool != NULL )
		CloseHandle(this->mut_bool);

	if( this->mut_initloop != NULL )
		CloseHandle(this->mut_initloop);
	if( this->mut_initterminator != NULL )
		CloseHandle(this->mut_initterminator);

	if( this->mut_nextloop != NULL )
		CloseHandle(this->mut_nextloop);
	if( this->mut_nextstep != NULL )
		CloseHandle(this->mut_nextstep);
	if( this->mut_nextterminator != NULL )
		CloseHandle(this->mut_nextterminator);

	if( this->mut_prevloop != NULL )
		CloseHandle(this->mut_prevloop);
	if( this->mut_prevstep != NULL )
		CloseHandle(this->mut_prevstep);
	if( this->mut_prevterminator != NULL )
		CloseHandle(this->mut_prevterminator);

	if( this->sem_nexts != NULL )
		CloseHandle(this->sem_nexts);
	if( this->sem_prevs != NULL )
		CloseHandle(this->sem_prevs);
	if( this->sem_nextlock != NULL )
		CloseHandle(this->sem_nextlock);
	if( this->sem_prevlock != NULL )
		CloseHandle(this->sem_prevlock);

	if( this->thrd_init != NULL )
		CloseHandle(this->thrd_init);
	if( this->thrd_next != NULL )
		CloseHandle(this->thrd_next);
	if( this->thrd_prev != NULL )
		CloseHandle(this->thrd_prev);

	delete this->cache;
}

void Cacher::init()
{
	if( this->source == NULL )
		return;
	if( this->source->getCount() <= 0 ){
		this->source = NULL;
		return;
	}
	this->cache->add( this->source->load()->loadThumb(this->isFull()) );

	this->mut_cache = CreateMutex(NULL,false,NULL);

	this->mut_initloop = CreateMutex(NULL,false,NULL);
	this->mut_initterminator = CreateMutex(NULL,true,NULL);

	this->mut_nextloop = CreateMutex(NULL,false,NULL);
	this->mut_nextstep = CreateMutex(NULL,false,NULL);
	this->mut_nextterminator = CreateMutex(NULL,true,NULL);

	this->mut_prevloop = CreateMutex(NULL,false,NULL);
	this->mut_prevstep = CreateMutex(NULL,false,NULL);
	this->mut_prevterminator = CreateMutex(NULL,true,NULL);

	this->sem_nexts = CreateSemaphore(NULL,0,INT_MAX,NULL);
	this->sem_prevs = CreateSemaphore(NULL,0,INT_MAX,NULL);
	this->sem_nextlock = CreateSemaphore(NULL,0,1,NULL);
	this->sem_prevlock = CreateSemaphore(NULL,0,1,NULL);

	this->thrd_init = CreateThread(NULL,NULL,
								  (LPTHREAD_START_ROUTINE)&Cacher::initAlloc,
								   this,NULL,NULL);

	this->thrd_next = CreateThread(NULL,NULL,
								  (LPTHREAD_START_ROUTINE)&Cacher::nextAlloc,
								   this,NULL,NULL);
	this->thrd_prev = CreateThread(NULL,NULL,
								  (LPTHREAD_START_ROUTINE)&Cacher::prevAlloc,
								   this,NULL,NULL);
}

DWORD WINAPI Cacher::initAlloc(LPVOID param)
{
	Cacher *that = (Cacher *)param;

	int count = 0;
	bool rNext = true;
	bool rPrev = true;
	Cell *tmp = NULL;

	WaitForSingleObject(that->mut_initloop,INFINITE);
	while( count < CACHE_SIZE || that->isFull() != NO ){
		if( WaitForSingleObject(that->mut_initterminator,0) != WAIT_TIMEOUT ){
			ReleaseMutex(that->mut_initterminator);
			ReleaseMutex(that->mut_initloop);
			return 0;
		}
		if( rNext == true )
		if( (rNext = that->getSource()->next(RIGHT)) == true ){
			WaitForSingleObject(that->mut_cache,INFINITE);

			if( (tmp = that->getSource()->load(RIGHT)) != NULL )
				if( that->getCache()->add( tmp->loadThumb(that->isFull()) ) )
					ReleaseSemaphore(that->sem_nexts,1,NULL);

			ReleaseMutex(that->mut_cache);
		}
		if( rPrev == true )
		if( (rPrev = that->getSource()->prev(LEFT)) == true ){
			WaitForSingleObject(that->mut_cache,INFINITE);

			if( (tmp = that->getSource()->load(LEFT)) != NULL )
				if( that->getCache()->addToHead( tmp->loadThumb(that->isFull()) ) )
					ReleaseSemaphore(that->sem_prevs,1,NULL);

			that->cache;
			ReleaseMutex(that->mut_cache);
		}
		if( rNext == false && rPrev == false )
			break;
		that->cache;
		count++;
	}
	ReleaseMutex(that->mut_initloop);

	return 0;
}
DWORD WINAPI Cacher::nextAlloc(LPVOID param)
{
	Cacher *that = (Cacher *)param;
	Cell *tmp = NULL;

	WaitForSingleObject(that->mut_nextloop,INFINITE);
	while( true ){
		WaitForSingleObject(that->sem_nextlock,INFINITE);

		int count = 0;
		WaitForSingleObject(that->mut_nextstep,INFINITE);
		while( count < CACHE_SIZE || that->isFull() != NO ){
			if( WaitForSingleObject(that->mut_nextterminator,0) != WAIT_TIMEOUT ){
				ReleaseMutex(that->mut_nextterminator);
				ReleaseMutex(that->mut_nextloop);
				return 0;
			}
			WaitForSingleObject(that->mut_cache,INFINITE);
			if( that->getSource()->next(RIGHT) ){
				if( (tmp = that->getSource()->load(RIGHT)) != NULL )
					if( that->getCache()->add( tmp->loadThumb(that->isFull()) ) )
						ReleaseSemaphore(that->sem_nexts,1,NULL);

				ReleaseMutex(that->mut_cache);
			}
			else {
				ReleaseMutex(that->mut_cache);
				break;
			}
			count++;
		}
		ReleaseMutex(that->mut_nextstep);
	}
	ReleaseMutex(that->mut_nextloop);
	return 0;
}
DWORD WINAPI Cacher::prevAlloc(LPVOID param)
{
	Cacher *that = (Cacher *)param;
	Cell *tmp = NULL;

	WaitForSingleObject(that->mut_prevloop,INFINITE);
	while( true ){
		WaitForSingleObject(that->sem_prevlock,INFINITE);

		int count = 0;
		WaitForSingleObject(that->mut_prevstep,INFINITE);
		while( count < CACHE_SIZE || that->isFull() != NO ){
			if( WaitForSingleObject(that->mut_prevterminator,0) != WAIT_TIMEOUT ){
				ReleaseMutex(that->mut_prevterminator);
				ReleaseMutex(that->mut_prevloop);
				return 0;
			}
			WaitForSingleObject(that->mut_cache,INFINITE);
			if( that->getSource()->prev(LEFT) ){
				if( (tmp = that->getSource()->load(LEFT)) != NULL )
					if( that->getCache()->addToHead( tmp->loadThumb(that->isFull()) ) )
						ReleaseSemaphore(that->sem_prevs,1,NULL);

				ReleaseMutex(that->mut_cache);
			}
			else {
				ReleaseMutex(that->mut_cache);
				break;
			}
			count++;
		}
		ReleaseMutex(that->mut_prevstep);
	}
	ReleaseMutex(that->mut_prevloop);
	return 0;
}

Catalog *Cacher::getSource()
{
	return this->source;
}

List<Cell> *Cacher::getCache()
{
	return this->cache;
}

void Cacher::setFull(bool val, bool thumbs)
{
	WaitForSingleObject(this->mut_bool,INFINITE);
	if( val == true && thumbs == true )
		this->full = YES;
	if( val == true && thumbs == false )
		this->full = NOTHUMBS;
	if( val == false )
		this->full = NO;
	ReleaseMutex(this->mut_bool);

	this->unlockNext();
	this->unlockPrev();
}
int Cacher::isFull()
{
	WaitForSingleObject(this->mut_bool,INFINITE);
	int val = this->full;
	ReleaseMutex(this->mut_bool);

	return val;
}

bool Cacher::isRunning()
{
	bool result = false;

	if( WaitForSingleObject(this->mut_initloop,0) == WAIT_TIMEOUT )
		result = true;
	else
		ReleaseMutex(this->mut_initloop);

	if( WaitForSingleObject(this->mut_nextstep,0) == WAIT_TIMEOUT )
		result = true;
	else
		ReleaseMutex(this->mut_nextstep);

	if( WaitForSingleObject(this->mut_prevstep,0) == WAIT_TIMEOUT )
		result = true;
	else
		ReleaseMutex(this->mut_prevstep);
	
	return result;
}

bool Cacher::next()
{
	bool result = false;

	if( this->getSource() == NULL )
		return result;

	WaitForSingleObject(this->mut_cache,INFINITE);
	if( this->getSource()->isThatTail() == true ){
		ReleaseMutex(this->mut_cache);
		return result;
	}
	ReleaseMutex(this->mut_cache);

	WaitForSingleObject(this->sem_nexts,INFINITE);
	ReleaseSemaphore(this->sem_prevs,1,NULL);

	WaitForSingleObject(this->mut_cache,INFINITE);
	if( (result = this->getCache()->next()) == true )
		this->getSource()->next();
	this->unlockNext();
	ReleaseMutex(this->mut_cache);

	return result;
}
bool Cacher::prev()
{
	bool result = false;

	if( this->getSource() == NULL )
		return result;

	WaitForSingleObject(this->mut_cache,INFINITE);
	if( this->getSource()->isThatHead() == true ){
		ReleaseMutex(this->mut_cache);
		return result;
	}
	ReleaseMutex(this->mut_cache);

	WaitForSingleObject(this->sem_prevs,INFINITE);
	ReleaseSemaphore(this->sem_nexts,1,NULL);

	WaitForSingleObject(this->mut_cache,INFINITE);
	if( (result = this->getCache()->prev()) )
		this->getSource()->prev();
	this->unlockPrev();
	ReleaseMutex(this->mut_cache);

	return result;
}

Cell *Cacher::getThat()
{
	Cell *tmp = NULL;

	WaitForSingleObject(this->mut_cache,INFINITE);
	tmp = this->getCache()->getThat();
	ReleaseMutex(this->mut_cache);

	if( tmp != NULL )
		return tmp;
	return NULL;
}

Image *Cacher::getThatImage()
{
	Cell *tmp = this->getThat();

	if( tmp != NULL )
		return tmp->getImage();
	return NULL;
}
Image *Cacher::getThatThumb(int width, int height)
{
	Cell *tmp = this->getThat();

	if( tmp != NULL )
		return tmp->getImageThumb(width,height);
	return NULL;
}

void Cacher::unlockNext()
{
	if( WaitForSingleObject(this->mut_initloop,0) != WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_initloop);
		if( WaitForSingleObject(this->mut_nextstep,0) != WAIT_TIMEOUT ){
			ReleaseMutex(this->mut_nextstep);
			if( this->getCache()->countRightoThat() < CACHE_LIMIT || this->isFull() != NO )
				ReleaseSemaphore(this->sem_nextlock,1,NULL);
		}
		if( WaitForSingleObject(this->mut_prevstep,0) != WAIT_TIMEOUT ){
			ReleaseMutex(this->mut_prevstep);
			int size = 0;
			if(	(size = this->getCache()->countLeftoThat()) > CACHE_SIZE &&
				this->isFull() == NO ){

				size -= CACHE_SIZE;
				for( int i = 0; i < size; i++ ){
					delete this->getCache()->removeHead();
					this->getSource()->next(LEFT);
				}
			}
		}
	}
}
void Cacher::unlockPrev()
{
	if( WaitForSingleObject(this->mut_initloop,0) != WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_initloop);
		if( WaitForSingleObject(this->mut_prevstep,0) != WAIT_TIMEOUT ){
			ReleaseMutex(this->mut_prevstep);
			if( this->getCache()->countLeftoThat() < CACHE_LIMIT || this->isFull() != NO )
				ReleaseSemaphore(this->sem_prevlock,1,NULL);
		}
		if( WaitForSingleObject(this->mut_nextstep,0) != WAIT_TIMEOUT ){
			ReleaseMutex(this->mut_nextstep);
			int size = 0;
			if( (size = this->getCache()->countRightoThat()) > CACHE_SIZE &&
				this->isFull() == NO ){

				size -= CACHE_SIZE;
				for( int i = 0; i < size; i++ ){
					delete this->getCache()->removeTail();
					this->getSource()->prev(RIGHT);
				}
			}
		}
	}
}

void Cacher::lockCache()
{
	WaitForSingleObject(this->mut_cache,INFINITE);
}
void Cacher::unlockCache()
{
	ReleaseMutex(this->mut_cache);
}

void Cacher::gotoCell(Cell *cell)
{
	if( cell == NULL )
		return;

	this->lockCache();

	int i, count = 0;
	Cell *left, *right;

	do {
		left = this->getCache()->getLeftoThat(count);
		right = this->getCache()->getRightoThat(count);

		if( right == NULL && left == NULL ){
			count = 0;
			break;
		}
		if( left == cell ){
			count = -count;
			break;
		}
		if( right == cell )
			break;

		count++;
	} while( true );

	this->unlockCache();

	if( count < 0 )
		for( i = 0; i > count; i-- )
			this->prev();
	if( count > 0 )
		for( i = 0; i < count; i++ )
			this->next();
}