/*
Exp_File.cpp
Inherited from Fiew 2.0
Represents the filesystem file as an object.
It is the smallest entity in Fedit Explorer.
*/

#include "stdafx.h"
#include "Core.h"

File::File(FwCHAR *path, int type, DWORD size, bool archived)
{
	this->filepath = path;

	this->filename = new FwCHAR();
	this->filename->getFilenameFrom(this->filepath);

	this->size = size;
	this->archived = archived;

	if( type == UNDEFINED )
		this->type = Explorer::getType(this->filepath);
	else
		this->type = type;

	this->mime = Explorer::getMime(this->type);
}

File::~File()
{
	delete this->filepath;
	delete this->filename;
}

void File::initSort(int sort)
{
	return;
}

FwCHAR *File::getFilePath()
{
	return this->filepath;
}

FwCHAR *File::getFileName()
{
	return this->filename;
}

int File::getType()
{
	return this->type;
}

int File::getMime()
{
	return this->mime;
}

DWORD File::getSize()
{
	return this->size;
}

bool File::isArchived()
{
	return this->archived;
}