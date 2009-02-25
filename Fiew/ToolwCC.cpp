/*
ToolwCC.cpp
This object represents the Tool Control Center that enables the user
to choose from a variety of tools, also contains the tools that are
activated through main menu items.
*/

#include "stdafx.h"
#include "Core.h"

ToolwCC::ToolwCC(Core *core, Toolws *controler) : Toolw(core,controler)
{
	this->toolCurrent = NULL;
	
	this->toolHand = new ToolHand();
	this->toolZoom = new ToolZoom();
	this->toolPencil = new ToolPencil();
	this->toolLine = new ToolLine(ICC_SHALIN);
	this->toolRect = new ToolLine(ICC_SHAREC);
	this->toolElli = new ToolLine(ICC_SHACIR);
	this->toolRRec = new ToolLine(ICC_SHARRC);
	this->toolMove = new ToolMove();
	this->toolSelectRect = new ToolSelectRect();
	this->toolSelectCirc = new ToolSelectRect(SELCIRC);
	this->toolSelectHor = new ToolSelectRect(SELHOR);
	this->toolSelectVer = new ToolSelectRect(SELVER);
	this->toolSelectPoly = new ToolSelectPoly(ICC_LASPOL);
	this->toolSelectFree = new ToolSelectPoly(ICC_LASMOS);
	this->toolSampleColor = new ToolSampleColor();
	this->toolCrop = new ToolCrop();
	this->toolSelectWand = new ToolSelectWand();
	this->toolBucket = new ToolBucket();
	this->toolText = new ToolText();
	this->toolEraser = new ToolPencil(PENRUB);

	this->toolCut = new ToolCopy(COPCUT);
	this->toolCopy = new ToolCopy(COPCOP);
	this->toolCopyMerged = new ToolCopy(COPCPM);
	this->toolPaste = new ToolCopy(COPPAS);
	this->toolClear = new ToolCopy(COPCLR);

	this->toolFilter = new ToolFilter();
	this->toolMerge = new ToolMerge();
	this->toolRaster = new ToolRaster();

	this->hColorFore = NULL;
	this->hColorBack = NULL;

	this->pen_light = new Pen(CLR_WHITE,1);
	this->pen_dark = new Pen(CLR_FRAME_DARK,1);

	this->brush_bkgnd = new SolidBrush(CLR_FRAME_DARK);
	this->brush_hilite = new SolidBrush(CLR_WHITE);

	this->color_fore = Color(255,255,255,255);
	this->color_back = Color(255,0,0,0);

	this->buttonPopup = NULL;
	this->tooltipControler = NULL;
}

ToolwCC::~ToolwCC()
{
	this->destroy();
}

void ToolwCC::destroy()
{
	delete this->toolHand;
	delete this->toolZoom;
	delete this->toolPencil;
	delete this->toolLine;
	delete this->toolRect;
	delete this->toolElli;
	delete this->toolRRec;
	delete this->toolMove;
	delete this->toolSelectRect;
	delete this->toolSelectCirc;
	delete this->toolSelectHor;
	delete this->toolSelectVer;
	delete this->toolSelectPoly;
	delete this->toolSelectFree;
	delete this->toolSampleColor;
	delete this->toolCrop;
	delete this->toolSelectWand;
	delete this->toolBucket;
	delete this->toolText;
	delete this->toolEraser;

	delete this->toolCut;
	delete this->toolCopy;
	delete this->toolCopyMerged;
	delete this->toolPaste;
	delete this->toolClear;

	delete this->toolFilter;
	delete this->toolMerge;
	delete this->toolRaster;

	delete this->pen_light;
	delete this->pen_dark;
	delete this->brush_bkgnd;
	delete this->brush_hilite;
}

HWND ToolwCC::initialize()
{
	this->buttons = new List<HWND__>();
	this->buttonHover = NULL;
	this->buttonValidator = NULL;

	this->hToolw = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			IDCL_TOOLW_CC,NULL,
			WS_POPUP | WS_CAPTION | WS_VISIBLE,
			5,TWPOS_CC,56,13 * TW_CC_BUTH,
			this->core->getWindowHandle(),
			NULL,this->core->getInstance(),NULL);

	/*
	this->tooltipControler = CreateWindowEx(
			NULL,
			TOOLTIPS_CLASS,
			NULL,
			TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			this->hToolw,
			NULL,
			NULL,
			NULL
			);
	*/

	if( this->tooltipControler != NULL ){
		SetWindowPos(this->tooltipControler,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SendMessage(this->tooltipControler,TTM_ACTIVATE,TRUE,NULL);
	}

	SetProp(this->hToolw,ATOM_THIS,this);
	SendMessage(this->hToolw,WM_CREATE,NULL,NULL);

	return this->hToolw;
}

Color ToolwCC::getForeColor()
{
	return this->color_fore;
}

Color ToolwCC::getBackColor()
{
	return this->color_back;
}

void ToolwCC::setForeColor(Color color)
{
	this->color_fore = color;
	this->setColor();
}

void ToolwCC::setBackColor(Color color)
{
	this->color_back = color;
	this->setColor();
}

void ToolwCC::setColor()
{
	this->invalidateButton(this->hColorFore);
	this->invalidateButton(this->hColorBack);
}

void ToolwCC::swapColor()
{
	Color temp = this->color_fore;
	this->color_fore = this->color_back;
	this->color_back = temp;

	this->setColor();
}

void ToolwCC::resetColor()
{
	this->color_fore = CLR_WHITE;
	this->color_back = CLR_BLACK;

	this->setColor();
}

Tool *ToolwCC::getTool()
{
	return this->toolCurrent;
}
/*
Set new Tool and inform children about it
*/
void ToolwCC::setTool(Tool *newTool)
{
	Tool *oldTool = this->toolCurrent;
	this->toolCurrent = newTool;

	this->core->getChildren()->gotoHead();
	while( this->core->getChildren()->getThat() != NULL ){
		this->core->getChildren()->getThat()->setTool(newTool);
		if( this->core->getChildren()->next() == false )
			break;
	}
	if( oldTool != NULL ){
		this->uncheckButton(oldTool->getId());
		HWND oldHwnd = GetDlgItem(this->hToolw,oldTool->getId());
		if( oldHwnd != NULL )
			InvalidateRect(oldHwnd,NULL,TRUE);
	}
	if( newTool != NULL ){
		this->checkButton(newTool->getId());
		HWND newHwnd = GetDlgItem(this->hToolw,newTool->getId());
		if( newHwnd != NULL )
			InvalidateRect(newHwnd,NULL,TRUE);
	}
}
/*
Inform children about a double click on the Tool
*/
void ToolwCC::sendToolDblClk(UINT message, WPARAM wParam, LPARAM lParam, int itemId)
{
	bool send = false;
	switch(itemId){
		case ICC_ZOO:
			send = true;
			break;
	}
	if( send == true ){
		ChildCore *child = this->core->getActiveChild();
		if( child != NULL )
			child->sendToolDblClk(message,wParam,lParam,itemId);
	}
}
/*
Create spearator
*/
void ToolwCC::createSepH(HWND hWnd, int x, int y, int w)
{
	CreateWindowEx(
		WS_EX_STATICEDGE,
		IDCL_STATIC,
		NULL,
		WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
		x,y,w,2,
		hWnd,NULL,this->core->getInstance(),NULL);
}
/*
Create Control Center button
*/
HWND ToolwCC::createCC_Button(HWND hWnd, WCHAR *icon, int x, int y, int id)
{
	HWND button = CreateWindowEx(
		NULL,
		IDCL_BUTTON,
		icon,
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		x,y,TW_CC_BUTW,TW_CC_BUTH,
		hWnd,(HMENU)id,
		this->core->getInstance(),NULL);

	if( button != NULL ){
		RECT client;
		GetClientRect(button,&client);

		TOOLINFO ti;
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
		ti.hwnd = this->hToolw;
		ti.hinst = this->core->getInstance();
		ti.uId = id;
		ti.lpszText = LPSTR_TEXTCALLBACK ;

		SetProp(button,ATOM_TTL,this->tooltipControler);
		SetProp(button,ATOM_THIS,this);

		SendMessage(this->tooltipControler,TTM_ADDTOOL,0,(LPARAM)(LPTOOLINFO)&ti);

		this->buttonProc = (WNDPROC)GetWindowLong(button,GWL_WNDPROC);
		SetWindowLong(button,GWL_WNDPROC,(LONG)ToolwCC::processButtons);

		Toolw::tooltipSubclass(button,this->controler->toolIdToName(id));

		this->buttons->add(button);
	}
	return button;
}
/*
Create button for 'more tools...'-styled popup
*/
void ToolwCC::createCC_PopupButton(HWND hWnd, WCHAR *icon, int x, int y, int id)
{
	HWND button = CreateWindowEx(
		NULL,
		IDCL_BUTTON,
		icon,
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		x,y,TW_CC_BUTW,TW_CC_BUTH,
		hWnd,(HMENU)id,
		this->core->getInstance(),NULL);

	if( button != NULL ){
		SetProp(button,ATOM_THIS,this);
		SetProp(button,ATOM_PARENT,hWnd);

		this->popupButtonProc = (WNDPROC)GetWindowLong(button,GWL_WNDPROC);
		SetWindowLong(button,GWL_WNDPROC,(LONG)ToolwCC::processPopupButtons);

		Toolw::tooltipSubclass(button,this->controler->toolIdToName(id));
	}
}
/*
Create 'more tools...'-styled popup
*/
HWND ToolwCC::createCC_Popup(HWND hButton, int x, int y)
{
	RECT client;
	DWORD popupExStyle = NULL;
	DWORD popupStyle = WS_POPUP | WS_BORDER;
	SetRect(
		&client,
		0,0,
		this->getPopupList(hButton)->getCount() * TW_CC_BUTW,
		TW_CC_BUTH
		);

	AdjustWindowRectEx(&client,popupStyle,FALSE,popupExStyle);

	HWND popup = CreateWindowEx(
		popupExStyle,
		IDCL_TOOLW_CCP,
		NULL,
		popupStyle,
		x,y - 1,
		client.right - client.left,
		client.bottom - client.top,
		hButton,
		NULL,
		this->core->getInstance(),
		NULL
		);

	if( popup != NULL ){
		this->buttonPopup = popup;
		this->controler->getToolwset()->add(popup);

		SetProp(popup,ATOM_PARENT,hButton);
		SetProp(popup,ATOM_THIS,this);
		SendMessage(popup,WM_CREATE,NULL,NULL);

		ShowWindow(popup,SW_SHOW);
	}
	return popup;
}
/*
Create the fore- and backcolor buttons
*/
int ToolwCC::createCC_Colorer(HWND hWnd, WCHAR *icon, int y)
{
	RECT client;
	GetClientRect(hWnd,&client);

	int wh = (int)(1.5 * TW_CC_BUTH);
	int xf = 0;
	int yf = y;
	int xb = client.right - wh;
	int yb = yf + (int)(0.5 * wh);

	this->hColorBack = CreateWindowEx(
		NULL,
		IDCL_BUTTON,
		icon,
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		xb,yb,wh,wh,
		hWnd,(HMENU)ICC_COLORBACK,
		this->core->getInstance(),NULL);

	this->hColorFore = CreateWindowEx(
		NULL,
		IDCL_BUTTON,
		icon,
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		xf,yf,wh,wh,
		hWnd,(HMENU)ICC_COLORFORE,
		this->core->getInstance(),NULL);

	int smw = 14;
	int smh = 14;
	HWND button = Dialogs::ownerdrawButton(hWnd,NULL,xb + wh - smw,yf,smw,smh,ICC_COLSWA,FALSE);
	Toolw::tooltipSubclass(button,this->controler->toolIdToName(ICC_COLSWA));

	button = Dialogs::ownerdrawButton(hWnd,NULL,xf,yb + wh - smh,smw,smh,ICC_COLRES,FALSE);
	Toolw::tooltipSubclass(button,this->controler->toolIdToName(ICC_COLRES));

	SetProp(this->hColorBack,ATOM_THIS,this);
	SetProp(this->hColorFore,ATOM_THIS,this);

	return yb - yf + wh;
}

void ToolwCC::paintButton(LPDRAWITEMSTRUCT button)
{
	if( this->buttonValidator != NULL &&
		this->buttonValidator != button->hwndItem )
		return;

	RECT client = button->rcItem;

	Bitmap *bmp = new Bitmap(client.right,client.bottom,Core::getPixelFormat());
	Graphics *g = new Graphics(bmp);

	if( button->itemState & ODS_SELECTED )
		g->Clear(CLR_WHITE);
	else
		g->Clear(CLR_FRAME_LIGHT);

	client.right -= 1;
	client.bottom -= 1;

	Pen *lite = this->pen_light;
	Pen *dark = this->pen_dark;

	if( button->itemState & ODS_SELECTED ){
		lite = this->pen_dark;
		dark = this->pen_light;
	}
	if( (button->itemState & ODS_SELECTED) || this->buttonHover == button->hwndItem ){
		g->DrawLine(lite,client.left,client.top,client.left,client.bottom);
		g->DrawLine(lite,client.left,client.top,client.right,client.top);
		g->DrawLine(dark,client.right,client.top,client.right,client.bottom);
		g->DrawLine(dark,client.right,client.bottom,client.left,client.bottom);
	}

	if( button->CtlID != NULL ){
		if( button->hwndItem == this->hColorFore ){
			SolidBrush *tmp = new SolidBrush(this->color_fore);
			g->FillRectangle(tmp,client.left,client.top,client.right,client.bottom);
			g->DrawRectangle(
				this->pen_light,client.left,client.top,client.right - 1,client.bottom - 1);

			delete tmp;
		}
		else if( button->hwndItem == this->hColorBack ){
			SolidBrush *tmp = new SolidBrush(this->color_back);
			g->FillRectangle(tmp,client.left,client.top,client.right,client.bottom);
			g->DrawRectangle(
				this->pen_light,client.left,client.top,client.right - 1,client.bottom - 1);

			delete tmp;
		}
		else {
			Image *icon = Core::getImageResource(button->CtlID,RC_PNG);
			g->DrawImage(icon,0,0);
			delete icon;

			if( this->getPopupList(button->hwndItem) != NULL ){
				int size = 4;
				Point arrow[3] = {
					Point(client.right,client.bottom - size),
					Point(client.right,client.bottom),
					Point(client.right - size,client.bottom) };

				g->FillPolygon(this->brush_bkgnd,arrow,3);
			}
		}
	}

	Graphics *gdc = new Graphics(button->hDC);
	gdc->DrawImage(bmp,0,0);

	delete gdc;
	delete bmp;
	delete g;

	if( button->itemAction & ODA_SELECT )
		InvalidateRect(button->hwndItem,NULL,TRUE);
}

void ToolwCC::invalidateButton(HWND hItem)
{
	if( hItem == NULL )
		return;

	RECT rect;
	GetClientRect(hItem,&rect);

	this->buttonValidator = hItem;
	InvalidateRect(hItem,&rect,TRUE);
	this->buttonValidator = NULL;
}

void ToolwCC::uncheckButtons(int exceptId)
{
	HWND except = NULL;
	if( exceptId != NULL )
		except = GetDlgItem(this->hToolw,exceptId);

	this->buttons->gotoHead();
	do {
		if( this->buttons->getThat() != except )
			SendMessage(this->buttons->getThat(),BM_SETSTATE,FALSE,NULL);
	} while( this->buttons->next() == true );
}

void ToolwCC::uncheckButton(int id)
{
	this->uncheckButton(GetDlgItem(this->hToolw,id));
}

void ToolwCC::uncheckButton(HWND hItem)
{
	SendMessage(hItem,BM_SETSTATE,FALSE,NULL);
}

void ToolwCC::checkButton(int id)
{
	this->checkButton(GetDlgItem(this->hToolw,id));
}

void ToolwCC::checkButton(HWND hItem)
{
	SendMessage(hItem,BM_SETSTATE,TRUE,NULL);
}

void ToolwCC::clickButton(HWND hItem)
{
	this->clickButton(this->hToolw,hItem);
}
/*
Emulate button click
*/
void ToolwCC::clickButton(HWND hWnd, HWND hItem)
{
	WPARAM wParam = MAKEWPARAM(GetDlgCtrlID(hItem),BN_CLICKED);
	LPARAM lParam = (LPARAM) hItem;

	SendMessage(hWnd,WM_COMMAND,wParam,lParam);
}

void ToolwCC::closePopup()
{
	if( this->buttonPopup != NULL ){
		DestroyWindow(this->buttonPopup);
		this->buttonPopup = NULL;
	}
}
/*
Subscribe a list of buttons for 'more tools...'-styled popup
for a specified button
hButton -	owner (button) of the popup list
list -		the list containing popup button control id and index in list, Point(id,index)
*/
void ToolwCC::addPopupList(HWND hButton, List<Point> *list)
{
	SetProp(hButton,ATOM_POPLIST,list);
}

List<Point> *ToolwCC::getPopupList(HWND hButton)
{
	return (List<Point> *)GetProp(hButton,ATOM_POPLIST);
}

/*
Process normal Control Center buttons
*/
LRESULT CALLBACK ToolwCC::processButtons(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwCC *that = (ToolwCC *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return 0;

	MSG msg;
	RECT tmp;
	POINT pt;
	HWND hTemp;
	HWND hToolTip = (HWND)GetProp(hDlg,ATOM_TTL);

	if( hToolTip != NULL ){
		switch(message){
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MOUSEMOVE:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				msg.hwnd = hDlg;
				msg.message = message;
				msg.wParam = wParam;
				msg.lParam = lParam;

				pt = Core::makePoint(lParam);
				// MapWindowPoints(hDlg,GetParent(hDlg),&pt,1);
				msg.pt = pt;
				msg.time = GetTickCount();

				SendMessage(hToolTip,TTM_RELAYEVENT,NULL,(LPARAM)&msg);
				break;
		}
	}

	switch (message)
	{
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			that->sendToolDblClk(message,wParam,lParam,GetDlgCtrlID(hDlg));
			break;
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
			that->uncheckButtons();
			that->checkButton(hDlg);
			that->clickButton(hDlg);
			that->closePopup();

			if( that->getPopupList(hDlg) != NULL ){
				if( message == WM_RBUTTONDOWN ){
					GetWindowRect(hDlg,&tmp);
					that->buttonPopup = that->createCC_Popup(hDlg,tmp.right,tmp.top);
				}
				else {
					SetTimer(hDlg,TIMER_TCC,TW_CC_POPTOUT,NULL);
				}
			}
			break;
		case WM_LBUTTONUP:
			KillTimer(hDlg,TIMER_TCC);
			break;
		// timer used when LBUTTONMOUSE hold on tool,
		// show 'more tools...'-styled popup list if timout was satisfied
		case WM_TIMER:
			KillTimer(hDlg,TIMER_TCC);
			switch(wParam){
				case TIMER_TCC:
					if( that->buttonPopup == NULL ){
						GetWindowRect(hDlg,&tmp);
						that->buttonPopup = that->createCC_Popup(hDlg,tmp.right,tmp.top);
					}
					break;
			}
			break;
		case WM_MOUSEMOVE:
			if( that->buttonHover != hDlg ){
				hTemp = that->buttonHover;
				that->buttonHover = hDlg;

				that->invalidateButton(hTemp);
				that->invalidateButton(hDlg);
			}
			return CallWindowProc(that->buttonProc,hDlg,message,wParam,lParam);
		default:
			return CallWindowProc(that->buttonProc,hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Process buttons form 'more tools...'-styled popup list
*/
LRESULT CALLBACK ToolwCC::processPopupButtons(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwCC *that = (ToolwCC *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return 0;

	HWND hTemp, hParent;

	switch (message)
	{
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
			that->clickButton((HWND)GetProp(hDlg,ATOM_PARENT),hDlg);
			break;
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			hParent = (HWND)GetProp(hDlg,ATOM_PARENT);
			hTemp = (HWND)GetProp(hParent,ATOM_PARENT);
			if( hTemp != NULL ){
				SetWindowLong(hTemp,GWL_ID,(LONG)GetDlgCtrlID(hDlg));
				SetProp(hTemp,ATOM_TTL,(HANDLE)that->controler->toolIdToName(GetDlgCtrlID(hDlg)));
				that->clickButton(hTemp);
				DestroyWindow(hParent);
			}
			break;
		case WM_MOUSEMOVE:
			if( (wParam & MK_LBUTTON) || (wParam & MK_RBUTTON) )
				that->clickButton((HWND)GetProp(hDlg,ATOM_PARENT),hDlg);

			if( that->buttonHover != hDlg ){
				hTemp = that->buttonHover;
				that->buttonHover = hDlg;

				that->invalidateButton(hTemp);
				that->invalidateButton(hDlg);
			}
			break;
		default:
			return CallWindowProc(that->buttonProc,hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Process the messages of the Tool Window Control Center itself
*/
LRESULT ToolwCC::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwCC *that = (ToolwCC *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	int toolId, offset;
	HWND hTemp, hButton;
	List<Point> *idList;
	NMTTDISPINFO *ttText;

	switch (message)
	{
		// create buttons that represent Tools
		case WM_CREATE:
			offset = 0;

			hButton = that->createCC_Button(hDlg,NULL,0,offset,ICC_SELREC);
			idList = new List<Point>();
			idList->add( new Point(ICC_SELREC,0) );
			idList->add( new Point(ICC_SELCIR,1) );
			idList->add( new Point(ICC_SELHOR,2) );
			idList->add( new Point(ICC_SELVER,3) );
			that->addPopupList(hButton,idList);

			that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_MOV);
			offset += TW_CC_BUTH;
			hButton = that->createCC_Button(hDlg,NULL,0,offset,ICC_LASPOL);
			idList = new List<Point>();
			idList->add( new Point(ICC_LASPOL,0) );
			idList->add( new Point(ICC_LASMOS,1) );
			that->addPopupList(hButton,idList);

			that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_MAG);
			offset += TW_CC_BUTH;
			that->createCC_Button(hDlg,NULL,0,offset,ICC_CRO);
			//that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,NULL);
			offset += TW_CC_BUTH;

			offset += TW_CC_SEP;
			that->createSepH(hDlg,0,offset,3 * TW_CC_BUTW);
			offset += 2 * TW_CC_SEP;

			that->createCC_Button(hDlg,NULL,0,offset,ICC_ERSNOR);
			that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_DRAPEN);
			offset += TW_CC_BUTH;
			that->createCC_Button(hDlg,NULL,0,offset,ICC_FILBUC);
			hButton = that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_SHALIN);
			offset += TW_CC_BUTH;

			idList = new List<Point>();
			idList->add( new Point(ICC_SHALIN,0) );
			idList->add( new Point(ICC_SHAREC,1) );
			idList->add( new Point(ICC_SHACIR,2) );
			//idList->add( new Point(ICC_SHARRC,3) );
			that->addPopupList(hButton,idList);

			//that->createCC_Button(hDlg,NULL,0,offset,NULL);
			//that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,NULL);
			//offset += TW_CC_BUTH;
			//that->createCC_Button(hDlg,NULL,0,offset,NULL);
			//that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,NULL);
			//offset += TW_CC_BUTH;

			offset += TW_CC_SEP;
			that->createSepH(hDlg,0,offset,3 * TW_CC_BUTW);
			offset += 2 * TW_CC_SEP;

			that->createCC_Button(hDlg,NULL,0,offset,ICC_SMPCOL); //ICC_VECSEL
			that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_TEX);
			offset += TW_CC_BUTH;
			//that->createCC_Button(hDlg,NULL,0,offset,ICC_VECDRA);
			//that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,NULL);
			//offset += TW_CC_BUTH;

			offset += TW_CC_SEP;
			that->createSepH(hDlg,0,offset,3 * TW_CC_BUTW);
			offset += 2 * TW_CC_SEP;

			//that->createCC_Button(hDlg,NULL,0,offset,ICC_SMPCOL);
			//that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_ZOO);
			//offset += TW_CC_BUTH;
			that->createCC_Button(hDlg,NULL,0,offset,ICC_HAN);
			that->createCC_Button(hDlg,NULL,TW_CC_BUTW,offset,ICC_ZOO);
			offset += TW_CC_BUTH;

			offset += TW_CC_SEP;
			that->createSepH(hDlg,0,offset,3 * TW_CC_BUTW);
			offset += 2 * TW_CC_SEP;

			offset += that->createCC_Colorer(hDlg,NULL,offset);

			offset += TW_CC_SEP;
			that->createSepH(hDlg,0,offset,3 * TW_CC_BUTW);
			offset += 2 * TW_CC_SEP;

			break;
		case WM_DRAWITEM:
			that->paintButton( (LPDRAWITEMSTRUCT)lParam );
			break;
		// process clicks on buttons
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED){
				toolId = LOWORD(wParam);
                switch(toolId){
					case ICC_SELREC:
						that->setTool(that->toolSelectRect);
						break;
					case ICC_SELCIR:
						that->setTool(that->toolSelectCirc);
						break;
					case ICC_SELHOR:
						that->setTool(that->toolSelectHor);
						break;
					case ICC_SELVER:
						that->setTool(that->toolSelectVer);
						break;
					case ICC_MOV:
						that->setTool(that->toolMove);
						break;
					case ICC_LASPOL:
						that->setTool(that->toolSelectPoly);
						break;
					case ICC_LASMOS:
						that->setTool(that->toolSelectFree);
						break;
					case ICC_MAG:
						that->setTool(that->toolSelectWand);
						break;
					case ICC_CRO:
						that->setTool(that->toolCrop);
						break;
					case ICC_ERSNOR:
						that->setTool(that->toolEraser);
						break;
					case ICC_DRAPEN:
						that->setTool(that->toolPencil);
						break;
					case ICC_FILBUC:
						that->setTool(that->toolBucket);
						break;
					case ICC_SHALIN:
						that->setTool(that->toolLine);
						break;
					case ICC_SHAREC:
						that->setTool(that->toolRect);
						break;
					case ICC_SHACIR:
						that->setTool(that->toolElli);
						break;
					case ICC_SHARRC:
						that->setTool(that->toolRRec);
						break;
					case ICC_TEX:
						that->setTool(that->toolText);
						break;
					case ICC_HAN:
						that->setTool(that->toolHand);
						break;
					case ICC_ZOO:
						that->setTool(that->toolZoom);
						break;
					case ICC_SMPCOL:
						that->setTool(that->toolSampleColor);
						break;
					case ICC_COLORFORE:
						if( that->core->getDialogs()->showDialog(
							(LPCTSTR)IDD_CLR,
							(DLGPROC)Dialogs::processDlg_Clr,
							(LPARAM)&that->color_fore) == YES )
							that->setColor();
						break;
					case ICC_COLORBACK:
						if( that->core->getDialogs()->showDialog(
							(LPCTSTR)IDD_CLR,
							(DLGPROC)Dialogs::processDlg_Clr,
							(LPARAM)&that->color_back) == YES )
							that->setColor();
						break;
					case ICC_COLRES:
						that->resetColor();
						break;
					case ICC_COLSWA:
						that->swapColor();
						break;
					default:
						break;
				}
			}
			break;
		case WM_NOTIFY:
			switch( ((LPNMHDR)lParam)->code ){
				case TTN_SHOW:
					return 0;
				case TTN_GETDISPINFO:
					ttText = (NMTTDISPINFO *)lParam;
					ttText->hinst = NULL;

					switch(ttText->hdr.idFrom){
						case ICC_SELREC:
							ttText->lpszText = that->toolSelectRect->getName()->toWCHAR();
							break;
						case ICC_SELCIR:
							ttText->lpszText = that->toolSelectCirc->getName()->toWCHAR();
							break;
						case ICC_SELHOR:
							ttText->lpszText = that->toolSelectHor->getName()->toWCHAR();
							break;
						case ICC_SELVER:
							ttText->lpszText = that->toolSelectVer->getName()->toWCHAR();
							break;
						case ICC_MOV:
							ttText->lpszText = that->toolMove->getName()->toWCHAR();
							break;
						case ICC_LASPOL:
							ttText->lpszText = that->toolSelectPoly->getName()->toWCHAR();
							break;
						case ICC_LASMOS:
							ttText->lpszText = that->toolSelectFree->getName()->toWCHAR();
							break;
						case ICC_MAG:
							ttText->lpszText = that->toolSelectWand->getName()->toWCHAR();
							break;
						case ICC_CRO:
							ttText->lpszText = that->toolCrop->getName()->toWCHAR();
							break;
						case ICC_ERSNOR:
							ttText->lpszText = that->toolEraser->getName()->toWCHAR();
							break;
						case ICC_DRAPEN:
							ttText->lpszText = that->toolPencil->getName()->toWCHAR();
							break;
						case ICC_FILBUC:
							ttText->lpszText = that->toolBucket->getName()->toWCHAR();
							break;
						case ICC_SHALIN:
							ttText->lpszText = that->toolLine->getName()->toWCHAR();
							break;
						case ICC_SHAREC:
							ttText->lpszText = that->toolRect->getName()->toWCHAR();
							break;
						case ICC_SHACIR:
							ttText->lpszText = that->toolElli->getName()->toWCHAR();
							break;
						case ICC_SHARRC:
							ttText->lpszText = that->toolRRec->getName()->toWCHAR();
							break;
						case ICC_TEX:
							ttText->lpszText = that->toolText->getName()->toWCHAR();
							break;
						case ICC_HAN:
							ttText->lpszText = that->toolHand->getName()->toWCHAR();
							break;
						case ICC_ZOO:
							ttText->lpszText = that->toolZoom->getName()->toWCHAR();
							break;
						case ICC_SMPCOL:
							ttText->lpszText = that->toolSampleColor->getName()->toWCHAR();
							break;
						case ICC_COLORFORE:
							/*if( that->core->getDialogs()->showDialog(
								(LPCTSTR)IDD_CLR,
								(DLGPROC)Dialogs::processDlg_Clr,
								(LPARAM)&that->color_fore) == YES )
								that->setColor();*/
							break;
						case ICC_COLORBACK:
							/*if( that->core->getDialogs()->showDialog(
								(LPCTSTR)IDD_CLR,
								(DLGPROC)Dialogs::processDlg_Clr,
								(LPARAM)&that->color_back) == YES )
								that->setColor();*/
							break;
						case ICC_COLRES:
							//that->resetColor();
							break;
						case ICC_COLSWA:
							//that->swapColor();
							break;
						default:
							break;
					}
			}
			break;
		case WM_NCMOUSEMOVE:
			if( that->buttonHover != NULL ){
				hTemp = that->buttonHover;
				that->buttonHover = NULL;
				
				that->invalidateButton(hTemp);
			}
			break;
		case WM_MOVING:
			that->closePopup();
			return DefWindowProc(hDlg,message,wParam,lParam);
		case WM_NCACTIVATE:
			return that->controler->overrideNCActivate(hDlg,wParam,lParam);
		case WM_DESTROY:
			that->destroy();
			break;
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Process 'more tools...'-styled popup windows
*/
LRESULT ToolwCC::processPopups(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolwCC *that = (ToolwCC *)GetProp(hDlg,ATOM_THIS);
	if( that == NULL )
		return DefWindowProc(hDlg,message,wParam,lParam);

	List<Point> *buttons;
	
	static HWND hLast;
	HWND hTemp;

	switch (message)
	{
		case WM_CREATE:
			buttons = that->getPopupList( (HWND)GetProp(hDlg,ATOM_PARENT) );

			buttons->gotoHead();
			do {
				that->createCC_PopupButton(
					hDlg,
					NULL,
					buttons->getThat()->Y * TW_CC_BUTW,
					0,
					buttons->getThat()->X
					);
			} while( buttons->next() == true );
			break;
		case WM_DRAWITEM:
			that->paintButton( (LPDRAWITEMSTRUCT)lParam );
			break;
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED){
				hTemp = GetDlgItem(hDlg,LOWORD(wParam));
				if( hLast == hTemp )
					return 0;

				buttons = that->getPopupList( (HWND)GetProp(hDlg,ATOM_PARENT) );

				buttons->gotoHead();
				do {
					that->uncheckButton( GetDlgItem(hDlg,buttons->getThat()->X) );
				} while( buttons->next() == true );

				that->checkButton(hTemp);
				hLast = hTemp;
			}
			break;
		case WM_NCACTIVATE:
			return that->controler->overrideNCActivate(hDlg,wParam,lParam);
		case WM_DESTROY:
			that->buttonPopup = NULL;
			that->controler->getToolwset()->remove(hDlg);
			return DefWindowProc(hDlg,message,wParam,lParam);
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}