#include "stdafx.h"
#include "Core.h"

ToolZoom::ToolZoom() : ToolSelecting(new FwCHAR(TOOL_ZOOM),NULL,ICC_ZOO)
{
	this->isSelecting = false;
	this->lastPin = Point(0,0);
	SetRect(&this->selection,0,0,0,0);

	this->isAll = false;

	this->cursor = Tool::createToolCursor(10,8,ICC_ZOO);
	this->cursorBackup = this->cursor;

	this->fillDock();
}

ToolZoom::~ToolZoom()
{
	DestroyIcon(this->cursorBackup);
}

void ToolZoom::fillDock()
{
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		TOOL_ZOOM_FITTO,
		WS_VISIBLE | WS_CHILD,
		0,0,100,20,
		this->hdocktool,
		(HMENU)33,
		this->core->getInstance(),
		NULL
		);

	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		TOOL_ZOOM_RESET,
		WS_VISIBLE | WS_CHILD,
		110,0,100,20,
		this->hdocktool,
		(HMENU)11,
		this->core->getInstance(),
		NULL
		);

	this->dlgAll = this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		TOOL_ZOOM_ALL,
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		220,0,150,20,
		this->hdocktool,
		(HMENU)22,
		this->core->getInstance(),
		NULL
		);
	CheckDlgButton(this->hdocktool,22,BST_UNCHECKED);
}

void ToolZoom::notify(int id)
{
	RECT client;
	double zm;

	switch(id){
		case 11:
			if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
				this->core->getChildren()->gotoHead();
				do {
					this->core->getChildren()->getThat()->setZoom(ZOOMINIT);
				} while( this->core->getChildren()->next() == true );
			}
			else {
				this->core->getActiveChild()->setZoom(ZOOMINIT);
			}
			break;
		case 22:
			this->isAll = (bool)IsDlgButtonChecked(this->hdocktool,22);
			break;
		case 33:
			if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
				this->core->getChildren()->gotoHead();
				do {
					client = this->core->getChildren()->getThat()->getDrawer()->getClientSize();
					zm = this->calcFit(client.right,client.bottom);
					this->core->getChildren()->getThat()->setZoom(zm);
				} while( this->core->getChildren()->next() == true );
			}
			else {
				client = this->core->getActiveChild()->getDrawer()->getClientSize();
				zm = this->calcFit(client.right,client.bottom);
				this->core->getActiveChild()->setZoom(zm);
			}
			break;
	}
}

void ToolZoom::capKeyDown(WPARAM wParam, LPARAM lParam)
{

}

void ToolZoom::capKeyUp(WPARAM wParam, LPARAM lParam)
{

}

void ToolZoom::capMouseDblClk(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
				this->core->getChildren()->gotoHead();
				do {
					this->core->getChildren()->getThat()->setZoom(ZOOMINIT);
				} while( this->core->getChildren()->next() == true );
			}
			else {
				this->chicore->setZoom(ZOOMINIT);
			}
			this->chicore->pinSkew(this->lastPin.X,this->lastPin.Y);
			break;
	}
}

void ToolZoom::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case RIGHT:
			this->isAlt = true;
		case LEFT:
			this->setMouseCapture();

			this->isSelecting = true;

			SetRect(
				&this->selection,
				this->mouse.x,
				this->mouse.y,
				this->mouse.x,
				this->mouse.y
				);
			break;
	}
}

void ToolZoom::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->isSelecting == true ){
		this->selection.right = this->mouse.x;
		this->selection.bottom = this->mouse.y;

		this->setDrawpath();
	}
}

void ToolZoom::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case RIGHT:
			this->isAlt = true;
		case LEFT:
			this->setMouseRelease();

			this->isSelecting = false;
			this->chicore->getDrawer()->setDrawpath(NULL);

			if( this->selection.left == this->selection.right &&
				this->selection.top == this->selection.bottom ){

				if( this->isAlt == false ){
					if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
						this->core->getChildren()->gotoHead();
						do {
							this->core->getChildren()->getThat()->addZoom(ZOOMLOSTEP);
						} while( this->core->getChildren()->next() == true );
					}
					else {
						this->chicore->addZoom(ZOOMLOSTEP);
					}
				}
				else if( this->isAlt == true ){
					if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
						this->core->getChildren()->gotoHead();
						do {
							this->core->getChildren()->getThat()->addZoom(-ZOOMLOSTEP);
						} while( this->core->getChildren()->next() == true );
					}
					else {
						this->chicore->addZoom(-ZOOMLOSTEP);
					}
				}

				this->chicore->pinSkew(this->selection.left,this->selection.top);
				this->lastPin = Point(this->selection.left,this->selection.top);
			}
			else {
				double sw = abs(this->selection.left - this->selection.right);
				double sh = abs(this->selection.top - this->selection.bottom);

				if( sw > ZOOMTOOLIMIT && sh > ZOOMTOOLIMIT ){
					double zm = this->calcZoom(sw,sh,this->chicore->getDrawer()->getZoom());

					if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
						this->core->getChildren()->gotoHead();
						do {
							this->core->getChildren()->getThat()->setZoom(zm);
						} while( this->core->getChildren()->next() == true );
					}
					else {
						this->chicore->setZoom(zm);
					}

					RECT pin;
					SetRect(
						&pin,
						min(this->selection.left,this->selection.right),
						min(this->selection.top,this->selection.bottom),
						max(this->selection.left,this->selection.right),
						max(this->selection.top,this->selection.bottom)
						);

					int pinx = pin.left + (pin.right - pin.left)/2;
					int piny = pin.top + (pin.bottom - pin.top)/2;

					this->chicore->pinSkew(pinx,piny);
					this->lastPin = Point(pinx,piny);
				}
			}
			break;
	}
}

void ToolZoom::setDrawpath()
{
	GraphicsPath *path = new GraphicsPath();

	path->AddRectangle( Rect(
		min(this->selection.left,this->selection.right),
		min(this->selection.top,this->selection.bottom),
		abs(this->selection.left - this->selection.right),
		abs(this->selection.top - this->selection.bottom) ) );

	this->chicore->getDrawer()->setDrawpenSize(1);
	this->chicore->getDrawer()->setDrawpath(path);
}

double ToolZoom::calcZoom(double sw, double sh, double zm)
{
	double ww = this->chicore->getWorkspace()->getPxWidth() * zm;
	double wh = this->chicore->getWorkspace()->getPxHeight() * zm;

	RECT client = this->chicore->getDrawer()->getClientSize();

	double zw = floor(sw * zm / ww);
	if( sw < ww ){
		zw = client.right / sw;
	}

	double zh = floor(sh * zm / wh);
	if( sh < wh ){
		zh = client.bottom / sh;
	}

	if( this->isAlt == false )
		zm += min(zw,zh);
	else
		zm -= min(zw,zh);

	return zm;
}

double ToolZoom::calcFit(double sw, double sh)
{
	double ww = this->chicore->getWorkspace()->getPxWidth();
	double wh = this->chicore->getWorkspace()->getPxHeight();

	double zw = sw / ww;
	double zh = sh / wh;

	return min(zw,zh);
}