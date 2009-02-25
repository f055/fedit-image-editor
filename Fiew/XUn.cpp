/*
XUn.cpp
Unzip object wrapper
*/

#include "stdafx.h"
#include "Core.h"

XUn::XUn(WCHAR *archivepath)
{
	this->path = new FwCHAR(archivepath);
	this->buffer = NULL;
	this->comment = NULL;

	this->extracting = false;

	this->error = 0;
	this->count = 0;

	this->buflen = 0;
	this->bufpot = 0;

	this->password = NULL;
	this->sem_pass = NULL;//CreateSemaphore(NULL,0,1,NULL);
}

XUn::~XUn()
{
	delete this->path;

	if( this->password != NULL )
		delete this->password;
	if( this->buffer != NULL )
		delete [] this->buffer;
	if( this->comment != NULL )
		delete [] this->comment;
	if( this->sem_pass != NULL )
		CloseHandle(this->sem_pass);
}

bool XUn::list(List<File> *list)
{
	return false;
}

bool XUn::extract(File *file, WCHAR *dest)
{
	return false;
}

bool XUn::isLoaded()
{
	return false;
}

int XUn::getLastError()
{
	return this->error;
}

void XUn::setLastError(int error)
{
	this->error = error;
}

int XUn::getBuflen()
{
	return this->buflen;
}

byte *XUn::getBuffer()
{
	return this->buffer;
}

void XUn::clearBuffer()
{
	this->buflen = 0;
	this->bufpot = 0;

	if( this->buffer != NULL )
		delete [] this->buffer;
	this->buffer = NULL;
}

void XUn::setBuffer(void *source, int size)
{
	if( this->buffer == NULL )
		this->buffer = new byte[this->buflen];

	if( size > 0 ){
		memcpy(&this->buffer[this->bufpot],source,size);
		this->bufpot += size;
	}
}

FwCHAR *XUn::getPassword()
{
	return this->password;
}
void XUn::setPassword(FwCHAR *password)
{
	this->password = password;
}
void XUn::sendPassReq()
{
	return;
}

/* *** */

XUnrar::XUnrar(WCHAR *archivepath) : XUn(archivepath)
{
	this->loaded = this->loadFunc();
	this->charpath = NULL;
	this->handle = NULL;

	if( this->loaded ){
		this->charpath = this->path->tochar();
	}
}

XUnrar::~XUnrar()
{
	if( this->handle != NULL )
		this->closeArchive(this->handle);

	if( this->charpath != NULL )
		delete [] this->charpath;
}

bool XUnrar::list(List<File> *list)
{
	if( !this->loaded )
		return false;

	bool counter = false;
	if( this->count == 0 )
		counter = true;

	bool noerror = false;

	if( this->reopen(RAR_OM_LIST) ){
		RARHeaderDataEx headerData;
		ZeroMemory(&headerData, sizeof(headerData));

		int type, read = 0;
		DWORD size;

		noerror = true;

		while( (read = this->readHeaderEx(this->handle,&headerData)) != ERAR_END_ARCHIVE ){
			if( counter == true )
				this->count++;

			if( read == 0 ){
				type = Explorer::getType(headerData.FileNameW);
				size = headerData.UnpSize;

				if( Explorer::getMime(type) == MIME_IMAGE && size > 0 )
					list->add( new File(new FwCHAR(headerData.FileNameW), type, size, true) );
				this->processFileW(this->handle,RAR_SKIP,NULL,NULL);
			}
			else {
				noerror = false;
				this->setLastError(read);
				break;
			}
		}
	}
	return noerror;
}

bool XUnrar::extract(File *file, WCHAR *dest)
{
	if( !this->loaded )
		return false;
	if( dest != NULL )
		this->extracting = true;

	bool noerror = false;
	if( this->reopen(RAR_OM_EXTRACT) ){
		RARHeaderDataEx headerData;
		ZeroMemory(&headerData, sizeof(headerData));

		int mode,read = 0;
		noerror = true;
		WCHAR *filepath = file->getFilePath()->toWCHAR();

		while( (read = this->readHeaderEx(this->handle,&headerData)) != ERAR_END_ARCHIVE ){
			if( read == 0 ){
				if( !_wcsicmp(filepath,headerData.FileNameW) ){
					mode = RAR_EXTRACT;
					if( this->extracting == false ){
						this->buflen = headerData.UnpSize;
						mode = RAR_TEST;
					}
					if( this->processFileW(this->handle,mode,NULL,dest) == 0 )
						return true;
				}
				else
					this->processFileW(this->handle,RAR_SKIP,NULL,NULL);
			}
			else {
				noerror = false;
				this->setLastError(read);
				break;
			}
		}
	}
	this->extracting = false;
	return noerror;
}

bool XUnrar::isLoaded()
{
	return this->loaded;
}

bool XUnrar::loadFunc()
{
	(FARPROC&)this->openArchive = GetProcAddress(Core::getRarDll(), "RAROpenArchive");
	(FARPROC&)this->openArchiveEx = GetProcAddress(Core::getRarDll(), "RAROpenArchiveEx");
	(FARPROC&)this->closeArchive = GetProcAddress(Core::getRarDll(), "RARCloseArchive");
	(FARPROC&)this->readHeader = GetProcAddress(Core::getRarDll(), "RARReadHeader");
	(FARPROC&)this->readHeaderEx = GetProcAddress(Core::getRarDll(), "RARReadHeaderEx");
	(FARPROC&)this->processFile = GetProcAddress(Core::getRarDll(), "RARProcessFile");
	(FARPROC&)this->processFileW = GetProcAddress(Core::getRarDll(), "RARProcessFileW");
	(FARPROC&)this->setCallback = GetProcAddress(Core::getRarDll(), "RARSetCallback");

	if( this->openArchive == NULL ||
		this->openArchiveEx == NULL ||
		this->closeArchive == NULL ||
		this->readHeader == NULL ||
		this->readHeaderEx == NULL ||
		this->processFile == NULL ||
		this->processFileW == NULL ||
		this->setCallback == NULL )
		return false;
	return true;
}

bool XUnrar::reopen(int mode)
{
	if( !this->loaded )
		return false;

	if( this->handle != NULL )
		this->closeArchive(this->handle);

	RAROpenArchiveDataEx archiveData;
	ZeroMemory(&archiveData, sizeof(archiveData));

	archiveData.ArcName = this->charpath;
	archiveData.ArcNameW = this->path->toWCHAR();
	archiveData.CmtBuf = this->comment;
	archiveData.OpenMode = mode;

	HANDLE temp = this->openArchiveEx(&archiveData);
	if( archiveData.OpenResult == 0 ){
		this->handle = temp;
		this->setCallback(this->handle,XUnrar::callbackProc,(LONG)this);
		return true;
	}
	return false;
}

int CALLBACK XUnrar::callbackProc(UINT msg,LONG userData,LONG p1,LONG p2){

	XUnrar *that = (XUnrar *)userData;
	int len = 0;

	switch(msg){
		case UCM_CHANGEVOLUME:
			return -1;
			break;
		case UCM_PROCESSDATA:
			if( that->extracting == false )
				that->setBuffer((void *)p1, p2);
			break;
		case UCM_NEEDPASSWORD:
			break;

			/*
			if( that->getPassword() == NULL ){
				that->sendPassReq();
				WaitForSingleObject(that->sem_pass,INFINITE);
			}
			len = that->getPassword()->toLength();
			if( len > p2 )
				len = p2;
			memcpy((void *)p1,that->getPassword(),len);
			break;
			*/
		default:
			break;
	}
	return 0;
}

/* *** */

XUnzip::XUnzip(WCHAR *archivepath) : XUn(archivepath)
{
	this->handle = NULL;
}

XUnzip::~XUnzip()
{
	if( this->handle != NULL )
		CloseZip(this->handle);
}

bool XUnzip::list(List<File> *list)
{
	bool noerror = false;

	if( (this->handle = OpenZip(this->path->toWCHAR(),0,ZIP_FILENAME)) != 0 ){
		ZIPENTRYW headerData;
		ZeroMemory(&headerData, sizeof(headerData));

		int result = ZR_OK;
		if( (result = GetZipItemW(this->handle,-1,&headerData)) == ZR_OK ){
			this->count = headerData.index;
			noerror = true;

			int type, i = 0;
			DWORD size;

			while( i < this->count ){
				if( (result = GetZipItemW(this->handle,i++,&headerData)) == ZR_OK ){
					type = Explorer::getType(headerData.name);
					size = headerData.unc_size;

					if( Explorer::getMime(type) == MIME_IMAGE && size > 0 )
						list->add( new File(new FwCHAR(headerData.name), type, size, true) );
				}
				else {
					this->setLastError(result);
					noerror = false;
					break;
				}
			}
		}
		else this->setLastError(result);
	}
	return noerror;
}

bool XUnzip::extract(File *file, WCHAR *dest)
{
	if( dest != NULL )
		this->extracting = true;

	bool noerror = false;

	int index = 0;
	WCHAR *path = file->getFilePath()->toWCHAR();

	ZIPENTRYW headerData;
	ZeroMemory(&headerData, sizeof(headerData));

	if( FindZipItemW(this->handle,path,false,&index,&headerData) == ZR_OK ){
		int tmplen = headerData.unc_size;
		byte *tmp = new byte[2 * tmplen];

		this->buflen = tmplen;

		int read = 0;
		ZRESULT zresult = ZR_MORE;
		while( zresult == ZR_MORE ){
			if( this->extracting == true ){
				zresult = UnzipItem(this->handle,index,dest,0,ZIP_FILENAME,&read);
			}
			else {
				zresult = UnzipItem(this->handle,index,tmp,tmplen,ZIP_MEMORY,&read);
				this->setBuffer(tmp,read); // it was tmplen - a nonsense?
			}
			if( zresult == ZR_OK ){
				noerror = true;
				break;
			}
		}
		delete [] tmp;
	}
	this->extracting = false;
	return noerror;
}

bool XUnzip::isLoaded()
{
	return true;
}