using namespace Gdiplus;

class Workspace;
class Frame;
class FrameText;
class Tool;

class HistoryElement
{
protected:
	Workspace *wspc;
	Frame *frame;
	Tool *tool;

	FwCHAR *name;
	bool canUndo, canRedo;

public:
	HistoryElement(Workspace *wspc, Frame *frame, Tool *tool);
	virtual ~HistoryElement();

	virtual bool undo();
	virtual bool redo();

	virtual void undoAct();
	virtual void redoAct();

	void boolUndo();
	void boolRedo();

	bool canUndone();
	bool canRedone();

	virtual FwCHAR *getName();
	virtual WORD getId();

	Workspace *getWorkspace();
	Frame *getFrame();
	Tool *getTool();

};

class HistoryBlankElement : public HistoryElement
{
public:
	HistoryBlankElement();
	~HistoryBlankElement();

	bool undo();
	bool redo();

	FwCHAR *getName();
	WORD getId();
};

class HEToolMove : public HistoryElement
{
public:
	int dx, dy;

	HEToolMove(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolMove();

	void set(int dx, int dy);

	void undoAct();
	void redoAct();
};

class HEToolDrawing : public HistoryElement
{
public:
	Bitmap *before, *after;
	Rect target;
	int toolId;

	HEToolDrawing(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolDrawing();

	void set(Bitmap *before, Bitmap *after, Rect target);
	void setToolId(int id);

	FwCHAR *getName();

	void undoAct();
	void redoAct();

private:
	void drawImage(Bitmap *image);
};

class HEToolSelecting : public HistoryElement
{
private:
	FwCHAR *deseleeName;

public:
	GraphicsPath *seleeBefore, *seleeAfter;

	HEToolSelecting(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolSelecting();

	void set(GraphicsPath *pathBefore, GraphicsPath *pathAfter);
	void setUpdate(GraphicsPath *pathAfter);

	FwCHAR *getName();

	void undoAct();
	void redoAct();
};

class HEToolCrop : public HistoryElement
{
public:
	RECT canvasChange;

	HEToolCrop(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolCrop();

	void set(RECT canvas);

	void undoAct();
	void redoAct();
};

class HEToolResize : public HistoryElement
{
public:
	List<Frame> *layersBefore, *layersAfter;
	Rect sizeBefore, sizeAfter;

	HEToolResize(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolResize();

	void set(List<Frame> *layersBefore, List<Frame> *layersAfter, Rect sizeBefore, Rect sizeAfter);

	FwCHAR *getName();

	void undoAct();
	void redoAct();
};

class HEToolRaster : public HistoryElement
{
public:
	Frame *beforeRaster, *afterRaster;

	HEToolRaster(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolRaster();

	void set(Frame *beforeRaster, Frame *afterRaster);

	void undoAct();
	void redoAct();
};

class HEToolManageFrame : public HistoryElement
{
private:
	int mode;
	FwCHAR *nameAdd, *nameRemove, *nameText;

public:
	Frame *afterLayer;

	HEToolManageFrame(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolManageFrame();

	void set(Frame *afterLayer, int mode);

	FwCHAR *getName();

	void undoAct();
	void redoAct();
};

class HEToolManageFrameOrder : public HistoryElement
{
public:
	int move;

	HEToolManageFrameOrder(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolManageFrameOrder();

	void set(int movement);

	FwCHAR *getName();

	void undoAct();
	void redoAct();
};

class HEToolManageText : public HistoryElement
{
private:
	FrameText *frameText;

public:
	struct TextState {
		FwCHAR *text;

		FontFamily *textFontFamily;
		StringFormat *textStringFormat;

		INT textFontStyle;
		REAL textFontSize;

		Point org;
		Color color;
		bool isAA;
	} textBefore, textAfter;

	HEToolManageText(Workspace *wspc, Frame *frame, Tool *tool);
	~HEToolManageText();

	static TextState set(
		FwCHAR *text,
		FontFamily *textFontFamily,
		StringFormat *textStringFormat,
		INT textFontStyle,
		REAL textFontSize,
		Point org,
		Color color,
		bool isAA
		);
	void setBefore(TextState before);
	void setAfter(TextState after);

	void setText(TextState state);
	static void freeTextState(TextState state);

	void undoAct();
	void redoAct();
};

/* History Controler */
class HistoryControler
{
private:
	Workspace *wspc;
	List<HistoryElement> *historyElems;
	
public:
	HistoryControler(Workspace *wspc);
	~HistoryControler();

	void add(HistoryElement *element);
	void remove();

	void gotoElement(HistoryElement *element);

	bool undo(HistoryElement *element);
	bool redo(HistoryElement *element);

	bool undo(bool forceUpdate = true);
	bool redo(bool forceUpdate = true);

	List<HistoryElement> *getHistoryElems();
};