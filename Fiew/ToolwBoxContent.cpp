/*
ToolwBoxContent.cpp
This object represents a container that is placed inside ToolwBox.
This class is never used, rather than that it is inherited by the
contextual content like Layers, History, Info
*/

#include "stdafx.h"
#include "Core.h"

ToolwBoxContent::ToolwBoxContent(ToolwBox *box, FwCHAR *name) : Toolw(box->getCore(),box->getControler())
{
	this->box = box;
	this->name = name;

	RECT client;
	GetClientRect(box->getWindowHandle(),&client);

	this->hToolw = CreateWindowEx(
		NULL,
		IDCL_TOOLW_BXCNT,
		name->toWCHAR(),
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		0,0,
		client.right,client.bottom,
		box->getWindowHandle(),
		NULL,box->getCore()->getInstance(),NULL
		);
	SetProp(this->hToolw,ATOM_THIS,this);

	this->box->addContent(this);
}

ToolwBoxContent::~ToolwBoxContent()
{
	if( this->name != NULL )
		delete name;
}

FwCHAR *ToolwBoxContent::getName()
{
	return this->name;
}

ToolwBox *ToolwBoxContent::getBox()
{
	return this->box;
}

void ToolwBoxContent::resizeContent(int dw, int dh)
{
	RECT content;
	GetWindowRect(this->hToolw,&content);

	SetWindowPos(
		this->hToolw,
		this->hToolw,
		NULL,NULL,
		content.right - content.left + dw,
		content.bottom - content.top + dh,
		SWP_NOZORDER | 
		SWP_NOOWNERZORDER | 
		SWP_NOACTIVATE
		);
}

void ToolwBoxContent::disable()
{

}

HWND ToolwBoxContent::createContent(int x, int y, int w, int h)
{
	HWND hWnd = CreateWindowEx(
		NULL,
		IDCL_TOOLW_BXPANCNT,
		NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		x,y,w,h,
		this->hToolw,
		NULL,
		Core::self->getInstance(),
		NULL
		);
	SetProp(hWnd,ATOM_OWNER,this);

	return hWnd;
}

LRESULT ToolwBoxContent::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hDlg,message,wParam,lParam);
}