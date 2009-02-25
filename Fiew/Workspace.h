using namespace Gdiplus;

class ChildCore;
class HistoryControler;
class Frame;

class Workspace
{
private:
	ChildCore *chicore;			// owner
	HistoryControler *history;	// history controler

	bool initialized;			// init flag

	float width,				// document width
		height,					// document height
		res;					// document resolution
	int pxwidth,				// pixel width
		pxheight,				// pixel height
		sunit,					// size unit
		runit,					// resolution unit
		dmode,					// color depth mode
		dunit,					// color depth unit
		bkgnd;					// init background mode

	GraphicsPath *selection;	// current selection

	List<Frame> *layers;		// layers list
	List<Frame> *selectedLayers;// selected layers list

public:
	Workspace(ChildCore *core,
			  float width, float height, int sunit,
			  float res, int runit,
			  int dmode, int dunit, 
			  int bkgnd);
	Workspace(ChildCore *core,
			  Bitmap *source);
	Workspace(ChildCore *core);
	~Workspace();

	void initialize();
	bool save(WCHAR *filename);
	bool load(WCHAR *filename);

	void update();
	void updateToolws();

	Bitmap *render();
	Bitmap *renderAbove(Rect clip);
	Bitmap *renderBelow(Rect clip, bool withGrid = false);

	void resizeCanvas(RECT canvas, bool historical = false, Tool *executor = NULL);
	void resizeImage(int width, int height, InterpolationMode mode = InterpolationModeInvalid, bool historical = false, Tool *executor = NULL);

	void setSelection(GraphicsPath *path);
	GraphicsPath *getSelection();

	HistoryControler *getHistory();

	bool moveSelectedUp(bool historical = false);
	bool moveSelectedDown(bool historical = false);

	void mergeDownSelected();
	void rasterizeSelected();

	bool isMergeReady();
	bool isRasterizeReady();

	int getPxWidth();
	int getPxHeight();

	void addLayerAfter(Frame *newLayer = NULL, Frame *layer = NULL, bool historical = false, Tool *executor = NULL);
	void deleteLayer(Frame *layer, bool historical = false);

	void setPxSize(int width, int height);
	void setPxSize(Rect size);
	void setLayers(List<Frame> *layers);
	List<Frame> *getLayers();
	List<Frame> *getSelectedLayers();
	Frame *getSelectedLayer();
	int getSelectedLayerIndex();

	Bitmap *getSelectedLayerProjection(bool inflate = false);

	Bitmap *getSelectedProjection(Rect *rect, int mode);
	void setSelectedProjection(Bitmap *bmp, int mode);

	void setSelectedLayer(Frame *layer);
	void addSelectedLayer(Frame *layer);

	ChildCore *getChicore();

private:

};