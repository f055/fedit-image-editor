#include "stdafx.h"
#include "Core.h"

ToolCrop::ToolCrop() : ToolSelecting(new FwCHAR(TOOL_CROP),NULL,ICC_CRO)
{
	SetRect(&this->selection,0,0,0,0);
	this->lastmouse.x = this->lastmouse.y;

	this->cropctrl = NULL;
	this->ctrl = FERROR;

	this->ratio = 1.0;
}

ToolCrop::~ToolCrop()
{
	if( this->cropctrl != NULL )
		delete this->cropctrl;
}

void ToolCrop::deactivate()
{
	this->finalize(true);

	if( this->cropctrl != NULL )
		delete this->cropctrl;
	this->cropctrl = NULL;
}

void ToolCrop::capMouseClientDlbClk(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->finalize();
			break;
	}
}

void ToolCrop::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->beginPaint();

			this->isMoving = false;
			if( this->cropctrl != NULL ){
				this->ctrl = this->getHoverCtrl();

				if( this->ctrl > FERROR )
					this->isMoving = true;
			}
			if( this->isMoving == false ){
				this->isSelecting = true;
				this->ratio = 1.0;

				this->chicore->getDrawer()->setDrawpath(NULL);
				this->setSelection(NULL);

				if( this->cropctrl != NULL )
					delete this->cropctrl;
				this->cropctrl = NULL;

				SetRect(
					&this->selection,
					this->mouse.x,
					this->mouse.y,
					this->mouse.x,
					this->mouse.y
				);
			}
			else {
				this->ratio = 
					(double)(this->selection.right - this->selection.left) / 
					(double)(this->selection.bottom - this->selection.top);

				this->lastmouse = this->mouse;
			}
			break;
	}
}

void ToolCrop::capMouseMove(WPARAM wParam, LPARAM lParam)
{
	if( this->isMoving == true ){
		int dx = this->mouse.x - this->lastmouse.x;
		int dy = this->mouse.y - this->lastmouse.y;

		this->lastmouse = this->mouse;

		switch(this->ctrl){
			case CROP:
				OffsetRect(&this->selection,dx,dy);
				break;
			case CROPTL:
				this->selection.top += dy;
				this->selection.left += dx;
				break;
			case CROPTM:
				this->selection.top += dy;
				break;
			case CROPTR:
				this->selection.top += dy;
				this->selection.right += dx;
				break;
			case CROPML:
				this->selection.left += dx;
				break;
			case CROPMR:
				this->selection.right += dx;
				break;
			case CROPBL:
				this->selection.bottom += dy;
				this->selection.left += dx;
				break;
			case CROPBM:
				this->selection.bottom += dy;
				break;
			case CROPBR:
				this->selection.bottom += dy;
				this->selection.right += dx;
				break;
		}
		this->setCropctrls();
		this->setCropath();
	}
	else if( this->isSelecting == true ){
		this->selection.right = this->mouse.x;
		this->selection.bottom = this->mouse.y;

		this->setCropctrls();
		this->setCropath();
	}

	int ctrl = this->ctrl;
	if( ctrl == FERROR )
		ctrl = this->getHoverCtrl();

	switch(ctrl){
			case CROP:
				this->cursor = LoadCursor(NULL,IDC_SIZEALL);
				break;
			case CROPML:
			case CROPMR:
				this->cursor = LoadCursor(NULL,IDC_SIZEWE);
				break;
			case CROPTR:
			case CROPBL:
				this->cursor = LoadCursor(NULL,IDC_SIZENESW);
				break;
			case CROPTM:
			case CROPBM:
				this->cursor = LoadCursor(NULL,IDC_SIZENS);
				break;
			case CROPTL:
			case CROPBR:
				this->cursor = LoadCursor(NULL,IDC_SIZENWSE);
				break;
			default:
				this->cursor = LoadCursor(NULL,IDC_CROSS);
				break;
	}
}

void ToolCrop::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->endPaint();

			if( this->isMoving == true ){
				this->isMoving = false;
				this->ctrl = FERROR;
			}
			else if( this->isSelecting == true ){
				this->isSelecting = false;
			}
			this->boundRect(&this->selection);
			this->setCropctrls();
			this->setCropath();

			break;
	}
}

void ToolCrop::capKeyDown(WPARAM wParam, LPARAM lParam)
{
	int dx, dy;

	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			if( this->isMoving == false && this->isSelecting == false ){
				this->isAA = true;

				if( this->cropctrl != NULL ){
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
					OffsetRect(&this->selection,dx,dy);
					this->setCropctrls();
					this->setCropath();
				}
			}
			break;
	}
}
void ToolCrop::capKeyUp(WPARAM wParam, LPARAM lParam)
{
	int dx, dy;

	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			if( this->isMoving == false && this->isSelecting == false && this->isAA == false ){
				this->isAA = true;

				if( this->cropctrl != NULL ){
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
					OffsetRect(&this->selection,dx,dy);
					this->setCropctrls();
					this->setCropath();
				}
			}
			this->isAA = false;
			break;
		case VK_RETURN:
			this->finalize();
			break;
	}
}

void ToolCrop::setCropctrls()
{
	this->limitRect(&this->selection);

	if( this->isShift == true ){
			int dx = this->selection.right - this->selection.left;
			int dy = this->selection.bottom - this->selection.top;

			if( this->isSelecting == true ){
				int sy,sx;

				sx = sy = min(abs(dx),abs(dy));

				if( dx >= 0 && dy >= 0 ){
					this->selection.right = this->selection.left + sx;
					this->selection.bottom = this->selection.top + sy;
				}
				else if( dx >= 0 && dy < 0 ){
					this->selection.right = this->selection.left + sx;
					this->selection.bottom = this->selection.top - sy;
				}
				else if( dx < 0 && dy >= 0 ){
					this->selection.right = this->selection.left - sx;
					this->selection.bottom = this->selection.top + sy;
				}
				else if( dx < 0 && dy < 0 ){
					this->selection.right = this->selection.left - sx;
					this->selection.bottom = this->selection.top - sy;
				}
			}
			else {
				double width = this->ratio * dy;
				double height = dx / this->ratio;

				int ww = dx;
				int wh = (int)height;

				int hw = (int)width;
				int hh = dy;

				switch(this->ctrl){
					case CROPTL:
						this->selection.top = this->selection.bottom - wh;
						this->selection.left = this->selection.right - ww;
						break;
					case CROPTM:
						this->selection.top = this->selection.bottom - hh;
						this->selection.right -= (dx - hw);
						break;
					case CROPTR:
						this->selection.top = this->selection.bottom - wh;
						this->selection.right = this->selection.left + ww;
						break;
					case CROPML:
						this->selection.left = this->selection.right - ww;
						this->selection.top += (dy - wh);
						break;
					case CROPMR:
						this->selection.right = this->selection.left + ww;
						this->selection.top += (dy - wh);
						break;
					case CROPBL:
						this->selection.bottom = this->selection.top + wh;
						this->selection.left = this->selection.right - ww;
						break;
					case CROPBM:
						this->selection.bottom = this->selection.top + hh;
						this->selection.left += (dx - hw);
						break;
					case CROPBR:
						this->selection.bottom = this->selection.top + wh;
						this->selection.right = this->selection.left + ww;
						break;
				}
			}				
	}
	RECT rect = this->selection;
	this->boundRect(&rect);

	RectF *selee = new RectF(
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top
		);

	double hs = 3 / this->chicore->getDrawer()->getZoom();
	RectF *ctl = new RectF(rect.left - hs,rect.top - hs,2 * hs, 2 * hs);
	RectF *ctm = new RectF(rect.left + (selee->Width/2) - hs,rect.top - hs,2 * hs, 2 * hs);
	RectF *ctr = new RectF(rect.right - hs,rect.top - hs,2 * hs, 2 * hs);

	RectF *cml = new RectF(rect.left -hs,rect.top + (selee->Height/2) - hs,2 * hs, 2 * hs);
	RectF *cmr = new RectF(rect.right -hs,rect.top + (selee->Height/2) - hs,2 * hs, 2 * hs);

	RectF *cbl = new RectF(rect.left - hs,rect.bottom - hs,2 * hs, 2 * hs);
	RectF *cbm = new RectF(rect.left + (selee->Width/2) - hs,rect.bottom - hs,2 * hs, 2 * hs);
	RectF *cbr = new RectF(rect.right - hs,rect.bottom - hs,2 * hs, 2 * hs);

	if( this->cropctrl != NULL )
		delete this->cropctrl;
	this->cropctrl = new List<RectF>();

	this->cropctrl->add(selee);
	this->cropctrl->add(ctl);
	this->cropctrl->add(ctm);
	this->cropctrl->add(ctr);
	this->cropctrl->add(cml);
	this->cropctrl->add(cmr);
	this->cropctrl->add(cbl);
	this->cropctrl->add(cbm);
	this->cropctrl->add(cbr);
}

void ToolCrop::setCropath()
{
	GraphicsPath *path = new GraphicsPath();
	GraphicsPath *selee = new GraphicsPath();

	this->cropctrl->gotoHead();
	selee->AddRectangle(*this->cropctrl->getThat());
	while( this->cropctrl->next() == true )
		path->AddRectangle(*this->cropctrl->getThat());

	this->chicore->getDrawer()->setDrawpenSize(1);
	this->chicore->getDrawer()->setDrawpath(path,true,false);
	this->setSelection(selee);
}

int ToolCrop::getHoverCtrl()
{
	if( this->cropctrl == NULL )
		return FERROR;

	int ctrl = CROPBR;

	this->cropctrl->gotoTail();
	do {
		if( this->cropctrl->getThat()->Contains((REAL)this->mouse.x,(REAL)this->mouse.y) == TRUE )
			break;
		ctrl--;
	} while( this->cropctrl->prev() == true );

	return ctrl;
}

void ToolCrop::boundRect(RECT *rect)
{
	SetRect(
		rect,
		max( min(rect->left,rect->right), 0 ),
		max( min(rect->top,rect->bottom), 0 ),
		min( max(rect->left,rect->right),
			this->chicore->getWorkspace()->getPxWidth() - 1 ),
		min( max(rect->top,rect->bottom),
			this->chicore->getWorkspace()->getPxHeight() - 1 )
		);
}

void ToolCrop::limitRect(RECT *rect)
{
	SetRect(
		rect,
		max( min(rect->left,this->chicore->getWorkspace()->getPxWidth() - 1), 0 ),
		max( min(rect->top,this->chicore->getWorkspace()->getPxHeight() - 1), 0 ),
		min( max(rect->right,0), this->chicore->getWorkspace()->getPxWidth() - 1 ),
		min( max(rect->bottom,0), this->chicore->getWorkspace()->getPxHeight() - 1 )
		);
}

void ToolCrop::finalize(bool prompt, bool isyes)
{
	int result = (isyes == true) ? IDYES : IDNO;

	if( this->cropctrl == NULL )
		return;

	if( prompt == true ){
		result = this->core->messageBox_Prompt(MESSAGE_CROP);
	}
	switch(result){
		case IDYES:
			this->boundRect(&this->selection);
			this->chicore->getWorkspace()->resizeCanvas(this->selection,false,this);
		case IDNO:
			this->chicore->getDrawer()->setDrawpath(NULL);
			this->setSelection(NULL);

			delete this->cropctrl;
			this->cropctrl = NULL;
			break;
		case IDCANCEL:
			break;
	}
}