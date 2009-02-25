/*
ToolwBoxLayers.cpp
This object represent the content of 'Layers' Tool Window.
It manages rendering of the representation of the current state of the layers
in the selected Workspace, and also passes messages to the Workspace upon
user interaction with 'Layers' Tool Window
*/

#include "stdafx.h"
#include "Core.h"

ToolwBoxLayers::ToolwBoxLayers(ToolwBox *box) : ToolwBoxContent(box,new FwCHAR(TOOLWBOX_LAYERS))
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	this->lastId = NULL;
	this->minId = NULL;
	this->maxId = NULL;

	// create top content pane
	this->topMargin = 20;
	this->hTopContents = this->createContent(
		0,
		0,
		client.right,
		this->topMargin
		);

	this->botMargin = 15;
	this->botButCount = 4;

	// create bottom buttons' content pane
	this->hBotContents = this->createContent(
		client.right - 2 * this->botMargin * this->botButCount - this->botMargin - 1,
		client.bottom - this->botMargin,
		2 * this->botButCount * this->botMargin,
		this->botMargin
		);

	// create scrollable layers content pane
	this->layersView = new ToolwBoxScroll(
		this->hToolw,
		0,
		this->topMargin,
		client.right,
		client.bottom - this->topMargin - this->botMargin,
		false,
		true
		);

	// for future use
	this->hComboStyle = NULL;/*this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_COMBO,
		NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
		0,
		0,
		90,
		20,
		this->hTopContents,
		NULL,
		this->core->getInstance(),
		NULL
		);*/

	// create lock checkbox
	this->hCheckLock = this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		TOOLWBOX_LAYERS_LOCK,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		0,
		0,
		90,
		20,
		this->hTopContents,
		(HMENU)22,
		this->core->getInstance(),
		NULL
		);
	EnableWindow(this->hCheckLock,FALSE);

	// create opacity controls
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		TOOLWBOX_LAYERS_OPACITY,
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		100,
		3,
		50,
		20,
		this->hTopContents,
		NULL,
		this->core->getInstance(),
		NULL
		);

	this->hInputOpacity = this->core->CreateWindowExSubstituteFont(
		WS_EX_CLIENTEDGE,
		IDCL_EDIT,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
		150,
		0,
		41,
		20,
		this->hTopContents,
		(HMENU)33,
		this->core->getInstance(),
		NULL
		);
	Dialogs::limitInputInt(this->hInputOpacity,0,100);
	EnableWindow(this->hInputOpacity,FALSE);

	// create buttons for bottm buttons' content pane 
	HWND button = Dialogs::ownerdrawButton(
		this->hBotContents,
		NULL,
		0,
		0,
		2 * this->botMargin,
		this->botMargin,
		ITC_DOWN,
		FALSE
		);
	Toolw::tooltipSubclass(button,this->controler->nameLayDown);

	button = Dialogs::ownerdrawButton(
		this->hBotContents,
		NULL,
		2 * this->botMargin,
		0,
		2 * this->botMargin,
		this->botMargin,
		ITC_UP,
		FALSE
		);
	Toolw::tooltipSubclass(button,this->controler->nameLayUp);

	button = Dialogs::ownerdrawButton(
		this->hBotContents,
		NULL,
		4 * this->botMargin,
		0,
		2 * this->botMargin,
		this->botMargin,
		ITC_NEW,
		FALSE
		);
	Toolw::tooltipSubclass(button,this->controler->nameNewLay);

	button = Dialogs::ownerdrawButton(
		this->hBotContents,
		NULL,
		6 * this->botMargin,
		0,
		2 * this->botMargin,
		this->botMargin,
		ITC_DEL,
		FALSE
		);
	Toolw::tooltipSubclass(button,this->controler->nameDelLay);

	this->layerContents = NULL;
}

ToolwBoxLayers::~ToolwBoxLayers()
{
	
}
/*
Load the representation of the layers of the current Workspace.
Each layer is represented by one button responsible for visibility,
one button responsible for thumbnail and one editable static text
control responsible for the name of the layer
*/
void ToolwBoxLayers::load()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		EnableWindow(this->hCheckLock,TRUE);
		EnableWindow(this->hInputOpacity,TRUE);

		this->layerContents = this->layersView->unplugContent();

		int x = 0;
		int y = 1;
		int height = LAYTHUMBSIZE;
		List<Frame> *layers = child->getWorkspace()->getLayers();

		int hshift = 0;
		int vshift = - child->getWorkspace()->getSelectedLayerIndex() * height;

		this->layerContents = this->layersView->createContent(
			this,
			NULL,
			layers->getCount() * height + layers->getCount() + 1,
			hshift,
			vshift,
			false,
			true
			);

		RECT client;
		GetClientRect(this->layerContents,&client);

		int firstId = 1000;
		int idJump = IDJUMP;

		this->minId = firstId;

		layers->gotoTail();
		do {
			HWND button = this->core->CreateWindowExSubstituteFont(
				NULL,
				IDCL_BUTTON,
				L"V",
				WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | BS_OWNERDRAW,
				x,
				y,
				height,// /2,
				height,
				this->layerContents,
				(HMENU)(firstId),
				this->core->getInstance(),
				NULL
				);
			SetProp(button,ATOM_OWNER,layers->getThat());
			Toolw::globalSubclass(button);

			button = this->core->CreateWindowExSubstituteFont(
				NULL,
				IDCL_BUTTON,
				layers->getThat()->getName()->toWCHAR(),
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | BS_OWNERDRAW,
				x + height,
				y,
				2 * height,
				height,
				this->layerContents,
				(HMENU)(firstId + 2),
				this->core->getInstance(),
				NULL
				);
			SetProp(button,ATOM_OWNER,layers->getThat());
			Toolw::globalSubclass(button);

			button = this->core->CreateWindowExSubstituteFont(
				NULL,
				IDCL_STATIC,
				layers->getThat()->getName()->toWCHAR(),
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | 
				SS_NOTIFY | SS_LEFT | SS_CENTERIMAGE | SS_WORDELLIPSIS,
				x + 3 * height,
				y,
				client.right - 3 * height,
				height,
				this->layerContents,
				(HMENU)(firstId + 3),
				this->core->getInstance(),
				NULL
				);
			SetProp(button,ATOM_OWNER,layers->getThat());
			Dialogs::editStaticText(button,x + 3 * height,y + height/2 - LAYNAMEMARGIN);

			y += height + 1;
			firstId += idJump;

		} while( layers->prev() == true );

		this->maxId = firstId;

		this->layerContents = this->layersView->plugContent();
		this->recontrol();
	}
	else {
		this->layerContents = this->layersView->clearContent();
		this->disable();
	}
}

void ToolwBoxLayers::refresh()
{
	if( this->layerContents != NULL )
		InvalidateRect(this->layerContents,NULL,TRUE);

	this->recontrol();
}
void ToolwBoxLayers::refreshCurrent()
{
	HWND handle = NULL;
	if( (handle = GetDlgItem(this->layerContents,this->lastId)) != NULL )
		InvalidateRect(handle,NULL,TRUE);
}
/*
Change the lock state and opacity value when user changes the selected layer
*/
void ToolwBoxLayers::recontrol()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		Frame *frame = child->getWorkspace()->getSelectedLayer();

		SetDlgItemInt(this->hTopContents,33,frame->getOpacity(),FALSE);
		CheckDlgButton(this->hTopContents,22,(frame->getIsLocked() == true) ? BST_CHECKED : BST_UNCHECKED);
	}
}

void ToolwBoxLayers::resizeContent(int dw, int dh)
{
	RECT content;
	GetWindowRect(this->hToolw,&content);

	SetWindowPos(
		this->hToolw,
		NULL,
		0,0,
		content.right - content.left + dw,
		content.bottom - content.top + dh,
		SWP_NOZORDER | 
		SWP_FRAMECHANGED
		);

	RECT client;
	GetClientRect(this->hToolw,&client);

	HDWP rc = BeginDeferWindowPos(3);

	rc = DeferWindowPos(
		rc,
		this->hTopContents,
		NULL,
		0,
		0,
		NULL,
		NULL,
		SWP_NOZORDER | SWP_NOSIZE |
		SWP_FRAMECHANGED
		);
	rc = DeferWindowPos(
		rc,
		this->layersView->getWindowHandle(),
		NULL,
		0,
		this->topMargin,
		client.right,
		client.bottom - this->topMargin - this->botMargin,
		SWP_NOZORDER | 
		SWP_FRAMECHANGED
		);
	rc = DeferWindowPos(
		rc,
		this->hBotContents,
		NULL,
		client.right - 2 * this->botMargin * this->botButCount - this->botMargin,
		client.bottom - this->botMargin,
		NULL,
		NULL,
		SWP_NOZORDER | SWP_NOSIZE |
		SWP_FRAMECHANGED
		);

	EndDeferWindowPos(rc);
	SendMessage(this->layersView->getWindowHandle(),WM_SIZE,NULL,NULL);

	if( dw != 0 ){
		GetClientRect(this->layerContents,&client);
		rc = BeginDeferWindowPos((this->maxId - this->minId) / IDJUMP);

		for( int i = this->minId; i < this->maxId; i += IDJUMP ){
			rc = DeferWindowPos(
				rc,
				GetDlgItem(this->layerContents,i + 3),
				NULL,
				NULL,
				NULL,
				client.right - 3 * LAYTHUMBSIZE,
				LAYTHUMBSIZE,
				SWP_NOZORDER | SWP_NOMOVE |
				SWP_FRAMECHANGED
				);
		}
		EndDeferWindowPos(rc);

		HWND handle = NULL;
		for( int i = this->minId; i < this->maxId; i += IDJUMP ){
			if( (handle = GetDlgItem(this->layerContents,i + 3)) != NULL )
				InvalidateRect(handle,NULL,TRUE);
		}
	}
}

void ToolwBoxLayers::disable()
{
	this->layerContents = this->layersView->clearContent();

	SetWindowText(this->hInputOpacity,NULL);

	EnableWindow(this->hCheckLock,FALSE);
	EnableWindow(this->hInputOpacity,FALSE);
}

void ToolwBoxLayers::paintButton(LPDRAWITEMSTRUCT button)
{
	if( this->core->getActiveChild() == NULL )
		return;

	RECT client = button->rcItem;

	Frame *owner = (Frame *)GetProp(button->hwndItem,ATOM_OWNER );
	if( owner == NULL )
		return;

	Bitmap *bmp = new Bitmap(client.right,client.bottom,Core::getPixelFormat());
	Graphics *g = Graphics::FromImage(bmp);

	switch(button->CtlID % IDJUMP){
		case 0:
			DrawEdge(button->hDC,&client,BDR_SUNKENOUTER,BF_RIGHT | BF_FLAT);

			if( owner->getIsVisible() == true ){
				SolidBrush *brush = new SolidBrush(CLR_FRAME_DARK);

				int size = 5;
				Rect fill(
					client.right/2 - size,
					client.bottom/2 - size,
					2 * size,
					2 * size
					);

				g->FillEllipse(brush,fill);

				delete brush;
			}
			break;
		case 2:
			if( this->core->getActiveChild()->getWorkspace()->getSelectedLayer() == owner ){
				this->lastId = button->CtlID;
				g->Clear(CLR_CAPTION_ACTIV);
			}
			else {
				g->Clear(CLR_FRAME_LIGHT);
			}

			int frame = 1;
			int rectangle = LAYTHUMBSIZE - 6;
			int thbSize = rectangle - 2 * frame;

			Bitmap *img = owner->getThumb(thbSize,thbSize);
			Rect target = Rect(3 + frame,3 + frame,thbSize,thbSize);
			
			g->DrawImage(
				img,
				target,
				0,0,
				img->GetWidth(),
				img->GetHeight(),
				UnitPixel);

			Pen *pen = new Pen(CLR_BLACK,1);
			g->DrawRectangle(pen,3,3,rectangle - 1,rectangle - 1);

			delete pen;
			break;
	}

	Graphics *gfx = Graphics::FromHDC(button->hDC);
	gfx->DrawImage(bmp,0,0,bmp->GetWidth(),bmp->GetHeight());

	delete g;
	delete gfx;
	delete bmp;
}

LRESULT ToolwBoxLayers::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int toolId;

	HDC hdc;
	Frame *owner;
	FrameText *ownerT;
	ChildCore *child;

	LPDRAWITEMSTRUCT drawitem;
	WCHAR *string;

	switch(message){
		case WM_CREATE:
			break;
		case WM_DRAWITEM:
			drawitem = (LPDRAWITEMSTRUCT)lParam;

			if( drawitem->CtlID % IDJUMP != 2 )
				Dialogs::paintButton(drawitem);
			this->paintButton(drawitem);
			break;
		case WM_NOTIFY:
			SendMessage(hDlg,WM_COMMAND,MAKEWPARAM((int)wParam,BN_CLICKED),NULL);
			break;
		case WM_CTLCOLORSTATIC:
			toolId = GetDlgCtrlID((HWND)lParam);
			owner = (Frame *)GetProp((HWND)lParam,ATOM_OWNER);
			if( owner != NULL ){
				child = this->core->getActiveChild();
				hdc = (HDC)wParam;

				if( toolId % IDJUMP == 3 )
					SetBkMode(hdc,TRANSPARENT);

				if( child->getWorkspace()->getSelectedLayer() == owner ){
					SetDCBrushColor(hdc,GetSysColor(COLOR_ACTIVECAPTION));
					SetTextColor(hdc,GetSysColor(COLOR_HIGHLIGHTTEXT));
				}
				else {
					SetDCBrushColor(hdc,GetSysColor(COLOR_BTNFACE));
					SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));					
				}

				return (LRESULT)GetStockObject(DC_BRUSH);
			}
			return DefWindowProc(hDlg,message,wParam,lParam);

		case WM_COMMAND:
			switch(HIWORD(wParam)){
			case STN_DBLCLK:
				// pass the doubleclick for the in-place editing of static text
				SendMessage((HWND)lParam,WM_LBUTTONDBLCLK,wParam,lParam);
				break;
			case EN_UPDATED:
				// finalize the editing of static text
				owner = (Frame *)GetProp((HWND)lParam,ATOM_OWNER);
				if( owner != NULL ){
					string = new WCHAR[MAX_PATH + 1];

					GetWindowText((HWND)lParam,string,MAX_PATH);

					owner->setName(new FwCHAR(string));

					delete string;
				}				
				break;
			case BN_CLICKED:
				toolId = LOWORD(wParam);
				child = this->core->getActiveChild();
				if( child != NULL ){
					switch(toolId){
						// down arrow - move layer down
						case ITC_DOWN:
							if( child->getWorkspace()->moveSelectedDown() == true )
								this->load();
							break;
						// up arrow - move layer up
						case ITC_UP:
							if( child->getWorkspace()->moveSelectedUp() == true )
								this->load();
							break;
						// new file - create new layer
						case ITC_NEW:
							child->getWorkspace()->addLayerAfter();
							this->recontrol();
							break;
						// trash bin - delete current layer
						case ITC_DEL:
							child->getWorkspace()->deleteLayer(
								child->getWorkspace()->getSelectedLayer() );
							this->recontrol();
							break;
						// lock checbox - toggle lock
						case 22:
							child->getWorkspace()->getSelectedLayer()->toggleLock();
							break;
						// opacity editbox - change opacity
						case 33:
							child->getWorkspace()->getSelectedLayer()->setOpacity(
								GetDlgItemInt(this->hTopContents,33,NULL,FALSE) );
							child->getWorkspace()->update();
							break;
						// set new selected layer
						default:
							owner = (Frame *)GetProp(GetDlgItem(this->layerContents,toolId),ATOM_OWNER);
							if( owner != NULL ){
								switch(toolId % IDJUMP){
									case 0:
										owner->toggleVisibility();
										child->getWorkspace()->update();

										InvalidateRect(GetDlgItem(this->layerContents,toolId),NULL,TRUE);
										InvalidateRect(GetDlgItem(this->layerContents,toolId + 2),NULL,TRUE);
										InvalidateRect(GetDlgItem(this->layerContents,toolId + 3),NULL,TRUE);
										break;
									case 3:
										toolId -= 1;
									case 2:
										child->getWorkspace()->setSelectedLayer(owner);

										InvalidateRect(GetDlgItem(this->layerContents,this->lastId),NULL,TRUE);
										InvalidateRect(GetDlgItem(this->layerContents,this->lastId + 1),NULL,TRUE);

										InvalidateRect(GetDlgItem(this->layerContents,toolId),NULL,TRUE);
										InvalidateRect(GetDlgItem(this->layerContents,toolId + 1),NULL,TRUE);

										if( toolId != this->lastId && owner->getType() == FRM_TEXT )
											this->controler->getToolwCC()->toolText->updateDock();

										this->lastId = toolId;

										
										this->recontrol();
										break;
								}
							}
							break;
					}
				}
				break;
			// double click
			case BN_DBLCLK:
				toolId = LOWORD(wParam);
				child = this->core->getActiveChild();
				if( child != NULL ){
					switch(toolId){
						case FERROR:
							break;
						// if currently selected is FrameText - begin editing it
						default:
							owner = (Frame *)GetProp(GetDlgItem(this->layerContents,toolId),ATOM_OWNER);
							if( owner != NULL ){
								switch(toolId % IDJUMP){
									case 3:
										toolId -= 1;
									case 2:
										if( owner->getType() == FRM_TEXT ){
											ownerT = (FrameText *)owner;
											this->controler->getToolwCC()->setTool(
												this->controler->getToolwCC()->toolText );
											((ToolText *)this->controler->getToolwCC()->toolText)->editText(ownerT);
										}
										break;
								}
							}
							break;
					}
				}
				break;
			}
			break;
		default:
			return DefWindowProc(hDlg,message,wParam,lParam);
	}
	return 0;
}