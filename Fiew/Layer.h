using namespace Gdiplus;

class ChildCore;

class Cell;

class Layer {
protected:
	ChildCore *core;

	Image *image, *scali;
	int type, fitmode, sidemode;

	int x, y, width, height, cwidth, cheight;
	int rollHor, rollVer, maxrollHor, maxrollVer, minrollVer, offrollVer;
	int offset, menuheight, rot, gifdir;
	double zoom;
	bool fulldraw, sidedraw, zoomdraw, fullscreen, cancel;

	GUID *dimension;
	UINT frameCount, frameThat;

	SolidBrush *Brush_Back;
	SolidBrush *Brush_DarkBack;
	SolidBrush *Brush_LiteBack;

	Pen *Pen_Border;
	Pen *Pen_DarkBorder;

	int FontSize;

	FontFamily *FontFamily_Arial;
	Font *Font_Default;

	HANDLE mut_animloop, mut_image, mut_terminator;
	HANDLE thrd_anim;

public:
	Layer(ChildCore *core, Image *image);
	virtual ~Layer();

	void loadContent(Image *source,int init = TOP);

	virtual void nextImage(int x = FERROR, int y = FERROR);
	virtual void prevImage(int x = FERROR, int y = FERROR);

	virtual Bitmap *render();
	void reset();
	virtual void repos();

	virtual void scroll(int hor, int ver, bool invalidate = true);
	virtual void scrollSet(int x, int y, bool invalidate = true);
	virtual void scrollHor(int val);
	virtual void scrollVer(int val);

	virtual void zoomer(double val = NULL);
	virtual void zoomat(double val = ZOOMINIT, bool invalidate = true);
	virtual void zoomend(bool invalidate = true);
	
	virtual void rotate(int val = NULL);
	virtual void rotateReset(bool novalid = false);

	int getX();
	int getY();

	int getWidth();
	int getHeight();
	File *getFile();
	Image *getImage();
	int getImageWidth();
	int getImageHeight();
	RECT getClientSize();

	SCROLLINFO getScrollHor();
	SCROLLINFO getScrollVer();
	RECT getScrollInfo();
	POINT getScrollMax();

	int getMaxrollHor();
	int getMaxrollVer();

	double getZoom();
	bool getSidedraw();
	int getSidemode();
	int getFitmode();

	virtual void setFitmode(int mode = DEFAULT);
	virtual void unsetFitmode();

	virtual void setSidemode(int mode = NULL);

	virtual void setSidedraw();

	void setMenuheight(int val);
	void setGifDir(int dir = DEFAULT);
	void setCancel(bool set = true);

	bool setWall();

	bool isCancel();
	bool isContent();

	void locate(int init = NULL);
	void invalidate(bool full = true);

	static void scaleEnlarge(Bitmap *src, Rect srcRect, Bitmap *bmp, double zoom = ZOOMINIT);
	static void scaleEnlarge(Bitmap *src, Bitmap *bmp, double zoom = ZOOMINIT);
	static void scaleInvert(Bitmap *src, Rect *srcRect, Bitmap *bmp, Rect *bmpRect = NULL, int hatch = NULL, double zoom = ZOOMINIT);

	static DWORD WINAPI anim(LPVOID param);

protected:
	void init(Image *source);

	void afterLoadContent(int init);

	bool nextFrame(bool back = false);
	bool prevFrame(bool back = false);

	void rotate(Image *image);

	void zoombegin();
	void rotateSet();

	void boundRoll();
	void boundZoom();

	double getZoom(int width, int height);
	double getZoom(int width, int height, int cwidth, int cheight);

	void animate();
	Image *scale(Image *source);
	
	bool isTopmost();
};

class Overlay : public Layer
{
public:
	Overlay(ChildCore *core, Image *image);
	~Overlay();

	virtual void nextImage(int x = FERROR, int y = FERROR);
	virtual void prevImage(int x = FERROR, int y = FERROR);

	Bitmap *render();

	virtual void zoomer(double val = NULL);
	virtual void zoomat(double val = ZOOMINIT, bool invalidate = true);
	virtual void zoomend(bool invalidate = true);
	
	virtual void rotate(int val = NULL);
	virtual void rotateReset(bool novalid = false);

	virtual void setFitmode(int mode = DEFAULT);
	virtual void unsetFitmode();
	virtual void setSidedraw();

	virtual void setSidemode(int mode = NULL);

	RECT getOverlayRect();

protected:
	virtual void hide();
};

class Thumblay : public Overlay
{
private:
	Explorer *explorer;
	Cacher *cacher;
	Core *hardcore;

	HWND hOwner;

	Cell *lastCell;
	int ticker;
	int picker;

public:
	Thumblay(ChildCore *core, Image *image);
	Thumblay(HWND hOwner, Core *core, FwCHAR *path);
	~Thumblay();

	void update(bool init = false);

	void nextImage(int x = FERROR, int y = FERROR);
	void prevImage(int x = FERROR, int y = FERROR);
	void prevImageDblClk(int x = FERROR, int y = FERROR);

	void scroll(int hor, int ver, bool invalidate = true);
	void scrollSet(int x, int y, bool invalidate = true);
	void scrollHor(int val);
	void scrollVer(int val);

	void zoomer(double val = NULL);
	void zoomat(double val = ZOOMINIT, bool invalidate = true);
	void zoomend(bool invalidate = true);
	
	void rotate(int val = NULL);
	void rotateReset(bool novalid = false);

	void setFitmode(int mode = DEFAULT);
	void unsetFitmode();
	void setSidedraw();

	void setSidemode(int mode = NULL);

	Cell *getLastCell();

	void hide();
	void subrender();

private:
	void setPicker(int newpick);
	int getPicker(int x, int y);

};

class Listlay : public Overlay
{
private:
	Cell *lastCell;
	int ticker;

public:
	Listlay(ChildCore *core, Image *image);
	~Listlay();

	void update(bool init = false);

	void nextImage(int x = FERROR, int y = FERROR);
	void prevImage(int x = FERROR, int y = FERROR);

	void scroll(int hor, int ver, bool invalidate = true);
	void scrollSet(int x, int y, bool invalidate = true);
	void scrollHor(int val);
	void scrollVer(int val);

	void zoomer(double val = NULL);
	void zoomat(double val = ZOOMINIT, bool invalidate = true);
	void zoomend(bool invalidate = true);
	
	void rotate(int val = NULL);
	void rotateReset(bool novalid = false);

	void setFitmode(int mode = DEFAULT);
	void unsetFitmode();
	void setSidedraw();

	void setSidemode(int mode = NULL);

	Cell *getLastCell();

private:
	void subrender();
};

class Gridlay : public Layer
{
private:
	Bitmap *gridblock;
	CachedBitmap *pattern;

	bool rendered;
	int oldx, oldy;

public:
	Gridlay(ChildCore *core);
	~Gridlay();

	Bitmap *render();
	bool isRendered();

	void preload(RECT client, RECT scrollInfo, int x, int y, int width, int height);

	void scroll(int hor, int ver, bool invalidate = true);
	void scrollSet(int x, int y, bool invalidate = true);
	void scrollHor(int val);
	void scrollVer(int val);

	void zoomer(double val = NULL);
	void zoomat(double val = ZOOMINIT, bool invalidate = true);
	void zoomend(bool invalidate = true);
};

class Snaplay : public Layer
{
private:
	Pen *penPattern, *penSolid;

	int oldWidth, oldHeight;
	double oldZoom;

public:
	Snaplay(ChildCore *core, Image *image);
	~Snaplay();

	Bitmap *render();

	void preload();
};