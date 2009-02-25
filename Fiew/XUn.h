#include "XUnrar.h"
#include "XUnzip.h"

class FwCHAR;
template<class T>
class List;
class Core;

class Archive;
class File;

class XUn
{
protected:
	Core *core;

	FwCHAR *path;
	byte *buffer;
	char *comment;

	bool extracting;

	int error;
	int count;
	int buflen, bufpot;

	FwCHAR *password;
	HANDLE sem_pass;

public:
	XUn(WCHAR *archivepath);
	virtual ~XUn();

	virtual bool list(List<File> *list);
	virtual bool extract(File *file, WCHAR *dest = NULL);

	virtual bool isLoaded();

	int getLastError();
	void setLastError(int error);

	FwCHAR *getPassword();
	void setPassword(FwCHAR *password);
	void sendPassReq();

	int getBuflen();
	byte *getBuffer();
	void clearBuffer();
	void setBuffer(void *source, int size);
};

class XUnrar : public XUn
{
private:
	bool loaded;
	char *charpath;

	HANDLE handle;

public:
	XUnrar(WCHAR *archivepath);
	~XUnrar();

	bool list(List<File> *list);
	bool extract(File *file, WCHAR *dest = NULL);

	bool isLoaded();

	static int CALLBACK callbackProc(UINT,LONG,LONG,LONG);

private:
	HANDLE (WINAPI *openArchive)(RAROpenArchiveData *);
	HANDLE (WINAPI *openArchiveEx)(RAROpenArchiveDataEx *);
	int (WINAPI *closeArchive)(HANDLE);
	int (WINAPI *readHeader)(HANDLE,RARHeaderData *);
	int (WINAPI *readHeaderEx)(HANDLE,RARHeaderDataEx *);
	int (WINAPI *processFile)(HANDLE,int,char *,char *);
	int (WINAPI *processFileW)(HANDLE,int,WCHAR *,WCHAR *);
	void (WINAPI *setCallback)(HANDLE,UNRARCALLBACK,LONG);

	bool loadFunc();
	bool reopen(int mode);
};

class XUnzip : public XUn
{
private:
	HZIP handle;

public:
	XUnzip(WCHAR *archivepath);
	~XUnzip();

	bool list(List<File> *list);
	bool extract(File *file, WCHAR *dest = NULL);

	bool isLoaded();
};