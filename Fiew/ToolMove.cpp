#include "stdafx.h"
#include "Core.h"

ToolMove::ToolMove() : Tool(new FwCHAR(TOOL_MOVE),NULL,ICC_MOV)
{
	this->mouseLast.x = this->mouseLast.y = 0;
	this->mouseMoving = false;
	this->keyMoving = false;

	this->cursor = Tool::createToolCursor(4,3,ICC_MOV);
	this->cursorBackup = this->cursor;
}

ToolMove::~ToolMove()
{
	DestroyIcon(this->cursorBackup);
}

void ToolMove::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			if( this->chicore->getWorkspace()->getSelectedLayer()->getIsLocked() == true ||
				this->chicore->getWorkspace()->getSelectedLayer()->getIsVisible() == false )
				return;

			this->setMouseCapture();

			this->mouseLast = this->mouse;
			this->mouseMoving = true;

			if( this->chicore->getWorkspace()->getSelection() != NULL ){
				ToolCopy *cut = (ToolCopy *)this->core->getToolws()->getToolwCC()->toolCut;
				ToolCopy *paste = (ToolCopy *)this->core->getToolws()->getToolwCC()->toolPaste;

				cut->activate();
				paste->activate();				
			}

			this->frameStart.x = this->chicore->getWorkspace()->getSelectedLayer()->getX();
			this->frameStart.y = this->chicore->getWorkspace()->getSelectedLayer()->getY();
			this->frameEnd = this->frameStart;

			break;
	}
}

void ToolMove::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->chicore->getWorkspace()->getSelectedLayer()->getIsLocked() == true ||
		this->chicore->getWorkspace()->getSelectedLayer()->getIsVisible() == false )
		this->loadCursor();
	else
		this->cursor = this->cursorBackup;

	if( this->mouseMoving == true ){
		double zoom = this->chicore->getDrawer()->getZoom();
		int x = (int)((this->mouse.x - this->mouseLast.x) / zoom);
		int y = (int)((this->mouse.y - this->mouseLast.y) / zoom);

		this->chicore->getWorkspace()->getSelectedLayer()->addXY(x,y);
		this->mouseLast = this->mouse;

		if( this->chicore->getWorkspace()->getSelection() != NULL ){
			Matrix off;
			off.Translate(x,y);
			this->chicore->getWorkspace()->getSelection()->Transform(&off);
		}

		this->frameEnd.x = this->chicore->getWorkspace()->getSelectedLayer()->getX();
		this->frameEnd.y = this->chicore->getWorkspace()->getSelectedLayer()->getY();

		this->chicore->getWorkspace()->update();
	}
}

void ToolMove::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->setMouseRelease();

			this->mouseMoving = false;

			int dx = this->frameEnd.x - this->frameStart.x;
			int dy = this->frameEnd.y - this->frameStart.y;
			if( dx != 0 || dy != 0 ){
				HEToolMove *he = new HEToolMove(
					this->chicore->getWorkspace(),
					this->chicore->getWorkspace()->getSelectedLayer(),
					this);
				he->set(dx,dy);

				this->chicore->getWorkspace()->getHistory()->add(he);
			}

			this->chicore->getWorkspace()->updateToolws();
			this->chicore->getWorkspace()->update();
			break;
	}
}

void ToolMove::capKeyDown(WPARAM wParam, LPARAM lParam)
{
	int dx, dy;

	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			if( this->keyMoving == false ){
				this->keyMoving = true;

				if( this->chicore->getWorkspace()->getSelection() != NULL ){
					ToolCopy *cut = (ToolCopy *)this->core->getToolws()->getToolwCC()->toolCut;
					ToolCopy *paste = (ToolCopy *)this->core->getToolws()->getToolwCC()->toolPaste;

					cut->activate();
					paste->activate();				
				}

				this->frameStart.x = this->chicore->getWorkspace()->getSelectedLayer()->getX();
				this->frameStart.y = this->chicore->getWorkspace()->getSelectedLayer()->getY();
				this->frameEnd = this->frameStart;
			}
			else {
				dx = dy = 0;
				switch(wParam){
					case VK_UP:
						dy = -1;
						break;
					case VK_DOWN:
						dy = 1;
						break;
					case VK_LEFT:
						dx = -1;
						break;
					case VK_RIGHT:
						dx = 1;
						break;
				}
				if( this->chicore->getWorkspace()->getSelection() != NULL ){
					Matrix off;
					off.Translate(dx,dy);
					this->chicore->getWorkspace()->getSelection()->Transform(&off);
				}

				this->chicore->getWorkspace()->getSelectedLayer()->addXY(dx,dy);
				this->chicore->getWorkspace()->update();

				this->frameEnd.x += dx;
				this->frameEnd.y += dy;
			}
			break;
	}
}

void ToolMove::capKeyUp(WPARAM wParam, LPARAM lParam)
{
	int dx, dy;

	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
				dx = dy = 0;
				switch(wParam){
					case VK_UP:
						dy = -1;
						break;
					case VK_DOWN:
						dy = 1;
						break;
					case VK_LEFT:
						dx = -1;
						break;
					case VK_RIGHT:
						dx = 1;
						break;
				}
				this->chicore->getWorkspace()->getSelectedLayer()->addXY(dx,dy);
				this->chicore->getWorkspace()->update();

				this->frameEnd.x += dx;
				this->frameEnd.y += dy;

				this->keyMoving = false;

				dx = this->frameEnd.x - this->frameStart.x;
				dy = this->frameEnd.y - this->frameStart.y;
				if( dx != 0 || dy != 0 ){
					HEToolMove *he = new HEToolMove(
						this->chicore->getWorkspace(),
						this->chicore->getWorkspace()->getSelectedLayer(),
						this);
					he->set(dx,dy);

					this->chicore->getWorkspace()->getHistory()->add(he);
				}

				this->chicore->getWorkspace()->updateToolws();
				this->chicore->getWorkspace()->update();
			break;
	}
}