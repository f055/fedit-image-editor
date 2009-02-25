/*
Explorer.cpp
Fedit file browser that enables browsing of folders and archive files.
Browsing is based on file headers and file extensions. Browsing is
different depending of opened file:
1. normal file - browses through all files in that file folder, including archives
2. folder - browses through all files in that folder, including archives
3. archive - browses through all files in that archive,
			 excluding files in the folder that archive exists,
			 and excluding any archive files inside that archive
*/

#include "stdafx.h"
#include "Core.h"

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFLEN 10

// files header information
char szTiffHeaderForMotorola[]	= "MM*";
char szTiffHeaderForIntel[]		= "II*";
char szPNGHeader[]				= "\x89PNG\r\n\x1a\n";
char szGIF87aHeader[]			= "GIF87a";
char szGIF89aHeader[]			= "GIF89a";
char szJPEGCommonHeader[]		= "\xFF\xD8\xFF";
char szJPEGCommonEOI[]			= "\xFF\xD9"; // for future use
char szBMPHeader[]				= "\x42\x4D";

char szZIPHeader[]				= "\x50\x4b\x03\x04";
char szRARHeader[]				= "Rar";

Explorer::Explorer(Core *core)
{
	this->core = core;
	this->root = NULL;
}

Explorer::~Explorer()
{
	if( this->root != NULL )
		delete this->root;
}

bool Explorer::browse(FwCHAR *path)
{
	int type = Explorer::getType(path);
	int mime = Explorer::getMime(type);
	Catalog *tmp = NULL;

	if( mime == MIME_ARCHIVE ){
		tmp = new Archive(path,type);
		tmp->initSort();
	}
	else if( mime == MIME_DIR ){
		if( path->toWCHAR()[path->toLength()] != '\\' )
			path->mergeWith(L"\\");
		tmp = new Catalog(path);
	}
	else if( mime == MIME_IMAGE ){
		FwCHAR *folder = new FwCHAR();
		folder->getFolderFrom(path);

		tmp = new Catalog(folder,path);
	}

	if( tmp != NULL ){
		if( tmp->getCount() <= 0 ){
			FwCHAR *notice = new FwCHAR(MESSAGE_EMPTY);
			notice->mergeWith(GUI_SEP);
			notice->mergeWith(tmp->getFilePath()->toWCHAR());

			this->core->messageBox_Info( notice->toWCHAR() );

			delete notice;
			delete tmp;
		}
		else {
			this->reset();
			this->root = tmp;
			return true;
		}
	}
	return false;
}

Catalog *Explorer::getRoot()
{
	return this->root;
}

FwCHAR *Explorer::getArchivePath()
{
	FwCHAR *path = NULL;
	Archive *archive = this->getArchive();
	if( archive != NULL )
		path = archive->getFilePath();
	return path;
}

void Explorer::reset()
{
	if( this->root != NULL )
		delete this->root;
	this->root = NULL;
}

bool Explorer::extract()
{
	bool result = false;

	Archive *archive = this->getArchive();
	if( archive != NULL )
		result = archive->extract();
	return result;
}

int Explorer::getMime(FwCHAR *path)
{
	return Explorer::getMime( Explorer::getType(path) );
}

int Explorer::getMime(int type)
{
	if( type == TYPE_NONE )
		return MIME_NONE;
	else if( type == TYPE_ZIP || type == TYPE_RAR )
		return MIME_ARCHIVE;
	else if( type == TYPE_DIR )
		return MIME_DIR;
	else
		return MIME_IMAGE;

	return MIME_NONE;
}

int Explorer::getType(FwCHAR *path)
{
	DWORD fileAtt = GetFileAttributes(path->toWCHAR());
	if( fileAtt == INVALID_FILE_ATTRIBUTES )
		return TYPE_NONE;

	if( fileAtt & FILE_ATTRIBUTE_DIRECTORY )
		return TYPE_DIR;

	int file;
	char pData[BUFLEN];

	file = _wopen(path->toWCHAR(),_O_BINARY);
	if( file == FERROR )
		return TYPE_NONE;

	_read(file,pData,BUFLEN);
	_close(file);

	if (!memcmp(pData, szTiffHeaderForIntel,3))
		return TYPE_TIFFINTEL;
	else if (!memcmp(pData, szTiffHeaderForMotorola,3))
		return TYPE_TIFFMOTOROLA;
	else if (!memcmp(pData, szPNGHeader,8))
		return TYPE_PNG;
	else if (!memcmp(pData, szGIF87aHeader,6))
		return TYPE_GIF87a;
	else if (!memcmp(pData, szGIF89aHeader,6))
		return TYPE_GIF89a;
	else if (!memcmp(pData, szZIPHeader,4))
		return TYPE_ZIP;
	else if (!memcmp(pData, szRARHeader,3))
		return TYPE_RAR;
	else if (!memcmp(pData, szBMPHeader,2))
	{
		// 7 to ten byte must be zero
		// 3 to 6 is size of image
		char szNull[] = "\x0\x0\x0\x0";
		if (!memcmp(pData+6, szNull,4))
			return TYPE_BITMAP;
	}
	else if (!memcmp(pData, szJPEGCommonHeader,3))
	{
		switch (((long)*(pData+3)) & 0xFF)
		{
		case 0xE0:
			return TYPE_JPEGJFIF;
			break;
		case 0xE1:
			return TYPE_JPEGEXIF;
			break;
		case 0xE2:
			return TYPE_JPEGAPP2;
			break;
		case 0xE3:
			return TYPE_JPEGAPP3;
			break;
		case 0xE4:
			return TYPE_JPEGAPP4;
			break;
		case 0xE5:
			return TYPE_JPEGAPP5;
			break;
		case 0xE6:
			return TYPE_JPEGAPP6;
			break;
		case 0xE7:
			return TYPE_JPEGAPP7;
			break;
		case 0xE8:
			return TYPE_JPEGAPP8;
			break;
		case 0xE9:
			return TYPE_JPEGAPP9;
			break;
		case 0xEA:
			return TYPE_JPEGAPPA;
			break;
		case 0xEB:
			return TYPE_JPEGAPPB;
			break;
		case 0xEC:
			return TYPE_JPEGAPPC;
			break;
		case 0xED:
			return TYPE_JPEGAPPD;
			break;
		case 0xEE:
			return TYPE_JPEGAPPE;
			break;
		case 0xEF:
			return TYPE_JPEGAPPF;
			break;
		default:
			break;
		}
	}
	return TYPE_NONE;
}

int Explorer::getType(WCHAR *name)
{
	WCHAR *ext = new WCHAR[10];
	int i = 0;
	int type = TYPE_NONE;

	while( name[i] != '\0' ) i++;
	while( name[i] != '.' && i > 0 ) i--;

	int j=0;
	while( name[i] != '\0' && j < 9 )
		ext[j++] = name[i++];
	ext[j] = '\0';

	if( !_wcsicmp(ext,L".jpeg") || !_wcsicmp(ext,L".jpg") )
		type = TYPE_JPG;
	else if( !_wcsicmp(ext,L".gif") )
		type = TYPE_GIF;
	else if( !_wcsicmp(ext,L".tif") || !_wcsicmp(ext,L".tiff") )
		type = TYPE_TIFF;
	else if( !_wcsicmp(ext,L".png") )
		type = TYPE_PNG;
	else if( !_wcsicmp(ext,L".bmp") )
		type = TYPE_BITMAP;
	else if( !_wcsicmp(ext,L".fed") )
		type = TYPE_FED;
	else if( !_wcsicmp(ext,L".ico") )
		type = TYPE_ICO;

	else if( !_wcsicmp(ext,L".rar") || !_wcsicmp(ext,L".cbr") )
		type =  TYPE_RAR;
	else if( !_wcsicmp(ext,L".zip") || !_wcsicmp(ext,L".cbz") )
		type =  TYPE_ZIP;

	delete [] ext;
	return type;
}

bool Explorer::isArchived()
{
	bool result = false;
	if( this->getArchive() != NULL )
		result = true;
	return result;
}

Archive *Explorer::getArchive()
{
	Archive *archive = NULL;
	if( this->root != NULL ){
		if( this->root->getMime() == MIME_ARCHIVE )
			archive = (Archive *)this->root;
		else if( this->root->getFiles() != NULL )
			if( this->root->getFiles()->getCount() > 0 )
				if( this->root->getFiles()->getThat()->getMime() == MIME_ARCHIVE )
					archive = (Archive *)this->root->getFiles()->getThat();
	}
	return archive;
}