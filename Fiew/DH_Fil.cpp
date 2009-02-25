/*
DH_Fil.cpp
DialogHelper object that controls the matrix filters dialog
*/

#include "stdafx.h"
#include "Core.h"

DialogHelper_Fil *DialogHelper_Fil::that = NULL;

DialogHelper_Fil::DialogHelper_Fil(HWND hWnd, ToolFilter::Info *fi)
{
	DialogHelper_Fil::that = this;

	this->windowHandle = hWnd;
	this->fi = fi;

	this->isMoving = false;
	this->isTracking = false;

	this->fiThumb.bmpEffect = NULL;
	this->fiThumb.bmpSource = fi->bmpSource;
	this->fiThumb.bw = fi->bw;
	this->fiThumb.bwalpha = fi->bwalpha;
	this->fiThumb.edgeTrace = fi->edgeTrace;

	this->fiThumb.filterId = fi->filterId;
	this->fiThumb.filterValue = fi->filterValue;
	this->fiThumb.filterByValue = fi->filterByValue;
	this->fiThumb.matrix = fi->matrix;

	this->fiThumb.maxVal = fi->maxVal;
	this->fiThumb.minVal = fi->minVal;
	this->fiThumb.smooth = false;

	SetWindowText(hWnd,ToolFilter::getFilterName(this->fi->filterId));
	SetWindowText(GetDlgItem(hWnd,IDC_STATIC_NAME),ToolFilter::getFilterName(this->fi->filterId));

	CheckDlgButton(hWnd,IDC_CHECK_BW,(fi->bw == true) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd,IDC_CHECK_BWA,(fi->bwalpha == true) ? BST_CHECKED : BST_UNCHECKED);

	if( this->fi->filterByValue == true ){
		Dialogs::limitInputInt(
			GetDlgItem(hWnd,IDC_VALUE),
			this->fi->minVal,
			this->fi->maxVal);
		SetDlgItemInt(hWnd,IDC_VALUE,this->fi->filterValue,FALSE);

		Dialogs::limitSliderInt(
			GetDlgItem(hWnd,IDC_VALUE),
			GetDlgItem(hWnd,IDC_SLIDER),
			this->fi->minVal,
			this->fi->maxVal,
			this->fi->filterValue);

		ShowWindow(GetDlgItem(hWnd,IDC_PREVIEW),SW_HIDE);
	}
	else {
		ShowWindow(GetDlgItem(hWnd,IDC_STATIC_VALUELABEL),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_VALUE),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_SLIDER),SW_HIDE);
	}

	for( int i = IDC_EDIT1; i <= IDC_EDIT25; i++ ){
		HWND hEdit = GetDlgItem(this->windowHandle,i);

		if( this->fi->filterByValue == true ){
			SendMessage(hEdit,EM_SETREADONLY,(WPARAM)TRUE,NULL);
		}
		else {
			Dialogs::limitInputInt(hEdit,MATRIXMIN,MATRIXMAX);
		}
	}
	int cmxsize = 5;
	this->customMatrixIds = ToolFilter::allocMatrix(cmxsize,cmxsize);

	int cmxcounter = IDC_EDIT1;
	for( int x = 0; x < cmxsize; x++ )
		for( int y = 0; y < cmxsize; y++ )
			this->customMatrixIds[x][y] = cmxcounter++;

	this->hThumb = GetDlgItem(this->windowHandle,IDC_THUMB);
	this->procThumb = (WNDPROC)GetWindowLong(this->hThumb,GWL_WNDPROC);
	SetWindowLong(this->hThumb,GWL_WNDPROC,(LONG)DialogHelper_Fil::procNewThumb);

	int w = fi->bmpSource->GetWidth();
	int h = fi->bmpSource->GetHeight();

	RECT client;
	GetClientRect(this->hThumb,&client);

	int dw = (w - client.right) / 2;
	int dh = (h - client.bottom) / 2;
	
	this->clip = Rect(
		max(dw,0),
		max(dh,0),
		min(w,client.right),
		min(h,client.bottom)
		);

	GetWindowRect(this->hThumb,&client);
	MapWindowPoints(NULL,this->windowHandle,(LPPOINT)&client,2);
	SetWindowPos(
		this->hThumb,
		NULL,
		client.left - min(dw,0),
		client.top - min(dh,0),
		this->clip.Width,
		this->clip.Height,
		SWP_FRAMECHANGED |
		SWP_NOZORDER
		);

	RECT scrl;
	SetRect(&scrl,0,0,0,0);
	SetRect(&client,0,0,w,h);

	this->grid = new Gridlay(Dialogs::core->getActiveChild());
	this->grid->preload(client,scrl,0,0,w,h);

	this->setTextMatrix();
	this->renderThumb();
}

DialogHelper_Fil::~DialogHelper_Fil()
{
	SetWindowLong(this->hThumb,GWL_WNDPROC,(LONG)this->procThumb);

	if( this->fiThumb.bmpEffect != NULL )
		delete this->fiThumb.bmpEffect;

	if( this->grid != NULL )
		delete this->grid;

	this->setThumb(NULL);
}
/*
Change in value of the 'Value' edit box notification.
Update the ToolFilter::Info structures and reapply filtering
onto the preview
*/
void DialogHelper_Fil::valueChange(int ctrlId)
{
	int value;
	ToolFilter::Matrix newMatrix;

	switch(ctrlId){
		case IDC_VALUE:
			value = GetDlgItemInt(this->windowHandle,ctrlId,NULL,FALSE);
			newMatrix = ToolFilter::allocateMatrix(this->fi->filterId,value);
			this->fi->matrix = newMatrix;
			this->fi->filterValue = value;

			this->fiThumb.matrix = newMatrix;
			this->fiThumb.filterValue = value;

			this->setTextMatrix();
			this->renderThumb();
			break;
		default:
			if( this->fi->filterByValue == false )
				EnableWindow(GetDlgItem(this->windowHandle,IDC_PREVIEW),TRUE);
			break;
	}
}
/*
Checkbutton click/change notification.
Update the ToolFilter::Info structure and refilter the preview
*/
void DialogHelper_Fil::checkChange(int ctrlId)
{
	ToolFilter::Matrix newMatrix;
	bool state;

	switch(ctrlId){
		// black & white on/off
		case IDC_CHECK_BW:
			state = IsDlgButtonChecked(this->windowHandle,ctrlId);
			this->fi->bw = state;
			this->fiThumb.bw = state;

			this->renderThumb();
			break;
		// use alpha channel on/off
		case IDC_CHECK_BWA:
			state = IsDlgButtonChecked(this->windowHandle,ctrlId);
			this->fi->bwalpha = state;
			this->fiThumb.bwalpha = state;

			this->renderThumb();
			break;
		// get the custom matrix from edit boxes, generate new preview
		case IDC_PREVIEW:
			newMatrix = this->getTextMatrix();

			this->fi->matrix = newMatrix;
			this->fiThumb.matrix = newMatrix;

			if( this->fi->filterByValue == false )
				EnableWindow(GetDlgItem(this->windowHandle,IDC_PREVIEW),FALSE);

			this->renderThumb();
			break;
	}
}
/*
Clear custom matrix edit boxes
*/
void DialogHelper_Fil::clearTextMatrix()
{
	for( int id = IDC_EDIT1; id <= IDC_EDIT25; id++ )
		SetWindowText(GetDlgItem(this->windowHandle,id),NULL);
}
/*
Fill custom matrix edit boxes with current matrix
*/
void DialogHelper_Fil::setTextMatrix()
{
	this->clearTextMatrix();

	int size = 5;
	int si, sj;

	int n = min(this->fi->matrix.mxw,size);
	int nn = min(this->fi->matrix.mxh,size);

	if( n != nn )
	{
		si = (int)((size - n)/2.0);
		sj = (int)((size - nn)/2.0);
	}
	else 
	{
		si = sj = (int)(ceil((size - n)/2.0));
	}

	for(int i = 0; i < n; i++){
		for(int j = 0; j < nn; j++){
			SetDlgItemInt(
				this->windowHandle,
				this->customMatrixIds[i + si][j + sj],
				this->fi->matrix.matrix[i][j],
				TRUE
				);
		}
	}
}
/*
Retrive a ToolFilter::Matrix object from the current
values of custom matrix edit boxes
*/
ToolFilter::Matrix DialogHelper_Fil::getTextMatrix()
{
	int size = 5;

	int n = size;
	int nn = size;

	int si, sj;
	si = sj = -1;

	int cid, val, **matrix = ToolFilter::allocMatrix(n,nn);

	cid = IDC_EDIT1;
	for(int i = 0; i < n; i++){
		for(int j = 0; j < nn; j++){
			val = GetDlgItemInt(this->windowHandle,cid,NULL,TRUE);
			matrix[i][j] = val;
			cid++;
		}
	}
	ToolFilter::Matrix mx;
	mx.bias = 0;
	mx.division = 1;
	mx.matrix = matrix;
	mx.mxw = n;
	mx.mxh = nn;

	return mx;
}
/*
Render the thumbnail image.
The rendering depends on the state of the image,
if the user is scrolling the preview no filtering is
applied for improved performance, when the user
is not scrolling the thumbnail the preview is filtered
with the current filter matrix
*/
void DialogHelper_Fil::renderThumb()
{
	int shx = 0;
	int shy = 0;
	int multi = 1;

	Bitmap *thumb;
	if( this->isMoving == true ){
		thumb = new Bitmap(this->clip.Width,this->clip.Height,this->fiThumb.bmpSource->GetPixelFormat());
		Graphics *g = Graphics::FromImage(thumb);

		g->DrawImage(
			this->fiThumb.bmpSource,
			Rect(0,0,this->clip.Width,this->clip.Height),
			this->clip.X,
			this->clip.Y,
			this->clip.Width,
			this->clip.Height,
			UnitPixel
			);
		delete g;
	}
	else {
		int w = floor(this->fiThumb.matrix.mxw / 2.0);
		int h = floor(this->fiThumb.matrix.mxh / 2.0);

		shx = max( (this->clip.X - w) , 0 ) - this->clip.X;
		shy = max( (this->clip.Y - h) , 0 ) - this->clip.Y;

		if( this->fiThumb.filterId == ID_FILTER_GAUSSIANBLUR ){
			multi = 6;
		}

		Rect subclip(this->clip.X,this->clip.Y,this->clip.Width,this->clip.Height);
		subclip.Width -= 2 * shx * multi;
		subclip.Height -= 2 * shy * multi;

		ToolFilter::applyFilter(&this->fiThumb,&subclip);

		thumb = this->fiThumb.bmpEffect;
		this->fiThumb.bmpEffect = NULL;
	}
	Bitmap *render = new Bitmap(thumb->GetWidth(),thumb->GetHeight(),thumb->GetPixelFormat());
	Graphics *gfx = Graphics::FromImage(render);

	if( this->grid != NULL ){
		gfx->DrawImage(
			this->grid->render(),
			Rect(-shx/multi,-shy/multi,this->clip.Width,this->clip.Height),
			this->clip.X,
			this->clip.Y,
			this->clip.Width,
			this->clip.Height,
			UnitPixel
			);
	}
	if( thumb != NULL ){
		gfx->DrawImage(thumb,shx,shy,thumb->GetWidth(),thumb->GetHeight());
		delete thumb;
	}
	delete gfx;
	thumb = render;

	this->setThumb(thumb);

	if( thumb != NULL )
		delete thumb;
}
/*
Upload the bitmap onto the thumbnail static control
*/
void DialogHelper_Fil::setThumb(Bitmap *bmp)
{
	HBITMAP hBmp = NULL;
	HBITMAP hTemp = NULL;

	if( bmp != NULL )
		bmp->GetHBITMAP(NULL,&hBmp);

	hTemp = (HBITMAP)SendMessage(
		this->hThumb,
		STM_SETIMAGE,
		(WPARAM)IMAGE_BITMAP,
		(LPARAM)hBmp
		);
	if( hTemp != NULL )
		DeleteObject(hTemp);
}
/*
Preview thumbnail procedure that tracks preview scrolling
*/
LRESULT CALLBACK DialogHelper_Fil::procNewThumb(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DialogHelper_Fil *that = DialogHelper_Fil::that;
	if( that == NULL )
		return 0;

	POINTS mouse;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = hDlg;

	switch(message){
		case WM_LBUTTONDOWN:
			that->isMoving = true;
			SetCapture(hDlg);

			that->lastMouse = MAKEPOINTS(lParam);
			that->renderThumb();
		case WM_MOUSEMOVE:
			if( that->isMoving == true ){
				mouse = MAKEPOINTS(lParam);

				int dx = mouse.x - that->lastMouse.x;
				int dy = mouse.y - that->lastMouse.y;

				int nx = that->clip.X - dx;
				int ny = that->clip.Y - dy;

				int mxw = that->fiThumb.bmpSource->GetWidth() - that->clip.Width;
				int mxh = that->fiThumb.bmpSource->GetHeight() - that->clip.Height;

				if( nx >= 0 || ny >= 0 || nx <= mxw || ny <= mxh ){
					that->clip.X = max( min(nx,mxw) , 0 );
					that->clip.Y = max( min(ny,mxh) , 0 );

					that->renderThumb();
				}
				that->lastMouse = mouse;
			}
			if( that->isTracking == false ){
				that->isTracking = (bool)TrackMouseEvent(&tme);
			}
			Dialogs::core->getGui()->setCursor(IDC_HAND);
			break;
		case WM_LBUTTONUP:
			that->isMoving = false;
			ReleaseCapture();

			that->renderThumb();
			break;
		case WM_MOUSELEAVE:
			that->isTracking = false;
			Dialogs::core->getGui()->setCursor();
			break;
		default:
			return CallWindowProc(that->procThumb,hDlg,message,wParam,lParam);
	}
	return 0;
}