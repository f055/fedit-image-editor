#include "stdafx.h"
#include "Core.h"

ToolSelectWand::ToolSelectWand() : ToolSelecting(new FwCHAR(TOOL_MAG),NULL,ICC_MAG)
{
	this->selectBmp = NULL;
	this->sourceBmp = NULL;

	this->dlgTolerance = NULL;
	this->dlgSlider = NULL;
	this->tolerance = 10;

	this->cursor = Tool::createToolCursor(10,7,ICC_MAG);
	this->cursorBackup = this->cursor;

	this->fillDock();
}

ToolSelectWand::~ToolSelectWand()
{
	if( this->selectBmp != NULL )
		delete this->selectBmp;

	DestroyIcon(this->cursorBackup);
}

void ToolSelectWand::fillDock()
{
	this->core->CreateWindowExSubstituteFont(
		NULL,
		IDCL_STATIC,
		DOCKDLG_TOLERANCE,
		WS_CHILD | WS_VISIBLE | SS_SIMPLE,
		0,3,60,20,
		this->hdocktool,
		NULL,this->core->getInstance(),NULL
		);

	this->dlgTolerance = this->core->CreateWindowExSubstituteFont(
		WS_EX_CLIENTEDGE,
		IDCL_EDIT,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
		60,0,30,20,
		this->hdocktool,
		(HMENU)111,this->core->getInstance(),NULL
		);
	Dialogs::limitInputInt(this->dlgTolerance,0,255);
	SetDlgItemInt(this->hdocktool,GetDlgCtrlID(this->dlgTolerance),this->tolerance,FALSE);

	this->dlgSlider = CreateWindowEx(
		NULL,
		IDCL_SLIDER,
		NULL,
		WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_NOTICKS,
		100,2,120,20,
		this->hdocktool,
		(HMENU)222,this->core->getInstance(),NULL
		);

	Dialogs::limitSliderInt(this->dlgTolerance,this->dlgSlider,0,255,this->tolerance);
}

void ToolSelectWand::notify(int id)
{
	switch(id){
		case 111:
			this->tolerance = GetDlgItemInt(this->hdocktool,id,NULL,FALSE);
			if( this->tolerance != SendMessage(this->dlgSlider,TBM_GETPOS,NULL,NULL) )
				SendMessage(this->dlgSlider,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)this->tolerance);
			break;
	}
}

void ToolSelectWand::capMouseDown(WPARAM wParam, LPARAM lParam, int button)
{
	switch(button){
		case LEFT:
			this->sourceBmp = this->chicore->getWorkspace()->getSelectedLayerProjection();

			if( this->selectBmp != NULL )
				delete this->selectBmp;
			this->selectBmp = new Bitmap(
				this->sourceBmp->GetWidth(),
				this->sourceBmp->GetHeight(),
				this->sourceBmp->GetPixelFormat()
				);

			this->setOldPath();

			Frame *select = this->chicore->getWorkspace()->getSelectedLayer();
			this->setSelection(
				ToolSelectWand::doMagic(
					this->sourceBmp,
					this->selectBmp,
					this->mouse.x,
					this->mouse.y,
					0, //select->getX(),
					0, //select->getY()
					this->tolerance),
				true,
				this->isCtrl
				);
			this->resetOldPath();

			this->chicore->getDrawer()->invalidate();

			delete this->selectBmp;
			delete this->sourceBmp;

			this->selectBmp = NULL;
			this->sourceBmp = NULL;
	}
}

void ToolSelectWand::capMouseMove(WPARAM wParam, LPARAM lParam)
{
}

void ToolSelectWand::capMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
}

GraphicsPath *ToolSelectWand::doMagic(Bitmap *sourceBmp, Bitmap *select, int x, int y, int shx, int shy, int tol)
{
	//x -= shx;
	//y -= shy;

	if( x < 0 || y < 0 || x >= (int)sourceBmp->GetWidth() || y >= (int)sourceBmp->GetHeight() )
		return NULL;

	HCURSOR oldCursor = Core::self->getGui()->setCursor(CURSOR_WAIT);

	Rect lockRect(0,0,sourceBmp->GetWidth(),sourceBmp->GetHeight());
	Bitmap *selectBmp = select;
	if( select == NULL )
		selectBmp = new Bitmap(lockRect.Width,lockRect.Height,sourceBmp->GetPixelFormat());

	UINT *src0, *sel0, piCol;
	BitmapData srcData, selData;

	sourceBmp->LockBits(
			&lockRect,
			ImageLockModeRead,
			sourceBmp->GetPixelFormat(),
			&srcData
			);
	selectBmp->LockBits(
			&lockRect,
			ImageLockModeWrite,
			selectBmp->GetPixelFormat(),
			&selData
			);
	src0 = (UINT *)srcData.Scan0;
	sel0 = (UINT *)selData.Scan0;

	piCol =	src0[y * srcData.Stride / 4 + x];
	UINT tolerance = tol;

	int x1, y1, w, h; 
    BOOL spanLeft, spanRight;
	List<Point> *stack = new List<Point>();
	List<Point> *historyStack = new List<Point>();

	List<Point> *leftStack = new List<Point>();
	List<Point> *rightStack = new List<Point>();
	Point *pop;
    
	stack->add(new Point(x,y));
	historyStack->add(stack->getTail());

	leftStack->add(stack->getTail());

	w = sourceBmp->GetWidth();
	h = sourceBmp->GetHeight();	

	while( stack->getCount() > 0 )
    {
		pop = stack->removeTail();
		x1 = pop->X;
		y1 = pop->Y;

        while( doMagicComparison(piCol,src0[max(y1,0) * srcData.Stride / 4 + x1],tolerance) && y1 >= 0 ) y1--;
        y1++;
        spanLeft = spanRight = 0;
        while( doMagicComparison(piCol,src0[min(y1,h - 1) * srcData.Stride / 4 + x1],tolerance) && y1 < h)
        {
			sel0[y1 * selData.Stride / 4 + x1] = Core::RGBtoUINT(0,0,0);
            if( !spanLeft && x1 > 0 && doMagicComparison(piCol,src0[y1 * srcData.Stride / 4 + x1 - 1],tolerance) ) 
            {
				if( ToolSelectWand::isonStack(historyStack,x1 - 1,y1) == false ){
					stack->add(new Point(x1 - 1,y1));
					historyStack->add(stack->getTail());

					leftStack->add(stack->getTail());
				}
                spanLeft = 1;
            }
            else if(spanLeft && x1 > 0 &&
				!doMagicComparison(piCol,src0[y1 * srcData.Stride / 4 + x1 - 1],tolerance) )
            {
                spanLeft = 0;
            }
            if(!spanRight && x1 < w - 1 && doMagicComparison(piCol,src0[y1 * srcData.Stride / 4 + x1 + 1],tolerance) ) 
            {
				if( ToolSelectWand::isonStack(historyStack,x1 + 1,y1) == false ){
					stack->add(new Point(x1 + 1, y1));
					historyStack->add(stack->getTail());

					leftStack->add(stack->getTail());
				}
                spanRight = 1;
            }
            else if(spanRight && x1 < w - 1 &&
				!doMagicComparison(piCol,src0[y1 * srcData.Stride / 4 + x1 + 1],tolerance) )
            {
                spanRight = 0;
            } 
            y1++;                   
        }
    }
	sourceBmp->UnlockBits(&srcData);
	selectBmp->UnlockBits(&selData);

	GraphicsPath *path = ToolSelectWand::doMagicSelection(selectBmp);

	if( shx != 0 || shy != 0 ){
		Matrix matrix;
		matrix.Translate((REAL)shx,(REAL)shy);

		path->Transform(&matrix);
	}

	delete historyStack;
	if( select == NULL )
		delete selectBmp;

	Core::self->getGui()->setCursor(oldCursor);
	return path;
}

GraphicsPath *ToolSelectWand::doMagicSelection(Bitmap *sourceBmp)
{
	Rect lockRect(0,0,sourceBmp->GetWidth(),sourceBmp->GetHeight());

	UINT *src0;
	BitmapData srcData;
	GraphicsPath *path = new GraphicsPath();

	sourceBmp->LockBits(
			&lockRect,
			ImageLockModeRead,
			sourceBmp->GetPixelFormat(),
			&srcData
			);
	src0 = (UINT *)srcData.Scan0;

	int x, y, w, h;
	w = sourceBmp->GetWidth();
	h = sourceBmp->GetHeight();

	Rect tempRect(0,0,0,0);
	bool rectorize = false;

	for( x = 0; x < w; x++ ){
		for( y = 0; y < h; y++ ){
			if( src0[y * srcData.Stride / 4 + x] != NULL ){
				if( rectorize == false ){
					rectorize = true;
					tempRect = Rect(x,y,1,1);
				}
				else {
					tempRect.Height++;
				}
			}
			else {
				if( rectorize == true ){
					rectorize = false;
					path->AddRectangle(tempRect);
				}
			}
		}
		if( rectorize == true ){
			rectorize = false;
			path->AddRectangle(tempRect);
		}
	}
	sourceBmp->UnlockBits(&srcData);

	path->Outline();

	return path;
}

bool ToolSelectWand::doMagicComparison(UINT pick, UINT trick, UINT tolerance)
{
	if( pick == NULL && trick == NULL )
		return true;
	if( pick == NULL || trick == NULL )
		return false;

	Color p = Core::UINTtoRGB(pick);
	Color t = Core::UINTtoRGB(trick);

	int drm = p.GetR() - tolerance;
	int drp = p.GetR() + tolerance;
	int dgm = p.GetG() - tolerance;
	int dgp = p.GetG() + tolerance;
	int dbm = p.GetB() - tolerance;
	int dbp = p.GetB() + tolerance;

	if( max(drm,0) > t.GetR() || min(drp,255) < t.GetR() )
		return false;
	if( max(dgm,0) > t.GetG() || min(dgp,255) < t.GetG() )
		return false;
	if( max(dbm,0) > t.GetB() || min(dbp,255) < t.GetB() )
		return false;

	return true;
}

bool ToolSelectWand::isonStack(List<Point> *stack, int x, int y)
{
	if( stack->getCount() <= 0 )
		return false;

	stack->gotoHead();
	do {
		if( stack->getThat()->X == x && stack->getThat()->Y == y )
			return true;
	} while( stack->next() == true );

	return false;
}