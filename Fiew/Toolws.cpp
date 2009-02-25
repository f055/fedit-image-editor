/*
Toolws.cpp
This object represents the controler of Tool Windows and is the connection
between the ToolWindows and the Core of the application or MDI children.
This object is not a strict connection, that is some messages can be passed
from Tool Windows to Core or children without the controler, however not the
other way around.
*/

#include "stdafx.h"
#include "Core.h"

int Toolws::boxCounter = NULL;

Toolws::Toolws(Core *core)
{
	this->core = core;
	this->toolwset = new List<HWND__>();

	this->nameColRes = new FwCHAR(BUT_COLRES);
	this->nameColCho = new FwCHAR(BUT_COLCHOOSE);
	this->nameColSwap = new FwCHAR(BUT_COLSWAP);
	this->nameDelHis = new FwCHAR(BUT_DELHIS);
	this->nameDelLay = new FwCHAR(BUT_DELLAY);
	this->nameLayDown = new FwCHAR(BUT_LAYDOWN);
	this->nameLayUp = new FwCHAR(BUT_LAYUP);
	this->nameNewLay = new FwCHAR(BUT_NEWLAY);
	this->nameRedo = new FwCHAR(BUT_REDO);
	this->nameUndo = new FwCHAR(BUT_UNDO);
}

Toolws::~Toolws()
{
	delete this->nameColRes;
	delete this->nameColCho;
	delete this->nameColSwap;
	delete this->nameDelHis;
	delete this->nameDelLay;
	delete this->nameLayDown;
	delete this->nameLayUp;
	delete this->nameNewLay;
	delete this->nameRedo;
	delete this->nameUndo;
}

void Toolws::show_CC()
{
	this->toolwDock = new ToolwDock(this->core,this);
	this->toolwset->add( this->toolwDock->initialize() );

	this->toolwCC = new ToolwCC(this->core,this);
	this->toolwset->add( this->toolwCC->initialize() );

	this->toolwBox = new ToolwBox(this->core,this);
	this->toolwset->add( this->toolwBox->getWindowHandle() );
	Toolws::boxCounter++;

	this->toolwBoxLayers = new ToolwBoxLayers(this->toolwBox);

	this->toolwBox = new ToolwBox(this->core,this,true);
	this->toolwset->add( this->toolwBox->getWindowHandle() );
	Toolws::boxCounter++;

	this->toolwBoxInfo = new ToolwBoxInfo(this->toolwBox);

	this->toolwBox = new ToolwBox(this->core,this);
	this->toolwset->add( this->toolwBox->getWindowHandle() );
	Toolws::boxCounter++;

	this->toolwBoxHistory = new ToolwBoxHistory(this->toolwBox);
}
void Toolws::hide(HWND toolw)
{
	this->toolwset->remove(toolw);
}

void Toolws::enableToolwBoxes()
{
	this->toolwBoxLayers->load();
	this->toolwBoxHistory->load();
}

void Toolws::disableToolwBoxes()
{
	this->toolwBoxInfo->disable();
	this->toolwBoxLayers->disable();
	this->toolwBoxHistory->disable();
}

void Toolws::toggleBoxInfo()
{
	int cmd = SW_SHOW;
	if( IsWindowVisible(this->toolwBoxInfo->getBox()->getWindowHandle()) )
		cmd = SW_HIDE;

	ShowWindow(this->toolwBoxInfo->getBox()->getWindowHandle(),cmd);
	this->core->getGui()->updateMenu();
}
void Toolws::toggleBoxLayers()
{
	int cmd = SW_SHOW;
	if( IsWindowVisible(this->toolwBoxLayers->getBox()->getWindowHandle()) )
		cmd = SW_HIDE;

	ShowWindow(this->toolwBoxLayers->getBox()->getWindowHandle(),cmd);
	this->core->getGui()->updateMenu();
}
void Toolws::toggleBoxHistory()
{
	int cmd = SW_SHOW;
	if( IsWindowVisible(this->toolwBoxHistory->getBox()->getWindowHandle()) )
		cmd = SW_HIDE;

	ShowWindow(this->toolwBoxHistory->getBox()->getWindowHandle(),cmd);
	this->core->getGui()->updateMenu();
}

List<HWND__> *Toolws::getToolwset()
{
	return this->toolwset;
}

ToolwCC *Toolws::getToolwCC()
{
	return this->toolwCC;
}

ToolwDock *Toolws::getToolwDock()
{
	return this->toolwDock;
}

ToolwBoxLayers *Toolws::getToolwBoxLayers()
{
	return this->toolwBoxLayers;
}

ToolwBoxInfo *Toolws::getToolwBoxInfo()
{
	return this->toolwBoxInfo;
}

ToolwBoxHistory *Toolws::getToolwBoxHistory()
{
	return this->toolwBoxHistory;
}
/*
These two methods ensure that the Tool Windows title bar
remains active even upon logical deactivation, and becomes
inactive in proper moments (like dialog boxes)
*/
LRESULT Toolws::overrideEnable(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	this->toolwset->gotoHead();
	do {
		if( this->toolwset->getThat() != hWnd )
			EnableWindow(this->toolwset->getThat(),(BOOL)wParam);
	} while( this->toolwset->next() != NULL );

	return DefWindowProc(hWnd, WM_ENABLE, wParam, lParam);
}
LRESULT Toolws::overrideNCActivate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hParam = (HWND)lParam;
	BOOL fKeepActive = (BOOL)wParam;
	BOOL fSyncOthers = TRUE;

	if( hParam == (HWND)-1 ){
		SendMessage(this->core->getMdiclientHandle(),WM_NCACTIVATE,wParam,lParam);
		return DefWindowProc(hWnd,WM_NCACTIVATE,fKeepActive,0);
	}
	if( hParam == this->core->getWindowHandle() ){
		fKeepActive = TRUE;
		fSyncOthers = TRUE;
	}
	this->toolwset->gotoHead();
	do {
		if( this->toolwset->getThat() == hParam ){
			fKeepActive = TRUE;
			fSyncOthers = FALSE;
			break;
		}
	} while( this->toolwset->next() != NULL );

	if( fSyncOthers == TRUE ){
		this->toolwset->gotoHead();
		do {
			if( this->toolwset->getThat() != hParam && 
				this->toolwset->getThat() != hWnd )
				SendMessage(this->toolwset->getThat(),WM_NCACTIVATE,fKeepActive,(LONG)-1);
			
		} while( this->toolwset->next() != NULL );

		SendMessage(this->core->getWindowHandle(),WM_NCACTIVATE,fKeepActive,(LONG)-1);
	}
	SendMessage(this->core->getMdiclientHandle(),WM_NCACTIVATE,fKeepActive,lParam);
	return DefWindowProc(hWnd, WM_NCACTIVATE, fKeepActive, lParam);
}

FwCHAR *Toolws::toolIdToName(int toolId)
{
	FwCHAR *result = NULL;

	switch(toolId){
		case ICC_SELREC:
			result = this->toolwCC->toolSelectRect->getName();
			break;
		case ICC_SELCIR:
			result = this->toolwCC->toolSelectCirc->getName();
			break;
		case ICC_SELHOR:
			result = this->toolwCC->toolSelectHor->getName();
			break;
		case ICC_SELVER:
			result = this->toolwCC->toolSelectVer->getName();
			break;
		case ICC_MOV:
			result = this->toolwCC->toolMove->getName();
			break;
		case ICC_LASPOL:
			result = this->toolwCC->toolSelectPoly->getName();
			break;
		case ICC_LASMOS:
			result = this->toolwCC->toolSelectFree->getName();
			break;
		case ICC_MAG:
			result = this->toolwCC->toolSelectWand->getName();
			break;
		case ICC_CRO:
			result = this->toolwCC->toolCrop->getName();
			break;
		case ICC_ERSNOR:
			result = this->toolwCC->toolEraser->getName();
			break;
		case ICC_DRAPEN:
			result = this->toolwCC->toolPencil->getName();
			break;
		case ICC_FILBUC:
			result = this->toolwCC->toolBucket->getName();
			break;
		case ICC_SHALIN:
			result = this->toolwCC->toolLine->getName();
			break;
		case ICC_SHAREC:
			result = this->toolwCC->toolRect->getName();
			break;
		case ICC_SHACIR:
			result = this->toolwCC->toolElli->getName();
			break;
		case ICC_SHARRC:
			result = this->toolwCC->toolRRec->getName();
			break;
		case ICC_TEX:
			result = this->toolwCC->toolText->getName();
			break;
		case ICC_HAN:
			result = this->toolwCC->toolHand->getName();
			break;
		case ICC_ZOO:
			result = this->toolwCC->toolZoom->getName();
			break;
		case ICC_SMPCOL:
			result = this->toolwCC->toolSampleColor->getName();
			break;
		case ICC_COLORFORE:
			break;
		case ICC_COLORBACK:
			break;
		case ICC_COLRES:
			result = this->nameColRes;
			break;
		case ICC_COLSWA:
			result = this->nameColSwap;
			break;
		default:
			break;
	}
	return result;
}