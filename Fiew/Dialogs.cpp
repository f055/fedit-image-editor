/*
Dialogs.cpp
One of the major objects in Fedit, consisting of mostly static methods
that create or modify interface controls
*/

#include "stdafx.h"
#include "Core.h"

Core *Dialogs::core = NULL;

Dialogs::Dialogs(Core *core)
{
	Dialogs::core = core;
}

Dialogs::~Dialogs()
{

}

int Dialogs::showDialog(LPCTSTR resource, DLGPROC dlgproc, LPARAM lParam)
{
	return (int)DialogBoxParam(
		this->core->getInstance(),
		(LPCTSTR)resource,
		this->core->getWindowHandle(),
		(DLGPROC)dlgproc,
		lParam
		);
}
/*
Dialog procedure for creating new file
*/
LRESULT CALLBACK Dialogs::processDlg_New(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Core *core = Dialogs::core;
	if( core == NULL )
		return FALSE;

	int wmId, wmEvent;
	FwCHAR *tmp1, *tmp2;

	ChildCore *child = NULL;
	static Bitmap *clipboard;

	switch (message)
	{
		case WM_INITDIALOG:
			// set untitled name
			tmp1 = new FwCHAR(UNTITLED);
			tmp2 = new FwCHAR(Core::getChicounter() + 1);
			tmp1->mergeWith(L"-");
			tmp1->mergeWith(tmp2);
			SetDlgItemText(hDlg,IDC_EDIT_NAME,tmp1->toWCHAR());
			delete tmp1;
			delete tmp2;

			// set default size
			SetDlgItemInt(hDlg,IDC_EDIT_W,400,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT_H,400,FALSE);

			// if clipboard is not empty, set its size
			clipboard = Core::getClipboardBitmap();
			if( clipboard != NULL ){
				SetDlgItemInt(hDlg,IDC_EDIT_W,clipboard->GetWidth(),FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_H,clipboard->GetHeight(),FALSE);
			}
			delete clipboard;

			// fill background mode combo
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_BKGND),CB_ADDSTRING,NULL,(LPARAM)BKGND_TRANS);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_BKGND),CB_ADDSTRING,NULL,(LPARAM)BKGND_FORCOL);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_BKGND),CB_ADDSTRING,NULL,(LPARAM)BKGND_WHITE);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_BKGND),CB_SETCURSEL,0,NULL);

			return TRUE;

		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);

			switch (wmId){
				case IDOK:
					EndDialog(hDlg, YES);
					// if OK, then create a new MDI child and add it
					child = new ChildCore(
							core,
							Core::getDlgItemString(hDlg,IDC_EDIT_NAME),
							Core::getDlgItemNumber(hDlg,IDC_EDIT_W,false),
							Core::getDlgItemNumber(hDlg,IDC_EDIT_H,false),
							1,
							72,
							1,
							1,
							8,
							(int)SendMessage(GetDlgItem(hDlg,IDC_COMBO_BKGND),CB_GETCURSEL,NULL,NULL)
							);
					core->neww(child);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, NO);
					return TRUE;
				default:
					break;
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/*
Dialog procedure for custom color dialog, managed by a Clr DialogHelper
*/
LRESULT CALLBACK Dialogs::processDlg_Clr(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Core *core = Dialogs::core;
	if( core == NULL )
		return FALSE;

	int wmId, wmEvent;
	static DialogHelper_Clr *helper = NULL;
	static Color *color;

	switch (message)
	{
		case WM_INITDIALOG:
			color = (Color *)lParam;
			helper = new DialogHelper_Clr(hDlg,*(color));
			break;
		case WM_NOTIFY:
			if( helper != NULL )
				helper->valueChange((int)wParam);
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);

			switch(wmEvent){
				case BN_CLICKED:
					if( helper != NULL )
						helper->radioChange();
					break;
			}
			switch (wmId){
				case IDOK:
					EndDialog(hDlg, YES);
					*color = helper->getNewColor();
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, NO);
					return TRUE;
				default:
					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg,NO);
			break;
		case WM_DESTROY:
			delete helper;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/*
Dialog procedure for resizing images and Workspace frame
*/
LRESULT CALLBACK Dialogs::processDlg_Res(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Core *core = Dialogs::core;
	if( core == NULL )
		return FALSE;

	ChildCore *child = core->getActiveChild();
	if( child == NULL )
		return FALSE;

	Workspace *wspc = child->getWorkspace();

	bool wasLastResize;
	int wmId, wmEvent, val;
	static bool isStyles, isConst, isResample, lastWasWidth, lastWasPxWidth, lastWasResize;
	static int Width, Height, pxWidth, pxHeight, newWidth, newHeight, newCvsWidth, newCvsHeight, newPtW, newPtH;

	switch (message)
	{
		case WM_INITDIALOG:
			// get current image size
			Width = pxWidth = wspc->getPxWidth();
			Height = pxHeight = wspc->getPxHeight();

			// set new image size
			newWidth = pxWidth;
			newHeight = pxHeight;

			// set new canvas size
			newCvsHeight = Height;
			newCvsWidth = Width;

			newPtW = 100;
			newPtH = 100;

			// fill controls with data
			SetDlgItemInt(hDlg,IDC_EDIT_PXWIDTH,pxWidth,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT_PXHEIGHT,pxHeight,FALSE);

			SetDlgItemInt(hDlg,IDC_EDIT_WIDTH,Width,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT_HEIGHT,Height,FALSE);

			Dialogs::limitInputInt(GetDlgItem(hDlg,IDC_EDIT_PXWIDTH),0,MAX_WIDTH);
			Dialogs::limitInputInt(GetDlgItem(hDlg,IDC_EDIT_PXHEIGHT),0,MAX_HEIGHT);

			Dialogs::limitInputInt(GetDlgItem(hDlg,IDC_EDIT_WIDTH),0,MAX_WIDTH);
			Dialogs::limitInputInt(GetDlgItem(hDlg,IDC_EDIT_HEIGHT),0,MAX_HEIGHT);

			SendMessage(GetDlgItem(hDlg,IDC_COMBO_RESAMPLE),CB_ADDSTRING,NULL,(LPARAM)IM_BICUBIC_HQ);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_RESAMPLE),CB_ADDSTRING,NULL,(LPARAM)IM_BICUBIC);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_RESAMPLE),CB_ADDSTRING,NULL,(LPARAM)IM_BILINEAR);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_RESAMPLE),CB_ADDSTRING,NULL,(LPARAM)IM_NN);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO_RESAMPLE),CB_SETCURSEL,0,NULL);

			CheckDlgButton(hDlg,IDC_RADIO1,BST_CHECKED);
			lastWasResize = true;

			CheckDlgButton(hDlg,IDC_CHECK_STYLES,TRUE);
			isStyles = true;
			CheckDlgButton(hDlg,IDC_CHECK_CONST,TRUE);
			isConst = true;
			CheckDlgButton(hDlg,IDC_CHECK_RESAMPLE,TRUE);
			isResample = true;

			lastWasWidth = true;
			lastWasPxWidth = true;
			break;
		case WM_NOTIFY:
			switch(wParam){
				// change image sizes
				case IDC_EDIT_PXWIDTH:
					lastWasResize = true;
					lastWasPxWidth = true;
					CheckDlgButton(hDlg,IDC_RADIO1,TRUE);
					CheckDlgButton(hDlg,IDC_RADIO2,FALSE);

					val = GetDlgItemInt(hDlg,IDC_EDIT_PXWIDTH,NULL,FALSE);

					newWidth = val;
					if( isConst == true ){
						double factor = (double)newWidth / (double)pxWidth;
						newHeight = (int)(factor * pxHeight);

						SetDlgItemInt(hDlg,IDC_EDIT_PXHEIGHT,newHeight,FALSE);
					}
					break;
				case IDC_EDIT_PXHEIGHT:
					lastWasResize = true;
					lastWasPxWidth = false;
					CheckDlgButton(hDlg,IDC_RADIO1,TRUE);
					CheckDlgButton(hDlg,IDC_RADIO2,FALSE);

					val = GetDlgItemInt(hDlg,IDC_EDIT_PXHEIGHT,NULL,FALSE);

					newHeight = val;
					if( isConst == true ){
						double factor = (double)newHeight / (double)pxHeight;
						newWidth = (int)(factor * pxWidth);

						SetDlgItemInt(hDlg,IDC_EDIT_PXWIDTH,newWidth,FALSE);
					}
					break;
				// change Workspace frame sizes
				case IDC_EDIT_WIDTH:
					lastWasResize = false;
					lastWasWidth = true;
					CheckDlgButton(hDlg,IDC_RADIO2,TRUE);
					CheckDlgButton(hDlg,IDC_RADIO1,FALSE);

					val = GetDlgItemInt(hDlg,IDC_EDIT_WIDTH,NULL,FALSE);

					newCvsWidth = val;
					if( isConst == true ){
						double factor = (double)newCvsWidth / (double)Width;
						newCvsHeight = (int)(factor * Height);

						SetDlgItemInt(hDlg,IDC_EDIT_HEIGHT,newCvsHeight,FALSE);
					}
					break;
				case IDC_EDIT_HEIGHT:
					lastWasResize = false;
					lastWasWidth = false;
					CheckDlgButton(hDlg,IDC_RADIO2,TRUE);
					CheckDlgButton(hDlg,IDC_RADIO1,FALSE);

					val = GetDlgItemInt(hDlg,IDC_EDIT_HEIGHT,NULL,FALSE);

					newCvsHeight = val;
					if( isConst == true ){
						double factor = (double)newCvsHeight / (double)Height;
						newCvsWidth = (int)(factor * Width);

						SetDlgItemInt(hDlg,IDC_EDIT_WIDTH,newCvsWidth,FALSE);
					}
					break;
			}
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);

			switch(wmEvent){
				case BN_CLICKED:
					switch(wmId){
						case IDC_CHECK_CONST:
							isConst = (bool) IsDlgButtonChecked(hDlg,IDC_CHECK_CONST);

							if( isConst == true && lastWasPxWidth == true )
								SendMessage(hDlg,WM_NOTIFY,IDC_EDIT_PXWIDTH,NULL);
							else if( isConst == true && lastWasPxWidth == false )
								SendMessage(hDlg,WM_NOTIFY,IDC_EDIT_PXHEIGHT,NULL);

							if( isConst == true && lastWasWidth == true )
								SendMessage(hDlg,WM_NOTIFY,IDC_EDIT_WIDTH,NULL);
							else if( isConst == true && lastWasWidth == false )
								SendMessage(hDlg,WM_NOTIFY,IDC_EDIT_HEIGHT,NULL);
							break;

						default:
							isStyles = (bool) IsDlgButtonChecked(hDlg,IDC_CHECK_STYLES);
							isResample = (bool) IsDlgButtonChecked(hDlg,IDC_CHECK_RESAMPLE);
							break;
					}
			}
			if( wmId == IDOKI ){
				if( IsDlgButtonChecked(hDlg,IDC_RADIO1) ){
					wmId = IDOK_RES;
				}
				else if( IsDlgButtonChecked(hDlg,IDC_RADIO2) ){
					wmId = IDOK_CANV;
				}
				else {
					wmId = IDCANCEL;
				}
			}
			switch (wmId){
				case IDOK_RES:
					// OK for image resize, perform resize
					wasLastResize = lastWasResize;
					EndDialog(hDlg, YES);
					if( isResample == true &&
						(pxWidth != newWidth || pxHeight != newHeight) ){
						val = (int)SendMessage(GetDlgItem(hDlg,IDC_COMBO_RESAMPLE),CB_GETCURSEL,0,NULL);
						InterpolationMode mode = InterpolationModeHighQualityBicubic;
						switch(val){
							case 1:
								mode = InterpolationModeBicubic;
								break;
							case 2:
								mode = InterpolationModeBilinear;
								break;
							case 3:
								mode = InterpolationModeNearestNeighbor;
								break;
						}
						wspc->resizeImage(newWidth,newHeight,mode);
					}
					return TRUE;
				case IDOK_CANV:
					// OK for canvas resize, perform resize
					EndDialog(hDlg, YES);
					if( (Width != newCvsWidth || Height != newCvsHeight) ){
						int wdf = (newCvsWidth - Width) / 2;
						int hdf = (newCvsHeight - Height) / 2;

						RECT canvas;
						SetRect(&canvas,-wdf,-hdf,Width + wdf,Height + hdf);

						wspc->resizeCanvas(canvas,false,Dialogs::core->getToolws()->getToolwCC()->toolCrop);
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, NO);
					return TRUE;
				default:
					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg,NO);
			break;
		case WM_DESTROY:
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/*
Dialog procedure for matrix filter dialog, managed by Fil DialogHelper
*/
LRESULT CALLBACK Dialogs::processDlg_Fil(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Core *core = Dialogs::core;
	if( core == NULL )
		return FALSE;

	int wmId, wmEvent;
	static DialogHelper_Fil *helper = NULL;
	static ToolFilter::Info *fi;

	switch (message)
	{
		case WM_INITDIALOG:
			fi = (ToolFilter::Info *)lParam;
			helper = new DialogHelper_Fil(hDlg,fi);
			break;
		case WM_NOTIFY:
			if( helper != NULL )
				helper->valueChange((int)wParam);
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);

			switch(wmEvent){
				case BN_CLICKED:
					if( helper != NULL )
						helper->checkChange(wmId);
					break;
			}
			switch (wmId){
				case IDOK:
					EndDialog(hDlg, YES);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, NO);
					return TRUE;
				default:
					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg,NO);
			break;
		case WM_DESTROY:
			delete helper;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/*
Limits a given slider control and associates it with an edit box
so the slide of the slider changes the values of the edit box
hInput -	edit box handle
hWnd -		slide control handle
min -		minimal slider value
max -		maximal slider value
val -		initial slider value
*/
void Dialogs::limitSliderInt(HWND hInput, HWND hWnd, int min, int max, int val)
{
	SendMessage(hWnd,TBM_SETRANGE,(WPARAM)TRUE,(LPARAM)MAKELONG(min,max));
	SendMessage(hWnd,TBM_SETPAGESIZE,0,(LPARAM)(int)(abs(max - min) / 10));

	SendMessage(hWnd,TBM_SETSEL,(WPARAM)FALSE,(LPARAM)MAKELONG(min,max)); 
	SendMessage(hWnd,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)val);

	SetProp(hWnd,ATOM_PROC,(WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC));
	SetProp(hWnd,ATOM_LIMIT,new Point(min,max));
	SetProp(hWnd,ATOM_OWNER,hInput);

	SetProp(hWnd,ATOM_BOOL,FALSE);
	SetProp(hWnd,ATOM_VAL,new Point(val,val));

	SetProp(hInput,ATOM_SLIDER,hWnd);

	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)Dialogs::procSliderInt);
}
/*
Procedure for processing limited slider information
*/
LRESULT CALLBACK Dialogs::procSliderInt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	HWND hInput = (HWND)GetProp(hDlg,ATOM_OWNER);
	BOOL state = (BOOL)GetProp(hDlg,ATOM_BOOL);

	Point *minmax = (Point *)GetProp(hDlg,ATOM_LIMIT);
	Point *oldval = (Point *)GetProp(hDlg,ATOM_VAL);

	int min = minmax->X;
	int max = minmax->Y;

	int val;
	LRESULT result;

	switch(message){
		case WM_LBUTTONDOWN:
			SetProp(hDlg,ATOM_BOOL,(HANDLE)TRUE);
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
		case WM_LBUTTONUP:
			SetProp(hDlg,ATOM_BOOL,FALSE);

			if( hInput != NULL ){
				SetDlgItemInt(GetParent(hInput),GetDlgCtrlID(hInput),oldval->X,TRUE);
				SendMessage(GetParent(hInput),WM_NOTIFY,GetDlgCtrlID(hInput),NULL);
			}
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
		case 512:
			result = CallWindowProc(proc,hDlg,message,wParam,lParam);

			val = (int)SendMessage(hDlg,TBM_GETPOS,NULL,NULL);
			if( val != oldval->X ){
				SetProp(hDlg,ATOM_VAL,new Point(val,val));

				// if associated with and edit box, update it
				if( hInput != NULL ){
					SetDlgItemInt(GetParent(hInput),GetDlgCtrlID(hInput),val,TRUE);
					if( state == FALSE )
						SendMessage(GetParent(hInput),WM_NOTIFY,GetDlgCtrlID(hInput),NULL);
				}
			}
			return result;
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}

	return 0;
}
/*
Limits a given edit box control identified by hWnd parameter
and gives it arrow key control functionality
*/
void Dialogs::limitInputInt(HWND hWnd, int min, int max)
{
	SetProp(hWnd,ATOM_PROC,(WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC));
	SetProp(hWnd,ATOM_LIMIT,new Point(min,max));

	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)Dialogs::procInputInt);
}
/*
For future use
*/
void Dialogs::limitInputText(HWND hWnd, int min, int max)
{

}
/*
Procedure for processing limited edit box messages
*/
LRESULT CALLBACK Dialogs::procInputInt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	HWND hSlider = (HWND)GetProp(hDlg,ATOM_SLIDER);

	Point *minmax = (Point *)GetProp(hDlg,ATOM_LIMIT);

	int min = minmax->X;
	int max = minmax->Y;

	int val;
	bool notify;
	LRESULT result;

	switch(message){
		// update edit box values on arrow keys
		case WM_KEYDOWN:
			result = CallWindowProc(proc,hDlg,message,wParam,lParam);

			val = 0;
			switch(wParam){
				case VK_UP:
					val++;
					break;
				case VK_DOWN:
					val--;
					break;
			}
			if( val != 0 ){
				val += GetDlgItemInt(
					GetParent(hDlg),
					GetDlgCtrlID(hDlg),
					NULL,
					TRUE);
				if( val >= min && val <= max ){
					SetDlgItemInt(
						GetParent(hDlg),
						GetDlgCtrlID(hDlg),
						val,
						TRUE);
					if( hSlider != NULL ){
						SendMessage(hSlider,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)val);
					}
					SendMessage(GetParent(hDlg),WM_NOTIFY,GetDlgCtrlID(hDlg),NULL);
				}
			}
			return result;

		case WM_KEYUP:
			result = CallWindowProc(proc,hDlg,message,wParam,lParam);

			val = GetDlgItemInt(
				GetParent(hDlg),
				GetDlgCtrlID(hDlg),
				NULL,
				TRUE);
			notify = false;
			if( val < min ){
				notify = true;
				val = min;
			}
			if( val > max ){
				notify = true;
				val = max;
			}
			if( notify == true ){
				FwCHAR *fmin = new FwCHAR(min);
				FwCHAR *fmax = new FwCHAR(max);
				FwCHAR *error = new FwCHAR(MESSAGE_LIMIT1);
				error->mergeWith(fmin);
				error->mergeWith(MESSAGE_LIMIT2);
				error->mergeWith(fmax);
				error->mergeWith(MESSAGE_LIMIT3);

				Core::self->messageBox_Error(error->toWCHAR());

				delete fmin;
				delete fmax;
				delete error;

				SetDlgItemInt(
					GetParent(hDlg),
					GetDlgCtrlID(hDlg),
					val,
					TRUE);
			}
			WCHAR buf[10];
			if( GetDlgItemText(GetParent(hDlg),GetDlgCtrlID(hDlg),buf,10) == 0 ){
				SetDlgItemInt(
					GetParent(hDlg),
					GetDlgCtrlID(hDlg),
					val,
					TRUE);
			if( hSlider != NULL )
				SendMessage(hSlider,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)val);
			}

			SendMessage(GetParent(hDlg),WM_NOTIFY,GetDlgCtrlID(hDlg),NULL);

			return result;
			break;
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}

	return 0;
}

LRESULT CALLBACK Dialogs::procInputText(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
/*
Converts a static text control into an in-place editable text control
x,y - location of the static text
*/
void Dialogs::editStaticText(HWND hWnd, int x, int y)
{
	SetProp(hWnd,ATOM_PROC,(WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC));
	SetProp(hWnd,ATOM_POINT,new Point(x,y));

	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)Dialogs::procStaticEditText);
	SendMessage(hWnd,WM_CREATE,NULL,NULL);
}
/*
Procedure for processing editable static text messages
*/
LRESULT CALLBACK Dialogs::procStaticEditText(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	Point *point = (Point *)GetProp(hDlg,ATOM_POINT);

	HWND hEdit;	
	WCHAR *string;

	switch(message){
		case WM_CREATE:
			break;
		// double click - trurn on editing mode
		case WM_LBUTTONDBLCLK:
			string = new WCHAR[MAX_PATH + 1];
			GetWindowText(hDlg,string,MAX_PATH);

			// create an edit box on top
			hEdit = Core::self->CreateWindowExSubstituteFont(
					WS_EX_TOPMOST,
					IDCL_EDIT,
					string,
					WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
					point->X - 2,
					point->Y + 1,
					(int)wcslen(string) * LAYNAMEMARGIN - LAYNAMEMARGIN,
					2 * LAYNAMEMARGIN,
					GetParent(hDlg),
					NULL,
					Core::self->getInstance(),
					NULL
					);
			SetProp(hEdit,ATOM_PROC,(WNDPROC)GetWindowLong(hEdit,GWL_WNDPROC));
			SetProp(hEdit,ATOM_OWNER,hDlg);

			SetWindowLong(hEdit,GWL_WNDPROC,(LONG)Dialogs::procStaticEditEdit);
			SendMessage(hEdit,WM_CREATE,NULL,NULL);

			delete string;
			break;
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Procedure for processing editable static control edit box
*/
LRESULT CALLBACK Dialogs::procStaticEditEdit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	HWND hOwner = (HWND)GetProp(hDlg,ATOM_OWNER);

	RECT client;
	POINTS mouse;
	WCHAR *string;

	switch(message){
		case WM_CREATE:
			// capture mouse to limit bugs
			SetCapture(hDlg);

			// select all current text
			SendMessage(hDlg,EM_SETSEL,0,-1);
			// set focus on edit box
			SetFocus(hDlg);
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:	
			mouse = MAKEPOINTS(lParam);
			GetClientRect(hDlg,&client);

			// if use clicked outside of the bounds of the
			// edit box that indicates the end of editing
			if( mouse.x < client.left || mouse.x > client.right ||
				mouse.y < client.top || mouse.y > client.bottom ){

				string = new WCHAR[MAX_PATH + 1];
				GetWindowText(hDlg,string,MAX_PATH);
				SetWindowText(hOwner,string);

				DestroyWindow(hDlg);
				ReleaseCapture();

				SendMessage(
					GetParent(hOwner),
					WM_COMMAND,
					MAKEWPARAM(GetDlgCtrlID(hOwner),EN_UPDATED),
					(LPARAM)hOwner);

				delete string;
				return 0;
			}
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
		case WM_KEYUP:
			switch(wParam){
				// finalize editing with change
				case VK_RETURN:
					string = new WCHAR[MAX_PATH + 1];
					GetWindowText(hDlg,string,MAX_PATH);
					SetWindowText(hOwner,string);

					DestroyWindow(hDlg);
					ReleaseCapture();

					SendMessage(
						GetParent(hOwner),
						WM_COMMAND,
						MAKEWPARAM(GetDlgCtrlID(hOwner),EN_UPDATED),
						(LPARAM)hOwner);

					delete string;
					return 0;
				// finalize editing wihtout change
				case VK_ESCAPE:
					DestroyWindow(hDlg);
					ReleaseCapture();

					SendMessage(
						GetParent(hOwner),
						WM_COMMAND,
						MAKEWPARAM(GetDlgCtrlID(hOwner),EN_UPDATED),
						(LPARAM)NULL);
					return 0;
			}
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Create an ownerdraw button that is of the style of the Tool Control Center buttons
*/
HWND Dialogs::ownerdrawButton(HWND hOwner, WCHAR *icon, int x, int y, int w, int h, int id, BOOL autocheck)
{
	HWND hWnd = CreateWindowEx(
		NULL,
		IDCL_BUTTON,
		icon,
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		x,y,w,h,
		hOwner,(HMENU)id,
		Core::self->getInstance(),
		NULL
		);

	return Dialogs::ownerdrawButton(hWnd,hOwner,autocheck);
}
/*
Convert an existing button into the ownerdraw button of Tool Control Center type
*/
HWND Dialogs::ownerdrawButton(HWND hWnd, HWND hOwner, BOOL autocheck)
{
	SetProp(hWnd,ATOM_PROC,(WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC));
	SetProp(hWnd,ATOM_OWNER,hOwner);

	if( autocheck == TRUE )
		SetProp(hWnd,ATOM_STL_AC,(HANDLE)TRUE);

	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)Dialogs::processButton);
	SendMessage(hWnd,WM_CREATE,NULL,NULL);

	return hWnd;
}
/*
Paint the ownerdraw button
*/
void Dialogs::paintButton(LPDRAWITEMSTRUCT button)
{
	RECT client = button->rcItem;

	Bitmap *bmp = new Bitmap(client.right,client.bottom,Core::getPixelFormat());
	Graphics *g = new Graphics(bmp);

	if( button->itemState & ODS_SELECTED )
		g->Clear(CLR_WHITE);
	else
		g->Clear(CLR_FRAME_LIGHT);

	client.right -= 1;
	client.bottom -= 1;

	Pen *lite = new Pen(CLR_WHITE,1);
	Pen *dark = new Pen(CLR_FRAME_DARK,1);

	if( button->itemState & ODS_SELECTED ){
		Pen *temp = lite;
		lite = dark;
		dark = temp;
	}
	if( (button->itemState & ODS_SELECTED) || ((UINT)GetProp(button->hwndItem,ATOM_BOOL) == TRUE) ){
		g->DrawLine(lite,client.left,client.top,client.left,client.bottom);
		g->DrawLine(lite,client.left,client.top,client.right,client.top);
		g->DrawLine(dark,client.right,client.top,client.right,client.bottom);
		g->DrawLine(dark,client.right,client.bottom,client.left,client.bottom);
	}
	if( button->CtlID != NULL ){
		Image *icon = Core::getImageResource(button->CtlID,RC_PNG);

		if( icon != NULL ){
			int x = ((int)client.right - (int)icon->GetWidth()) / 2;
			int y = ((int)client.bottom - (int)icon->GetWidth()) / 2;
			g->DrawImage(icon,x,y,icon->GetWidth(),icon->GetHeight());
			delete icon;
		}
	}
	Graphics *gdc = new Graphics(button->hDC);
	gdc->DrawImage(bmp,0,0);

	delete lite;
	delete dark;

	delete gdc;
	delete bmp;
	delete g;

	if( button->itemAction & ODA_SELECT )
		InvalidateRect(button->hwndItem,NULL,TRUE);
}
/*
Process the ownerdraw button messages
*/
LRESULT CALLBACK Dialogs::processButton(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	HWND hOwner = (HWND)GetProp(hDlg,ATOM_OWNER);
	BOOL aChk = (BOOL)GetProp(hDlg,ATOM_STL_AC);

	UINT state = NULL;
	LRESULT result = NULL;

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = hDlg;	

	switch (message)
	{
		case WM_CREATE:
			break;
		// emulate doubleclick
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			result = CallWindowProc(proc,hDlg,message,wParam,lParam);

			SendMessage(hOwner,WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(hDlg),BN_DBLCLK),(LPARAM)hDlg);
			return result;
		// emulate click
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
			result = NULL;
			
			if( aChk == TRUE ){
				state = SendMessage(hDlg,BM_GETSTATE,NULL,NULL);
				if( state & BST_PUSHED ){
					SendMessage(hDlg,BM_SETSTATE,FALSE,NULL);
					//SendMessage(hDlg,BM_SETCHECK,(WPARAM)BST_UNCHECKED,NULL);
				}
				else {
					SendMessage(hDlg,BM_SETSTATE,TRUE,NULL);
					//SendMessage(hDlg,BM_SETCHECK,(WPARAM)BST_CHECKED,NULL);
				}
				SendMessage(hOwner,WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(hDlg),BN_CLICKED),(LPARAM)hDlg);
			}
			else {
				result = CallWindowProc(proc,hDlg,message,wParam,lParam);
			}
			return result;
		case BM_SETCHECK:
			result = CallWindowProc(proc,hDlg,message,wParam,lParam);
			InvalidateRect(hDlg,NULL,TRUE);
			return result;
		// emulate hovering changes in the look of the button
		case WM_MOUSEMOVE:
			if( (UINT)GetProp(hDlg,ATOM_BOOL) == FALSE ){
				SetProp(hDlg,ATOM_BOOL,(HANDLE)TRUE);
				TrackMouseEvent(&tme);

				InvalidateRect(hDlg,NULL,TRUE);
			}
			break;
		case WM_MOUSELEAVE:
			if( (UINT)GetProp(hDlg,ATOM_BOOL) == TRUE ){
				SetProp(hDlg,ATOM_BOOL,(HANDLE)FALSE);

				InvalidateRect(hDlg,NULL,TRUE);
			}
			break;
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
	return 0;
}
/*
Create a small color box that toggles the custom color control upon doubleclick
*/
HWND Dialogs::colorPicker(HWND hOwner, Color color, int x, int y, int id, int w, int h)
{
	if( w == NULL )
		w = 35;
	if( h == NULL )
		h = 20;

	HWND hWnd = CreateWindowEx(
		NULL,
		IDCL_BUTTON,
		NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		x,y,w,h,
		hOwner,(HMENU)id,
		Core::self->getInstance(),
		NULL
		);
	SetProp(hWnd,ATOM_PROC,(WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC));
	SetProp(hWnd,ATOM_OWNER,hOwner);

	SetProp(hWnd,ATOM_COLOR,(HANDLE)Core::RGBtoUINT(color));

	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)Dialogs::processPicker);
	SendMessage(hWnd,WM_CREATE,NULL,NULL);

	return hWnd;
}
/*
Process the small color box control
*/
LRESULT CALLBACK Dialogs::processPicker(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetProp(hDlg,ATOM_PROC);
	HWND hOwner = (HWND)GetProp(hDlg,ATOM_OWNER);
	UINT color = (UINT)GetProp(hDlg,ATOM_COLOR);

	LRESULT result = NULL;

	PAINTSTRUCT ps;
	Graphics *g;
	Color tcl;
	HDC hdc;

	switch (message)
	{
		case WM_CREATE:
			break;
		case WM_LBUTTONUP:
			// user requested change of color, bring up the custom color dialog
			tcl = Core::UINTtoRGB(color);

			if( Core::self->getDialogs()->showDialog(
					(LPCTSTR)IDD_CLR,
					(DLGPROC)Dialogs::processDlg_Clr,
					(LPARAM)&tcl) == YES ){
				SetProp(hDlg,ATOM_COLOR,(HANDLE)Core::RGBtoUINT(tcl));

				SendMessage(hOwner,WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(hDlg),BN_CLICKED),(LPARAM)hDlg);
				InvalidateRect(hDlg,NULL,TRUE);
			}
			break;
		case WM_MOUSEMOVE:
			Core::self->getGui()->setCursor(IDC_HAND);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hDlg,&ps);

			g = Graphics::FromHDC(hdc);
			g->Clear(Core::UINTtoRGB(color));
			delete g;

			EndPaint(hDlg,&ps);
			break;
		default:
			return CallWindowProc(proc,hDlg,message,wParam,lParam);
	}
	return 0;
}