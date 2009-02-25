/*
FrameText.cpp
FrameText object represents text layers and inherits from Frame.
It contains info essential for displaying the text, the text itself,
and the GraphicsPath representing the text.
*/

#include "stdafx.h"
#include "Core.h"

FrameText::FrameText(Workspace *wspc,
					 FwCHAR *text,
					 FontFamily *family,
					 StringFormat *format,
					 INT style,
					 REAL size,
					 Point org,
					 Color col,
					 bool aa,
					 int x,
					 int y) : Frame(wspc,text)
{
	this->x = x;
	this->y = y;

	this->shiftY = 0;

	this->text = NULL;
	this->textPath = NULL;

	this->textFontFamily = NULL;
	this->textFontSize = 20;
	this->textFontStyle = FontStyleRegular;
	this->textStringFormat = NULL;
	this->textRect = Rect(0,0,0,0);
	this->textOrigin = Point(0,0);

	this->isAA = aa;
	this->isEdited = false;

	this->icon = Core::getImageResource(ICC_TEX,RC_PNG);

	this->setText(text,family,format,style,size,org,col,false);
}

FrameText::~FrameText()
{
	if( this->text != NULL )
		delete this->text;
	if( this->textPath != NULL )
		delete this->textPath;

	if( this->textFontFamily != NULL )
		delete this->textFontFamily;
	if( this->textStringFormat != NULL )
		delete this->textStringFormat;

	if( this->icon != NULL )
		delete this->icon;
}

Frame *FrameText::clone()
{
	Frame *clone = new FrameText(
		this->workspace,
		new FwCHAR(this->text->toWCHAR()),
		this->textFontFamily->Clone(),
		(this->textStringFormat != NULL) ? this->textStringFormat->Clone() : NULL,
		this->textFontStyle,
		this->textFontSize,
		this->textOrigin,
		this->color,
		this->isAA,
		this->x,
		this->y
		);
	clone->setName( new FwCHAR(this->name->toWCHAR()) );

	return clone;
}

void FrameText::applyPaint(Bitmap *srcBmp, Rect trgtRect, Rect srcRect, Tool *executor, bool effect, int toolId, bool historical)
{
}
void FrameText::applyClear(Gdiplus::GraphicsPath *clrPath, Tool *executor, bool historical)
{
}
void FrameText::applyEffect(Bitmap *effect, Rect *trgtRect, Tool *executor, int toolId, bool historical)
{
}

void FrameText::resizeFrame(double wFactor, double hFactor, InterpolationMode mode)
{
	this->textOrigin.X = (int)(this->textOrigin.X * wFactor);
	this->textOrigin.Y = (int)(this->textOrigin.Y * hFactor);

	this->textFontSize = (REAL)(this->textFontSize * hFactor);

	this->x = (int)(this->x * wFactor);
	this->y = (int)(this->y * hFactor);

	this->setText(false);
}
/*
Toggle editing mode
*/
void FrameText::beginEdit()
{
	if( this->isLocked == true || this->isVisible == false )
		return;

	this->isEdited = true;
	this->workspace->update();
}

void FrameText::endEdit(FwCHAR *newText)
{
	if( this->isEdited == true ){
		this->isEdited = false;

		if( this->text != NULL )
			delete this->text;
		this->text = newText;

		this->setText();
	}
}

void FrameText::setText(bool update)
{
	this->setText(
		this->text,
		this->textFontFamily,
		this->textStringFormat,
		this->textFontStyle,
		this->textFontSize,
		this->textOrigin,
		this->color,
		update);
}

void FrameText::setText(FontFamily *family,
						StringFormat *format,
						INT style,
						REAL size,
						Color col,
						bool isAA,
						bool update)
{
	this->color = col;
	this->isAA = isAA;

	this->setText(
		this->text,
		family,
		format,
		style,
		size,
		this->textOrigin,
		col,
		update);
}
/*
Set text data
*/
void FrameText::setText(FwCHAR *text,
						FontFamily *family,
						StringFormat *format,
						INT style,
						REAL size,
						Point org,
						Color col,
						bool update)
{
	bool updateHistory = false;

	// save current state
	HEToolManageText::TextState stateBefore;
	if( update == true ){
		stateBefore = HEToolManageText::set(
			(text != NULL) ? new FwCHAR(text->toWCHAR()) : NULL,
			(family != NULL) ? family->Clone() : NULL,
			(format != NULL) ? format->Clone() : NULL,
			style,size,org,col,this->isAA);
	}

	// change text if neccessary
	if( this->text != NULL && this->text != text ){
		delete this->text;
		updateHistory = true;
	}
	this->text = text;

	// change font family if necessary
	if( this->textFontFamily != NULL && this->textFontFamily != family ){
		WCHAR ofm[LF_FACESIZE], nfm[LF_FACESIZE];
		this->textFontFamily->GetFamilyName(ofm);
		family->GetFamilyName(nfm);

		FwCHAR *tester = new FwCHAR(ofm);
		FwCHAR *cmperer = new FwCHAR(nfm);
		if( tester->equals(cmperer) == false )
			updateHistory = true;
		delete tester;
		delete cmperer;

		delete this->textFontFamily;
	}
	this->textFontFamily = family;

	if( this->textFontFamily == NULL )
		this->textFontFamily = new FontFamily(FONT);

	// change string format if neccessary
	if( this->textStringFormat != NULL && this->textStringFormat != format){
		delete this->textStringFormat;
		updateHistory = true;
	}
	this->textStringFormat = format;

	// change size and style
	if( this->textFontStyle != style ){
		this->textFontStyle = style;
		updateHistory = true;
	}
	if( this->textFontSize != size ){
		this->textFontSize = size;
		updateHistory = true;
	}

	// caclulate font bounding rectangle and origin
	double zoom = ZOOMINIT;
	if( this->workspace->getChicore()->getDrawer() != NULL )
		zoom = this->workspace->getChicore()->getDrawer()->getZoom();

	int fpxsize = ToolText::getFontPxSize(this->textFontFamily,this->textFontStyle,this->textFontSize);
	int fsize = (int)(fpxsize * zoom);
	int wsize = this->text->toMaxLineLength();
	int lsize = this->text->toLines();

	this->textRect = ToolText::stringToBox(fsize,wsize,lsize);
	if( this->textOrigin.Equals(org) == false ){
		this->textOrigin = org;
		updateHistory = true;
	}

	// change color
	if( this->color.ToCOLORREF() != col.ToCOLORREF() ){
		this->color = col;
		updateHistory = true;
	}

	// create GraphicsPath representing the string
	if( this->textPath != NULL )
		delete this->textPath;
	this->textPath = new GraphicsPath();

	this->textPath->AddString(
		this->text->toWCHAR(),
		-1,
		this->textFontFamily,
		this->textFontStyle,
		this->textFontSize,
		Point(0,0),
		this->textStringFormat
		);

	Rect bounds;
	this->textPath->GetBounds(&bounds);

	int xdif = this->textRect.X - bounds.X;

	int ascent = ToolText::getFontPxAscent(this->textFontFamily,this->textFontStyle,this->textFontSize);
	int ydif = this->shiftY = org.Y - ascent;

	Matrix matrix;
	matrix.Translate((REAL)xdif,0);
	this->textPath->Transform(&matrix);

	// fill history if neccessary and update the interface and view
	if( update == true ){
		if( updateHistory == true ){
			HEToolManageText::TextState stateAfter = HEToolManageText::set(
				(this->text != NULL) ? new FwCHAR(this->text->toWCHAR()) : NULL,
				(this->textFontFamily != NULL) ? this->textFontFamily->Clone() : NULL,
				(this->textStringFormat != NULL) ? this->textStringFormat->Clone() : NULL,
				this->textFontStyle,
				this->textFontSize,
				this->textOrigin,
				this->color,
				this->isAA);

			HEToolManageText *he = new HEToolManageText(
				this->workspace,this,Core::self->getToolws()->getToolwCC()->toolText);

			he->setBefore(stateBefore);
			he->setAfter(stateAfter);

			this->workspace->getHistory()->add(he);
		}
		else {
			HEToolManageText::freeTextState(stateBefore);
		}

		this->workspace->updateToolws();
		this->workspace->update();
	}
}
/*
Render the text as bitmap
*/
Bitmap *FrameText::getRender()
{
	if( this->isVisible == true && this->isEdited == false ){
		if( this->image != NULL )
			delete this->image;
		this->image = new Bitmap(this->textRect.Width,this->textRect.Height,Core::getPixelFormat());

		if( this->gfx != NULL )
			delete this->gfx;
		this->gfx = Graphics::FromImage(this->image);
		if( this->isAA == true )
			this->gfx->SetSmoothingMode(SmoothingModeHighQuality);
		else
			this->gfx->SetSmoothingMode(SmoothingModeHighSpeed);

		SolidBrush *brush = new SolidBrush(this->color);
		this->gfx->FillPath(brush,this->textPath);

		//Font *font = new Font(this->textFontFamily,this->textFontSize,this->textFontStyle,UnitPixel);
		//this->gfx->DrawString(this->text->toWCHAR(),this->text->toLength(),font,PointF(0,0),NULL,brush);
		//delete font;

		delete brush;
		return this->image;
	}
	return NULL;
}

Bitmap *FrameText::getThumb(int w, int h)
{
	Bitmap *thumb = new Bitmap(w,h,this->image->GetPixelFormat());
	Graphics *gfx = Graphics::FromImage(thumb);
	gfx->Clear(CLR_WHITE);

	int iw = this->icon->GetWidth();
	int ih = this->icon->GetHeight();

	int ix = (w - iw) / 2;
	int iy = (h - ih) / 2;

	gfx->DrawImage(this->icon,ix,iy,iw,ih);

	delete gfx;
	return thumb;
}

int FrameText::getType()
{
	return FRM_TEXT;
}

int FrameText::getY()
{
	return this->y + this->shiftY;
}

void FrameText::setColor(Color color)
{
	this->color = color;

	this->workspace->update();
}

void FrameText::setAA(bool val)
{
	this->isAA = val;

	this->workspace->update();
}

FontFamily *FrameText::getFamily()
{
	return this->textFontFamily;
}

StringFormat *FrameText::getFormat()
{
	return this->textStringFormat;
}

INT FrameText::getStyle()
{
	return this->textFontStyle;
}

REAL FrameText::getSize()
{
	return this->textFontSize;
}

Rect FrameText::getRect()
{
	return this->textRect;
}

Color FrameText::getColor()
{
	return this->color;
}

bool FrameText::getIsAA()
{
	return this->isAA;
}
FwCHAR *FrameText::getText()
{
	return this->text;
}

bool FrameText::setStream(IStream *stream, CLSID *encoder)
{
	FrameText::TextContainer tc;

	tc.col = Core::RGBtoUINT(this->color);
	this->textFontFamily->GetFamilyName(tc.family);
	tc.isAA = this->isAA;
	tc.orgx = this->textOrigin.X;
	tc.orgy = this->textOrigin.Y;
	tc.size = this->textFontSize;
	tc.style = this->textFontStyle;
	tc.textlen = this->text->toLength() + 1;
	tc.text = NULL;

	ULONG read = 0;
	if( stream->Write(&tc,sizeof(FrameText::TextContainer),&read) == S_OK )
		if( stream->Write(this->text->toWCHAR(),tc.textlen * sizeof(WCHAR),&read) == S_OK )
			return true;

	return false;
}

IStream *FrameText::getStream(int &len, CLSID *encoder)
{
	return NULL;
}