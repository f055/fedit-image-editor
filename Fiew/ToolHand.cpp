#include "stdafx.h"
#include "Core.h"

ToolHand::ToolHand() : Tool(new FwCHAR(TOOL_HAND),NULL,ICC_HAN)
{
	this->mouseLast.x = this->mouseLast.y = 0;
	this->mouseMoving = false;

	this->isAll = false;

	this->cursor = Tool::createToolCursor(12,10,ICC_HAN);
	this->cursorBackup = this->cursor;

	this->fillDock();
}

ToolHand::~ToolHand()
{
	DestroyIcon(this->cursorBackup);
}

void ToolHand::fillDock()
{
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		TOOL_HAND_RESET,
		WS_VISIBLE | WS_CHILD,
		0,0,100,20,
		this->hdocktool,
		(HMENU)11,
		this->core->getInstance(),
		NULL
		);

	this->dlgAll = this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_BUTTON,
		TOOL_HAND_ALL,
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		110,0,150,20,
		this->hdocktool,
		(HMENU)22,
		this->core->getInstance(),
		NULL
		);
	CheckDlgButton(this->hdocktool,22,BST_UNCHECKED);
}

void ToolHand::notify(int id)
{
	switch(id){
		case 11:
			if( this->isAll == true && this->core->getChildren()->getCount() > 0 ){
				this->core->getChildren()->gotoHead();
				do {
					this->core->getChildren()->getThat()->setSkew(0,0);
				} while( this->core->getChildren()->next() == true );
			}
			else {
				this->chicore->setSkew(0,0);
			}
			break;
		case 22:
			this->isAll = IsDlgButtonChecked(this->hdocktool,22);
			break;
	}
}

void ToolHand::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->setMouseCapture();

			this->mouseLast = this->mouse;
			this->mouseMoving = true;
			break;
		case MID:
			break;
		case RIGHT:
			break;
	}
}

void ToolHand::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->mouseMoving == true ){
		int x = this->mouse.x - this->mouseLast.x;
		int y = this->mouse.y - this->mouseLast.y;

		if( this->isAll == true ){
			this->core->getChildren()->gotoHead();
			do {
				this->core->getChildren()->getThat()->addSkew(x,y);
			} while( this->core->getChildren()->next() == true );
		}
		else {
			this->chicore->addSkew(x,y);
		}
		this->mouseLast = this->mouse;
	}
}

void ToolHand::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->setMouseRelease();

			this->mouseMoving = false;
			break;
		case MID:
			break;
		case RIGHT:
			break;
	}
}