#include "stdafx.h"
#include "Core.h"

ToolText::ToolText() : ToolSelecting(new FwCHAR(TOOL_TEXT),LoadCursor(NULL,IDC_IBEAM),ICC_TEX)
{
	this->hEdit = NULL;
	this->hPane = NULL;

	this->dlgFonts = NULL;
	this->dlgSize = NULL;
	this->dlgAA = NULL;

	this->dlgBold = NULL;
	this->dlgItalic = NULL;
	this->dlgUline = NULL;

	this->dlgColor = NULL;

	this->currentFontSize = 20;
	this->isFontAA = true;
	this->color = CLR_WHITE;

	this->editFont = NULL;
	this->editFrame = NULL;

	this->currentFontName = NULL;
	this->currentFontFamily = NULL;

	this->currentFontStyle = FontStyleRegular;
	this->currentFontUnit = UnitPixel;

	this->fillDock();
}

ToolText::~ToolText()
{
	if( this->editFont != NULL )
		DeleteObject(this->editFont);

	if( this->currentFontName != NULL )
		delete this->currentFontName;
	if( this->currentFontFamily != NULL )
		delete this->currentFontFamily;
}

void ToolText::deactivate()
{
	this->finalize();
}

void ToolText::fillDock()
{
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		DOCKDLG_FONT,
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		0,3,60,20,
		this->hdocktool,
		NULL,this->core->getInstance(),NULL
		);

	this->loadFonts();

	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		DOCKDLG_FONTSIZE,
		WS_CHILD | WS_VISIBLE | SS_SIMPLE,
		220,3,30,20,
		this->hdocktool,
		NULL,this->core->getInstance(),NULL
		);

	this->dlgSize = this->core->CreateWindowExSubstituteFont(
		WS_EX_CLIENTEDGE,
		IDCL_EDIT,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
		250,0,30,20,
		this->hdocktool,
		(HMENU)222,this->core->getInstance(),NULL
		);
	Dialogs::limitInputInt(this->dlgSize,1,MAXFONTSIZE);
	SetDlgItemInt(this->hdocktool,GetDlgCtrlID(this->dlgSize),(UINT)this->currentFontSize,FALSE);

	this->dlgAA = this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		DOCKDLG_AA,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		296,0,105,20,
		this->hdocktool,
		(HMENU)333,this->core->getInstance(),NULL
		);
	if( this->isFontAA == true )
		CheckDlgButton(this->hdocktool,GetDlgCtrlID(this->dlgAA),BST_CHECKED);
	else
		CheckDlgButton(this->hdocktool,GetDlgCtrlID(this->dlgAA),BST_UNCHECKED);

	this->dlgBold = Dialogs::ownerdrawButton(
		this->hdocktool,
		NULL,
		410,
		0,
		20,
		20,
		IDC_TXTBLD,
		TRUE
		);
	this->dlgItalic = Dialogs::ownerdrawButton(
		this->hdocktool,
		NULL,
		430,
		0,
		20,
		20,
		IDC_TXTITA,
		TRUE
		);
	this->dlgUline = Dialogs::ownerdrawButton(
		this->hdocktool,
		NULL,
		450,
		0,
		20,
		20,
		IDC_TXTULN,
		TRUE
		);

	this->dlgColor = Dialogs::colorPicker(
		this->hdocktool,
		this->color,
		480,
		0,
		444
		);
	Toolw::tooltipSubclass(this->dlgColor,this->core->getToolws()->nameColCho);
}

void ToolText::updateDock()
{
	this->chicore = this->core->getActiveChild();
	if( this->chicore != NULL && this->chicore->isConstructed() == true ){
		if( this->chicore->getWorkspace()->getSelectedLayer()->getType() == FRM_TEXT ){
			FrameText *frame = (FrameText *)this->chicore->getWorkspace()->getSelectedLayer();

			FontFamily *family = frame->getFamily();
			StringFormat *format = frame->getFormat();
			INT style = frame->getStyle();
			REAL size = frame->getSize();
			Color col = frame->getColor();
			bool isAA = frame->getIsAA();

			WCHAR buffer[LF_FACESIZE];
			family->GetFamilyName(buffer);
			SendMessage(this->dlgFonts,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)buffer);

			SendMessage(this->dlgBold,BM_SETSTATE,( style & FontStyleBold ),NULL);
			SendMessage(this->dlgItalic,BM_SETSTATE,( style & FontStyleItalic ),NULL);
			SendMessage(this->dlgUline,BM_SETSTATE,( style & FontStyleUnderline ),NULL);

			SetProp(this->dlgColor,ATOM_COLOR,(HANDLE)Core::RGBtoUINT(col));
			InvalidateRect(this->dlgColor,NULL,TRUE);

			CheckDlgButton(this->hdocktool,GetDlgCtrlID(this->dlgAA),(isAA == true) ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemInt(this->hdocktool,GetDlgCtrlID(this->dlgSize),(UINT)size,FALSE);		
		}
	}
}

void ToolText::notify(int id)
{
	if( this->dlgSize != NULL )
		this->currentFontSize = (REAL)GetDlgItemInt(this->hdocktool,GetDlgCtrlID(this->dlgSize),NULL,FALSE);

	this->currentFontStyle = FontStyleRegular;
	if( SendMessage(this->dlgBold,BM_GETSTATE,NULL,NULL) & BST_PUSHED )
		this->currentFontStyle |= FontStyleBold;
	if( SendMessage(this->dlgItalic,BM_GETSTATE,NULL,NULL) & BST_PUSHED )
		this->currentFontStyle |= FontStyleItalic;
	if( SendMessage(this->dlgUline,BM_GETSTATE,NULL,NULL) & BST_PUSHED )
		this->currentFontStyle |= FontStyleUnderline;

	if( this->dlgAA != NULL )
		this->isFontAA = (bool)IsDlgButtonChecked(this->hdocktool,GetDlgCtrlID(this->dlgAA));
	if( this->dlgColor != NULL )
		this->color = Core::UINTtoRGB((UINT)GetProp(this->dlgColor,ATOM_COLOR));

	this->setCurrentFont();

	if( this->hEdit == NULL && id != NULL ){
		this->chicore = this->core->getActiveChild();
		if( this->chicore != NULL ){
			if( this->chicore->getWorkspace()->getSelectedLayer()->getType() == FRM_TEXT ){
				FrameText *frame = (FrameText *)this->chicore->getWorkspace()->getSelectedLayer();
				switch(id){
					case NULL:
						break;
					case 444:
						frame->setColor(this->color);
						break;
					case 333:
						frame->setAA(this->isFontAA);
						break;
					default:
						frame->setText(
							new FontFamily(this->currentFontName->toWCHAR()),
							NULL,
							this->currentFontStyle,
							this->currentFontSize,
							this->color,
							this->isFontAA
							);
						break;
				}
			}
		}
	}
}

int ToolText::getCurrentFontIndex()
{
	return (int)SendMessage(this->dlgFonts,CB_GETCURSEL,NULL,NULL);
}

WCHAR *ToolText::getCurrentFontName()
{
	WCHAR *buffer;

	int cix = this->getCurrentFontIndex();
	int bufsize = (int)SendMessage(this->dlgFonts,CB_GETLBTEXTLEN,(WPARAM)cix,NULL);

	buffer = new WCHAR[bufsize + 1];

	if( SendMessage(this->dlgFonts,CB_GETLBTEXT,(WPARAM)cix,(LPARAM)buffer) != CB_ERR )
		return buffer;

	return NULL;
}

void ToolText::setCurrentFont()
{
	WCHAR *fontFamily = this->getCurrentFontName();
	if( fontFamily != NULL ){
		if( this->currentFontName != NULL )
			delete this->currentFontName;
		this->currentFontName = new FwCHAR(fontFamily);

		if( this->currentFontFamily != NULL )
			delete this->currentFontFamily;
		this->currentFontFamily = new FontFamily(fontFamily);

		delete fontFamily;

		this->setEditFont();
	}
}

void ToolText::setEditFont()
{
	if( this->hEdit != NULL ){			
		if( this->editFont != NULL )
			DeleteObject(this->editFont);

		this->editFont = this->core->CreateHFONT(
			this->currentFontName->toWCHAR(),
			(UINT)( this->currentFontSize * this->core->getActiveChild()->getDrawer()->getZoom() ),
			this->currentFontStyle,
			this->currentFontUnit
			);

		SendMessage(this->hEdit,WM_SETFONT,(WPARAM)this->editFont,TRUE);
	}
}

void ToolText::loadFonts()
{
	int count = 0;
	int found = 0;
	WCHAR buffer[LF_FACESIZE];

	FontFamily *families;
	InstalledFontCollection ifc;

	count = ifc.GetFamilyCount();
	families = new FontFamily[count];

	ifc.GetFamilies(count,families,&found);

	if( this->dlgFonts != NULL )
		DestroyWindow(this->dlgFonts);

	this->dlgFonts = this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_COMBO,
		NULL,
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWN | CBS_SORT,
		60,
		0,
		150,
		300,
		this->hdocktool,
		(HMENU)111,
		this->core->getInstance(),
		NULL
		);

	HWND testWindow = CreateWindowEx(
		NULL,
		IDCL_EDIT,
		NULL,
		WS_CHILD,
		0,0,100,20,
		this->hdocktool,
		NULL,
		this->core->getInstance(),
		NULL
		);

	HFONT getFont, testFont = NULL;
	for( int i = 0; i < found; i++ ){
		families[i].GetFamilyName(buffer);
		testFont = this->core->CreateHFONT(buffer,10);
		SendMessage(testWindow,WM_SETFONT,(WPARAM)testFont,NULL);
		getFont = (HFONT)SendMessage(testWindow,WM_GETFONT,NULL,NULL);
		if( testFont == getFont ){
			SendMessage(this->dlgFonts,CB_ADDSTRING,NULL,(LPARAM)buffer);
		}
		DeleteObject(testFont);
	}
	DestroyWindow(testWindow);

	SendMessage(this->dlgFonts,CB_SETCURSEL,1,NULL);
	this->setCurrentFont();

	delete [] families;
}

void ToolText::editText(FrameText *owner)
{
	if( this->editFrame == NULL ){
		double zoom = owner->getWorkspace()->getChicore()->getDrawer()->getZoom();
		int rx = owner->getWorkspace()->getChicore()->getDrawer()->getRectScene().X;
		int ry = owner->getWorkspace()->getChicore()->getDrawer()->getRectScene().Y;

		int x = (int)(owner->getX() * zoom + rx);
		int y = (int)(owner->getY() * zoom + ry);
		int w = (int)(owner->getRect().Width * zoom);
		int h = (int)(owner->getRect().Height * zoom);

		owner->beginEdit();
		this->editFrame = owner;
		this->createEdit(owner->getText()->toWCHAR(),x,y,w,h);
	}
}

void ToolText::finalize()
{
	WCHAR *string = new WCHAR[MAX_PATH + 1];
	GetWindowText(this->hEdit,string,MAX_PATH);

	if( wcslen(string) > 0 ){
		if( this->editFrame == NULL ){
			FrameText *frame = new FrameText(
				this->chicore->getWorkspace(),
				new FwCHAR(string),
				new FontFamily(this->currentFontName->toWCHAR()),
				NULL,
				this->currentFontStyle,
				this->currentFontSize,
				Point(0,this->getCurrentFontPxAscent()),
				this->color,
				this->isFontAA,
				this->pointWorkspace.x,
				this->pointWorkspace.y
				);
			this->chicore->getWorkspace()->addLayerAfter(
				frame,
				NULL,
				false,
				this->core->getToolws()->getToolwCC()->toolText);
		}
		else {
			this->editFrame->endEdit(new FwCHAR(string));
			this->editFrame = NULL;
		}
	}

	DestroyWindow(this->hPane);
	ReleaseCapture();

	this->hEdit = NULL;
	this->hPane = NULL;

	delete string;
}

Rect ToolText::stringToBox(int fsize, int wsize, int lsize)
{
	wsize = wsize * fsize * 3/4;
	fsize = fsize * 3/2;

	int width = wsize + fsize;
	int height = (lsize + 1) * fsize + fsize;

	return Rect(0,0,width,height);
}

int ToolText::getCurrentFontPxSize()
{
	return ToolText::getFontPxSize(this->currentFontFamily,this->currentFontStyle,this->currentFontSize);
}

int ToolText::getCurrentFontPxAscent()
{
	return ToolText::getFontPxAscent(this->currentFontFamily,this->currentFontStyle,this->currentFontSize);
}

void ToolText::locate()
{
	if( this->hEdit != NULL ){
		RECT rect;
		SendMessage(this->hEdit,EM_GETRECT,NULL,(LPARAM)&rect);

		int fsize = (int)( this->getCurrentFontPxSize() * this->core->getActiveChild()->getDrawer()->getZoom() );
		int wsize = 0;

		int lineidx, linelen, lines = (int)SendMessage(this->hEdit,EM_GETLINECOUNT,NULL,NULL);
		for( int i = 0; i <= lines; i++ ){
			lineidx = (int)SendMessage(this->hEdit,EM_LINEINDEX,(WPARAM)i,NULL);
			linelen = (int)SendMessage(this->hEdit,EM_LINELENGTH,(WPARAM)lineidx,NULL);
			wsize = max(wsize,linelen);
		}
		Rect box = ToolText::stringToBox(fsize,wsize,lines);

		SetWindowPos(
			this->hEdit,
			NULL,
			NULL,
			NULL,
			box.Width,
			box.Height,
			SWP_NOZORDER | SWP_NOMOVE |
			SWP_FRAMECHANGED
			);
		SetWindowPos(
			this->hPane,
			NULL,
			NULL,
			NULL,
			box.Width,
			box.Height,
			SWP_NOZORDER | SWP_NOMOVE |
			SWP_FRAMECHANGED
			);
	}
}

int ToolText::getFontPxAscent(FontFamily *fontFamily, INT fontStyle, REAL fontSize)
{
	int div = fontFamily->GetEmHeight(fontStyle);
	if( div <= 0 )
		return (int)fontSize;

	return (int)(fontSize * fontFamily->GetCellAscent(fontStyle) / fontFamily->GetEmHeight(fontStyle));
}

int ToolText::getFontPxSize(FontFamily *fontFamily, INT fontStyle, REAL fontSize)
{
	int div = fontFamily->GetEmHeight(fontStyle);
	if( div <= 0 )
		div = (int)fontSize;

	int size = 
		( fontFamily->GetCellAscent(fontStyle) +
		  fontFamily->GetCellDescent(fontStyle) ) / div;

	if( size <= 0 )
		size = (int)(2 * fontSize);
	else
		size = (int)(size * fontSize);

	return size;
}

void ToolText::createEdit(WCHAR *string, int x, int y, int w, int h)
{
	this->hPane = CreateWindowEx(
			NULL,
			IDCL_STATIC,
			NULL,
			WS_VISIBLE | WS_CHILD,
			x,
			y,
			w,
			h,
			this->core->getActiveChildHandle(),
			NULL,
			this->core->getInstance(),
			NULL
			);
	SetProp(this->hPane,ATOM_PROC,(WNDPROC)GetWindowLong(this->hPane,GWL_WNDPROC));
	SetProp(this->hPane,ATOM_OWNER,this);

	SetWindowLong(this->hPane,GWL_WNDPROC,(LONG)ToolText::paneProc);
	SendMessage(this->hPane,WM_CREATE,NULL,NULL);

	this->hEdit = CreateWindowEx(
			NULL,
			IDCL_EDIT,
			string,
			WS_VISIBLE | /*WS_BORDER |*/ WS_CHILD | ES_MULTILINE,
			0,
			0,
			w,
			h,
			this->hPane,
			NULL,
			this->core->getInstance(),
			NULL
			);
	SetProp(this->hEdit,ATOM_PROC,(WNDPROC)GetWindowLong(this->hEdit,GWL_WNDPROC));
	SetProp(this->hEdit,ATOM_OWNER,this);

	SetWindowLong(this->hEdit,GWL_WNDPROC,(LONG)ToolText::editProc);
	this->setEditFont();

	SendMessage(this->hEdit,EM_SETMARGINS,(WPARAM)(EC_LEFTMARGIN | EC_RIGHTMARGIN),MAKELPARAM(0,0));
	SendMessage(this->hEdit,WM_CREATE,NULL,NULL);
}

void ToolText::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	POINTS smx = MAKEPOINTS(lParam);

	if( this->hEdit == NULL ){
		this->notify(NULL);

		this->pointWorkspace = this->mouse;

		this->pointScreen.x = smx.x;
		this->pointScreen.y = smx.y;

		int fsize = this->getCurrentFontPxSize();

		this->pointWorkspace.y -= (int)( this->getCurrentFontPxAscent() / this->chicore->getDrawer()->getZoom() );
		this->pointScreen.y -= this->getCurrentFontPxAscent();

		this->createEdit(NULL,this->pointScreen.x,this->pointScreen.y,2 * fsize,2 * fsize);
	}
	else {
		
	}
}

LRESULT CALLBACK ToolText::editProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	ToolText *that = (ToolText *)GetProp(hDlg,ATOM_OWNER);

	if( that == NULL )
		return CallWindowProc(proc,hDlg,message,wParam,lParam);

	RECT client;
	POINTS smx;

	switch(message){
		case WM_CREATE:
			SetCapture(hDlg);
			SendMessage(hDlg,EM_SETSEL,0,-1);
			SetFocus(hDlg);
			break;
		case WM_KEYDOWN:
		case WM_SETFONT:
		case WM_LBUTTONUP:
			that->locate();
			that->core->getActiveChild()->invalidate(false);

			return CallWindowProc(proc,hDlg,message,wParam,lParam);
		case WM_LBUTTONDOWN:
			smx = MAKEPOINTS(lParam);

			GetClientRect(hDlg,&client);
			//MapWindowPoints(NULL,that->chicore->getWindowHandle(),(LPPOINT)&client,2);

			if( smx.x < client.left || smx.x > client.right ||
				smx.y < client.top || smx.y > client.bottom ){

				that->finalize();
			}
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
	return 0;
}

LRESULT CALLBACK ToolText::paneProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	ToolText *that = (ToolText *)GetProp(hDlg,ATOM_OWNER);

	if( that == NULL )
		return CallWindowProc(proc,hDlg,message,wParam,lParam);

	HDC hDc;

	switch(message){
		case WM_CREATE:
			break;
		case WM_CTLCOLOREDIT:			
			hDc = (HDC)wParam;
			
			SetBkMode(hDc,TRANSPARENT);
			SetTextColor(hDc,that->color.ToCOLORREF());

			return (LRESULT)GetStockObject(NULL_BRUSH);
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
	return 0;
}