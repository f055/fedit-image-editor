/*
Exp_Archive.cpp
Inherited from Fiew 2.0
Class inherits from Catalog. Represents the archive (RAR or ZIP) file
as an object. Enables standard file browsing operations.
*/

#include "stdafx.h"
#include "Core.h"

Archive::Archive(FwCHAR *path, int type, DWORD size) : Catalog(path,type,size)
{
	this->pack = NULL;
	this->unsortedFiles = NULL;
	
	this->sem_sorted = NULL;

	this->sorted = false;
	this->terminate = false;

	switch( type ){
		case TYPE_RAR:
			this->pack = new XUnrar( path->toWCHAR() );
			break;
		case TYPE_ZIP:
			this->pack = new XUnzip( path->toWCHAR() );
			break;
		default:
			this->pack = new XUn( path->toWCHAR() );
			break;
	}
}

Archive::~Archive()
{
	if( WaitForSingleObject(this->mut_loop,0) == WAIT_TIMEOUT ){
		this->terminate = true;
		ReleaseMutex(this->mut_terminator);
		WaitForSingleObject(this->mut_loop,INFINITE);
	}
	if( this->sem_sorted != NULL )
		CloseHandle(this->sem_sorted);

	if( this->pack != NULL )
		delete this->pack;
	if( this->unsortedFiles != NULL )
		delete this->unsortedFiles;
}

void Archive::init(FwCHAR *thatpath)
{
	if( this->pack->isLoaded() == true ){
		this->unsortedFiles = new List<File>();
		this->pack->list(this->unsortedFiles);
		this->count = this->unsortedFiles->getCount();
	}
}

List<File> *Archive::getUnsortedFiles()
{
	return this->unsortedFiles;
}

void Archive::setSorted()
{
	this->sorted = true;
}

bool Archive::isSorted()
{
	return this->sorted;
}

int Archive::getCount()
{
	return this->count;
}

bool Archive::extract()
{
	if( this->getCount() <= 0 )
		return false;

	File *file = NULL;

	WaitForSingleObject(this->mut_list,INFINITE);
	file = this->getFiles()->getThat();
	ReleaseMutex(this->mut_list);

	return this->extract(file);
}
bool Archive::extract(File *file)
{
	if( this->getCount() <= 0 )
		return false;

	if( file == NULL )
		return false;

	FwCHAR *dest = new FwCHAR();
	dest->getFolderFrom(this->filepath);
	dest->mergeWith(file->getFileName());

	bool result = this->pack->extract(file,dest->toWCHAR());

	delete dest;
	return result;
}

void Archive::list(Core *core)
{
	if( this->getCount() <= 0 )
		return;

	WaitForSingleObject(this->mut_step,INFINITE);
	if( this->isSorted() == false ){
		ReleaseMutex(this->mut_step);
		WaitForSingleObject(this->sem_sorted,INFINITE);
	}
	else
		ReleaseMutex(this->mut_step);

	WaitForSingleObject(this->mut_list,INFINITE);
	if( this->sortSide == HEADSORT )
		this->getFiles()->gotoHead();
	else if( this->sortSide == TAILSORT )
		this->getFiles()->gotoTail();
	ReleaseMutex(this->mut_list);

	bool result = false;
	do {
		WaitForSingleObject(this->mut_step,INFINITE);
		if( this->isSorted() == false ){
			ReleaseMutex(this->mut_step);
			WaitForSingleObject(this->sem_sorted,INFINITE);
		}
		else
			ReleaseMutex(this->mut_step);

		WaitForSingleObject(this->mut_list,INFINITE);
		core->messageBox_Info(this->getFiles()->getThat()->getFilePath()->toWCHAR());

		if( this->sortSide == HEADSORT )
			result = this->getFiles()->next();
		else if( this->sortSide == TAILSORT )
			result = this->getFiles()->prev();
		ReleaseMutex(this->mut_list);
	} while( result );
}

bool Archive::next(int id)
{
	if( this->getCount() <= 0 )
		return false;

	WaitForSingleObject(this->mut_step,INFINITE);
	if( this->isSorted() == false ){
		ReleaseMutex(this->mut_step);
		if( this->sortSide == HEADSORT )
			WaitForSingleObject(this->sem_sorted,INFINITE);
	}
	else
		ReleaseMutex(this->mut_step);
	
	WaitForSingleObject(this->mut_list,INFINITE);
	bool result = this->getFiles()->next(id);
	ReleaseMutex(this->mut_list);

	if( this->sortSide == TAILSORT && result == true )
		ReleaseSemaphore(this->sem_sorted,1,NULL);

	if( result == true && id == NULL )
		this->idx++;
	return result;
}

bool Archive::prev(int id)
{
	if( this->getCount() <= 0 )
		return false;

	WaitForSingleObject(this->mut_step,INFINITE);
	if( this->isSorted() == false ){
		ReleaseMutex(this->mut_step);
		if( this->sortSide == TAILSORT )
			WaitForSingleObject(this->sem_sorted,INFINITE);
	}
	else {
		ReleaseMutex(this->mut_step);
	}

	WaitForSingleObject(this->mut_list,INFINITE);
	bool result = this->getFiles()->prev(id);
	ReleaseMutex(this->mut_list);

	if( this->sortSide == HEADSORT && result == true )
		ReleaseSemaphore(this->sem_sorted,1,NULL);

	if( result == true && id == NULL )
		this->idx--;
	return result;
}

Cell *Archive::load(int id)
{
	if( this->getCount() <= 0 )
		return NULL;

	if( id == NULL ){
		WaitForSingleObject(this->mut_step,INFINITE);
		if( this->isSorted() == false ){
			ReleaseMutex(this->mut_step);
			WaitForSingleObject(this->sem_sorted,INFINITE);
			ReleaseSemaphore(this->sem_sorted,1,NULL);
		}
		else
			ReleaseMutex(this->mut_step);
	}

	WaitForSingleObject(this->mut_list,INFINITE);
	File *that = this->getFiles()->getThat();
	if( id == LEFT )
		that = this->getFiles()->getLeft();
	else if( id == RIGHT )
		that = this->getFiles()->getRight();
	ReleaseMutex(this->mut_list);

	if( that == NULL )
		return NULL;

	if( this->pack->extract(that) ){
		Cell *cell = new Cell(that,this->pack->getBuffer());
		this->pack->clearBuffer();

		if( cell->isLoaded() )
			return cell;
	}
	return NULL;
}

bool Archive::isThatHead()
{
	bool result = false;

	if( this->getCount() < 2 )
		return true;

	if( this->isSorted() == true )
		result = this->getFiles()->isThatHead();
	else if( this->sortSide == HEADSORT ){
		WaitForSingleObject(this->mut_list,INFINITE);
		result = this->getFiles()->isThatHead();
		ReleaseMutex(this->mut_list);
	}
	return result;
}
bool Archive::isThatTail()
{
	bool result = false;

	if( this->getCount() < 2 )
		return true;

	if( this->isSorted() == true )
		result = this->getFiles()->isThatTail();
	else if( this->sortSide == TAILSORT ){
		WaitForSingleObject(this->mut_list,INFINITE);
		result = this->getFiles()->isThatTail();
		ReleaseMutex(this->mut_list);
	}
	return result;
}

DWORD WINAPI Archive::sort(LPVOID param)
{
	Archive *that = (Archive *)param;

	int result;
	List<File>::Node *ext, *tmp;

	WaitForSingleObject(that->mut_loop,INFINITE);
	while( that->getUnsortedFiles()->getCount() > 0 ){
		if( that->sortSide == HEADSORT )
			ext = that->getUnsortedFiles()->getThatHead();
		else if( that->sortSide == TAILSORT )
			ext = that->getUnsortedFiles()->getThatTail();
		tmp = ext->getNext();

		while( tmp != NULL ){
			if( WaitForSingleObject(that->mut_terminator,0) != WAIT_TIMEOUT ){
				if( that->terminate == true ){
					ReleaseMutex(that->mut_terminator);
					ReleaseMutex(that->mut_loop);
					return 0;
				}
				ReleaseMutex(that->mut_terminator);
			}

			result = FwCHAR::compare(ext->getObj()->getFilePath(),
									 tmp->getObj()->getFilePath());

			if( (result > 0 && that->sortSide == HEADSORT) ||
				(result < 0 && that->sortSide == TAILSORT) )
				ext = tmp;
			
			tmp = tmp->getNext();
		}
		WaitForSingleObject(that->mut_list,INFINITE);
		that->getUnsortedFiles()->remove(ext);
		if( that->sortSide == HEADSORT )
			that->getFiles()->add(ext);
		else if( that->sortSide == TAILSORT )
			that->getFiles()->addToHead(ext);
		ReleaseMutex(that->mut_list);

		ReleaseSemaphore(that->sem_sorted,1,NULL);
	}
	ReleaseMutex(that->mut_loop);
	WaitForSingleObject(that->mut_step,INFINITE);
	that->setSorted();
	ReleaseMutex(that->mut_step);
	return 0;
}

void Archive::initSort(int side)
{
	if( this->isSorted() == true )
		return;
	if( this->mut_loop != NULL )
		if( WaitForSingleObject(this->mut_loop,0) == WAIT_TIMEOUT )
			return;

	this->init();
	if( this->pack->isLoaded() == false )
		return;
	if( this->getCount() <= 0 )
		return;

	this->sortSide = side;
	if( side == TAILSORT )
		this->idx = this->getCount() - 1;

	Archive::sortStep(this);
	
	if( this->getCount() > 1 ){
		this->mut_list = CreateMutex(NULL,false,NULL);
		this->mut_loop = CreateMutex(NULL,false,NULL);
		this->mut_step = CreateMutex(NULL,false,NULL);
		this->mut_terminator = CreateMutex(NULL,true,NULL);
			
		this->sem_sorted = CreateSemaphore(NULL,0,count,NULL);

		
		this->thrd_sorting = CreateThread(NULL,NULL,
										 (LPTHREAD_START_ROUTINE)&Archive::sort,
										  this,NULL,NULL);
	}
	else
		this->setSorted();
}

DWORD WINAPI Archive::sortStep(LPVOID param)
{
	Archive *that = (Archive *)param;

	int result;
	List<File>::Node *ext, *tmp;

	if( that->sortSide == HEADSORT )
		ext = that->getUnsortedFiles()->getThatHead();
	else
		ext = that->getUnsortedFiles()->getThatTail();
	if( ext == NULL )
		return 0;

	tmp = ext->getNext();

	while( tmp != NULL ){
		result = FwCHAR::compare(ext->getObj()->getFilePath(),
								 tmp->getObj()->getFilePath());

		if( (result > 0 && that->sortSide == HEADSORT) ||
			(result < 0 && that->sortSide == TAILSORT) )
			ext = tmp;
			
		tmp = tmp->getNext();
	}
	that->getUnsortedFiles()->remove(ext);
	if( that->sortSide == HEADSORT )
		that->getFiles()->add(ext);
	else if( that->sortSide == TAILSORT )
		that->getFiles()->addToHead(ext);
	
	return 0;
}
