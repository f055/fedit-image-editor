/*
ToolwBoxInfo.cpp
This object represents the contents of 'Info' Tool Window.
It is a simple content pane without any scrolls.
*/

#include "stdafx.h"
#include "Core.h"

ToolwBoxInfo::ToolwBoxInfo(ToolwBox *box) : ToolwBoxContent(box,new FwCHAR(TOOLWBOX_INFO))
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	int xRGB = 80;
	int yRGB = 10;

	int xCMYK = 130;
	int yCMYK = 7;

	int xXY = 5;
	int yXY = client.bottom / 2 + 25;

	int xWH = 100;
	int yWH = client.bottom / 2 + 25;

	int width = 20;
	int height = 15;

	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		TOOLWBOX_INFO_COLOR,
		WS_VISIBLE | WS_CHILD | SS_CENTER,
		10,
		10,
		50,
		20,
		this->hToolw,
		NULL,
		this->core->getInstance(),
		NULL
		);
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		TOOLWBOX_INFO_POS,
		WS_VISIBLE | WS_CHILD | SS_CENTER,
		10,
		client.bottom / 2 + 5,
		50,
		20,
		this->hToolw,
		NULL,
		this->core->getInstance(),
		NULL
		);
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		TOOLWBOX_INFO_RECT,
		WS_VISIBLE | WS_CHILD | SS_CENTER,
		90,
		client.bottom / 2 + 5,
		50,
		20,
		this->hToolw,
		NULL,
		this->core->getInstance(),
		NULL
		);

	CreateWindowEx(
		WS_EX_STATICEDGE,
		IDCL_STATIC,
		NULL,
		WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
		0,
		client.bottom / 2,
		client.right,
		2,
		this->hToolw,
		NULL,
		this->core->getInstance(),
		NULL
		);

	this->createPair(xRGB,yRGB,width,height,L"R: ",this->dlgColR);
	this->createPair(xRGB,yRGB + height + 2,width,height,L"G: ",this->dlgColG);
	this->createPair(xRGB,yRGB + 2 * height + 4,width,height,L"B: ",this->dlgColB);

	this->createPair(xCMYK,yCMYK,width,height - 2,L"C: ",this->dlgColC);
	this->createPair(xCMYK,yCMYK + height - 2,width,height - 2,L"M: ",this->dlgColM);
	this->createPair(xCMYK,yCMYK + 2 * height - 4,width,height - 2,L"Y: ",this->dlgColY);
	this->createPair(xCMYK,yCMYK + 3 * height - 6,width,height - 2,L"K: ",this->dlgColK);

	width += 15;

	this->createPair(xXY,yXY,width,height,L"X: ",this->dlgPosX);
	this->createPair(xXY,yXY + height + 2,width,height,L"Y: ",this->dlgPosY);

	this->createPair(xWH,yWH,width,height,L"W: ",this->dlgSizW);
	this->createPair(xWH,yWH + height + 2,width,height,L"H: ",this->dlgSizH);
}

ToolwBoxInfo::~ToolwBoxInfo()
{

}

void ToolwBoxInfo::createPair(int x, int y, int w, int h, WCHAR *label, HWND &handle)
{
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		label,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		x,y,w,h,
		this->hToolw,
		NULL,
		this->core->getInstance(),
		NULL
		);
	handle = this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		NULL,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		x + w,y,w,h,
		this->hToolw,
		NULL,
		this->core->getInstance(),
		NULL
		);
}

void ToolwBoxInfo::loadColor(Color rgb)
{
	CMYKCOLOR cmyk = Core::RGBtoCMYK(rgb);

	this->loadLabel(this->dlgColR,rgb.GetR());
	this->loadLabel(this->dlgColG,rgb.GetG());
	this->loadLabel(this->dlgColB,rgb.GetB());

	this->loadLabel(this->dlgColC,cmyk.C);
	this->loadLabel(this->dlgColM,cmyk.M);
	this->loadLabel(this->dlgColY,cmyk.Y);
	this->loadLabel(this->dlgColK,cmyk.K);
}

void ToolwBoxInfo::loadPos(Point pos)
{
	this->loadLabel(this->dlgPosX,pos.X);
	this->loadLabel(this->dlgPosY,pos.Y);
}

void ToolwBoxInfo::loadSize(Rect size)
{
	this->loadLabel(this->dlgSizW , max(size.Width - 1,0) );
	this->loadLabel(this->dlgSizH , max(size.Height - 1,0) );
}

void ToolwBoxInfo::loadLabel(HWND handle, UINT value)
{
	FwCHAR *buffer = new FwCHAR(value);
	SetWindowText(handle,buffer->toWCHAR());
	delete buffer;

	InvalidateRect(handle,NULL,TRUE);
}

void ToolwBoxInfo::nullLabel(HWND handle)
{
	SetWindowText(handle,L"");
	InvalidateRect(handle,NULL,TRUE);
}

void ToolwBoxInfo::resizeContent(int dw, int dh)
{

}

void ToolwBoxInfo::disable()
{
	this->nullLabel(this->dlgColR);
	this->nullLabel(this->dlgColG);
	this->nullLabel(this->dlgColB);

	this->nullLabel(this->dlgColC);
	this->nullLabel(this->dlgColM);
	this->nullLabel(this->dlgColY);
	this->nullLabel(this->dlgColK);

	this->nullLabel(this->dlgPosX);
	this->nullLabel(this->dlgPosY);

	this->nullLabel(this->dlgSizW);
	this->nullLabel(this->dlgSizH);
}

LRESULT ToolwBoxInfo::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_CREATE:
			break;
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}
