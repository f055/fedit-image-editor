using namespace Gdiplus;

class Core;
class Toolws;

class Tool;

class Toolw
{
protected:
	Core *core;
	Toolws *controler;

	HWND hToolw;

public:
	Toolw(Core *core, Toolws *controler);
	virtual ~Toolw();

	virtual HWND initialize();
	HWND getWindowHandle();

	Core *getCore();
	Toolws *getControler();

	static void globalSubclass(HWND hDlg);
	static LRESULT CALLBACK globalProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static void tooltipSubclass(HWND hDlg, FwCHAR *tip);
	static LRESULT CALLBACK tooltipProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK tooltipLabelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual void destroy();
};

class ToolwBoxContent;

/* Popup Boxes Controler */
class ToolwBox : public Toolw
{
protected:
	RECT sizeContent;
	List<ToolwBoxContent> *contents;

	bool noresize;

public:
	ToolwBox(Core *core, Toolws *controler, bool noresize = false);
	~ToolwBox();

	void addContent(ToolwBoxContent *content);
	void removeContent(ToolwBoxContent *content);

	void arrange();

	static LRESULT CALLBACK processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processContents(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	void resizeContent();
};

/* Tools Independent Scrollable Container */

class ToolwBoxScroll
{
protected:
	HWND hToolw, hContent, hOldContent;

	int width, height, scrollHorMax, scrollVerMax, scrollHor, scrollVer, rollHor, rollVer;
	bool isScrollHor, isScrollVer;

public:
	ToolwBoxScroll(HWND hOwner, int x, int y, int w, int h, bool hor, bool ver);
	~ToolwBoxScroll();

	HWND getWindowHandle();

	HWND createContent(HANDLE owner, int w = NULL, int h = NULL, int hs = NULL, int vs = NULL, bool hss = false, bool vss = false);
	HWND clearContent();

	HWND unplugContent();
	HWND plugContent();

	void setSkew(int x, int y);
	void addSkew(int x, int y);

	int getScrollHorMax();
	int getScrollVerMax();

	static LRESULT CALLBACK processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processContents(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void locate(int hs = NULL, int vs = NULL, bool hss = false, bool bss = false);

	SCROLLINFO getScrollHor();
	SCROLLINFO getScrollVer();
	void setScrollBar();
	void setContentPos();

	void boundRoll();

	void resizeContent();

	RECT getContentRect();
	RECT getBoxRect();

};

/* Popup Boxes Content */

class ToolwBoxContent : public Toolw
{
protected:
	ToolwBox *box;
	FwCHAR *name;

public:
	ToolwBoxContent(ToolwBox *box, FwCHAR *name);
	virtual ~ToolwBoxContent();

	virtual void resizeContent(int dw, int dh);
	virtual void disable();

	FwCHAR *getName();
	ToolwBox *getBox();

	virtual LRESULT processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	
protected:
	HWND createContent(int x, int y, int w, int h);
};

class ToolwBoxInfo : public ToolwBoxContent
{
private:
	HWND dlgColR, dlgColG, dlgColB,
		 dlgColC, dlgColM, dlgColY, dlgColK,
		 dlgPosX, dlgPosY, dlgSizW, dlgSizH;

public:
	ToolwBoxInfo(ToolwBox *box);
	~ToolwBoxInfo();

	void resizeContent(int dw, int dh);
	void disable();

	void loadColor(Color rgb);
	void loadPos(Point pos);
	void loadSize(Rect size);

	void loadLabel(HWND handle, UINT value);
	void nullLabel(HWND handle);

	LRESULT processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void createPair(int x, int y, int w, int h, WCHAR *label, HWND &handle);

};

class ToolwBoxLayers : public ToolwBoxContent
{
private:
	ToolwBoxScroll *layersView;
	HWND layerContents;

	HWND hTopContents, hBotContents, hComboStyle, hInputOpacity, hCheckLock;
	int topMargin, botMargin, botButCount, lastId, minId, maxId;

public:
	ToolwBoxLayers(ToolwBox *box);
	~ToolwBoxLayers();

	void load();
	void refresh();
	void refreshCurrent();

	void recontrol();

	void resizeContent(int dw, int dh);
	void disable();

	LRESULT processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void paintButton(LPDRAWITEMSTRUCT button);
};

class ToolwBoxHistory : public ToolwBoxContent
{
private:
	ToolwBoxScroll *historyView;
	HWND historyContents;

	HWND hBotContents;
	int botMargin, botButCount, lastId, minId, maxId;

public:
	ToolwBoxHistory(ToolwBox *box);
	~ToolwBoxHistory();

	void load();
	void refresh();
	void recontrol();

	void resizeContent(int dw, int dh);
	void disable();

	LRESULT processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void paintButton(LPDRAWITEMSTRUCT button);
};

/* Tools Control Center Toolwindow */
class ToolwCC : public Toolw
{
public:
	Tool	*toolCurrent,
			*toolHand,
			*toolZoom,
			*toolPencil,
			*toolLine,
			*toolRect,
			*toolElli,
			*toolRRec,
			*toolMove,
			*toolSelectRect,
			*toolSelectCirc,
			*toolSelectHor,
			*toolSelectVer,
			*toolSelectPoly,
			*toolSelectFree,
			*toolSampleColor,
			*toolCrop,
			*toolSelectWand,
			*toolBucket,
			*toolText,
			*toolEraser,
			
			*toolCut,
			*toolCopy,
			*toolCopyMerged,
			*toolPaste,
			*toolClear,

			*toolFilter,
			*toolMerge,
			*toolRaster;

protected:
	HWND hColorFore, hColorBack;
	Color color_fore, color_back;

	HWND buttonHover, buttonValidator, buttonPopup, tooltipControler;
	WNDPROC buttonProc, popupButtonProc;
	List<HWND__> *buttons;

	Pen *pen_light, *pen_dark;
	SolidBrush *brush_bkgnd, *brush_hilite;

public:
	ToolwCC(Core *core, Toolws *controler);
	~ToolwCC();

	HWND initialize();

	Tool *getTool();
	void setTool(Tool *tool);

	Color getForeColor();
	Color getBackColor();
	void setForeColor(Color color);
	void setBackColor(Color color);
	void setColor();

	void swapColor();
	void resetColor();

	static LRESULT CALLBACK processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processButtons(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processPopups(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processPopupButtons(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void destroy();

	HWND createCC_Button(HWND hWnd, WCHAR *icon, int x, int y, int id);
	HWND createCC_Popup(HWND hButton, int x, int y);
	void createCC_PopupButton(HWND hWnd, WCHAR *icon, int x, int y, int id);
	int  createCC_Colorer(HWND hWnd, WCHAR *icon, int y);
	void createSepH(HWND hWnd, int x, int y, int w);

	void paintButton(LPDRAWITEMSTRUCT button);
	void invalidateButton(HWND hItem);
	
	void uncheckButtons(int exceptId = NULL);
	void uncheckButton(HWND hItem);
	void uncheckButton(int id);

	void checkButton(int id);
	void checkButton(HWND hItem);

	void clickButton(HWND hItem);
	void clickButton(HWND hWnd, HWND hItem);

	void closePopup();
	void addPopupList(HWND hButton, List<Point> *list);
	List<Point> *getPopupList(HWND hButton);

	FwCHAR *toolIdToName(int toolId);

	void sendToolDblClk(UINT message, WPARAM wParam, LPARAM lParam, int itemId);

};

/* Docked Toolwindow */
class ToolwDock : public Toolw
{
protected:
	bool isDocked, isDragged;
	int x, y, width, height;
	int dockPlace, lastPlace;

	DWORD styleDocked, styleUndocked;
	POINTS lastMouse;

	List<HWND__> *children;

public:
	ToolwDock(Core *core, Toolws *controler);
	~ToolwDock();

	HWND initialize();

	void toggleDock(int place = TOP);

	void addChild(HWND hWnd, HWND zorder = HWND_TOP, Tool *owner = NULL);
	void setChildSize(HWND hWnd);
	void setChildPos(HWND hWnd);
	void setChild(HWND hWnd);
	void hideChild();

	bool addPos(int x, int y);
	void setPos(int x, int y);
	void setSize(int w, int h = TW_DOCK_H);
	void setDock(int x, int y, int w, int h = TW_DOCK_H);
	void setDock();

	void setSiblings();

	int getX();
	int getY();
	int getWidth();
	int getHeight();
	int getDockedHeight();

	int getDockState();

	int processFrame(HWND hWnd);
	static LRESULT CALLBACK processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processTools(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void destroy();

	void paint(HDC hdc);

	void capMouseDown(POINTS p, UINT button);
	void capMouseMove(POINTS p);
	void capMouseUp(POINTS p, UINT button);

	void dock(int place = TOP);
	void undock(int place = TOP);
};

/* Toolwindows Controler */
class Toolws
{
private:
	Core *core;
	List<HWND__> *toolwset;

	ToolwCC *toolwCC;
	ToolwDock *toolwDock;
	ToolwBox *toolwBox;

	ToolwBoxLayers *toolwBoxLayers;
	ToolwBoxInfo *toolwBoxInfo;
	ToolwBoxHistory *toolwBoxHistory;

public:
	FwCHAR *nameColRes,
		   *nameColSwap, 
		   *nameNewLay, 
		   *nameDelLay,
		   *nameLayUp, 
		   *nameLayDown,
		   *nameUndo,
		   *nameRedo,
		   *nameDelHis,
		   *nameColCho;

	Toolws(Core *core);
	~Toolws();

	void show_CC();
	void hide(HWND toolw);

	void enableToolwBoxes();
	void disableToolwBoxes();
	void arrangeToolBoxes();

	void toggleBoxInfo();
	void toggleBoxLayers();
	void toggleBoxHistory();

	List<HWND__> *getToolwset();

	ToolwCC *getToolwCC();
	ToolwDock *getToolwDock();

	ToolwBoxLayers *getToolwBoxLayers();
	ToolwBoxInfo *getToolwBoxInfo();
	ToolwBoxHistory *getToolwBoxHistory();

	LRESULT overrideEnable(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT overrideNCActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);

	FwCHAR *toolIdToName(int toolId);

	static int boxCounter;

};