/*
DH_Clr.cpp
DialogHelper object that controls the custom color chooser dialog
*/

#include "stdafx.h"
#include "Core.h"

DialogHelper_Clr *DialogHelper_Clr::that = NULL;

DialogHelper_Clr::DialogHelper_Clr(HWND hWnd, Color current)
{
	DialogHelper_Clr::that = this;

	this->windowHandle = hWnd;
	this->clrCurrent = current;
	this->clrNew = current;

	this->clrCmyk = Core::RGBtoCMYK(this->clrNew);
	this->clrHSV = Core::RGBtoHSV(this->clrNew);

	this->bmpField = new Bitmap(this->intFieldWidth,this->intFieldWidth,PixelFormat32bppARGB);
	this->bmpSlid = new Bitmap(this->intSlidWidth,this->intFieldWidth,PixelFormat32bppARGB);
	this->bmpCmp = new Bitmap(this->intCmpWidth,this->intCmpWidth,Core::getPixelFormat());

	this->hField = GetDlgItem(hWnd,IDC_CLRFIELD);
	this->hSlid = GetDlgItem(hWnd,IDC_CLRSLID);
	this->hCmp = GetDlgItem(hWnd,IDC_CLRCMP);

	SetWindowPos(
		this->hField,
		HWND_TOP,
		7,7,
		this->intFieldWidth,
		this->intFieldWidth,
		SWP_SHOWWINDOW);
	SetWindowPos(
		this->hSlid,
		HWND_TOP,
		270,7,
		this->intSlidWidth,
		this->intFieldWidth,
		SWP_SHOWWINDOW);
	SetWindowPos(
		this->hCmp,
		HWND_TOP,
		300,7,
		this->intCmpWidth,
		this->intCmpWidth,
		SWP_SHOWWINDOW);

	this->procField = (WNDPROC)GetWindowLong(this->hField,GWL_WNDPROC);
	SetWindowLong(this->hField,GWL_WNDPROC,(LONG)DialogHelper_Clr::procNewField);

	this->procSlid = (WNDPROC)GetWindowLong(this->hSlid,GWL_WNDPROC);
	SetWindowLong(this->hSlid,GWL_WNDPROC,(LONG)DialogHelper_Clr::procNewSlid);

	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_H),0,360);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_S),0,100);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_V),0,100);

	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_R),0,255);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_G),0,255);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_B),0,255);

	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_L),0,255);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_A),0,255);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_LAB),0,255);

	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_C),0,100);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_M),0,100);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_Y),0,100);
	Dialogs::limitInputInt(GetDlgItem(hWnd,IDC_V_K),0,100);

	CheckRadioButton(this->windowHandle,IDC_RB_H,IDC_RB_LAB,IDC_RB_H);

	this->render();
	this->setCtrls();
}

DialogHelper_Clr::~DialogHelper_Clr()
{
	SetWindowLong(this->hField,GWL_WNDPROC,(LONG)this->procField);
	SetWindowLong(this->hSlid,GWL_WNDPROC,(LONG)this->procSlid);

	this->setBitmap(NULL,IDC_CLRFIELD);
	this->setBitmap(NULL,IDC_CLRSLID);
	this->setBitmap(NULL,IDC_CLRCMP);

	delete this->bmpCmp;
	delete this->bmpField;
	delete this->bmpSlid;
}

Color DialogHelper_Clr::getNewColor()
{
	return this->clrNew;
}

Color DialogHelper_Clr::getOldColor()
{
	return this->clrCurrent;
}
/*
Renders the big color field using bits operations
*/
void DialogHelper_Clr::renderField()
{
	int checked = this->getCheckedRadio();
	UINT *field, x, y, color;
	BitmapData bitmapData;
	HSVCOLOR hsv;
	LabCOLOR lab;

	Rect rect(0,0,this->intFieldWidth,this->intFieldWidth);

	this->bmpField->LockBits(
		&rect,
		ImageLockModeWrite,
		PixelFormat32bppARGB,
		&bitmapData);
	field = (UINT *)bitmapData.Scan0;

	for( y = 0; y < this->intFieldWidth; y++ ){
		for( x = 0; x < this->intFieldWidth; x++ ){
			switch(checked){
				case IDC_RB_H:
					hsv = this->clrHSV;
					hsv.S = (int)(x * 100 / 255);
					hsv.V = 100 - (int)(y * 100 / 255);

					color = Core::RGBtoUINT( Core::HSVtoRGB(hsv) );
					break;
				case IDC_RB_S:
					hsv = this->clrHSV;
					hsv.H = ((int)(x * 360 / 255))%360;
					hsv.V = 100 - (int)(y * 100 / 255);

					color = Core::RGBtoUINT( Core::HSVtoRGB(hsv) );
					break;
				case IDC_RB_V:
					hsv = this->clrHSV;
					hsv.H = ((int)(x * 360 / 255))%360;
					hsv.S = 100 - (int)(y * 100 / 255);

					color = Core::RGBtoUINT( Core::HSVtoRGB(hsv) );
					break;
				case IDC_RB_R:
					color = Core::RGBtoUINT(this->clrNew.GetR(),255 - y,x);
					break;
				case IDC_RB_G:
					color = Core::RGBtoUINT(255 - y,this->clrNew.GetG(),x);
					break;
				case IDC_RB_B:
					color = Core::RGBtoUINT(x,255 - y,this->clrNew.GetB());
					break;
				default:
					color = 0xffffffff;
					break;
			}
			field[y * bitmapData.Stride / 4 + x] = color;
		}
	}
	this->bmpField->UnlockBits(&bitmapData);

	this->setField();
}
/*
Renders the thin color slider using bits operations
*/
void DialogHelper_Clr::renderSlid()
{
	int checked = this->getCheckedRadio();
	UINT *slid, x, y, color;
	BitmapData bitmapData;
	HSVCOLOR hsv;
	LabCOLOR lab;

	Rect rect(0,0,this->intSlidWidth,this->intFieldWidth);

	this->bmpSlid->LockBits(
		&rect,
		ImageLockModeWrite,
		PixelFormat32bppARGB,
		&bitmapData);
	slid = (UINT *)bitmapData.Scan0;

	for( y = 0; y < this->intFieldWidth; y++ ){
		for( x = 0; x < this->intSlidWidth; x++ ){
			switch(checked){
				case IDC_RB_H:
					hsv = this->clrHSV;
					hsv.H = ((int)((255 - y) * 360 / 255))%360;
					hsv.S = 100;
					hsv.V = 100;
					color = Core::RGBtoUINT( Core::HSVtoRGB(hsv) );
					break;
				case IDC_RB_S:
					hsv = this->clrHSV;
					hsv.S = (int)((255 - y) * 100 / 255);
					color = Core::RGBtoUINT( Core::HSVtoRGB(hsv) );
					break;
				case IDC_RB_V:
					hsv = this->clrHSV;
					hsv.V = (int)((255 - y) * 100 / 255);
					color = Core::RGBtoUINT( Core::HSVtoRGB(hsv) );
					break;
				case IDC_RB_R:
					color = Core::RGBtoUINT(255 - y,this->clrNew.GetG(),this->clrNew.GetB());
					break;
				case IDC_RB_G:
					color = Core::RGBtoUINT(this->clrNew.GetR(),255 - y,this->clrNew.GetB());
					break;
				case IDC_RB_B:
					color = Core::RGBtoUINT(this->clrNew.GetR(),this->clrNew.GetG(),255 - y);
					break;
				default:
					color = 0xffffffff;
					break;
			}
			slid[y * bitmapData.Stride / 4 + x] = color;
		}
	}
	this->bmpSlid->UnlockBits(&bitmapData);

	this->setSlid();
}
/*
Renders the top color comparison using bits operations
*/
void DialogHelper_Clr::renderCmp()
{
	Graphics *g = Graphics::FromImage(this->bmpCmp);
	
	SolidBrush *olc = new SolidBrush(this->clrCurrent);
	SolidBrush *nwc = new SolidBrush(this->clrNew);

	int half = (int)(this->intCmpWidth/2);
	g->FillRectangle(nwc,0,0,this->intCmpWidth,half);
	g->FillRectangle(olc,0,half,this->intCmpWidth,half);

	delete g;
	delete olc;
	delete nwc;

	this->setCmp();
}
/*
General render
*/
void DialogHelper_Clr::render()
{
	this->renderField();
	this->renderSlid();
	this->renderCmp();
}
/*
Draws the color circle marke on big color field and uploads it onto the static control
*/
void DialogHelper_Clr::setField()
{
	HSVCOLOR hsv;
	LabCOLOR lab;

	Bitmap *bmp =
		this->bmpField->Clone(
			0,0,
			this->bmpField->GetWidth(),
			this->bmpField->GetHeight(),
			this->bmpField->GetPixelFormat()
			);

	POINT o;
	switch( this->getCheckedRadio() ){
		case IDC_RB_H:
			hsv = this->clrHSV;
			o.x = (int)(hsv.S * 255 / 100);
			o.y = 255 - (int)(hsv.V * 255 / 100);
			break;
		case IDC_RB_S:
			hsv = this->clrHSV;
			o.x = (int)(hsv.H * 255 / 360);
			o.y = 255 - (int)(hsv.V * 255 / 100);
			break;
		case IDC_RB_V:
			hsv = this->clrHSV;
			o.x = (int)(hsv.H * 255 / 360);
			o.y = 255 - (int)(hsv.S * 255 / 100);
			break;
		case IDC_RB_R:
			o.x = this->clrNew.GetB();
			o.y = 255 - this->clrNew.GetG();
			break;
		case IDC_RB_G:
			o.x = this->clrNew.GetB();
			o.y = 255 - this->clrNew.GetR();
			break;
		case IDC_RB_B:
			o.x = this->clrNew.GetR();
			o.y = 255 - this->clrNew.GetG();
			break;
	}
	Color color;
	int x, y1, y2, size = 4;

	for( int i = -size; i <= size; i++ ){
		x = o.x + i;
		y1 = o.y + abs(i) - (size + 1);
		y2 = o.y - abs(i) + (size + 1);
		if( i == 0 ){
			y1++;
			y2--;
		}
		if( Core::isInBmp(bmp,x,y1) == true ){
			if( bmp->GetPixel(x,y1,&color) == Ok )
				bmp->SetPixel(x,y1,Core::invClr(color));
		}
		if( Core::isInBmp(bmp,x,y2) == true ){
			if( bmp->GetPixel(x,y2,&color) == Ok )
				bmp->SetPixel(x,y2,Core::invClr(color));
		}
		if( i == -size || i == size ){
			if( Core::isInBmp(bmp,x,o.y) == true ){
				if( bmp->GetPixel(x,o.y,&color) == Ok )
					bmp->SetPixel(x,o.y,Core::invClr(color));
			}
		}
	}
	this->setBitmap(bmp,IDC_CLRFIELD);
	delete bmp;
}
/*
Draws the color slider cursor and uploads it onto the static control
*/
void DialogHelper_Clr::setSlid()
{
	HSVCOLOR hsv;
	LabCOLOR lab;

	Bitmap *bmp =
		this->bmpSlid->Clone(
			0,0,
			this->bmpSlid->GetWidth(),
			this->bmpSlid->GetHeight(),
			this->bmpSlid->GetPixelFormat()
			);

	POINT o;
	switch( this->getCheckedRadio() ){
		case IDC_RB_H:
			hsv = this->clrHSV;
			o.x = 0;
			o.y = 255 - (int)(hsv.H * 255 / 360);
			break;
		case IDC_RB_S:
			hsv = this->clrHSV;
			o.x = 0;
			o.y = 255 - (int)(hsv.S * 255 / 100);
			break;
		case IDC_RB_V:
			hsv = this->clrHSV;
			o.x = 0;
			o.y = 255 - (int)(hsv.V * 255 / 100);
			break;
		case IDC_RB_R:
			o.x = 0;
			o.y = 255 - this->clrNew.GetR();
			break;
		case IDC_RB_G:
			o.x = 0;
			o.y = 255 - this->clrNew.GetG();
			break;
		case IDC_RB_B:
			o.x = 0;
			o.y = 255 - this->clrNew.GetB();
			break;
	}
	int i, j, x1, x2, y, size = 4;
	Color color;

	for( i = -size; i <= size; i++ ){
		x1 = size - abs(i);
		x2 = this->intSlidWidth - size + abs(i) - 1;
		y = o.y + i;

		if( Core::isInBmp(bmp,x1,y) == true ){
			bmp->GetPixel(x1,y,&color);
			bmp->SetPixel(x1,y,Core::invClr(color));

			bmp->GetPixel(x2,y,&color);
			bmp->SetPixel(x2,y,Core::invClr(color));

			for( j = 0; j < x1; j++ )
				bmp->SetPixel(j,y,CLR_FRAME_LIGHT);
			for( j = x2 + 1; j < this->intSlidWidth; j++ )
				bmp->SetPixel(j,y,CLR_FRAME_LIGHT);
		}
	}
	this->setBitmap(bmp,IDC_CLRSLID);
	delete bmp;
}
/*
Uploads the color comparison onto the static control
*/
void DialogHelper_Clr::setCmp()
{
	this->setBitmap(this->bmpCmp,IDC_CLRCMP);
}
/*
Performs color changes after one of the edit box values was changed.
The changed values vary depending on the current main value selection
(main value is the one selected by the radio button and presented on the thin slider)
*/
void DialogHelper_Clr::valueChange(int ctrlId)
{
	switch(ctrlId){
		case IDC_V_H:
		case IDC_V_S:
		case IDC_V_V:
			this->clrHSV.H = GetDlgItemInt(this->windowHandle,IDC_V_H,NULL,FALSE);
			this->clrHSV.S = GetDlgItemInt(this->windowHandle,IDC_V_S,NULL,FALSE);
			this->clrHSV.V = GetDlgItemInt(this->windowHandle,IDC_V_V,NULL,FALSE);

			this->clrNew = Core::HSVtoRGB(this->clrHSV);
			this->clrCmyk = Core::RGBtoCMYK(this->clrNew);
			//set Lab, HEX
			break;
		case IDC_V_R:
		case IDC_V_G:
		case IDC_V_B:
			this->clrNew = Color(
				GetDlgItemInt(this->windowHandle,IDC_V_R,NULL,FALSE),
				GetDlgItemInt(this->windowHandle,IDC_V_G,NULL,FALSE),
				GetDlgItemInt(this->windowHandle,IDC_V_B,NULL,FALSE)
				);

			this->clrHSV = Core::RGBtoHSV(this->clrNew);
			this->clrCmyk = Core::RGBtoCMYK(this->clrNew);
			//set Lab, HEX
			break;
		case IDC_V_C:
		case IDC_V_M:
		case IDC_V_Y:
		case IDC_V_K:
			this->clrCmyk.C = GetDlgItemInt(this->windowHandle,IDC_V_C,NULL,FALSE);
			this->clrCmyk.M = GetDlgItemInt(this->windowHandle,IDC_V_M,NULL,FALSE);
			this->clrCmyk.Y = GetDlgItemInt(this->windowHandle,IDC_V_Y,NULL,FALSE);
			this->clrCmyk.K = GetDlgItemInt(this->windowHandle,IDC_V_K,NULL,FALSE);

			this->clrNew = Core::CMYKtoRGB(this->clrCmyk);
			this->clrHSV = Core::RGBtoHSV(this->clrNew);
			//set Lab, HEX
			break;
	}
	this->setCtrls(ctrlId);
	this->render();
}
/*
Set the new values into controls except the changed control
that represents the changed value
*/
void DialogHelper_Clr::setCtrls(int except)
{
	switch(this->getCheckedRadio()){
		case IDC_RB_H:
		case IDC_RB_S:
		case IDC_RB_V:
			this->clrNew = Core::HSVtoRGB(this->clrHSV);
			this->clrCmyk = Core::RGBtoCMYK(this->clrNew);
			//set Lab, HEX
			break;
		case IDC_RB_R:
		case IDC_RB_G:
		case IDC_RB_B:
			this->clrHSV = Core::RGBtoHSV(this->clrNew);
			this->clrCmyk = Core::RGBtoCMYK(this->clrNew);
			//set Lab, HEX
			break;
	}

	if( except != IDC_V_H )
		SetDlgItemInt(this->windowHandle,IDC_V_H,this->clrHSV.H,FALSE);
	if( except != IDC_V_S )
		SetDlgItemInt(this->windowHandle,IDC_V_S,this->clrHSV.S,FALSE);
	if( except != IDC_V_V )
		SetDlgItemInt(this->windowHandle,IDC_V_V,this->clrHSV.V,FALSE);

	if( except != IDC_V_R )
		SetDlgItemInt(this->windowHandle,IDC_V_R,this->clrNew.GetR(),FALSE);
	if( except != IDC_V_G )
		SetDlgItemInt(this->windowHandle,IDC_V_G,this->clrNew.GetG(),FALSE);
	if( except != IDC_V_B )
		SetDlgItemInt(this->windowHandle,IDC_V_B,this->clrNew.GetB(),FALSE);

	if( except != IDC_V_C )
		SetDlgItemInt(this->windowHandle,IDC_V_C,this->clrCmyk.C,FALSE);
	if( except != IDC_V_M )
		SetDlgItemInt(this->windowHandle,IDC_V_M,this->clrCmyk.M,FALSE);
	if( except != IDC_V_Y )
		SetDlgItemInt(this->windowHandle,IDC_V_Y,this->clrCmyk.Y,FALSE);
	if( except != IDC_V_K )
		SetDlgItemInt(this->windowHandle,IDC_V_K,this->clrCmyk.K,FALSE);
}
/*
Uploads the given bitmap into a static control
*/
void DialogHelper_Clr::setBitmap(Bitmap *bmp, int itemId)
{
	HBITMAP hBmp = NULL;
	HBITMAP hTemp = NULL;

	if( bmp != NULL )
		bmp->GetHBITMAP(NULL,&hBmp);

	hTemp = (HBITMAP)SendMessage(
		GetDlgItem(this->windowHandle,itemId),
		STM_SETIMAGE,
		(WPARAM)IMAGE_BITMAP,
		(LPARAM)hBmp
		);
	if( hTemp != NULL )
		DeleteObject(hTemp);
}
/*
Radio button change notification
*/
void DialogHelper_Clr::radioChange()
{
	this->renderField();
	this->renderSlid();
}

int DialogHelper_Clr::getCheckedRadio()
{
	for( int i = IDC_RB_H; i <= IDC_RB_LAB; i++ )
		if( IsDlgButtonChecked(this->windowHandle,i) == BST_CHECKED )
			return i;

	return NULL;
}
/*
Big color field procedure that tracks the mouse button
movement to retrive selected color
*/
LRESULT CALLBACK DialogHelper_Clr::procNewField(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DialogHelper_Clr *that = DialogHelper_Clr::that;
	if( that == NULL )
		return 0;

	static bool slide = false;
	int x, y;
	POINTS p;

	switch(message){
		case WM_LBUTTONDOWN:
			slide = true;
			SetCapture(GetDlgItem(that->windowHandle,IDC_CLRFIELD));
		case WM_MOUSEMOVE:
			if( slide == true ){
				p = MAKEPOINTS(lParam);

				x = min( max(p.x,0) , (int)that->bmpField->GetWidth() - 1 );
				y = min( max(p.y,0) , (int)that->bmpField->GetHeight() - 1 );

				if( Core::isInBmp(that->bmpField,x,y) == true ){
					switch(that->getCheckedRadio()){
						case IDC_RB_H:
							that->clrHSV.S = (int)(x * 100 / 255);
							that->clrHSV.V = (int)((255 - y) * 100 / 255);
							break;
						case IDC_RB_S:
							that->clrHSV.H = (int)(x * 360 / 255);
							that->clrHSV.V = (int)((255 - y) * 100 / 255);
							break;
						case IDC_RB_V:
							that->clrHSV.H = (int)(x * 360 / 255);
							that->clrHSV.S = (int)((255 - y) * 100 / 255);
							break;
						case IDC_RB_R:
						case IDC_RB_G:
						case IDC_RB_B:
							that->bmpField->GetPixel(x,y,&that->clrNew);
							break;
					}
					that->setCtrls();
					that->renderSlid();
					that->renderCmp();
					that->setField();					
				}
			}
			break;
		case WM_LBUTTONUP:
			slide = false;
			ReleaseCapture();
			break;
		default:
			return CallWindowProc(that->procField,hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Thin slider procedure that tracks mouse movement
to retrive selected color
*/
LRESULT CALLBACK DialogHelper_Clr::procNewSlid(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DialogHelper_Clr *that = DialogHelper_Clr::that;
	if( that == NULL )
		return 0;

	static bool slide = false;
	int x, y;
	POINTS p;

	switch(message){
		case WM_LBUTTONDOWN:
			slide = true;
			SetCapture(hDlg);
		case WM_MOUSEMOVE:
			if( slide == true ){
				p = MAKEPOINTS(lParam);

				x = min( max(p.x,0) , (int)that->bmpSlid->GetWidth() - 1 );
				y = min( max(p.y,0) , (int)that->bmpSlid->GetHeight() - 1 );

				if( Core::isInBmp(that->bmpSlid,x,y) == true ){
					switch(that->getCheckedRadio()){
						case IDC_RB_H:
							that->clrHSV.H = (int)((255 - y) * 360 / 255);
							break;
						case IDC_RB_S:
							that->clrHSV.S = (int)((255 - y) * 100 / 255);
							break;
						case IDC_RB_V:
							that->clrHSV.V = (int)((255 - y) * 100 / 255);
							break;
						case IDC_RB_R:
						case IDC_RB_G:
						case IDC_RB_B:
							that->bmpSlid->GetPixel(x,y,&that->clrNew);
							break;
					}
					that->setCtrls();
					that->renderField();
					that->renderCmp();
					that->setSlid();					
				}
			}
			break;
		case WM_LBUTTONUP:
			slide = false;
			ReleaseCapture();
			break;
		default:
			return CallWindowProc(that->procSlid,hDlg,message,wParam,lParam);
	}
	return 0;
}