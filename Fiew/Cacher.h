class FwCHAR;

class Catalog;
class File;

using namespace Gdiplus;

class Cell
{
private:
	File *file;				// File object associated with that image
	Bitmap *thumb;			// Image thumbnail
	IStream *stream;		// Image raw data as IStream

public:
	Cell(File *file, byte *data);
	Cell(File *file);
	~Cell();

	File *getFile();
	Image *getImage();
	Bitmap *getImageThumb();
	Bitmap *getImageThumb(int width, int height);
	IStream *getStream();

	bool isLoaded();

	Cell *loadThumb(int load);

private:
	bool loadStream(byte *data, DWORD len);
};

class Cacher 
{
private:
	List<Cell> *cache;		// cache data - Cell list
	Catalog *source;		// cache source destination of Catalog type
	Core *core;				// owner

	int full;

protected:
	HANDLE mut_cache, mut_bool;

	HANDLE mut_initloop, mut_initterminator;
	HANDLE mut_nextloop, mut_nextstep, mut_nextterminator;
	HANDLE mut_prevloop, mut_prevstep, mut_prevterminator;

	HANDLE thrd_init, thrd_next, thrd_prev;

	HANDLE sem_nexts, sem_prevs, sem_nextlock, sem_prevlock;

public:
	Cacher(Core *core);
	~Cacher();

	void init();

	Catalog *getSource();
	List<Cell> *getCache();

	void setFull(bool val, bool thumbs = true);
	int isFull();
	bool isRunning();

	bool next();
	bool prev();
	Cell *getThat();
	Image *getThatImage();
	Image *getThatThumb(int width, int height);

	void unlockNext();
	void unlockPrev();

	void lockCache();
	void unlockCache();

	void gotoCell(Cell *cell);

	static DWORD WINAPI initAlloc(LPVOID param);
	static DWORD WINAPI nextAlloc(LPVOID param);
	static DWORD WINAPI prevAlloc(LPVOID param);
};