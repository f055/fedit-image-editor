/*
Exp_Folder.cpp
Inherited from Fiew 2.0
Class inherits from File. Represents the filesystem folder 
as an object. Enables standard file browsing operations.
*/

#include "stdafx.h"
#include "Core.h"

Catalog::Catalog(FwCHAR *path, int type, DWORD size) : File(path,type,size)
{
	this->sortSide = HEADSORT;
	this->count = NULL;
	this->idx = NULL;

	this->mut_list = NULL;
	this->mut_loop = NULL;
	this->mut_step = NULL;
	this->mut_terminator = NULL;

	this->thrd_sorting = NULL;

	this->files = new List<File>();
	this->init();
}

Catalog::Catalog(FwCHAR *path, FwCHAR *filepath, int type) : File(path,type)
{
	this->sortSide = HEADSORT;
	this->count = NULL;
	this->idx = NULL;

	this->mut_list = NULL;
	this->mut_loop = NULL;
	this->mut_step = NULL;
	this->mut_terminator = NULL;

	this->thrd_sorting = NULL;

	this->files = new List<File>();
	this->init(filepath);
}

Catalog::~Catalog()
{
	if( WaitForSingleObject(this->mut_loop,0) == WAIT_TIMEOUT ){
		ReleaseMutex(this->mut_terminator);
		WaitForSingleObject(this->mut_loop,INFINITE);
	}
	if( this->mut_terminator != NULL )
		CloseHandle(this->mut_terminator);
	if( this->mut_step != NULL )
		CloseHandle(this->mut_step);
	if( this->mut_loop != NULL )
		CloseHandle(this->mut_loop);
	if( this->mut_list != NULL )
		CloseHandle(this->mut_list);

	if( this->thrd_sorting != NULL )
		CloseHandle(this->thrd_sorting);

	delete this->files;
}

void Catalog::init(FwCHAR *thatpath)
{
	if( this->getMime() == MIME_ARCHIVE )
		return;

	WIN32_FIND_DATA FFdata;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	FwCHAR *folder = new FwCHAR( this->getFilePath()->toWCHAR() );
	folder->mergeWith(DIRMERGE);

	hFind = FindFirstFile(folder->toWCHAR(),&FFdata);

	if( hFind != INVALID_HANDLE_VALUE ){
		File *that = NULL;
		FwCHAR *file = new FwCHAR(this->getFilePath()->toWCHAR() );
		file->mergeWith(FFdata.cFileName);

		int type = Explorer::getType(file);
		int mime = Explorer::getMime(type);
		DWORD size = (FFdata.nFileSizeHigh * (MAXDWORD + 1)) + FFdata.nFileSizeLow;

		if( mime == MIME_IMAGE )
			this->files->add( that = new File(file,type,size) );
		else if( mime == MIME_ARCHIVE )
			this->files->add( that = new Archive(file,type,size) );
		else
			delete file;

		if( thatpath != NULL && that != NULL ){
			if( that->getFilePath()->equals(thatpath) )
				this->files->setThat(that);
			that = NULL;
		}
		while( FindNextFile(hFind,&FFdata) ){
			file = new FwCHAR(this->getFilePath()->toWCHAR() );
			file->mergeWith(FFdata.cFileName);

			type = Explorer::getType(file);
			mime = Explorer::getMime(type);
			size = (FFdata.nFileSizeHigh * (MAXDWORD + 1)) + FFdata.nFileSizeLow;

			if( mime == MIME_IMAGE )
				this->files->add( that = new File(file,type,size) );
			else if( mime == MIME_ARCHIVE )
				this->files->add( that = new Archive(file,type,size) );
			else
				delete file;

			if( thatpath != NULL && that != NULL ){
				if( that->getFilePath()->equals(thatpath) )
					this->files->setThat(that);
				that = NULL;
			}
		}
		FindClose(hFind);
		this->idx = this->files->countLeftoThat();
	}
	delete folder;
}

int Catalog::getCount()
{
	int count = 0;

	File *that = this->getFiles()->getThat();
	if( that != NULL )
		if( that->getMime() == MIME_ARCHIVE )
			count = ((Archive *)that)->getCount();
	if( count <= 0 )
		count = this->getFiles()->getCount();

	return count;
}

List<File> *Catalog::getFiles()
{
	return this->files;
}

int Catalog::getIdx()
{
	int idx = this->idx;

	File *that = this->getFiles()->getThat();
	if( that != NULL )
		if( that->getMime() == MIME_ARCHIVE )
			idx = ((Archive *)that)->getIdx();

	return idx;
}

void Catalog::list(Core *core)
{
	this->getFiles()->gotoHead();
	do {
		core->messageBox_Info(this->getFiles()->getThat()->getFilePath()->toWCHAR());
	} while( this->getFiles()->next() );
}

DWORD WINAPI Catalog::trigSort(LPVOID param)
{
	
	Catalog *that = (Catalog *)param;
	File *right = NULL;
	File *left = NULL;
	int count = 1;

	WaitForSingleObject(that->mut_loop,INFINITE);
	do {
		if( WaitForSingleObject(that->mut_terminator,0) != WAIT_TIMEOUT ){
			ReleaseMutex(that->mut_terminator);
			ReleaseMutex(that->mut_loop);
			return 0;
		}
		WaitForSingleObject(that->mut_step,INFINITE);

		left = that->getFiles()->getLeftoThat(count);
		right = that->getFiles()->getRightoThat(count);

		if( left != NULL )
			left->initSort();
		if( right != NULL )
			right->initSort();
		count++;

		ReleaseMutex(that->mut_step);

	} while( left != NULL || right != NULL );
	ReleaseMutex(that->mut_loop);
	
	return 0;
}

void Catalog::initSort(int sort)
{
	
	this->mut_list = CreateMutex(NULL,false,NULL);
	this->mut_loop = CreateMutex(NULL,false,NULL);
	this->mut_step = CreateMutex(NULL,false,NULL);
	this->mut_terminator = CreateMutex(NULL,true,NULL);

	this->sortSide = sort;

	this->getFiles()->getThat()->initSort(sort);

	this->thrd_sorting = CreateThread(NULL,NULL,
									 (LPTHREAD_START_ROUTINE)&Catalog::trigSort,
									  this,NULL,NULL);
	
}

bool Catalog::next(int id)
{
	if( this->getFiles()->getCount() <= 0 )
		return false;

	this->sortSide = HEADSORT;

	File *that = this->getFiles()->getThat();
	if( id == LEFT )
		that = this->getFiles()->getLeft();
	else if( id == RIGHT )
		that = this->getFiles()->getRight();
	bool result = false;

	if( that == NULL )
		return result;

	if( that->getMime() == MIME_ARCHIVE ){
		Archive *arch = (Archive *)that;
		result = arch->next(id);
	}
	if( result == false )
		result = this->getFiles()->next(id);
	else
		return result;

	if( result == true && id == NULL )
		this->idx++;
	
	return result;
}

bool Catalog::prev(int id)
{
	if( this->getFiles()->getCount() <= 0 )
		return false;

	this->sortSide = TAILSORT;

	File *that = this->getFiles()->getThat();
	if( id == LEFT )
		that = this->getFiles()->getLeft();
	else if( id == RIGHT )
		that = this->getFiles()->getRight();
	bool result = false;

	if( that == NULL )
		return result;

	if( that->getMime() == MIME_ARCHIVE ){
		Archive *arch = (Archive *)that;
		result = arch->prev(id);
	}
	if( result == false )
		result = this->getFiles()->prev(id);
	else
		return result;

	if( result == true && id == NULL )
		this->idx--;

	return result;
}

Cell *Catalog::load(int id)
{
	if( this->getFiles()->getCount() <= 0 )
		return NULL;

	File *that = this->getFiles()->getThat();
	if( id == LEFT )
		that = this->getFiles()->getLeft();
	else if( id == RIGHT )
		that = this->getFiles()->getRight();

	if( that == NULL )
		return NULL;

	if( that->getMime() == MIME_ARCHIVE ){
		Archive *arch = (Archive *)that;
		arch->initSort(this->sortSide);

		if( arch->getCount() <= 0 ){
			delete this->getFiles()->remove(that);
			return this->load(id);
		}		
		return arch->load(id);
	}
	Cell *cell = new Cell(that);

	if( cell->isLoaded() )
		return cell;

	return NULL;
}

bool Catalog::isThatHead()
{
	bool result = this->getFiles()->isThatHead();

	if( result == true ){
		File *that = this->getFiles()->getThat();
		if( that != NULL )
			if( that->getMime() == MIME_ARCHIVE ){
				Archive *arch = (Archive *)that;
				result = arch->isThatHead();
			}
	}
	return result;
}
bool Catalog::isThatTail()
{
	bool result = this->getFiles()->isThatTail();

	if( result == true ){
		File *that = this->getFiles()->getThat();
		if( that != NULL )
			if( that->getMime() == MIME_ARCHIVE ){
				Archive *arch = (Archive *)that;
				result = arch->isThatTail();
			}
	}
	return result;
}