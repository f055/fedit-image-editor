class ChildCore;

using namespace Gdiplus;

class Drawer
{
private:
	ChildCore *core;			// owner

	List<Layer> *layers;		// Layers list
	Layer *scene, *overlay;		// handles to specific Layers
	Snaplay *snaplay;
	Gridlay *gridlay;
	Thumblay *thumblay;
	Listlay *listlay;

	int zoom,					// zoom value, DRZOOMINIT compatible
		modeInvalidate;			// invalidation mode
	Bitmap *bmpScene,			// last rendered scene bitmap
		*bmpInvalidate;			// invalidate bitmap buffer
	Rect rectScene,				// scene rectangle
		rectInvalidate;			// invalidation rectangle
	bool fulldraw,				// perform full redraw flag
		zoomdraw,				// use zoom flag
		visAbout,				// Overlays flags
		visManual, 
		visThumbs, 
		visList, 
		isPreload;				// transparency grid preloaded flag

	Pen *penSelection,			// pen used for drawing selections
		*penDraw;				// pen used for drawing draw paths
	GraphicsPath *drawpath;		// draw path buffer

public:
	Drawer(ChildCore *core);
	~Drawer();

	void paint(HDC hdc);
	void reset(Image *image);

	Layer *getTopmost();
	Layer *getScene();
	Overlay *getOverlay();
	Thumblay *getThumblay();
	Listlay *getListlay();
	RECT getClientSize();

	Rect getRectScene();
	Bitmap *getBmpScene();

	Point getCenter();

	double getZoom();

	void updateOverlay();

	void hideOverlay(bool invalid = true);
	void showAbout();
	void showManual();
	void showThumbs();
	void showList();

	bool isAbout();
	bool isManual();
	bool isThumbs();
	bool isList();

	void setMenuheight(int val = 0);
	void invalidate(bool full = true);
	void invalidate(Rect *rect, Bitmap *bmp, int mode = INVDEF, bool full = false);

	void zoomer(double val);
	void zoomSet(double val);
	void zoomReset();

	void scroll(int x, int y);
	void scrollSet(int x, int y);
	void scrollPinpoint(int x, int y);

	void setDrawpath(GraphicsPath *path, bool zoom = true, bool invalidate = true);
	void setDrawpenSize(int size);
	void setDrawpenStyle(DashStyle style = DashStyleSolid);

	void preloadBackground(Layer *lay);

	bool renderBackground(Graphics *scenegfx, double zoom = 1.0, bool supress = false);
	bool renderBackground(Graphics *scenegfx, Rect rect);

	static void setInterpolation(Graphics *g, double zoom = ZOOMINIT);
	static void setHQInterpolation(Graphics *g, double zoom = ZOOMINIT);

private:

	void renderGraphicsPath(Bitmap *scene, GraphicsPath *path, Pen *pen, int hatch = NULL, bool zoom = true, bool zoomround = true);
	void renderSelection(Bitmap *scene);
	void renderDraw(Bitmap *scene);
};