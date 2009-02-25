/*
Core.cpp
The core of the application that manages all other components,
connects them and distributes messages to and among them.
It also contains global usage variables and methods
*/

#include "stdafx.h"
#include "Core.h"

/*
RAR Dll handle for reading rar archives
*/
HMODULE Core::rarDll = NULL;
/*
MDI Child windows counter for naming and positioning purposes
*/
int Core::chicounter = NULL;

/*
Global pointer to self
*/
Core *Core::self = NULL;
/*
Backup clipboard bitmap data in case standard system clipboard is unavailable
*/
Bitmap *Core::clipboardBitmap = NULL;

Core::Core(WCHAR *cmdLine, HINSTANCE instance)
{
	this->instance = instance;

	this->windowHandle = NULL;
	this->mdiclientHandle = NULL;

	this->commandLine = NULL;
	this->loadDlls();

	this->MDIWndProc = NULL;
	this->children = NULL;

	this->dialogs = NULL;
	this->toolws = NULL;

	this->explorer = NULL;
	this->cacher = NULL;
	this->drawer = NULL;
	this->gui = NULL;

	if( cmdLine != NULL ){
		this->commandLine = new FwCHAR(cmdLine);
		if( this->commandLine->toLength() < 3 )
			this->commandLine = NULL;
	}
	this->hFontShell = NULL;

	this->initialized = false;

	Core::self = this;
}

Core::~Core()
{
	this->destroy();
}

void Core::destroy()
{
	KillTimer(this->windowHandle,TIMER_MBB);
	KillTimer(this->windowHandle,TIMER_MCH);
	KillTimer(this->windowHandle,TIMER_MMM);
	KillTimer(this->windowHandle,TIMER_OVL);
	KillTimer(this->windowHandle,TIMER_THB);

	if( this->drawer != NULL )
		delete this->drawer;
	this->drawer = NULL;

	if( this->cacher != NULL )
		delete this->cacher;
	this->cacher = NULL;

	if( this->explorer != NULL )
		delete this->explorer;
	this->explorer = NULL;

	if( this->gui != NULL )
		delete this->gui;
	this->gui = NULL;

	if( this->hFontShell != NULL )
		DeleteObject(this->hFontShell);
	this->hFontShell = NULL;

	this->commandLine = NULL;

	this->freeDlls();
}

HMODULE Core::getRarDll()
{
	return Core::rarDll;
}

void Core::getLastError()
{
	TCHAR szBuf[2 * MAX_PATH]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf,L"Failed with error %d: %s",dw,lpMsgBuf);

	return;
}
/*
Core initializer executed at main window WM_CREATE message.
*/
void Core::initialize(HWND windowHandle)
{
	this->windowHandle = windowHandle;

	INITCOMMONCONTROLSEX iccex;

	iccex.dwICC = ICC_WIN95_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	if( !InitCommonControlsEx(&iccex) )
		this->messageBox_Error(L"Init of ICC_WIN95_CLASSES Common Controls Failed.");

	iccex.dwICC = ICC_BAR_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	if( !InitCommonControlsEx(&iccex) )
		this->messageBox_Error(L"Init of ICC_BAR_CLASSES Common Controls Failed.");

	iccex.dwICC = ICC_TAB_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	if( !InitCommonControlsEx(&iccex) )
		this->messageBox_Error(L"Init of ICC_TAB_CLASSES Common Controls Failed.");

	this->initializeFonts();

	this->dialogs = new Dialogs(this);
	this->toolws = new Toolws(this);
	this->gui = new Interface(this);

	this->initializeMdi();
	this->initializeToolws();
	this->gui->updateMenu();
	this->initialized = true;

	this->open(this->commandLine);
}
/*
Initializer for fonts used in the user interface
*/
void Core::initializeFonts()
{
	this->hFontShell = this->CreateHFONT(FONT_STATIC,FONTSIZE_STATIC);
}

void Core::initializeMdi()
{
	this->children = new List<ChildCore>();

	CLIENTCREATESTRUCT ccs; 
  
	ccs.hWindowMenu = GetSubMenu(GetMenu(this->windowHandle), 6); 
	ccs.idFirstChild = ID_FIRSTCHILD;

	int yshift = this->getToolws()->getToolwDock()->getHeight();
 
	this->mdiclientHandle = CreateWindowEx(NULL/*WS_EX_CLIENTEDGE*/,MDICLIENT,(LPCTSTR)NULL, 
		WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE, 
		0,0,0,0,this->windowHandle,(HMENU)IDR_MENU,this->instance,(LPSTR)&ccs);

	this->MDIWndProc = (WNDPROC)GetWindowLong(this->mdiclientHandle,GWL_WNDPROC);
	SetWindowLong(this->mdiclientHandle,GWL_WNDPROC,(LONG)MdiProc);
}
/*
Initializer for Tool Windows, like Control Center, History, Layers etc.
*/
void Core::initializeToolws()
{
	this->getToolws()->getToolwDock()->toggleDock();

	/*
	Set the initial tool - Hand
	*/
	// dont send such messages to buttons with popup lists
	SendMessage(
		GetDlgItem(this->getToolws()->getToolwCC()->getWindowHandle(),ICC_HAN),
		WM_LBUTTONDOWN,
		NULL,
		NULL
		);
}
/*
Adds a newly created child window to the list of child windows
and initialize the child window
*/
bool Core::neww(ChildCore *child)
{
	this->children->add(child);
	return child->initialize();
}
/*
Show thumbnail dialog for browsing folders and archives contents
*/
bool Core::openFolder(FwCHAR *path)
{
	bool result = false;

	if( this->dialogs->showDialog(
				(LPCTSTR)IDD_IMG,
				(DLGPROC)Core::processFakeDialogs,
				(LPARAM)path) == YES ){
		return true;
	}
	return false;
}
/*
Open a file from path
*/
bool Core::open(FwCHAR *path, bool save)
{
	bool result = false;

	if( path == NULL )
		return result;
	if( path->toWCHAR() == NULL )
		return result;
	path->stripBraces();

	ChildCore *child = NULL;
	this->gui->setCursor(CURSOR_WAIT);

	int type = Explorer::getType(path->toWCHAR());
	int mime = Explorer::getMime(type);

	switch(mime){
		case MIME_IMAGE:
			child = new ChildCore(this,path,type);

			if( child != NULL ){
				if( child->isConstructed() == true ){
					this->children->add(child);
					result = child->initialize();
				}
				else {
					delete child;
					result = false;
				}
			}
			if( result == true && save == true )
				this->gui->addLastItem(path->toWCHAR());

			break;
		case MIME_ARCHIVE:
			result = this->openFolder(path);

			if( result == true && save == true )
				this->gui->addLastItem(path->toWCHAR());
			break;
		case MIME_DIR:
			result = this->openFolder(path);
			break;
	}
	this->gui->setCursor();

	if( result == false )
		this->messageBox_Error(MESSAGE_CANNOTOPEN);

	this->gui->updateMenu();

	return result;
}
bool Core::open(WCHAR *path, bool save)
{
	if( path == NULL )
		return false;

	FwCHAR *tmp = new FwCHAR(path);
	return this->open(tmp,save);
}
/*
Extraction of currently selected archived file.
Inherited from Fiew 2.0, for future use.
*/
void Core::extract()
{
	bool result = false;

	this->gui->setMessage(MESSAGE_EXTRACT);
	this->gui->setCursor(CURSOR_WAIT);
	result = this->explorer->extract();
	this->gui->setCursor(CURSOR_CLIENT);
	this->gui->updateText();

	if( result == false )
		this->messageBox_Error(MESSAGE_EXTRACTERROR);
	else
		this->messageBox_Info(MESSAGE_EXTRACTEDIMAGE);
}
/*
Setting the current image as wallpaper.
Inherited from Fiew 2.0, for future use.
*/
void Core::setwall()
{
	bool result = false;

	this->gui->setMessage(MESSAGE_PROCESS);
	this->gui->setCursor(CURSOR_WAIT);
	result = this->drawer->getScene()->setWall();
	this->gui->setCursor(CURSOR_CLIENT);
	this->gui->updateText();

	if( result == false )
		this->messageBox_Error(MESSAGE_SETWALLERROR);
	else
		this->messageBox_Info(MESSAGE_SETWALLEDIMAGE);
}

void Core::close()
{
	if( this->getActiveChild() != NULL ){
		SendMessage(
			this->getActiveChild()->getWindowHandle(),
			WM_CLOSE,
			NULL,
			NULL
			);
	}
}
/*
Extraction of embedded resources. Used for extraction unrar.dll.
*/
bool Core::extractResource(WORD id, WCHAR *filename)
{
	bool result = false;

	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(id), RC_DLL);
	if( hResource != NULL ){
		HGLOBAL hLoader = LoadResource(NULL,hResource);
		if( hLoader != NULL ){
			byte *data = (byte *)LockResource(hLoader);
			DWORD len = SizeofResource(NULL,hResource);

			if( data != NULL && len > 0 ){
				HANDLE hFile = CreateFile(filename,
										GENERIC_READ | GENERIC_WRITE,
										0,
										NULL,
										CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL);
				if( hFile != INVALID_HANDLE_VALUE ){
					DWORD written = 0;

					int write = WriteFile(hFile,
							data,
							len,
							&written,
							NULL);
					if( len == written && write == TRUE )
						result = true;
				}
				CloseHandle(hFile);
			}
			UnlockResource(hLoader);
		}
		FreeResource(hLoader);
	}
	return result;
}

void Core::loadDlls()
{
	Core::rarDll = LoadLibrary(RARDLL);
	if( Core::rarDll == NULL )
		if( this->extractResource(IDR_DLLRAR,RARDLL) )
			Core::rarDll = LoadLibrary(RARDLL);
}

void Core::freeDlls()
{
	if( Core::rarDll != NULL )
		FreeLibrary( Core::rarDll );
	Core::rarDll = NULL;
}
/*
Retrive CLSID image encoders basing on a string name like "image/jpg"
*/
int Core::getEncoder(WCHAR* format, CLSID* pClsid)
{
	UINT num = 0; 
	UINT size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}    
	}
	free(pImageCodecInfo);
	return FERROR;
}

HWND Core::getMdiclientHandle()
{
	return this->mdiclientHandle;
}
HWND Core::getWindowHandle()
{
	return this->windowHandle;
}
HWND Core::getActiveChildHandle()
{
	return (HWND)SendMessage(this->mdiclientHandle,WM_MDIGETACTIVE,0,0);
}

ChildCore *Core::getActiveChild()
{
	HWND hChild = this->getActiveChildHandle();
	if( hChild == NULL || this->children->getCount() <= 0 )
		return NULL;

	this->children->gotoHead();
	do {
		if( this->children->getThat()->getWindowHandle() == hChild )
			return this->children->getThat();

	} while( this->children->next() == true );

	return NULL;
}

HINSTANCE Core::getInstance()
{
	return this->instance;
}

List<ChildCore> *Core::getChildren()
{
	return this->children;
}

Dialogs *Core::getDialogs()
{
	return this->dialogs;
}

Toolws *Core::getToolws()
{
	return this->toolws;
}

Interface *Core::getGui()
{
	return this->gui;
}

Explorer *Core::getExplorer()
{
	return this->explorer;
}
void Core::setExplorer(Explorer *explorer)
{
	this->explorer = explorer;
}

Cacher *Core::getCacher()
{
	return this->cacher;
}

Drawer *Core::getDrawer()
{
	return this->drawer;
}

RECT Core::getMdiClientSize()
{
	RECT client;
	GetClientRect(this->mdiclientHandle,&client);

	return client;
}

RECT Core::getClientSize()
{
	RECT client;
	GetClientRect(this->windowHandle,&client);

	return client;
}

void Core::mdiArrange()
{
	SendMessage(this->mdiclientHandle,WM_MDIICONARRANGE,NULL,NULL);
}
void Core::mdiCascade()
{
	SendMessage(this->mdiclientHandle,WM_MDICASCADE,MDITILE_ZORDER,NULL);
}
void Core::mdiTileHor()
{
	SendMessage(this->mdiclientHandle,WM_MDITILE,MDITILE_HORIZONTAL,NULL);
}
void Core::mdiTileVer()
{
	SendMessage(this->mdiclientHandle,WM_MDITILE,MDITILE_VERTICAL,NULL);
}
/*
Force redraw of all Tool Windows
*/
void Core::redrawAll(RECT *rect)
{
	RedrawWindow(
		this->windowHandle,
		rect,
		NULL,
		RDW_INVALIDATE | 
		RDW_UPDATENOW | 
		RDW_ALLCHILDREN );

	/*
	this->toolws->getToolwset()->gotoHead();
	while( this->toolws->getToolwset()->next() == true ){
		RedrawWindow(
			this->toolws->getToolwset()->getThat(),
			NULL,NULL,
			RDW_INVALIDATE |
			RDW_UPDATENOW |
			RDW_ALLCHILDREN );
	}
	*/
}
/*
Customized message box shortcuts
*/
int Core::messageBox_Info(WCHAR *string)
{
	return MessageBox(
		Core::self->getWindowHandle(),
		string,
		MESSAGE_INFO,
		MB_OK | MB_ICONINFORMATION
		);
}
int Core::messageBox_Error(WCHAR *string)
{
	return MessageBox(
		Core::self->getWindowHandle(),
		string,
		MESSAGE_ERROR,
		MB_OK | MB_ICONERROR
		);
}
int Core::messageBox_Prompt(WCHAR *string)
{
	return MessageBox(
		Core::self->getWindowHandle(),
		string,
		MESSAGE_WARNING,
		MB_YESNOCANCEL | MB_ICONQUESTION
		);
}

int Core::tickChicounter()
{
	Core::chicounter = (Core::chicounter + 1) % CHICTRLIMIT;

	return Core::chicounter;
}
int Core::getChicounter()
{
	return Core::chicounter;
}
/*
Retrive the current pixel format of the display.
Used to ensure the compatiblity of every Gdiplus::Bitmap object
and current display for fast drawing.
*/
int Core::getPixelFormat()
{
	int pixelFormat, size = 1;
	Bitmap *temp = new Bitmap(size,size);
	Image *test = NULL;

	test = temp->GetThumbnailImage(size,size);
	pixelFormat = test->GetPixelFormat();

	delete test;
	delete temp;

	return pixelFormat;
}
/*
Retrive window text to custom types
*/
FwCHAR *Core::getDlgItemString(HWND hDlg, int itemId)
{
	FwCHAR *result = NULL;

	int len = GetWindowTextLength(GetDlgItem(hDlg, itemId));
	if( len > 0 ){
		WCHAR *buf = new WCHAR[len + 1];

		if( GetDlgItemText(hDlg,itemId,buf,len + 1) != NULL )
			result = new FwCHAR(buf);

		delete buf;
	}
	return result;
}
float Core::getDlgItemNumber(HWND hDlg, int itemId, bool sign)
{
	BOOL result = NULL;
	float number = (float)GetDlgItemInt(hDlg,itemId,&result,(BOOL)sign);

	return number;
}
/*
Retrive embedded images from resources
*/
Image *Core::getImageResource(WORD id, LPCWSTR type)
{
	Image *img = NULL;

	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(id), type);
	if( hResource != NULL ){
		HGLOBAL hLoader = LoadResource(NULL,hResource);
		if( hLoader != NULL ){
			byte *data = (byte *)LockResource(hLoader);
			DWORD len = SizeofResource(NULL,hResource);

			if( data != NULL && len > 0 ){
				IStream *stream = NULL;
				HGLOBAL buffer = NULL;

				if( (buffer = GlobalAlloc(GPTR,len)) != NULL )
					if( CreateStreamOnHGlobal(buffer,true,(LPSTREAM*)&stream) == S_OK )
						if( stream->Write((void *)data,len,NULL) == S_OK )
							img = Image::FromStream(stream);
				if( stream != NULL )
					stream->Release();
			}
			UnlockResource(hLoader);
		}
		FreeResource(hLoader);
	}
	return img;
}

bool Core::isInBmp(Bitmap *bmp, int x, int y)
{
	if( x >= 0 && x < (int)bmp->GetWidth() &&
		y >= 0 && y < (int)bmp->GetHeight() )
		return true;
	return false;
}
/*
Color inversion with mid value correction to prevent from
obtaining similar colors while inverting mid ranged colors.
*/
Color Core::invClr(Color color, int grey)
{
	int shift = 64;
	int mid = 128;

	int R = 255 - color.GetR();
	int G = 255 - color.GetG();
	int B = 255 - color.GetB();

	if( R > mid - shift && R < mid ) R -= shift;
	if( R < mid + shift && R >= mid ) R += shift;
	if( G > mid - shift && G < mid ) G -= shift;
	if( G < mid + shift && G >= mid ) G += shift;
	if( B > mid - shift && B < mid ) B -= shift;
	if( B < mid + shift && B >= mid ) B += shift;

	return Color(255,R + grey,G + grey,B + grey);
}
/*
Color conversion methods
*/
UINT Core::RGBtoUINT(int r, int g, int b, int a)
{
	return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

UINT Core::RGBtoUINT(Color rgb)
{
	return Core::RGBtoUINT(rgb.GetR(),rgb.GetG(),rgb.GetB());
}

Color Core::UINTtoRGB(UINT uint)
{
	int r,g,b,a;

	a = (uint >> 24) & 0xff;
	r = (uint >> 16) & 0xff;
	g = (uint >> 8 ) & 0xff;
	b = (uint & 0xff);

	return Color(a,r,g,b);
}

RGBCOLOR Core::UINTtoRGBCOLOR(UINT uint)
{
	RGBCOLOR rgba;

	rgba.A = (uint >> 24) & 0xff;
	rgba.R = (uint >> 16) & 0xff;
	rgba.G = (uint >> 8 ) & 0xff;
	rgba.B = (uint & 0xff);

	return rgba;
}
UINT Core::RGBCOLORtoUINT(RGBCOLOR rgba)
{
	return Core::RGBtoUINT(rgba.R,rgba.G,rgba.B,rgba.A);
}

CMYKCOLOR Core::RGBtoCMYK(Color rgb)
{
	CMYKCOLOR cmyk;
	double r, g, b, c, m, y, k, mi;

	r = rgb.GetR() / 255.0;
	g = rgb.GetG() / 255.0;
	b = rgb.GetB() / 255.0;

	c = 1 - r;
	m = 1 - g;
	y = 1 - b;

	mi = c;
	mi = min(mi,m);
	mi = min(mi,y);

	if( mi == 1 ){
		cmyk.C = 0;
		cmyk.M = 0;
		cmyk.Y = 0;
		cmyk.K = 100;
	}
	else {
		k = mi;
		cmyk.C = (int)( 100 * (c - k)/(1 - k) );
		cmyk.M = (int)( 100 * (m - k)/(1 - k) );
		cmyk.Y = (int)( 100 * (y - k)/(1 - k) );
		cmyk.K =( int)( 100 * k );
	}
	return cmyk;
}

Color Core::CMYKtoRGB(CMYKCOLOR cmyk)
{
	double r, g, b, c, m, y, k;

	c = cmyk.C / 100.0;
	m = cmyk.M / 100.0;
	y = cmyk.Y / 100.0;
	k = cmyk.K / 100.0;

	r = (1 - c) * (1 - k);
	g = (1 - m) * (1 - k);
	b = (1 - y) * (1 - k);

	r *= 255;
	g *= 255;
	b *= 255;

	return Color((BYTE)r,(BYTE)g,(BYTE)b);
}

HSVCOLOR Core::RGBtoHSV(Color rgb)
{
	double r, g, b, h, s, v, mi, delta;

	r = rgb.GetR();
	g = rgb.GetG();
	b = rgb.GetB();

	if( b > r && b > g ){
		v = b;
		if( v != 0 ){
			if( r > g )
				mi = g;
			else
				mi = r;
			delta = v - mi;
			if( delta != 0 ){
				s = delta / v;
				h = 4 + (r - g)/delta;
			}
			else {
				s = 1.0 / v;
				h = 4 + (r - g);
			}
			h *= 60;
			if( h < 0 )
				h += 360;
		}
		else {
			h = 0;
			s = 0;
		}
	}
	else if( g > r ){
		v = g;
		if( v != 0 ){
			if( r > b )
				mi = b;
			else
				mi = r;
			delta = v - mi;
			if( delta != 0 ){
				s = delta / v;
				h = 2 + (b - r)/delta;
			}
			else {
				s = 1.0 / v;
				h = 2 + (b - r);
			}
			h *= 60;
			if( h < 0 )
				h += 360;
		}
		else {
			h = 0;
			s = 0;
		}
	}
	else {
		v = r;
		if( v != 0 ){
			if( g > b )
				mi = b;
			else
				mi = g;
			delta = v - mi;
			if( delta != 0 ){
				s = delta / v;
				h = (g - b)/delta;
			}
			else {
				s = 1.0 / v;
				h = g - b;
			}
			h *= 60;
			if( h < 0 )
				h += 360;
		}
		else {
			h = 0;
			s = 0;
		}
	}

	HSVCOLOR color;
	color.H = (WORD)h;
	color.S = (WORD)(s * 100);
	color.V = (WORD)(v * 100 / 255);

	return color;
}

Color Core::HSVtoRGB(HSVCOLOR hsv)
{
	int i;
	double r, g, b, h, s, v, f, hf, pv, qv, tv;

	h = hsv.H;
	s = hsv.S / 100.0;
	v = hsv.V / 100.0;

	if( v == 0 ){
		r = 0;
		g = 0;
		b = 0;
	}
	else if( s == 0 ){
		r = v;
		g = v;
		b = v;
	}
	else {
		hf = h / 60;
		i = (int)floor(hf);
		f = hf - i;
		pv = v * (1 - s);
		qv = v * (1 - s * f);
		tv = v * (1 - s * (1 - f));

		switch(i){
			case 0:
				r = v;
				g = tv;
				b = pv;
				break;
			case 1:
				r = qv;
				g = v;
				b = pv;
				break;
			case 2:
				r = pv;
				g = v;
				b = tv;
				break;
			case 3:
				r = pv;
				g = qv;
				b = v;
				break;
			case 4:
				r = tv;
				g = pv;
				b = v;
				break;
			case 5:
				r = v;
				g = pv;
				b = qv;
				break;
			case 6:
				r = v;
				g = tv;
				b = pv;
				break;
			case -1:
				r = v;
				g = pv;
				b = qv;
				break;
			default:
				r = 0;
				g = 0;
				b = 0;
				break;
		}
	}
	r *= 255;
	g *= 255;
	b *= 255;

	return Color((BYTE)r,(BYTE)g,(BYTE)b);
}

/*LabCOLOR Core::RGBtoLab(Color rgb)
{
}

Color Core::LabtoRGB(LabCOLOR lab)
{
}*/
/*
POINTS to POINT conversion from a LPARAM message from mouse messages
*/
POINT Core::makePoint(LPARAM lParam)
{
	POINT result;
	POINTS mps = MAKEPOINTS(lParam);
	
	result.x = mps.x;
	result.y = mps.y;

	return result;
}
/*
Fast bit bitmap nullifictaion
*/
void Core::clearBitmap(Bitmap *bmp, Rect rect)
{
	UINT *bmp0, x, y;
	BitmapData bmpData;

	rect.X = min(max(rect.X,0),(int)bmp->GetWidth());
	rect.Y = min(max(rect.Y,0),(int)bmp->GetHeight());

	rect.Width = min(rect.X + rect.Width,(int)bmp->GetWidth() - rect.X);
	rect.Height = min(rect.Y + rect.Height,(int)bmp->GetHeight() - rect.Y);

	bmp->LockBits(&rect,ImageLockModeWrite,bmp->GetPixelFormat(),&bmpData);

	bmp0 = (UINT *)bmpData.Scan0;
	for( x = 0; x < rect.Width; x++ )
		for( y = 0; y < rect.Height; y++ )
			bmp0[y * bmpData.Stride / 4 + x] = NULL;

	bmp->UnlockBits(&bmpData);
}
/*
Retrive bitmap from clipboard
*/
Bitmap *Core::getClipboardBitmap()
{
	Bitmap *bmp = NULL;

	/*
	Try to find the bitmap in clipboard
	*/
	if( IsClipboardFormatAvailable(CF_BITMAP) ){

		if( OpenClipboard(NULL) ){
			
			HBITMAP ptrBmp = (HBITMAP)GetClipboardData(CF_BITMAP);
			if( ptrBmp != NULL ){
				Bitmap *tmp = Bitmap::FromHBITMAP(ptrBmp,NULL);
				
				bmp = new Bitmap(tmp->GetWidth(),tmp->GetHeight(),Core::getPixelFormat());

				Graphics *g = Graphics::FromImage(bmp);
				g->DrawImage(tmp,0,0,tmp->GetWidth(),tmp->GetHeight());
				delete g;
				delete tmp;
			}
			CloseClipboard();
		}
	}
	/*
	If no bitmap on clipboard, then try the internal clipboard
	*/
	else if( Core::clipboardBitmap != NULL ){
		bmp = Core::clipboardBitmap->Clone(
			Rect(0,0,Core::clipboardBitmap->GetWidth(),Core::clipboardBitmap->GetHeight()),
			Core::clipboardBitmap->GetPixelFormat()
			);
	}
	return bmp;
}
/*
Set bitmap onto clipboard
*/
void Core::setClipboardBitmap(Bitmap *bmp)
{
	if( !OpenClipboard(Core::self->getWindowHandle()) )
		return;
	EmptyClipboard();

	if( Core::clipboardBitmap != NULL )
		delete Core::clipboardBitmap;
	/*
	First fill the internal clipboard
	*/
	Core::clipboardBitmap = bmp->Clone(
		Rect(0,0,bmp->GetWidth(),bmp->GetHeight()),
		bmp->GetPixelFormat()
		);

	Bitmap *opaque = new Bitmap(bmp->GetWidth(),bmp->GetHeight(),PixelFormat24bppRGB);
	Graphics *gfx = Graphics::FromImage(opaque);

	gfx->Clear(CLR_WHITE);
	gfx->DrawImage(bmp,0,0,bmp->GetWidth(),bmp->GetHeight());
	delete gfx;

	HBITMAP ptrBmp = NULL;
	HPALETTE palBmp = NULL;
	opaque->GetHBITMAP(NULL,&ptrBmp);

	/*
	Convert to a device independent bitmap
	*/
	Core::convertToDIB(ptrBmp,palBmp);

	if( ptrBmp == NULL ){
		palBmp = NULL;
	}
	else {
		SetClipboardData(CF_PALETTE,palBmp);
		SetClipboardData(CF_DIB,ptrBmp);
	}
	CloseClipboard();

	delete opaque;
}

bool Core::isClipboardBitmap()
{
	if( IsClipboardFormatAvailable(CF_BITMAP) )
		return true;
	return false;
}
/*
Confert a Gdiplus font information to HFONT
*/
HFONT Core::CreateHFONT(WCHAR *fontFamily, UINT size, INT style, Unit unit)
{
	Graphics *gfx = Graphics::FromHWND(this->windowHandle);

	FontFamily *family = new FontFamily(fontFamily);
	Font *font = new Font(family,(REAL)size,style,unit);

	LOGFONTW logFont;
	font->GetLogFontW(gfx,&logFont);

	HFONT hFont = CreateFontIndirect(&logFont);

	delete gfx;
	delete font;
	delete family;

	return hFont;
}
/*
Create an interface object with subsituted font.
MS Shell Dlg font by default.
*/
HWND Core::CreateWindowExSubstituteFont(DWORD dwExStyle,
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
										LPVOID lpParam)
{
	HWND hwnd = CreateWindowEx(
		dwExStyle,
		lpClassName,
		lpWindowName,
		dwStyle,
		x,y,nWidth,nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam);

	if( this->hFontShell != NULL )
		SendMessage(hwnd,WM_SETFONT,(WPARAM)this->hFontShell,TRUE);

	return hwnd;
}
/*
Process application window messages
*/
LRESULT Core::processMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	HWND hChild;
	HDC hdc;

	switch (message)
	{
		case WM_CREATE:
			if( this->initialized == false ){
				this->initialize(hWnd);
			}
			break;

		case WM_DROPFILES:
			this->gui->openDropFile(wParam);
			break;

		case WM_TIMER:
		if( wParam != TIMER_MMM )
			KillTimer(hWnd,wParam);
			switch(wParam){
				case TIMER_MBB:
					this->gui->unblockMBB();
					break;
				case TIMER_MCH:
					this->gui->setCursor();
					break;
				case TIMER_OVL:
					this->drawer->hideOverlay();
					this->gui->updateMenu();
					break;
				case TIMER_THB:
					this->drawer->updateOverlay();
					break;
				case TIMER_MMM:
					this->gui->movemMM();
					break;
			}
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
			hChild = this->getActiveChildHandle();
			if( hChild != NULL )
				SendMessage(hChild,message,wParam,lParam);
			break;
		case WM_INITMENUPOPUP:
			this->gui->updateMenu();
			break;
		case WM_CLOSE:
			if( this->children->getCount() > 0 ){
				this->children->gotoHead();
				do {
					SendMessage(this->children->getThat()->getWindowHandle(),WM_CLOSE,NULL,NULL);
				} while( this->children->next() == true );
			}
			return DefFrameProc(hWnd,this->mdiclientHandle,message,wParam,lParam);
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			
			switch (wmId)
			{
				case FERROR:
					break;
				default:
					this->gui->processMenu(wmId);
					
					if(LOWORD(wParam) >= ID_FIRSTCHILD)
						return DefFrameProc(hWnd,this->mdiclientHandle,message,wParam,lParam);
					
					hChild = this->getActiveChildHandle();
					if( hChild != NULL )
						SendMessage(hChild, WM_COMMAND, wParam, lParam);
			}
			break;

		case WM_ERASEBKGND:
			return 1;
		case WM_NCACTIVATE:
			if( this->getToolws() != NULL ){
				return this->getToolws()->overrideNCActivate(hWnd,wParam,lParam);
			}
			return DefFrameProc(hWnd,this->mdiclientHandle,message,wParam,lParam);
		case WM_ENABLE:
			if( this->getToolws() != NULL ){
				SendMessage(this->mdiclientHandle,message,wParam,lParam);

				return this->getToolws()->overrideEnable(hWnd,wParam,lParam);
			}
			return DefFrameProc(hWnd,this->mdiclientHandle,message,wParam,lParam);
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefFrameProc(hWnd,this->mdiclientHandle,message,wParam,lParam);
	}
	return 0;
}
/*
Process MDI frame messages
*/
LRESULT Core::processMdiMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	LRESULT result;
	Graphics *g;
	HDC hdc;

	switch (message)
	{
		case WM_LBUTTONDBLCLK:
			this->gui->openFile();
			break;
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			g = Graphics::FromHDC(hdc);
			g->Clear(CLR_FRAME_DARK);
			delete g;
			EndPaint(hWnd, &ps);
			break;
		case WM_MDIDESTROY:
			result = CallWindowProc(this->MDIWndProc,hWnd,message,wParam,lParam);
			if( this->children->getCount() <= 0 )
				this->toolws->disableToolwBoxes();
			return result;
		case WM_SIZE:
			if( this->getToolws() != NULL ){
				if( this->getToolws()->getToolwDock() != NULL ){
					this->getToolws()->getToolwDock()->processFrame(hWnd);
				}
			}
			return CallWindowProc(this->MDIWndProc,hWnd,message,wParam,lParam);
		default:
			return CallWindowProc(this->MDIWndProc,hWnd,message,wParam,lParam);
	}
	return 0;
}
/*
Process simple dialogs that display an image, like About dialog
*/
LRESULT CALLBACK Core::processDialogs(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	Graphics *g;
	HDC hdc;

	static int resId;
	static Image *img;

	switch (message)
	{
		/*
		Load the image, resize and center the dialog window
		*/
		case WM_INITDIALOG:
			resId = (int)lParam;
			img = Core::getImageResource(resId,RC_PNG);
			if( img != NULL ){
				int w = img->GetWidth();
				int h = img->GetHeight();
				int x = (GetDeviceCaps(GetDC(NULL),HORZRES) - w) / 2;
				int y = (GetDeviceCaps(GetDC(NULL),VERTRES) - h) / 2;

				SetWindowPos(hDlg,NULL,x,y,w,h,SWP_NOZORDER | SWP_FRAMECHANGED);
				SetCapture(hDlg);
			}
			else {
				EndDialog(hDlg,NO);
			}
			break;
		case WM_ERASEBKGND:
			return 1;
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
			EndDialog(hDlg,YES);
			break;
		case WM_MOUSEMOVE:
			Core::self->getGui()->setCursor(CURSOR_HAND);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hDlg, &ps);
			g = Graphics::FromHDC(hdc);
			if( img != NULL )
				g->DrawImage(img,0,0,img->GetWidth(),img->GetHeight());
			else
				g->Clear(CLR_WHITE);
			delete g;
			EndPaint(hDlg, &ps);
			break;
		case WM_DESTROY:
			delete img;
			img = NULL;
			ReleaseCapture();
			break;
		default:
			return TRUE;
	}
	return FALSE;
}
/*
Process dialogs that display thumbnail images
*/
LRESULT CALLBACK Core::processFakeDialogs(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int dir, key;

	PAINTSTRUCT ps;
	POINTS ms;
	HDC hdc;

	Graphics *g;
	Image *img;

	static Thumblay *layer;

	switch (message)
	{
		case WM_INITDIALOG:
			layer = new Thumblay(hDlg,Core::self,(FwCHAR *)lParam);

			img = layer->getImage();
			if( img != NULL ){
				int w = img->GetWidth();
				int h = img->GetHeight();
				int x = (GetDeviceCaps(GetDC(NULL),HORZRES) - w) / 2;
				int y = (GetDeviceCaps(GetDC(NULL),VERTRES) - h) / 2;

				SetWindowPos(hDlg,NULL,x,y,w,h,SWP_NOZORDER | SWP_FRAMECHANGED);
				SetCapture(hDlg);
			}
			else {
				EndDialog(hDlg,NO);
			}
			break;

		case WM_KEYUP:
		case WM_KEYDOWN:
			switch(wParam){
				case VK_UP:
					layer->scrollVer(1);
					layer->subrender();
					InvalidateRect(hDlg,NULL,TRUE);
					break;
				case VK_DOWN:
					layer->scrollVer(-1);
					layer->subrender();
					InvalidateRect(hDlg,NULL,TRUE);
					break;
				case VK_LEFT:
					layer->scrollHor(1);
					layer->subrender();
					InvalidateRect(hDlg,NULL,TRUE);
					break;
				case VK_RIGHT:
					layer->scrollHor(-1);
					layer->subrender();
					InvalidateRect(hDlg,NULL,TRUE);
					break;
				case VK_RETURN:
					layer->hide();
					break;
				case VK_ESCAPE:
					layer->nextImage(0,0);
					break;
			}
			break;
		case WM_RBUTTONDOWN:
			layer->nextImage(0,0);
			break;
		case WM_LBUTTONDOWN:
			ms = MAKEPOINTS(lParam);
			layer->prevImage(ms.x,ms.y);
			layer->subrender();
			InvalidateRect(hDlg,NULL,TRUE);
			break;
		case WM_LBUTTONDBLCLK:
			ms = MAKEPOINTS(lParam);
			layer->prevImageDblClk(ms.x,ms.y);
			break;
		case WM_MOUSEMOVE:
			Core::self->getGui()->setCursor(CURSOR_HAND);
			break;
		case WM_MOUSEWHEEL:
			dir = GET_WHEEL_DELTA_WPARAM(wParam);
			key = GET_KEYSTATE_WPARAM(wParam);

			if( dir > 0 ){
				if( key == MK_LBUTTON ){
					if( layer != NULL )
						layer->scrollHor(SCROLLSTEP);
				}
				else
					if( layer != NULL )
						layer->scrollVer(SCROLLSTEP);
			}
			else {
				if( key == MK_LBUTTON ){
					if( layer != NULL )
						layer->scrollHor(-SCROLLSTEP);
				}
				else
					if( layer != NULL )
						layer->scrollVer(-SCROLLSTEP);
			}
			layer->subrender();
			InvalidateRect(hDlg,NULL,TRUE);
			break;

		case WM_TIMER:
			KillTimer(hDlg,wParam);
			switch(wParam){
				case TIMER_THB:
					layer->update();
					break;
			}
			break;
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
			hdc = BeginPaint(hDlg, &ps);
			img = layer->getImage();

			g = Graphics::FromHDC(hdc);
			if( img != NULL )
				g->DrawImage(img,0,0,img->GetWidth(),img->GetHeight());
			else
				g->Clear(CLR_WHITE);
			
			delete g;
			EndPaint(hDlg, &ps);
			break;
		case WM_DESTROY:
			img = NULL;
			delete layer;
			break;
		default:
			return TRUE;
	}
	return FALSE;
}

// This function converts the given bitmap to a DIB.
// Returns true if the conversion took place,
// false if the conversion either unneeded or unavailable
bool Core::convertToDIB(HBITMAP &hBitmap, HPALETTE &hPalette)
{
  bool bConverted = false;
  BITMAP stBitmap;
  if( GetObject(hBitmap, sizeof(stBitmap), 
            &stBitmap) )
  {
    // that is a DFB. Now we attempt to create
    // a DIB with the same sizes and pixel format.
    HDC hScreen = GetDC(NULL);
    if (hScreen)
    {
      union {
        BITMAPINFO stBitmapInfo;
        BYTE pReserveSpace[sizeof(BITMAPINFO) 
                     + 0xFF * sizeof(RGBQUAD)];
      };
      ZeroMemory(pReserveSpace, sizeof(pReserveSpace));
      stBitmapInfo.bmiHeader.biSize = sizeof(stBitmapInfo.bmiHeader);
      stBitmapInfo.bmiHeader.biWidth = stBitmap.bmWidth;
      stBitmapInfo.bmiHeader.biHeight = stBitmap.bmHeight;
      stBitmapInfo.bmiHeader.biPlanes = 1;
      stBitmapInfo.bmiHeader.biBitCount = stBitmap.bmBitsPixel;
      stBitmapInfo.bmiHeader.biCompression = BI_RGB;

      if (stBitmap.bmBitsPixel <= 8)
      {
        stBitmapInfo.bmiHeader.biClrUsed = 
                        1 << stBitmap.bmBitsPixel;
        // This image is paletted-managed.
        // Hence we have to synthesize its palette.
      }
      stBitmapInfo.bmiHeader.biClrImportant = 
                       stBitmapInfo.bmiHeader.biClrUsed;

      PVOID pBits;
      HBITMAP hDib = CreateDIBSection(hScreen, 
        &stBitmapInfo, DIB_RGB_COLORS, &pBits, NULL, 0);

      if (hDib)
      {
        // ok, we're lucky. Now we have
        // to transfer the image to the DFB.
        HDC hMemSrc = CreateCompatibleDC(NULL);
        if (hMemSrc)
        {
          HGDIOBJ hOldSrc = SelectObject(hMemSrc, hBitmap);
          if (hOldSrc)
          {
            HDC hMemDst = CreateCompatibleDC(NULL);
            if (hMemDst)
            {
              HGDIOBJ hOldDst = SelectObject(hMemDst, hDib);
              if (hOldDst)
              {
                if (stBitmap.bmBitsPixel <= 8)
                {
                  // take the DFB's palette and set it to our DIB
                  hPalette = 
                    (HPALETTE) GetCurrentObject(hMemSrc, OBJ_PAL);
                  if (hPalette)
                  {
                    PALETTEENTRY pPaletteEntries[0x100];
                    UINT nEntries = GetPaletteEntries(hPalette, 
                                    0, stBitmapInfo.bmiHeader.biClrUsed, 
                                    pPaletteEntries);
                    if (nEntries)
                    {
                      //ASSERT(nEntries <= 0x100);
                      for (UINT nIndex = 0; nIndex < nEntries; nIndex++)
                        pPaletteEntries[nEntries].peFlags = 0;
                      SetDIBColorTable(hMemDst, 0, 
                        nEntries, (RGBQUAD*) pPaletteEntries);

                    }
                  }
                }

                // transfer the image using BitBlt function.
                // It will probably end in the
                // call to driver's DrvCopyBits function.
                if (BitBlt(hMemDst, 0, 0, stBitmap.bmWidth, 
                      stBitmap.bmHeight, hMemSrc, 0, 0, SRCCOPY))
                  bConverted = true; // success

                SelectObject(hMemDst, hOldDst);
              }
              DeleteDC(hMemDst);
            }
            SelectObject(hMemSrc, hOldSrc);
          }
          DeleteDC(hMemSrc);
        }

        if (bConverted)
        {
          DeleteObject(hBitmap); // it's no longer needed
          hBitmap = hDib;
        }
        else
          DeleteObject(hDib);
      }
      ReleaseDC(NULL, hScreen);
    }
  }
  return bConverted;
}