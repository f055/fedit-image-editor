using namespace Gdiplus;

class Frame
{
protected:
	Workspace *workspace;		// owner
	FwCHAR *name;				// frame display name

	Bitmap *image,				// image data
		*thumb;					// thumbnail data
	Graphics *gfx;				// image graphics

	ImageAttributes *iatt;		// image attributes

	int x,						// Frame x position
		y,						// Frame y position
		opacity;				// transparency

	bool isLocked,				// lock flag
		isVisible,				// visibility flag
		isEmpty;				// empty flag

public:
	Frame(Workspace *wspc, int width, int height);
	Frame(Workspace *wspc, Bitmap *source, int x, int y);
	Frame(Workspace *wspc, Bitmap *source);
	Frame(Workspace *wspc, Image *source);
	Frame(Workspace *wspc, FwCHAR *name);
	virtual ~Frame();

	virtual void applyPaint(Bitmap *srcBmp, Rect trgtRect, Rect srcRect, Tool *executor, bool effect = false, int toolId = FERROR, bool historical = false);
	virtual void applyClear(GraphicsPath *clrPath, Tool *executor, bool historical = false);
	virtual void applyEffect(Bitmap *effect, Rect *trgtRect, Tool *executor, int toolId, bool historical = false);

	virtual void resizeFrame(double wFactor, double hFactor, InterpolationMode mode = InterpolationModeInvalid);

	void addXY(int x, int y, bool force = false);
	void setXY(int x, int y, bool force = false);

	virtual int getX();
	virtual int getY();
	virtual int getWidth();
	virtual int getHeight();

	void setOpacity(int value);
	int getOpacity();

	void setVisibility(bool val);
	void setLock(bool val);

	void toggleLock();
	void toggleEmpty(bool val);
	void toggleVisibility();
	bool getIsLocked();
	bool getIsVisible();
	bool getIsEmpty();

	bool isFrameReady();

	void setName(FwCHAR *name);
	FwCHAR *getName();

	virtual Bitmap *getRender();
	virtual Graphics *getGraphics();
	virtual Bitmap *getThumb(int w, int h);
	virtual void resetThumb();

	ImageAttributes *getIatt();

	virtual int getType();

	virtual bool setStream(IStream *stream, CLSID *encoder);
	virtual IStream *getStream(int &len, CLSID *encoder);

	Workspace *getWorkspace();

	virtual Frame *clone();

private:
	void initialize(FwCHAR *text = NULL);

};

class FrameText : public Frame
{
private:
	FwCHAR *text;						// layer text
	GraphicsPath *textPath;				// layer text as Path

	FontFamily *textFontFamily;			// text font family
	StringFormat *textStringFormat;		// text string format

	INT textFontStyle;					// text style
	REAL textFontSize;					// text size
	Rect textRect;						// text bounding rectangle
	Point textOrigin;					// text origin point

	Color color;						// text color

	Image *icon;						// icon used for thumbnail

	int shiftY;							// vertical position shift
	bool isAA,							// antialiasing flag
		isEdited;						// editing state flag

public:
	struct TextContainer {
		INT style;
		REAL size;
		INT orgx, orgy;
		UINT col;
		bool isAA;
		WCHAR family[LF_FACESIZE];
		int textlen;
		WCHAR *text;
	};

	FrameText(Workspace *wspc,
			  FwCHAR *text,
			  FontFamily *family,
			  StringFormat *format,
			  INT style,
			  REAL size,
			  Point org,
			  Color col,
			  bool aa,
			  int x,
			  int y);
	~FrameText();

	virtual void applyPaint(Bitmap *srcBmp, Rect trgtRect, Rect srcRect, Tool *executor, bool effect = false, int toolId = FERROR, bool historical = false);
	virtual void applyClear(GraphicsPath *clrPath, Tool *executor, bool historical = false);
	virtual void applyEffect(Bitmap *effect, Rect *trgtRect, Tool *executor, int toolId, bool historical = false);

	void resizeFrame(double wFactor, double hFactor, InterpolationMode mode = InterpolationModeInvalid);

	void beginEdit();
	void endEdit(FwCHAR *newText);

	void setText(bool update = true);
	void setText(FontFamily *family,
				 StringFormat *format,
				 INT style,
				 REAL size,
				 Color col,
				 bool isAA,
				 bool update = true);
	void setText(FwCHAR *text,
				 FontFamily *family,
				 StringFormat *format,
				 INT style,
				 REAL size,
				 Point org,
				 Color col,
				 bool update = true);

	void setAA(bool val);
	void setColor(Color color);

	Bitmap *getRender();
	Bitmap *getThumb(int w, int h);

	FwCHAR *getText();

	FontFamily *getFamily();
	StringFormat *getFormat();
	INT getStyle();
	REAL getSize();
	Rect getRect();
	Color getColor();
	bool getIsAA();

	int getType();
	int getY();

	bool setStream(IStream *stream, CLSID *encoder);
	IStream *getStream(int &len, CLSID *encoder);

	Frame *clone();
};