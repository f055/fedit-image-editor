#include <math.h>

#include "commctrl.h"

#include "FwCHAR.h"
#include "List.h"
#include "XUn.h"

#include "ChildCore.h"
#include "Workspace.h"
#include "History.h"
#include "Frame.h"
#include "Tool.h"

#include "Dialogs.h"
#include "Toolws.h"

#include "Interface.h"
#include "Explorer.h"
#include "Cacher.h"
#include "Layer.h"
#include "Drawer.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MdiProc(HWND, UINT, WPARAM, LPARAM);

class Core
{
protected:
	HWND windowHandle;				// application handle
	HWND mdiclientHandle;			// mdi client handle
	HINSTANCE instance;				// application instance

	WNDPROC MDIWndProc;				// original mdi window procedure
	List<ChildCore> *children;		// list of opened child windows

	Dialogs *dialogs;				// dialogs controler
	Toolws *toolws;					// tool windows controler

	Interface *gui;					// gui controler
	Explorer *explorer;				// unused
	Cacher *cacher;					// unused
	Drawer *drawer;					// unused

	FwCHAR *commandLine;			// command line data
	HFONT hFontShell;				// default interface font

	bool initialized;				// init flag

	static HMODULE rarDll;			// unrar.dll handle
	static int chicounter;			// child windows counter

public:
	Core(WCHAR *cmdLine, HINSTANCE instance);
	~Core();

	static LRESULT CALLBACK processDialogs(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processFakeDialogs(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT processMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT processMdiMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND getWindowHandle();
	HWND getMdiclientHandle();
	HWND getActiveChildHandle();
	ChildCore *getActiveChild();
	HINSTANCE getInstance();

	List<ChildCore> *getChildren();

	Dialogs *getDialogs();
	Toolws* getToolws();

	Interface *getGui();
	Explorer *getExplorer();
	void setExplorer(Explorer *explorer);

	Cacher *getCacher();
	Drawer *getDrawer();

	RECT getMdiClientSize();
	RECT getClientSize();

	void mdiArrange();
	void mdiCascade();
	void mdiTileVer();
	void mdiTileHor();

	bool neww(ChildCore *child);
	bool openFolder(FwCHAR *path);
	bool open(FwCHAR *path, bool save = true);
	bool open(WCHAR *path, bool save = true);
	void extract();
	void setwall();
	void close();

	void redrawAll(RECT *rect);

	int messageBox_Info(WCHAR *string);
	int messageBox_Error(WCHAR *string);
	int messageBox_Prompt(WCHAR *string);

	HFONT CreateHFONT(WCHAR *fontFamily, UINT size, INT style = FontStyleRegular, Unit unit = UnitPoint);

	HWND CreateWindowExSubstituteFont(
		DWORD dwExStyle,
		LPCTSTR lpClassName,
		LPCTSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance,
		LPVOID lpParam
		);

	static HMODULE getRarDll();
	static void getLastError();
	static int getEncoder(WCHAR* format, CLSID* pClsid);
	static int tickChicounter();
	static int getChicounter();
	static int getPixelFormat();

	static FwCHAR *getDlgItemString(HWND hDlg, int itemId);
	static float getDlgItemNumber(HWND hDlg, int itemId, bool sign);

	static Image *getImageResource(WORD id, LPCWSTR type);

	static bool isInBmp(Bitmap *bmp, int x, int y);
	static Color invClr(Color color, int grey = NULL);

	static UINT RGBtoUINT(int r, int g, int b, int a = 255);
	static UINT RGBtoUINT(Color rgb);
	static Color UINTtoRGB(UINT uint);
	static RGBCOLOR UINTtoRGBCOLOR(UINT uint);
	static UINT RGBCOLORtoUINT(RGBCOLOR rgba);
	static CMYKCOLOR RGBtoCMYK(Color rgb);
	static Color CMYKtoRGB(CMYKCOLOR cmyk);
	static HSVCOLOR RGBtoHSV(Color rgb);
	static Color HSVtoRGB(HSVCOLOR hsv);
	static LabCOLOR RGBtoLab(Color rgb);
	static Color LabtoRGB(LabCOLOR lab);

	static POINT makePoint(LPARAM lParam);
	static void clearBitmap(Bitmap *bmp, Rect rect);

	static Bitmap *getClipboardBitmap();
	static void setClipboardBitmap(Bitmap *bmp);
	static bool isClipboardBitmap();

	static bool convertToDIB(HBITMAP &hBitmap, HPALETTE &hPalette);

	static Core *self;
	static Bitmap *clipboardBitmap;

private:
	void initialize(HWND windowHandle);
	void initializeMdi();
	void initializeFonts();
	void initializeToolws();

	bool extractResource(WORD id, WCHAR *filename);

	void loadDlls();
	void freeDlls();

	void destroy();
};