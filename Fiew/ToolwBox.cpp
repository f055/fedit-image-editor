/*
ToolwBox.cpp
This object represents a Tool Window that contains content like Layer, History, Info.
It is a container and is prepared for the future option of moving inside contents
between other ToolwBox windows.
*/

#include "stdafx.h"
#include "Core.h"

ToolwBox::ToolwBox(Core *core, Toolws *controler, bool noresize) : Toolw(core,controler)
{
	this->contents = new List<ToolwBoxContent>();
	this->noresize = noresize;

	int right = GetDeviceCaps(GetDC(NULL),HORZRES);
	DWORD style = WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN;

	if( noresize == false )
		style |= WS_SIZEBOX;

	this->hToolw = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		IDCL_TOOLW_BOX,
		NULL,
		style,
		right - TW_BX_MINW - 5,
		TWPOS_CC + Toolws::boxCounter * (TW_BX_MINH + 5),
		TW_BX_MINW,
		TW_BX_MINH,
		this->core->getWindowHandle(),
		NULL,
		this->core->getInstance(),
		NULL
		);

	SetProp(this->hToolw,ATOM_THIS,this);

	GetClientRect(this->hToolw,&this->sizeContent);
}

ToolwBox::~ToolwBox()
{

}

void ToolwBox::addContent(ToolwBoxContent *content)
{
	this->contents->add(content);

	if( this->contents->getCount() == 1 )
		SetWindowText(this->hToolw,content->getName()->toWCHAR());
	else
		SetWindowText(this->hToolw,NULL);
}

void ToolwBox::removeContent(ToolwBoxContent *content)
{
	this->contents->add(content);
}

void ToolwBox::resizeContent()
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	int dw = client.right - this->sizeContent.right;
	int dh = client.bottom - this->sizeContent.bottom;

	this->sizeContent = client;

	this->contents->gotoHead();
	do {
		this->contents->getThat()->resizeContent(dw,dh);
	} while( this->contents->next() == true );
}

LRESULT CALLBACK ToolwBox::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwBox *that = (ToolwBox *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	RECT *rect;
	int maxw, maxh;

	switch(message){
		case WM_SIZE:
			that->resizeContent();
			break;
		case WM_CLOSE:
			ShowWindow(hDlg,SW_HIDE);
			break;
		case WM_SIZING:
			rect = (RECT *)lParam;

			maxw = max(TW_BX_MINW,rect->right - rect->left);
			maxh = max(TW_BX_MINH,rect->bottom - rect->top);

			switch(wParam){
				case WMSZ_BOTTOM:
					rect->bottom = rect->top + maxh;
					break;
				case WMSZ_BOTTOMLEFT:
					rect->bottom = rect->top + maxh;
					rect->left = rect->right - maxw;
					break;
				case WMSZ_BOTTOMRIGHT:
					rect->bottom = rect->top + maxh;
					rect->right = rect->left + maxw;
					break;
				case WMSZ_LEFT:
					rect->left = rect->right - maxw;
					break;
				case WMSZ_RIGHT:
					rect->right = rect->left + maxw;
					break;
				case WMSZ_TOP:
					rect->top = rect->bottom - maxh;
					break;
				case WMSZ_TOPLEFT:
					rect->top = rect->bottom - maxh;
					rect->left = rect->right - maxw;
					break;
				case WMSZ_TOPRIGHT:
					rect->top = rect->bottom - maxh;
					rect->right = rect->left + maxw;
					break;
			}
			return 1;
		case WM_NCACTIVATE:
			return that->controler->overrideNCActivate(hDlg,wParam,lParam);
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}

LRESULT CALLBACK ToolwBox::processContents(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwBoxContent *that = (ToolwBoxContent *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);
	else
		return that->processMessages(hDlg,message,wParam,lParam);

	return 0;
}