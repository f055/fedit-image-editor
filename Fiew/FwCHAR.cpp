/*
FwCHAR.cpp
Partially inherited from Fiew 2.0
Custom Fedit WCHAR object for managing strings
*/

#include "stdafx.h"
#include "FwCHAR.h"

#include <locale>
using namespace std;

FwCHAR::FwCHAR()
{
	this->string = new WCHAR[1];
	this->string[0] = '\0';
}

FwCHAR::FwCHAR(int source)
{
	WCHAR *tmp = new WCHAR[MAX_LOADSTRING];
	_itow(source,tmp,10);

	this->string = new WCHAR[wcslen(tmp) + 1];

	int i = 0;
	while( tmp[i] != '\0' )
		this->string[i] = tmp[i++];

	this->string[i] = '\0';
	delete [] tmp;
}

FwCHAR::FwCHAR(WCHAR *source)
{
	if( source != NULL ){
		this->string = new WCHAR[wcslen(source) + 1];

		int i = 0;
		while( source[i] != '\0' )
			this->string[i] = source[i++];

		this->string[i] = '\0';
	}
	else {
		this->string = new WCHAR[1];
		this->string[0] = '\0';
	}
}

FwCHAR::~FwCHAR()
{
	delete [] this->string;
}

void FwCHAR::mergeWith(WCHAR *source)
{
	if( source == NULL )
		return;

	WCHAR *tmp = this->string;
	this->string = new WCHAR[wcslen(tmp) + wcslen(source) + 1];

	wcscpy(this->string,tmp);
	wcscpy(&this->string[this->toLength()],source);
	
	delete [] tmp;
}

void FwCHAR::mergeWith(FwCHAR *source)
{
	if( source == NULL )
		return;

	this->mergeWith(source->toWCHAR());
}

void FwCHAR::getFolderFrom(FwCHAR *source)
{
	if( source == NULL )
		return;

	int len, offset = 0;
	int i = len = source->toLength();
	while( (source->toWCHAR()[i] != '\\' && source->toWCHAR()[i] != '/') && i > 0 ){
		offset++;
		i--;
	}

	delete [] this->string;

	int copy, diff = len - offset;
	if( diff > 0 )
		copy = diff + 1;
	else
		copy = len + 1;

	this->string = new WCHAR[copy + 1];
	for( i = 0; i < copy; i++ )
		this->string[i] = source->toWCHAR()[i];
	this->string[i] = '\0';
}

void FwCHAR::getFilenameFrom(FwCHAR *source)
{
	if( source == NULL )
		return;

	int len, offset = 0;
	int i = len = source->toLength();
	while( (source->toWCHAR()[i] != '\\' && source->toWCHAR()[i] != '/') && i > 0 ){
		offset++;
		i--;
	}

	delete [] this->string;

	int diff = len - offset;
	if( diff > 0 ){
		this->string = new WCHAR[offset + 1];
		wcscpy(this->string,&source->toWCHAR()[diff + 1]);
	}
	else {
		this->string = new WCHAR[len + 1];
		wcscpy(this->string,source->toWCHAR());
	}
}

WCHAR *FwCHAR::toWCHAR()
{
	return this->string;
}

char *FwCHAR::tochar()
{
	char *target = new char[this->toLength() + 1];

	int i = 0;
	while( this->string[i] != '\0' )
		target[i] = (char)this->string[i++];
	target[i] = '\0';

	return target;
}

bool FwCHAR::equals(FwCHAR *source)
{
	if( wcscmp(this->string,source->toWCHAR()) == 0 )
		return true;
	return false;
}

void FwCHAR::stripBraces()
{
	if( this->string[0] == '"' ){
		int i = 1;
		while( this->string[i] != '"' ){
			this->string[i-1] = this->string[i];
			i++;
		}
		this->string[--i] = '\0';
	}
}

void FwCHAR::stripExtension()
{
	int len = this->toLength();
	int dotloc = len;

	while( this->string[dotloc] != '.' && dotloc >= 0 ){
		if( this->string[dotloc] == '\\' ) return;
		dotloc--;
	}
	if( dotloc >= 0 )
		this->string[dotloc] = '\0';
}

int FwCHAR::toLength()
{
	return (int)wcslen(this->string);
}
/*
Counts what is the maximal length of lines the string has
*/
int FwCHAR::toMaxLineLength()
{
	int len = this->toLength();
	int maxline = 0;

	int counter = 0;
	for( int i = 0; i <= len; i++ ){
		if( this->string[i] != '\n' ){
			counter++;
		}
		else {
			maxline = max(maxline,counter);
			counter = 0;
		}
	}
	maxline = max(maxline,counter);

	return maxline;
}
/*
Counts the lines of the string basing on the newline \n character
*/
int FwCHAR::toLines()
{
	int len = this->toLength();
	int lines = 1;

	for( int i = 0; i <= len; i++ ){
		if( this->string[i] == '\n' ){
			lines++;
		}
	}
	return lines;
}
/*
Compares 2 strings using locale data
*/
int FwCHAR::compare(FwCHAR *string1, FwCHAR *string2)
{
	locale loc(ENGLISH_LOCALE);

	FwCHAR *fol1, *fol2, *fil1, *fil2;
	int result = 0;

	fol1 = new FwCHAR();
	fol2 = new FwCHAR();

	fol1->getFolderFrom(string1);
	fol2->getFolderFrom(string2);

	if( fol1->toLength() < 3 || fol2->toLength() < 3 )
		result = 0;
	else
		result = wcscmp(fol1->toWCHAR(),fol2->toWCHAR());
				/*use_facet<collate<WCHAR>>(loc).compare(fol1->toWCHAR(),
							 &fol1->toWCHAR()[wcslen(fol1->toWCHAR())],
														fol2->toWCHAR(),
							&fol2->toWCHAR()[wcslen(fol2->toWCHAR())]);*/
	if( result == 0 ){
		fil1 = new FwCHAR();
		fil2 = new FwCHAR();

		fil1->getFilenameFrom(string1);
		fil2->getFilenameFrom(string2);

		result = wcscmp(fil1->toWCHAR(),fil2->toWCHAR());
				/*use_facet<collate<WCHAR>>(loc).compare(fil1->toWCHAR(),
							 &fil1->toWCHAR()[wcslen(fil1->toWCHAR())],
														fil2->toWCHAR(),
							 &fil2->toWCHAR()[wcslen(fil2->toWCHAR())]);*/
		delete fil1;
		delete fil2;
	}
	delete fol1;
	delete fol2;

	return result;
}