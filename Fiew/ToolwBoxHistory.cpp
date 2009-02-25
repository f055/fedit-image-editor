/*
ToolwBoxHistory.cpp
This object represents the content of the 'History' Tool Window.
It manages rendering of the representation of the current state of the history
in the selected Workspace, and also passes messages to the Workspace upon
user interaction with 'History' Tool Window
*/

#include "stdafx.h"
#include "Core.h"

ToolwBoxHistory::ToolwBoxHistory(ToolwBox *box) : ToolwBoxContent(box,new FwCHAR(TOOLWBOX_HISTORY))
{
	RECT client;
	GetClientRect(this->hToolw,&client);

	this->lastId = NULL;
	this->minId = NULL;
	this->maxId = NULL;

	this->botMargin = 15;
	this->botButCount = 3;

	// create bottom buttons' content pane
	this->hBotContents = this->createContent(
		client.right - 2 * this->botMargin * this->botButCount - this->botMargin - 1,
		client.bottom - this->botMargin,
		2 * this->botButCount * this->botMargin,
		this->botMargin
		);

	// create scrollable history content pane
	this->historyView = new ToolwBoxScroll(
		this->hToolw,
		0,
		0,
		client.right,
		client.bottom - this->botMargin,
		false,
		true
		);

	// add buttons to buttons' content pane
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
	Toolw::tooltipSubclass(button,this->controler->nameRedo);

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
	Toolw::tooltipSubclass(button,this->controler->nameUndo);

	button = Dialogs::ownerdrawButton(
		this->hBotContents,
		NULL,
		4 * this->botMargin,
		0,
		2 * this->botMargin,
		this->botMargin,
		ITC_DEL,
		FALSE
		);
	Toolw::tooltipSubclass(button,this->controler->nameDelHis);

	this->historyContents = NULL;
}

ToolwBoxHistory::~ToolwBoxHistory()
{
	
}
/*
Load the History contents from the current Workspace.
The method goes through all HistoryElements of current
HistoryControler creating appropriate child windows
in the scrollable content pane that represent each HistoryElement
*/
void ToolwBoxHistory::load()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		this->historyContents = this->historyView->unplugContent();

		int x = 0;
		int y = 1;
		int height = HISTHUMBSIZE;
		List<HistoryElement> *elems = child->getWorkspace()->getHistory()->getHistoryElems();

		int hshift = 0;
		int vshift = - elems->countLeftoThat() * height;

		int elemsCount = elems->getCount();

		this->historyContents = this->historyView->createContent(
			this,
			NULL,
			elemsCount * height + elemsCount + 1,
			hshift,
			vshift,
			false,
			true
			);

		RECT client;
		GetClientRect(this->historyContents,&client);

		int firstId = 1000;
		int idJump = IDJUMP;

		this->minId = firstId;

		HistoryElement *pointOfNoReturn = elems->getThat();

		elems->gotoHead();
		do {
			HWND button = this->core->CreateWindowExSubstituteFont(
				NULL,
				IDCL_BUTTON,
				elems->getThat()->getName()->toWCHAR(),
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | BS_OWNERDRAW,
				x,
				y,
				height,
				height,
				this->historyContents,
				(HMENU)(firstId),
				this->core->getInstance(),
				NULL
				);
			SetProp(button,ATOM_OWNER,elems->getThat());
			Toolw::globalSubclass(button);

			button = this->core->CreateWindowExSubstituteFont(
				NULL,
				IDCL_STATIC,
				elems->getThat()->getName()->toWCHAR(),
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | 
				SS_NOTIFY | SS_CENTER | SS_CENTERIMAGE | SS_WORDELLIPSIS,
				x + height,
				y,
				client.right - height,
				height,
				this->historyContents,
				(HMENU)(firstId + 1),
				this->core->getInstance(),
				NULL
				);
			SetProp(button,ATOM_OWNER,elems->getThat());
			Toolw::globalSubclass(button);

			y += height + 1;
			firstId += idJump;
		} while( elems->next() == true );
		elems->gotoThat(pointOfNoReturn);

		this->maxId = firstId;

		this->historyContents = this->historyView->plugContent();
		this->recontrol();
	}
	else {
		this->historyContents = this->historyView->clearContent();
		this->disable();
	}
}
/*
Invalidate all windows representing HistoryElements
*/
void ToolwBoxHistory::refresh()
{
	HWND handle = NULL;
	for( int i = this->minId; i < this->maxId; i += IDJUMP ){
		if( (handle = GetDlgItem(this->historyContents,i + 1)) != NULL )
			InvalidateRect(handle,NULL,TRUE);
	}

	this->recontrol();
}

void ToolwBoxHistory::recontrol()
{

}

void ToolwBoxHistory::resizeContent(int dw, int dh)
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
		this->historyView->getWindowHandle(),
		NULL,
		0,
		0,
		client.right,
		client.bottom - this->botMargin,
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
	SendMessage(this->historyView->getWindowHandle(),WM_SIZE,NULL,NULL);

	if( dw != 0 ){
		GetClientRect(this->historyContents,&client);
		rc = BeginDeferWindowPos((this->maxId - this->minId) / IDJUMP);

		for( int i = this->minId; i < this->maxId; i += IDJUMP ){
			rc = DeferWindowPos(
				rc,
				GetDlgItem(this->historyContents,i + 1),
				NULL,
				NULL,
				NULL,
				client.right - HISTHUMBSIZE,
				HISTHUMBSIZE,
				SWP_NOZORDER | SWP_NOMOVE |
				SWP_FRAMECHANGED
				);
		}
		EndDeferWindowPos(rc);

		HWND handle = NULL;
		for( int i = this->minId; i < this->maxId; i += IDJUMP ){
			if( (handle = GetDlgItem(this->historyContents,i + 1)) != NULL )
				InvalidateRect(handle,NULL,TRUE);
		}
	}
}

void ToolwBoxHistory::disable()
{
	this->historyContents = this->historyView->clearContent();
}

void ToolwBoxHistory::paintButton(LPDRAWITEMSTRUCT button)
{
	if( this->core->getActiveChild() == NULL )
		return;

	RECT client = button->rcItem;

	HistoryElement *owner = (HistoryElement *)GetProp(button->hwndItem,ATOM_OWNER );
	if( owner == NULL )
		return;

	Bitmap *bmp = new Bitmap(client.right,client.bottom,Core::getPixelFormat());
	Graphics *g = Graphics::FromImage(bmp);

	switch(button->CtlID % IDJUMP){
		case 0:
			if( this->core->getActiveChild()->getWorkspace()->getHistory()->getHistoryElems()->getThat() == owner ){
				this->lastId = button->CtlID;
				//g->Clear(CLR_CAPTION_ACTIV);
			}

			Image *img = Core::getImageResource(owner->getId(),RC_PNG);
			if( img != NULL ){
				g->DrawImage(
					img,
					Rect(0,0,img->GetWidth(),img->GetHeight()),
					0,0,
					img->GetWidth(),
					img->GetHeight(),
					UnitPixel);
				delete img;
			}
			break;
	}

	Graphics *gfx = Graphics::FromHDC(button->hDC);
	gfx->DrawImage(bmp,0,0,bmp->GetWidth(),bmp->GetHeight());

	delete g;
	delete gfx;
	delete bmp;
}

LRESULT ToolwBoxHistory::processMessages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int toolId;

	HDC hdc;
	HistoryElement *owner;
	ChildCore *child;

	LPDRAWITEMSTRUCT drawitem;

	switch(message){
		case WM_CREATE:
			break;
		case WM_DRAWITEM:
			drawitem = (LPDRAWITEMSTRUCT)lParam;


			Dialogs::paintButton(drawitem);
			this->paintButton(drawitem);
			break;
		case WM_NOTIFY:
			SendMessage(hDlg,WM_COMMAND,MAKEWPARAM((int)wParam,BN_CLICKED),NULL);
			break;
		case WM_CTLCOLORSTATIC:		
			toolId = GetDlgCtrlID((HWND)lParam);
			owner = (HistoryElement *)GetProp((HWND)lParam,ATOM_OWNER);
			if( owner != NULL ){
				child = this->core->getActiveChild();
				HistoryControler *hc = child->getWorkspace()->getHistory();
				hdc = (HDC)wParam;

				if( toolId % IDJUMP == 1 )
					SetBkMode(hdc,TRANSPARENT);

				if( hc->getHistoryElems()->getThat() == owner ){
					SetDCBrushColor(hdc,GetSysColor(COLOR_ACTIVECAPTION));
					SetTextColor(hdc,GetSysColor(COLOR_HIGHLIGHTTEXT));
				}
				else {
					SetDCBrushColor(hdc,GetSysColor(COLOR_BTNFACE));

					if( hc->getHistoryElems()->getIndex(owner) <= hc->getHistoryElems()->countLeftoThat() )
						SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
					else
						SetTextColor(hdc,GetSysColor(COLOR_BTNSHADOW));
				}

				return (LRESULT)GetStockObject(DC_BRUSH);
			}
			return DefWindowProc(hDlg,message,wParam,lParam);

		case WM_COMMAND:
			switch(HIWORD(wParam)){
			case BN_CLICKED:
				toolId = LOWORD(wParam);
				child = this->core->getActiveChild();
				if( child != NULL ){
					switch(toolId){
						// down arrow clicked - redo
						case ITC_DOWN:
							child->getWorkspace()->getHistory()->redo(true);
							break;
						// up arrow clicked - undo
						case ITC_UP:
							child->getWorkspace()->getHistory()->undo(true);
							break;
						// trash bin clicked - delete elements
						case ITC_DEL:
							child->getWorkspace()->getHistory()->remove();
							break;
						// HistoryElement window clicked - jump to that element
						default:
							owner = (HistoryElement *)GetProp(GetDlgItem(this->historyContents,toolId),ATOM_OWNER);
							if( owner != NULL ){
								switch(toolId % IDJUMP){
									case 1:
										child->getWorkspace()->getHistory()->gotoElement(owner);
										child->getWorkspace()->update();

										Core::self->getToolws()->getToolwBoxLayers()->load();

										//InvalidateRect(GetDlgItem(this->historyContents,this->lastId),NULL,TRUE);
										//InvalidateRect(GetDlgItem(this->historyContents,toolId),NULL,TRUE);

										this->lastId = toolId;
										this->recontrol();
										this->refresh();
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