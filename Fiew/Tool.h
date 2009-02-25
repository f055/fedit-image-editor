class FwCHAR;
class ChildCore;

class Tool
{
protected:
	Core *core;				// application Core
	ChildCore *chicore;		// current Tool owner
	FwCHAR *name;			// Tool name

	HCURSOR cursor,			// publicly accessible Tool cursor
		cursorBackup;		// private backup cursor
	UINT id;				// Tool id (same as the Tool icon id in resource)

	HWND hdocktool;			// Tool dock content handle

	POINT mouse;			// current mouse position

	bool isAlt,				// is Alt pressed
		isCtrl,				// is Ctrl pressed
		isShift;			// is Shift pressed

public:
	Tool(FwCHAR *name, HCURSOR cursor, UINT id);
	virtual ~Tool();

	// Process windows messages
	LRESULT processMessages(
		Core *core,
		ChildCore *chicore,
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam,
		int area
		);

	UINT getId();
	FwCHAR *getName();
	HCURSOR getCursor();

	// Tool activation triggered when the tool is set
	virtual void activate();
	// Tool dock content activation
			void activateDock();
	// Assign dock content to the application Dock Window 
			void submitDock();
	// Fill dock content with interface controls
	virtual void fillDock();
	// Update the dock contents with the current state of Workspace
	virtual void updateDock();

	// Tool deactivation triggered before the new tool is set
	virtual void deactivate();
	// Notification from the Tool's dock content id - control's id that triggered the notify
	virtual void notify(int id);

	// Calculate mouse position in Workspace coordinates
	static POINT getWorkspaceMousePos(ChildCore *chicore, LPARAM lParam);

	// Create cursor from tool icon
	static HCURSOR createToolCursor(int hotXspot, int hotYspot, int toolId);

	// Process mouse messages
	virtual void capMouseDblClk(WPARAM wParam, LPARAM lParam, int button);

private:
	virtual void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseWheel(WPARAM wParam, LPARAM lParam);
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
	// Process keyboard messages
	virtual void capKeyDown(WPARAM wParam, LPARAM lParam);
	virtual void capKeyUp(WPARAM wParam, LPARAM lParam);

	// Get mouse position
	virtual POINT getMousePos(LPARAM lParam);
	// Get additional keys pressed when clicking processing mouse messages
	void getKeyStatus(WPARAM wParam);

	virtual void updateInterface(WPARAM wParam, LPARAM lParam);

protected:
	// Update Tool Windows
	void updateToolws(WPARAM wParam, LPARAM lParam);

	void setMouseCapture();
	void setMouseRelease();

	// Is current Workspace Frame ready for applying Tool actions
	bool isLayerReady();
	// Set Tool cursor as current mouse cursor
	bool loadCursor();
};

/*
Menu Tools - activate only
*/
struct RGBCOLOR { UINT R,G,B,A; };

class ToolMerge : public Tool
{
public:
	ToolMerge();
	~ToolMerge();
};

class ToolRaster : public Tool
{
public:
	ToolRaster();
	~ToolRaster();
};

class ToolFilter : public Tool
{
private:
	int lastFilterId, lastFilterValue;
	bool lastActivate, isLastActivate, lastBw, lastAlpha;

public:
	struct Matrix {
		int **matrix,		// matrix 2 dimensional array
			mxw,			// matrix width
			mxh;			// matrix height
		double division, 
			bias;
	} lastCustomMatrix;		// matrix structure

	struct Info {
		// [in]
		Bitmap *bmpSource;

		bool edgeTrace,		// edge trace filter flag
			smooth,			// preview filtering flag
			bw,				// black & white only flag
			bwalpha,		// use alpha channel flag
			filterByValue;	// use filterValue or load custom matrix flag

		int filterId, filterValue, minVal, maxVal;

		// [in][out]
		ToolFilter::Matrix matrix;

		// [out]
		Bitmap *bmpEffect;

	} filterInfo;			// filter info structure

	ToolFilter();
	~ToolFilter();

	void activate();
	// try to activate recently applied filtering again
	void activateAgain();

	bool canActivateAgain();

	void setFilterId(int id);
	int getFilterId();

	static double calcMatrixWeight(ToolFilter::Matrix *matrix);

	// main filtering routine
	static Rect applyFilter(ToolFilter::Info *fi, Rect *clip = NULL, bool once = false);

	// get an average value from the color
	static INT  scalePixel(RGBCOLOR color, bool alpha = true);
	// get the color from the bitmap at specified point
	static UINT getPixel(BitmapData *bmp, int x, int y);
	// return pixel color value after applying matrix values onto the specified pixel
	// fi -		ToolFilter::Info structure with all filtering data
	// bmp -	source bitmap
	// x -		pixel x coordinate
	// y -		pixel y coordinate
	// w -		floored half width of applied matrix
	// h -		floored half height of applied matrix
	static UINT filterPixel(ToolFilter::Info *fi, BitmapData *bmp, int x, int y, int w, int h);

	// allocate int** matrix array
	static int **allocMatrix(int mxw, int mxh, int set = 0);
	// allocate ToolFilter::Matrix structure
	static ToolFilter::Matrix allocateMatrix(int filterId, int value);

	static ToolFilter::Matrix allocMatrixGeneric();
	// allocate specified filters' Matrixes
	static ToolFilter::Matrix allocMatrixBlur(int value);
	static ToolFilter::Matrix allocMatrixBlurGauss(int value);
	static ToolFilter::Matrix allocMatrixSharpen(int value);
	static ToolFilter::Matrix allocMatrixEdgetrace(int value, int mode = NULL);
	static ToolFilter::Matrix allocMatrixEmboss(int value, int mode);
	static ToolFilter::Matrix allocMatrixHighlight(int value);
	static ToolFilter::Matrix allocMatrixDefocus(int value);
	static ToolFilter::Matrix allocMatrixOldstone(int value);

	static WCHAR *getFilterName(int filterId);
};

class ToolCopy : public Tool
{
private:
	int mode;
	bool noclipboard;

public:
	ToolCopy(int mode);
	~ToolCopy();

	void activate();
	void setNoclipboard();
};

class ToolFill : public Tool
{
public:
	ToolFill();
	~ToolFill();

	void activate();
};

class ToolStroke : public Tool
{
public:
	ToolStroke();
	~ToolStroke();

	void activate();
};

/*
Control Center Tools
*/
class ToolHand : public Tool
{
private:
	POINT mouseLast;
	HWND dlgAll;

	bool mouseMoving, isAll;

public:
	ToolHand();
	~ToolHand();

	void fillDock();
	void notify(int id);

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
};

class ToolSampleColor : public Tool
{
private:
	bool isMoving;
	int button;

public:
	ToolSampleColor();
	~ToolSampleColor();

	bool getPixel(POINT pixel, Bitmap *scene, Color *color);

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	void pickColor();
};

/*
Selecting Tools
ToolSelecting class is inherited by selecting Tools
*/

class ToolSelecting : public Tool
{
protected:
	int rectmargin;
	bool isSelecting, isMoving, isAA;

	GraphicsPath *currentPath, *oldPath;

public:
	ToolSelecting(FwCHAR *name, HCURSOR cursor, UINT id);
	virtual ~ToolSelecting();

	void beginPaint();
	void endPaint();

	virtual void activate();
	virtual void deactivate();

	virtual void capMouseDblClk(WPARAM wParam, LPARAM lParam, int button);

private:
	virtual void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseWheel(WPARAM wParam, LPARAM lParam);
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
	
	virtual void capKeyDown(WPARAM wParam, LPARAM lParam);
	virtual void capKeyUp(WPARAM wParam, LPARAM lParam);

protected:
	POINT getMousePos(LPARAM lParam);

	// can the selection be moved
	bool getSelectionMoveState(int x, int y);
	// set selection on the Workspace
	// path -		selection path
	// final -		if true create a HistoryElement
	// shiftJoin -	if true connect the new path with the old
	void setSelection(GraphicsPath *path, bool final = false, bool shiftJoin = false);
	void setSelection(List<Point> *poly, bool close, bool clip, bool final = false);
	void setSelection(RECT rect, int mode = SELRECT, bool final = false);

	void setOldPath();
	void resetOldPath(bool final = true);

	void setUpdate(GraphicsPath *path);

	void move(int x, int y);
	void update();
};
// Rectangle type of selection (Rectangle, Ellipse, 1px lines)
class ToolSelectRect : public ToolSelecting
{
private:
	RECT selection;
	int mode;

public:
	ToolSelectRect(int mode = SELRECT);
	~ToolSelectRect();

	void setSelectAll();
	void deselect();
	void setSelectInverse();

private:
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
};
// Polygonal type of selection (Polygonal Lasso, Free Lasso)
class ToolSelectPoly : public ToolSelecting
{
private:
	List<Point> *poly;

	int mode;

public:
	ToolSelectPoly(int mode);
	~ToolSelectPoly();

private:
	virtual void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	bool isEnd();
	void endPoly();
};
// Magic Wand
class ToolSelectWand : public ToolSelecting
{
private:
	Bitmap *selectBmp, *sourceBmp;

	HWND dlgTolerance, dlgSlider;
	int tolerance;

public:
	ToolSelectWand();
	~ToolSelectWand();

	void fillDock();
	void notify(int id);

	// magic color recognition
	static GraphicsPath *doMagic(Bitmap *sourceBmp,
								 Bitmap *select,
								 int x,
								 int y,
								 int shx,
								 int shy,
								 int tol);
	// magic shape vectorisation
	static GraphicsPath *doMagicSelection(Bitmap *source);

	// magic color comparison
	// pick -		original color
	// trick -		matching color
	// tolerance -	matching tolerance
	static bool doMagicComparison(UINT pick, UINT trick, UINT tolerance);

private:
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	static bool isonStack(List<Point> *stack, int x, int y);
};
// Cropping Tool
class ToolCrop : public ToolSelecting
{
private:
	RECT selection;
	POINT lastmouse;

	List<RectF> *cropctrl;
	double ratio;
	int ctrl;

public:
	ToolCrop();
	~ToolCrop();

	void deactivate();
	void finalize(bool prompt = false, bool isyes = true);

private:
	void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	void capKeyDown(WPARAM wParam, LPARAM lParam);
	void capKeyUp(WPARAM wParam, LPARAM lParam);

	void setCropctrls();
	void setCropath();

	int getHoverCtrl();

	void boundRect(RECT *rect);
	void limitRect(RECT *rect);
};

class ToolText : public ToolSelecting
{
private:
	HWND hEdit, hPane, dlgFonts, dlgSize, dlgAA, dlgBold, dlgItalic, dlgUline, dlgColor;
	FrameText *editFrame;

	HFONT editFont;
	INT currentFontStyle;
	Unit currentFontUnit;
	FwCHAR *currentFontName;
	FontFamily *currentFontFamily;

	POINT pointScreen, pointWorkspace;
	Color color;

	REAL currentFontSize;
	bool isFontAA;

public:
	ToolText();
	~ToolText();

	void fillDock();
	void updateDock();

	void deactivate();

	void notify(int id);

	// editing methods
	void editText(FrameText *owner);
	void createEdit(WCHAR *string, int x, int y, int w, int h);

	static int getFontPxAscent(FontFamily *fontFamily, INT fontStyle, REAL fontSize);
	static int getFontPxSize(FontFamily *fontFamily, INT fontStyle, REAL fontSize);
	// calculate text bounding box
	static Rect stringToBox(int fsize, int wsize, int lsize);
	// window procedures for in-place edit controls
	static LRESULT CALLBACK editProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK paneProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);

	void loadFonts();

	int getCurrentFontIndex();
	int getCurrentFontPxSize();
	int getCurrentFontPxAscent();

	WCHAR *getCurrentFontName();

	void setCurrentFont();
	void setEditFont();

protected:
	void finalize();
	void locate();
	
};
// Zoom Tool
class ToolZoom : public ToolSelecting
{
private:
	HWND dlgAll;
	RECT selection;
	Point lastPin;

	bool isAll;

public:
	ToolZoom();
	~ToolZoom();

	void fillDock();
	void notify(int id);

	void capMouseDblClk(WPARAM wParam, LPARAM lParam, int button);

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	void capKeyDown(WPARAM wParam, LPARAM lParam);
	void capKeyUp(WPARAM wParam, LPARAM lParam);

	void setDrawpath();
	double calcZoom(double sw, double sh, double zm);
	double calcFit(double sw, double sh);
};

/*
Drawing Tools
ToolDrawing class is inherited by all drawing Tools
*/

class ToolDrawing : public Tool
{
protected:
	SolidBrush *brushColor;
	Pen *penColor;

	int size, rectmargin;
	bool isDraw, isAA;

	HWND dlgSize, dlgAA, dlgSlider;

	POINT drawmouse;
	RECT drawrect, srcrect;
	Rect cliprect;
	Graphics *drawgfx;
	Bitmap *drawbmp, *sceneAbove, *sceneBelow, *sceneBetween;

public:
	ToolDrawing(FwCHAR *name, HCURSOR cursor, UINT id);
	virtual ~ToolDrawing();

	// prepare for drawing, create neccessary objects
	void beginPaint();
	// end drawing, apply the draw onto the selected Frame
	void endPaint();
	// update the MDI child scene
	void update(Rect rect, int mode = INVDEF);

	virtual void activate();
	virtual void deactivate();
	virtual void notify(int id);

	virtual void fillDock();

	virtual void capMouseDblClk(WPARAM wParam, LPARAM lParam, int button);

private:
	virtual void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseWheel(WPARAM wParam, LPARAM lParam);
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
	
	virtual void capKeyDown(WPARAM wParam, LPARAM lParam);
	virtual void capKeyUp(WPARAM wParam, LPARAM lParam);

	virtual Rect getDrawrect();
	Rect getSrcrect(int margin);

	void updateInterface(WPARAM wParam, LPARAM lParam);

protected:
	void destroy();
	// change the size of the rectangle that stores the drawn area
	void setDrawrectDown(int mode = INFLATE);
	void setDrawrectMove(int mode = INFLATE);
	void setDrawrectUp(int mode = INFLATE);

	virtual POINT getMousePos(LPARAM lParam);
};
/*
ToolDrawingLinear is inherited by line drawing Toools
*/
class ToolDrawingLinear : public ToolDrawing
{
protected:
	GraphicsPath *drawpath;

	POINT startLinear;
	POINT mouseLinear;

public:
	ToolDrawingLinear(FwCHAR *name, HCURSOR cursor, UINT id);
	virtual ~ToolDrawingLinear();

	virtual void capMouseDblClk(WPARAM wParam, LPARAM lParam, int button);

private:
	virtual void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseWheel(WPARAM wParam, LPARAM lParam);
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
	
	virtual void capKeyDown(WPARAM wParam, LPARAM lParam);
	virtual void capKeyUp(WPARAM wParam, LPARAM lParam);

	virtual Rect getDrawrect();

	POINT getMousePos(LPARAM lParam);

protected:
	// prepare for creating a GraphicsPath
	void beginPath();
	// update the path
	void updatePath();
	// apply the path onto the temporary drawing bitmap
	void endPath();
};
/*
ToolDrawingStrict differs in the way it calculates the mouse position
and is used when ToolDrawing and ToolDrawingLinear cannot be used
*/
class ToolDrawingStrict : public ToolDrawing
{
public:
	ToolDrawingStrict(FwCHAR *name, HCURSOR cursor, UINT id);
	virtual ~ToolDrawingStrict();

	virtual void capMouseDblClk(WPARAM wParam, LPARAM lParam, int button);

private:
	virtual void capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseWheel(WPARAM wParam, LPARAM lParam);
	virtual void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	virtual void capMouseMove(WPARAM wParam, LPARAM lParam);
	virtual void capMouseUp(WPARAM wParam, LPARAM lParam, int button);
	
	virtual void capKeyDown(WPARAM wParam, LPARAM lParam);
	virtual void capKeyUp(WPARAM wParam, LPARAM lParam);

	POINT getMousePos(LPARAM lParam);

protected:
	void beginPaint();
	//void endPaint();
};

class ToolPencil : public ToolDrawing
{
private:
	List<PointF> *curve;
	int counter, mode;

public:
	ToolPencil(int mode = PENPEN);
	~ToolPencil();

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	Rect getDrawrect();
};

class ToolBucket : public ToolDrawingStrict
{
private:
	Color colorPick;
	Bitmap *sourceBmp;

	bool isPreselect;

public:
	ToolBucket();
	~ToolBucket();

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	// call Magic Wand functionality
	void callMagic(WPARAM wParam, LPARAM lParam);
	void callColor(Color *color);
};

class ToolLine : public ToolDrawingLinear
{
private:
	POINT start;
	Rect prevRect, drawRect;

	int mode, rrcRound;

public:
	ToolLine(int mode);
	~ToolLine();

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	void setDrawrect();
	Rect getDrawrect();

	void drawLine();
};

class ToolMove : public Tool
{
private:
	POINT mouseLast, frameStart, frameEnd;

	bool mouseMoving, keyMoving;

public:
	ToolMove();
	~ToolMove();

private:
	void capMouseDown(WPARAM wParam, LPARAM lParam, int button);
	void capMouseMove(WPARAM wParam, LPARAM lParam);
	void capMouseUp(WPARAM wParam, LPARAM lParam, int button);

	void capKeyDown(WPARAM wParam, LPARAM lParam);
	void capKeyUp(WPARAM wParam, LPARAM lParam);
};