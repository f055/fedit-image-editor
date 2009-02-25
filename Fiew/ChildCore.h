using namespace Gdiplus;

class FwCHAR;
class Core;

class Drawer;
class Cacher;
class Explorer;
class Interface;

class Workspace;
class Tool;

class ChildCore
{
protected:
	Core *core;					// owner
	HWND windowHandle;			// child window handle

	FwCHAR *filepath,			// path of edited file (if any)
		*title;					// filename of edited file

	bool initialized,			// initialize flag
		constructed,			// done constructor flag
		repaint,				// force repaint flag
		isTracked;				// mouse leave tracking flag
	int maxrollHor,				// max horizontal scroll value
		maxrollVer;				// min -- // --

	Drawer *drawer;				// Drawer object for painting the view
	Workspace *workspace;		// Workspace object for image editing control
	Tool *toolCurrent;			// Currently selected Tool

public:
	ChildCore(Core *parent,
			  FwCHAR *filepath,
			  int type);
	ChildCore(Core *parent,
			  Bitmap *temp,
			  FwCHAR *title = NULL);
	ChildCore(Core *parent,
			  FwCHAR *title,
			  float width, float height, int sunit,
			  float res, int runit,
			  int dmode, int dunit, 
			  int bkgnd);
	~ChildCore();
	void reset();

	bool initialize();
	bool isInitialized();
	bool isConstructed();
	void invalidate(bool repaint = true);

	bool save(WCHAR *filepath,int type);
	
	void setText();

	void addSkew(int x, int y);
	void setSkew(int x, int y);
	void pinSkew(int x, int y);

	void addZoom(double zoom);
	void setZoom(double zoom);

	void setTool(Tool *tool);
	void sendToolDblClk(UINT message, WPARAM wParam, LPARAM lParam, int itemId);

	void setScrollBar(SCROLLINFO hor, SCROLLINFO ver);

	HWND getWindowHandle();

	Explorer *getExplorer();
	Interface *getGui();
	Cacher *getCacher();
	Drawer *getDrawer();

	Workspace *getWorkspace();

	FwCHAR *getFilepath();

	static LRESULT CALLBACK processMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void destroy();

};