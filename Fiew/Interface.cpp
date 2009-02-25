/*
Interface.cpp
Partially inherited from Fiew 2.0
Interface object manages interactions between the main menu
and the application Core and controls the state of that menu,
also it controls the common dialog boxes for file managment
*/

#include "stdafx.h"
#include "Core.h"

#include "shlobj.h"

Interface::Interface(Core *core)
{
	this->core = core;

	/* fiew */
	this->mState = NULL;
	this->mButton = NULL;
	this->mX = this->mY = 0;
	this->dX = this->dY = 0;
	this->mBB = false;
	this->mWH = false;
	this->mMM = false;

	this->lastFolder = NULL;
	this->lastItems = NULL;
	this->lastItemIds = NULL;

	this->fullscreen = false;
	this->fullpath = false;
	this->menuvisible = true;
	this->menuheight = NULL;

	this->textmessage = NULL;

	this->windowLong = NULL;
	this->windowMenu = NULL;
	this->menuFile = NULL;
	this->menuView = NULL;
	this->menuHelp = NULL;

	this->cursor = LoadCursor(NULL,CURSOR_CLIENT);

	this->initLocale();
	this->update();

	this->initialize();
}

Interface::~Interface()
{
	if( this->lastFolder != NULL )
		delete this->lastFolder;
	if( this->lastItems != NULL )
		delete this->lastItems;
	if( this->lastItemIds != NULL )
		delete this->lastItemIds;
}

void Interface::initialize()
{
	this->lastItems = new List<FwCHAR>();
	this->lastItemIds = new List<Point>();

	if( this->core->getToolws() != NULL ){
		this->core->getToolws()->show_CC();
	}
}
/*
Initializes menu with the proper language
*/
void Interface::initLocale()
{
	MENUITEMINFO mif;
	// HMENU main, submain;
	HWND window = this->core->getWindowHandle();

	mif.cbSize = sizeof(MENUITEMINFO);
	this->windowMenu = LoadMenu(this->core->getInstance(),MAKEINTRESOURCE(IDR_MENU));

	// ****
	SetMenu(this->core->getWindowHandle(),this->windowMenu);
	return;
	// File
	/*mif.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(this->windowMenu,0,true,&mif);
	main = mif.hSubMenu;
	this->menuFile = main;
	mif.fMask = MIIM_STRING;
	mif.fType = MFT_STRING;
	mif.dwTypeData = MENU_FILE;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(this->windowMenu,0,true,&mif);
	
	mif.dwTypeData = MENU_FILE_OPEN;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,0,true,&mif);
	mif.dwTypeData = MENU_FILE_OPENFOLDER;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,1,true,&mif);
	mif.dwTypeData = MENU_FILE_CLOSE;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,2,true,&mif);
	mif.dwTypeData = MENU_FILE_EXTRACT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,4,true,&mif);
	mif.dwTypeData = MENU_FILE_SETWALL;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,5,true,&mif);
	mif.dwTypeData = MENU_FILE_EXIT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,7,true,&mif);

	// View
	mif.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(this->windowMenu,1,true,&mif);
	main = mif.hSubMenu;
	this->menuView = main;
	mif.fMask = MIIM_STRING;
	mif.fType = MFT_STRING;
	mif.dwTypeData = MENU_VIEW;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(this->windowMenu,1,true,&mif);

	mif.dwTypeData = MENU_VIEW_FULLSCREEN;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,0,true,&mif);
	mif.dwTypeData = MENU_VIEW_THUMBNAILS;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,2,true,&mif);
	mif.dwTypeData = MENU_VIEW_LIST;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,3,true,&mif);
	mif.dwTypeData = MENU_VIEW_FLOWSCROLL;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,9,true,&mif);
	// View_Fitto
	mif.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(main,5,true,&mif);
	submain = mif.hSubMenu;
	mif.fMask = MIIM_STRING;
	mif.fType = MFT_STRING;
	mif.dwTypeData = MENU_VIEW_FITTO;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,5,true,&mif);

	mif.dwTypeData = MENU_VIEW_FITTO_SCREENOV;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,0,true,&mif);
	mif.dwTypeData = MENU_VIEW_FITTO_SCREEN;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,1,true,&mif);
	mif.dwTypeData = MENU_VIEW_FITTO_HEIGHT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,3,true,&mif);
	mif.dwTypeData = MENU_VIEW_FITTO_WIDTH;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,4,true,&mif);
	mif.dwTypeData = MENU_VIEW_FITTO_NUMPAD;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,5,true,&mif);
	mif.dwTypeData = MENU_VIEW_FITTO_LEFT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,7,true,&mif);
	mif.dwTypeData = MENU_VIEW_FITTO_RIGHT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,8,true,&mif);
	// View_Zoom
	mif.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(main,6,true,&mif);
	submain = mif.hSubMenu;
	mif.fMask = MIIM_STRING;
	mif.fType = MFT_STRING;
	mif.dwTypeData = MENU_VIEW_ZOOM;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,6,true,&mif);

	mif.dwTypeData = MENU_VIEW_ZOOM_ZOOMIN;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,9,true,&mif);
	mif.dwTypeData = MENU_VIEW_ZOOM_ZOOMOUT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,10,true,&mif);
	// View_Rotate
	mif.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(main,7,true,&mif);
	submain = mif.hSubMenu;
	mif.fMask = MIIM_STRING;
	mif.fType = MFT_STRING;
	mif.dwTypeData = MENU_VIEW_ROTATE;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,7,true,&mif);

	mif.dwTypeData = MENU_VIEW_ROTATE_RESET;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(submain,4,true,&mif);

	// Help
	mif.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(this->windowMenu,2,true,&mif);
	main = mif.hSubMenu;
	this->menuHelp = main;
	mif.fMask = MIIM_STRING;
	mif.fType = MFT_STRING;
	mif.dwTypeData = MENU_HELP;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(this->windowMenu,2,true,&mif);
	
	mif.dwTypeData = MENU_HELP_ABOUT;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,0,true,&mif);
	mif.dwTypeData = MENU_HELP_MANUAL;
	mif.cch = wcslen(mif.dwTypeData);
	SetMenuItemInfo(main,1,true,&mif);

	SetMenu(this->core->getWindowHandle(),this->windowMenu);*/
}
/*
Not used
*/
void Interface::update()
{
	return;

	this->updateMenu();
	this->updateText();
}
/*
Updates the state of the menu contents
*/
void Interface::updateMenu()
{
	// toggle main menus ability
	ChildCore *child = this->core->getActiveChild();
	UINT state = (child != NULL) ? MF_ENABLED : (MF_GRAYED | MF_DISABLED);
	EnableMenuItem(this->windowMenu,1,state | MF_BYPOSITION);
	EnableMenuItem(this->windowMenu,2,state | MF_BYPOSITION);
	EnableMenuItem(this->windowMenu,3,state | MF_BYPOSITION);
	EnableMenuItem(this->windowMenu,4,state | MF_BYPOSITION);
	EnableMenuItem(this->windowMenu,5,state | MF_BYPOSITION);

	// toggle finishing file operations
	EnableMenuItem(this->windowMenu,ID_FILE_SAVE,state);
	EnableMenuItem(this->windowMenu,ID_FILE_SAVEAS,state);
	EnableMenuItem(this->windowMenu,ID_FILE_CLOSE,state);
	EnableMenuItem(this->windowMenu,ID_FILE_CLOSEALL,state);

	// set recently opened files
	if( this->lastItems->getCount() > 0 ){
		if( this->lastItemIds->getCount() > 0 ){
			this->lastItemIds->gotoHead();
			do {
				DeleteMenu(this->windowMenu,this->lastItemIds->getThat()->X,MF_BYCOMMAND);
			} while( this->lastItemIds->next() == true );

			delete this->lastItemIds;
			this->lastItemIds = new List<Point>();
		}
		int id, pos;
		HMENU file;
		MENUITEMINFO mif, nif;

		mif.cbSize = sizeof(MENUITEMINFO);
		mif.fMask = MIIM_SUBMENU;

		GetMenuItemInfo(this->windowMenu,0,true,&mif);
		file = mif.hSubMenu;

		pos = 10;
		id = 55000;
		nif.cbSize = sizeof(MENUITEMINFO);
		nif.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
		nif.fType = MFT_STRING;

		this->lastItems->gotoTail();
		do {
			nif.wID = id + pos;
			nif.hSubMenu = file;
			nif.hbmpChecked = NULL;
			nif.hbmpUnchecked = NULL;
			nif.dwTypeData = this->lastItems->getThat()->toWCHAR();
			nif.cch = this->lastItems->getThat()->toLength();
			nif.hbmpItem = NULL;

			if( InsertMenuItem(file,pos - 1,TRUE,&nif) == TRUE ){
				this->lastItemIds->add(
					new Point(
						nif.wID,
						(UINT)this->lastItems->getThat()
						)
					);
				pos++;
			}

		} while( this->lastItems->prev() == true );

		nif.fMask = MIIM_FTYPE | MIIM_ID;
		nif.fType = MFT_SEPARATOR;

		nif.wID = id + pos;
		nif.hSubMenu = file;
		nif.hbmpChecked = NULL;
		nif.hbmpUnchecked = NULL;
		nif.dwTypeData = NULL;
		nif.cch = NULL;
		nif.hbmpItem = NULL;

		if( InsertMenuItem(file,pos - 1,TRUE,&nif) == TRUE ){
			this->lastItemIds->add(
				new Point(
					nif.wID,
					(UINT)this->lastItems->getThat()
					)
				);
		}
	}

	if( child != NULL ){
		// toggle edit main menu contents
		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->getHistory()->getHistoryElems()->isThatHead() == false )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_EDIT_UNDO,state);

		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->getHistory()->getHistoryElems()->isThatTail() == false )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_EDIT_REDO,state);

		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->getSelection() != NULL )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_EDIT_CUT,state);
		EnableMenuItem(this->windowMenu,ID_EDIT_COPY,state);
		EnableMenuItem(this->windowMenu,ID_EDIT_COPYMERGED,state);
		EnableMenuItem(this->windowMenu,ID_EDIT_CLEAR,state);
		EnableMenuItem(this->windowMenu,ID_SELECT_DESELECT,state);

		state = MF_GRAYED | MF_DISABLED;
		if( Core::clipboardBitmap != NULL ||
			IsClipboardFormatAvailable(CF_BITMAP) )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_EDIT_PASTE,state);

		// toggle layer menu contents
		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->getLayers()->getCount() > 1 )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_LAYER_DELETE,state);

		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->getLayers()->getTail() !=
			child->getWorkspace()->getSelectedLayer() )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_LAYER_MOVEUP,state);

		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->getLayers()->getHead() !=
			child->getWorkspace()->getSelectedLayer() )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_LAYER_MOVEDOWN,state);

		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->isMergeReady() == true )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_LAYER_MERGEDOWN,state);

		state = MF_GRAYED | MF_DISABLED;
		if( child->getWorkspace()->isRasterizeReady() == true )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_LAYER_RASTER,state);
		EnableMenuItem(this->windowMenu,ID_LAYER_EDITTEXT,state);

		state = MF_UNCHECKED;
		if( child->getWorkspace()->getSelectedLayer()->getIsLocked() )
			state = MF_CHECKED;
		CheckMenuItem(this->windowMenu,ID_LAYER_LOCKED,state);

		state = MF_UNCHECKED;
		if( child->getWorkspace()->getSelectedLayer()->getIsVisible() )
			state = MF_CHECKED;
		CheckMenuItem(this->windowMenu,ID_LAYER_VISIBLE,state);

		// toggle filter menu options
		state = MF_GRAYED | MF_DISABLED;
		if( ((ToolFilter *)this->core->getToolws()->getToolwCC()->toolFilter)->canActivateAgain() )
			state = MF_ENABLED;
		EnableMenuItem(this->windowMenu,ID_FILTER_LASTFILTER,state);
	}
	// toggle view menu options
	state = MF_UNCHECKED;
	if( this->isFullscreen() )
			state = MF_CHECKED;
	CheckMenuItem(this->windowMenu,ID_VIEW_FULLSCREEN,state);

	state = MF_UNCHECKED;
	if( IsWindowVisible(this->core->getToolws()->getToolwBoxInfo()->getBox()->getWindowHandle()) )
			state = MF_CHECKED;
	CheckMenuItem(this->windowMenu,ID_WINDOW_SHOWINFO,state);

	state = MF_UNCHECKED;
	if( IsWindowVisible(this->core->getToolws()->getToolwBoxLayers()->getBox()->getWindowHandle()) )
			state = MF_CHECKED;
	CheckMenuItem(this->windowMenu,ID_WINDOW_SHOWLAYERS,state);

	state = MF_UNCHECKED;
	if( IsWindowVisible(this->core->getToolws()->getToolwBoxHistory()->getBox()->getWindowHandle()) )
			state = MF_CHECKED;
	CheckMenuItem(this->windowMenu,ID_WINDOW_SHOWHISTORY,state);

	// redraw the menu
	DrawMenuBar(this->core->getWindowHandle());

	return;
}
/*
Not used
*/
void Interface::updateText()
{
	FwCHAR *text = new FwCHAR(APP_TITLE);

	Cacher *cacher = this->core->getCacher();
	Drawer *drawer = this->core->getDrawer();
	Explorer *explorer = this->core->getExplorer();

	Catalog *root = NULL;
	bool archived = false;
	if( explorer != NULL ){
		archived = explorer->isArchived();
		root = explorer->getRoot();
	}
	if( drawer != NULL )
		if( drawer->getScene() != NULL )
			if( drawer->getScene()->isContent() == true ){
				Layer *scene = drawer->getScene();
				File *file = scene->getFile();
				if( drawer->isList() == true || 
					drawer->isThumbs() == true )
					if( cacher != NULL )
						if( cacher->getThat() != NULL )
							if( cacher->getThat()->getFile() != NULL )
								file = cacher->getThat()->getFile();

				text->mergeWith(L" - \"");
				if( this->fullpath == true ){
					if( archived == true ){
						text->mergeWith(explorer->getArchivePath());
						text->mergeWith(L"\\~");
						text->mergeWith(file->getFilePath());
					}
					else
						text->mergeWith(file->getFilePath());
				}
				else {
					if( archived == true )
						text->mergeWith(L"~");
					text->mergeWith(file->getFileName());
				}
				if( drawer->isList() == false && 
					drawer->isThumbs() == false ){

					text->mergeWith(L"\" [");
					FwCHAR *width = new FwCHAR(scene->getImageWidth());
					FwCHAR *height = new FwCHAR(scene->getImageHeight());
					FwCHAR *zoom = new FwCHAR((int)(scene->getZoom() * 100));
					text->mergeWith(width);
					text->mergeWith(L"x");
					text->mergeWith(height);
					text->mergeWith(L" @ ");
					text->mergeWith(zoom);
					text->mergeWith(L"%] ");

					delete width;
					delete height;
					delete zoom;
				}
				else {
					text->mergeWith(L"\"");
				}
				if( root != NULL ){
					text->mergeWith(L" - #");
					FwCHAR *count = new FwCHAR(root->getCount());
					FwCHAR *idx = new FwCHAR(root->getIdx() + 1);
					text->mergeWith(idx);
					text->mergeWith(GUI_OF);
					text->mergeWith(count);

					delete count;
					delete idx;
				}
			}
	if( this->textmessage != NULL ){
		text->mergeWith(L" - ");
		text->mergeWith(this->textmessage);
	}
	SetWindowText(this->core->getWindowHandle(),text->toWCHAR());

	delete text;
}
void Interface::newFile()
{
	this->core->getDialogs()->showDialog((LPCTSTR)IDD_NEW,(DLGPROC)Dialogs::processDlg_New);
}
void Interface::openFile()
{
	OPENFILENAME ofn;
	bool result = false;

	int szSize = 2 * MAX_PATH;
	WCHAR *szFile = new WCHAR[szSize];
	szFile[0] = '\0';

	FwCHAR *folder = new FwCHAR();
	if( this->core->getExplorer() != NULL )
		if( this->core->getExplorer()->getRoot() != NULL )
			folder->getFolderFrom(
				this->core->getExplorer()->getRoot()->getFilePath() );

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->core->getWindowHandle();
	ofn.lpstrFile = szFile;
	ofn.lpstrInitialDir = folder->toWCHAR();

	ofn.nMaxFile = szSize;
	ofn.lpstrFilter = OPENFILEFILTER;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	
	ofn.Flags =
		OFN_PATHMUSTEXIST | 
		OFN_FILEMUSTEXIST |
		OFN_NONETWORKBUTTON |
		OFN_ALLOWMULTISELECT |
		OFN_EXPLORER;

	this->blockMBB();
	if( GetOpenFileName(&ofn) ){
		FwCHAR *tester = new FwCHAR(szFile);
		tester->stripExtension();
		
		if( tester->toLength() == wcslen(szFile) ){
			int ptr = (int)wcslen(szFile) + 1;

			FwCHAR *iterator, *patherator;
			iterator = new FwCHAR(&szFile[ptr]);
			while( iterator->toLength() != 0 ){
				patherator = new FwCHAR(tester->toWCHAR());
				if( patherator->toWCHAR()[patherator->toLength()] != '\\' )
					patherator->mergeWith(L"\\");
				patherator->mergeWith(iterator);

				result = this->core->open( patherator->toWCHAR() );
				ptr += iterator->toLength() + 1;

				delete iterator;
				delete patherator;

				iterator = new FwCHAR(&szFile[ptr]);
			}
			delete iterator;
		}
		else {
			result = this->core->open(szFile);
		}
		delete tester;
	}
	this->timerMBB();

	delete [] szFile;
	delete folder;
}
void Interface::openFolder()
{
	BROWSEINFO bnfo;
	bool result = false;

	int szSize = MAX_PATH;
	WCHAR *szFile = new WCHAR[szSize];
	szFile[0] = '\0';

	ZeroMemory(&bnfo, sizeof(bnfo));
	bnfo.hwndOwner = this->core->getWindowHandle();

	bnfo.pszDisplayName = NULL;

	bnfo.pidlRoot = NULL;	
	bnfo.lpfn = Interface::BFFCallback;
	bnfo.lParam = (LPARAM)this->lastFolder;

	bnfo.iImage = NULL;
	bnfo.lpszTitle = MESSAGE_FOLDERBROWSER;

	bnfo.ulFlags = 
		BIF_RETURNONLYFSDIRS | 
		BIF_DONTGOBELOWDOMAIN | 
		BIF_NONEWFOLDERBUTTON;

	LPITEMIDLIST output = NULL;
	if( (output = SHBrowseForFolder(&bnfo)) != NULL ){
		if( SHGetPathFromIDList(output,szFile) == YES ){
			FwCHAR *folder = new FwCHAR(szFile);

			if( this->lastFolder != NULL )
				delete this->lastFolder;
			this->lastFolder = new FwCHAR(folder->toWCHAR());

			result = this->core->openFolder(folder);
		}
		CoTaskMemFree(output);
	}
	delete [] szFile;
}
int CALLBACK Interface::BFFCallback(HWND hWnd, UINT message, LPARAM lParam, LPARAM lpData)
{
	FwCHAR *select = (FwCHAR *)lpData;

	switch(message){
		case BFFM_INITIALIZED:
			if( select != NULL )
				SendMessage(hWnd,BFFM_SETSELECTION,TRUE,(LPARAM)select->toWCHAR());
			break;
	}
	return 0;
}

void Interface::openDropFile(WPARAM wParam)
{
	bool result = false;
	int szSize = 2 * MAX_PATH;
	WCHAR *szFile = new WCHAR[szSize];
	szFile[0] = '\0';

	if( DragQueryFile((HDROP)wParam,0,szFile,szSize) ){
		result = this->core->open(szFile);
		DragFinish((HDROP)wParam);
	}
	delete [] szFile;
}

bool Interface::saveFile()
{
	bool result = false;

	ChildCore *child = this->core->getActiveChild();
	char *chstr = child->getFilepath()->tochar();
	OFSTRUCT ofs;

	if( OpenFile(chstr,&ofs,OF_EXIST) == HFILE_ERROR){
		result = this->saveFileAs();
	}
	else {
		FwCHAR *name = new FwCHAR();
		name->getFilenameFrom(this->core->getActiveChild()->getFilepath());

		result = this->core->getActiveChild()->save(
			this->core->getActiveChild()->getFilepath()->toWCHAR(),
			Explorer::getType(name->toWCHAR())
			);
		delete name;
	}
	delete [] chstr;

	return result;
}

bool Interface::saveFileAs()
{
	OPENFILENAME ofn;
	bool result = false;

	int szSize = 2 * MAX_PATH;
	WCHAR *szFile = new WCHAR[szSize];
	WCHAR *szFileName = new WCHAR[szSize];
	szFile[0] = '\0';

	FwCHAR *folder = new FwCHAR();
	folder->getFolderFrom(
		this->core->getActiveChild()->getFilepath() );
	FwCHAR *file = new FwCHAR();
	file->getFilenameFrom(
		this->core->getActiveChild()->getFilepath() );
	FwCHAR *filename = new FwCHAR(file->toWCHAR());
	filename->stripExtension();

	wcscpy(szFile,file->toWCHAR());
	wcscpy(szFileName,filename->toWCHAR());

	int type = Explorer::getType(file->toWCHAR());

	delete file;
	delete filename;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->core->getWindowHandle();
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szFile;
	ofn.lpstrInitialDir = folder->toWCHAR();

	ofn.lpfnHook = Interface::saveHookProc;
	ofn.nMaxFile = szSize;
	ofn.nMaxFileTitle = szSize;
	ofn.lpstrFilter = SAVEFILEFILTER;
	ofn.nFilterIndex = this->extToFilter(type);
	ofn.lpstrDefExt = this->extToOfn(type);
	
	ofn.Flags =
		OFN_EXPLORER |
		OFN_ENABLEHOOK |
		OFN_PATHMUSTEXIST |
		OFN_OVERWRITEPROMPT |
		OFN_NONETWORKBUTTON;

	//this->blockMBB();
	if( GetSaveFileName(&ofn) ){
		result = this->core->getActiveChild()->save(ofn.lpstrFile,this->ofnToExt(ofn.nFilterIndex));
		if( result == true ){
			this->addLastItem(ofn.lpstrFile);
		}
	}
	//this->timerMBB();

	delete [] szFile;
	delete [] szFileName;
	delete folder;

	return result;
}
/*
Convert save dialog box extension combo index to application defined types
*/
int Interface::ofnToExt(int filterId)
{
	switch(filterId){
		case 1:
			return TYPE_FED;
		case 2:
			return TYPE_JPG;
		case 3:
			return TYPE_PNG;
		case 4:
			return TYPE_GIF;
		case 5:
			return TYPE_TIFF;
		case 6:
			return TYPE_BITMAP;
		case 7:
			return TYPE_ICO;
	}
	return FERROR;
}
int Interface::extToFilter(int ext)
{
	switch(ext){
		case TYPE_FED:
			return 1;
		case TYPE_JPG:
			return 2;
		case TYPE_PNG:
			return 3;
		case TYPE_GIF:
			return 4;
		case TYPE_TIFF:
			return 5;
		case TYPE_BITMAP:
			return 6;
		case TYPE_ICO:
			return 7;
		default:
			return NULL;
	}
}
WCHAR *Interface::extToOfn(int ext)
{
	WCHAR *newExt = NULL;
	switch(ext){
		case TYPE_FED:
			newExt = L"fed";
			break;
		case TYPE_JPG:
			newExt = L"jpg";
			break;
		case TYPE_PNG:
			newExt = L"png";
			break;
		case TYPE_GIF:
			newExt = L"gif";
			break;
		case TYPE_TIFF:
			newExt = L"tiff";
			break;
		case TYPE_BITMAP:
			newExt = L"bmp";
			break;
		case TYPE_ICO:
			newExt = L"ico";
			break;
		default:
			newExt = L"fed";
			break;
	}
	return newExt;
}

UINT_PTR CALLBACK Interface::saveHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPOFNOTIFY notify;
	WCHAR *newExt;

	switch(message){
		case WM_NOTIFY:
			notify = (LPOFNOTIFY)lParam;
			switch(notify->hdr.code){
				case CDN_TYPECHANGE:
					newExt = NULL;
					switch(notify->lpOFN->nFilterIndex){
						case 1:
							newExt = L"fed";
							break;
						case 2:
							newExt = L"jpg";
							break;
						case 3:
							newExt = L"png";
							break;
						case 4:
							newExt = L"gif";
							break;
						case 5:
							newExt = L"tiff";
							break;
						case 6:
							newExt = L"bmp";
							break;
						case 7:
							newExt = L"ico";
							break;
					}
					CommDlg_OpenSave_SetDefExt(notify->hdr.hwndFrom, (LPARAM)newExt);
					break;
			}
			break;
	}
	return 0;
}

void Interface::close()
{
	this->core->close();
}

void Interface::closeAll()
{
	while( this->core->getActiveChild() != NULL )
		this->core->close();
}

void Interface::undo()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL )
		child->getWorkspace()->getHistory()->undo();
}

void Interface::redo()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL )
		child->getWorkspace()->getHistory()->redo();
}

void Interface::cut()
{
	this->toolToMenu(this->core->getToolws()->getToolwCC()->toolCut);
}

void Interface::copy()
{
	this->toolToMenu(this->core->getToolws()->getToolwCC()->toolCopy);
}

void Interface::copyMerged()
{
	this->toolToMenu(this->core->getToolws()->getToolwCC()->toolCopyMerged);
}

void Interface::paste()
{
	this->toolToMenu(this->core->getToolws()->getToolwCC()->toolPaste);
}

void Interface::clear()
{
	this->toolToMenu(this->core->getToolws()->getToolwCC()->toolClear);
}
/*
Call matrix filter dialog or apply the recently used filter
*/
void Interface::filter(int id)
{
	switch(id){
		case ID_FILTER_LASTFILTER:
			((ToolFilter *)this->core->getToolws()->getToolwCC()->toolFilter)->activateAgain();
			break;
		default:
			((ToolFilter *)this->core->getToolws()->getToolwCC()->toolFilter)->setFilterId(id);
			this->toolToMenu(this->core->getToolws()->getToolwCC()->toolFilter);
			break;
	}
}

void Interface::toolToMenu(Tool *tool)
{
	tool->activate();
}

void Interface::resizeImage()
{
	this->core->getDialogs()->showDialog((LPCTSTR)IDD_RES,(DLGPROC)Dialogs::processDlg_Res);
}

void Interface::showMenu()
{
	if( this->fullscreen == true && this->menuvisible == false ){
		SetMenu(this->core->getWindowHandle(),this->windowMenu);
		this->menuvisible = true;
		this->showCursor();

		if( this->core->getDrawer() != NULL )
			this->core->getDrawer()->setMenuheight(this->menuheight);
	}
}
void Interface::hideMenu()
{
	if( this->fullscreen == true && this->menuvisible == true ){
		SetMenu(this->core->getWindowHandle(),NULL);
		this->menuvisible = false;
		if( this->core->getDrawer() != NULL )
			this->core->getDrawer()->setMenuheight();
	}
}

void Interface::addLastItem(WCHAR *item)
{
	this->lastItems->add(new FwCHAR(item));

	if( this->lastItems->getCount() > LASTITEMS ){
		this->lastItems->removeHead();
	}
}

void Interface::callLastItem(int id)
{
	if( this->lastItemIds->getCount() > 0 ){
		this->lastItemIds->gotoHead();
		do {
			if( this->lastItemIds->getThat()->X == id ){
				FwCHAR *path = (FwCHAR *)this->lastItemIds->getThat()->Y;
				this->core->open(path->toWCHAR(),false);
				return;
			}
		} while( this->lastItemIds->next() == true );
	}
}

bool Interface::isFullscreen()
{
	return this->fullscreen;
}
/*
Not used
*/
void Interface::setMessage(WCHAR *message)
{
	this->textmessage = message;
	this->updateText();
	this->textmessage = NULL;
}

void Interface::processMenu(int id)
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	if( drawer != NULL )
		layer = drawer->getTopmost();

	this->callLastItem(id);

	switch(id){
		case ID_FILE_NEW:
			this->newFile();
			break;
		case ID_FILE_OPEN:
			this->openFile();
			break;
		case ID_FILE_OPENFOLDER:
			this->openFolder();
			break;
		case ID_FILE_CLOSE:
			this->close();
			break;
		case ID_FILE_CLOSEALL:
			this->closeAll();
			break;
		case ID_FILE_SAVE:
			this->saveFile();
			break;
		case ID_FILE_SAVEAS:
			this->saveFileAs();
			break;
		case ID_FILE_EXTRACT:
			this->core->extract();
			break;
		case ID_FILE_SETASWALLPAPER:
			this->core->setwall();
			break;
		case ID_FILE_EXIT:
			DestroyWindow(this->core->getWindowHandle());
			break;
		case ID_EDIT_UNDO:
			this->undo();
			break;
		case ID_EDIT_REDO:
			this->redo();
			break;
		case ID_EDIT_CUT:
			this->cut();
			break;
		case ID_EDIT_COPY:
			this->copy();
			break;
		case ID_EDIT_COPYMERGED:
			this->copyMerged();
			break;
		case ID_EDIT_PASTE:
			this->paste();
			break;
		case ID_EDIT_CLEAR:
			this->clear();
			break;
		case ID_IMAGE_IMAGESIZE:
			this->resizeImage();
			break;
		case ID_LAYER_ADD:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->addLayerAfter();
				this->core->getActiveChild()->getWorkspace()->updateToolws();
				this->core->getActiveChild()->getWorkspace()->update();
			}
			break;
		case ID_LAYER_DELETE:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->deleteLayer(
					this->core->getActiveChild()->getWorkspace()->getSelectedLayer() );
				this->core->getActiveChild()->getWorkspace()->updateToolws();
				this->core->getActiveChild()->getWorkspace()->update();
			}
		case ID_LAYER_MOVEUP:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->moveSelectedUp();
				this->core->getActiveChild()->getWorkspace()->updateToolws();
				this->core->getActiveChild()->getWorkspace()->update();
			}
			break;
		case ID_LAYER_MOVEDOWN:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->moveSelectedDown();
				this->core->getActiveChild()->getWorkspace()->updateToolws();
				this->core->getActiveChild()->getWorkspace()->update();
			}
			break;
		case ID_LAYER_MERGEDOWN:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->mergeDownSelected();
				this->core->getActiveChild()->getWorkspace()->update();
			}
			break;
		case ID_LAYER_RASTER:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->rasterizeSelected();
			}
			break;
		case ID_LAYER_EDITTEXT:
			if( this->core->getActiveChild() != NULL ){
				if( this->core->getActiveChild()->getWorkspace()->getSelectedLayer()->getType() == FRM_TEXT ){
					this->core->getToolws()->getToolwCC()->setTool(this->core->getToolws()->getToolwCC()->toolText );
					((ToolText *)this->core->getToolws()->getToolwCC()->toolText)->editText(
						(FrameText *)this->core->getActiveChild()->getWorkspace()->getSelectedLayer() );
				}
			}
			break;
		case ID_LAYER_LOCKED:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->getSelectedLayer()->toggleLock();
				this->core->getActiveChild()->getWorkspace()->updateToolws();
				this->core->getActiveChild()->getWorkspace()->update();
			}
			break;
		case ID_LAYER_VISIBLE:
			if( this->core->getActiveChild() != NULL ){
				this->core->getActiveChild()->getWorkspace()->getSelectedLayer()->toggleVisibility();
				this->core->getActiveChild()->getWorkspace()->updateToolws();
				this->core->getActiveChild()->getWorkspace()->update();
			}
			break;
		case ID_SELECT_SELECTALL:
			((ToolSelectRect *)this->core->getToolws()->getToolwCC()->toolSelectRect)->setSelectAll();
			break;
		case ID_SELECT_DESELECT:
			((ToolSelectRect *)this->core->getToolws()->getToolwCC()->toolSelectRect)->deselect();
			break;
		case ID_SELECT_INVERSESELECTION:
			((ToolSelectRect *)this->core->getToolws()->getToolwCC()->toolSelectRect)->setSelectInverse();
			break;
		case ID_FILTER_LASTFILTER:
			((ToolFilter *)this->core->getToolws()->getToolwCC()->toolFilter)->activateAgain();
			break;
		case ID_FILTER_BLUR:
		case ID_FILTER_GAUSSIANBLUR:
		case ID_FILTER_SHARPEN:
		case ID_FILTER_EDGETRACE:
		case ID_EMBOSS_EAST:
		case ID_EMBOSS_SOUTH:
		case ID_EMBOSS_SOUTHEAST:
		case ID_FILTER_HIGHLIGHT:
		case ID_FILTER_DEFOCUS:
		case ID_FILTER_OLDSTONE:
		case ID_FILTER_CUSTOM:
			this->filter(id);
			break;
		case ID_VIEW_FULLSCREEN:
			this->setFullscreen(AUTO);
			this->update();
			break;
		case ID_VIEW_THUMBNAILS:
			if( drawer != NULL ){
				drawer->showThumbs();
				this->updateMenu();
			}
			break;
		case ID_VIEW_LIST:
			if( drawer != NULL ){
				drawer->showList();
				this->updateMenu();
			}
			break;
		case ID_FITTO_SCREENIFOVERSIZED:
			if( layer != NULL ){
				layer->setFitmode(FITSCREENOV);
				this->update();
			}
			break;
		case ID_FITTO_SCREEN:
			if( layer != NULL ){
				layer->setFitmode(FITSCREEN);
				this->update();
			}
			break;
		case ID_FITTO_WIDTH:
			if( layer != NULL ){
				layer->setFitmode(FITWIDTH);
				this->update();
			}
			break;
		case ID_FITTO_HEIGHT:
			if( layer != NULL ){
				layer->setFitmode(FITHEIGHT);
				this->update();
			}
			break;
		case ID_FITTO_NUMPAD:
			if( layer != NULL ){
				layer->setFitmode(FITNUMPAD);
				this->update();
			}
			break;
		case ID_FITTO_LEFT:
			if( layer != NULL ){
				layer->setSidemode(LEFT);
				this->updateMenu();
			}
			break;
		case ID_FITTO_RIGHT:
			if( layer != NULL ){
				layer->setSidemode(RIGHT);
				this->updateMenu();
			}
			break;
		case ID_ZOOM_100:
			if( layer != NULL ){
				layer->zoomat();
				this->updateText();
			}
			break;
		case ID_ZOOM_75:
			if( layer != NULL ){
				layer->zoomat(0.75);
				this->updateText();
			}
			break;
		case ID_ZOOM_50:
			if( layer != NULL ){
				layer->zoomat(0.5);
				this->updateText();
			}
			break;
		case ID_ZOOM_25:
			if( layer != NULL ){
				layer->zoomat(0.25);
				this->updateText();
			}
			break;
		case ID_ZOOM_200:
			if( layer != NULL ){
				layer->zoomat(2.0);
				this->updateText();
			}
			break;
		case ID_ZOOM_300:
			if( layer != NULL ){
				layer->zoomat(3.0);
				this->updateText();
			}
			break;
		case ID_ZOOM_500:
			if( layer != NULL ){
				layer->zoomat(5.0);
				this->updateText();
			}
			break;
		case ID_ZOOM_ZOOMIN:
			if( layer != NULL ){
				layer->zoomer(ZOOMLOSTEP);
				this->updateText();
			}
			break;
		case ID_ZOOM_ZOOMOUT:
			if( layer != NULL ){
				layer->zoomer(-ZOOMLOSTEP);
				this->updateText();
			}
			break;
		case ID_ROTATE_RESET:
			if( layer != NULL ){
				layer->rotateReset();
				this->updateText();
			}
			break;
		case ID_ROTATE_90:
			if( layer != NULL ){
				layer->rotate(1);
				this->updateText();
			}
			break;
		case ID_ROTATE_180:
			if( layer != NULL ){
				layer->rotate(2);
				this->updateText();
			}
			break;
		case ID_ROTATE_270:
			if( layer != NULL ){
				layer->rotate(3);
				this->updateText();
			}
			break;
		case ID_VIEW_FLOWSCROLL:
			if( layer != NULL ){
				layer->setSidedraw();
				this->updateMenu();
			}
			break;
		case ID_WINDOW_TILEHORIZONTALLY:
			this->core->mdiTileHor();
			break;
		case ID_WINDOW_TILEVERTICALLY:
			this->core->mdiTileVer();
			break;
		case ID_WINDOW_CASCADE:
			this->core->mdiCascade();
			break;
		case ID_WINDOW_ARRANGE:
			this->core->mdiArrange();
			break;
		case ID_WINDOW_SHOWINFO:
			if( this->core->getToolws() != NULL )
				this->core->getToolws()->toggleBoxInfo();
			break;
		case ID_WINDOW_SHOWLAYERS:
			if( this->core->getToolws() != NULL )
				this->core->getToolws()->toggleBoxLayers();
			break;
		case ID_WINDOW_SHOWHISTORY:
			if( this->core->getToolws() != NULL )
				this->core->getToolws()->toggleBoxHistory();
			break;
		case ID_HELP_ABOUTFIEW:
			this->core->getDialogs()->showDialog(
				(LPCTSTR)IDD_IMG,
				(DLGPROC)Core::processDialogs,
				(LPARAM)IDR_ABOUT);
			break;
		/*case ID_HELP_MANUAL:
			if( drawer != NULL ){
				drawer->showManual();
				this->updateMenu();
			}*/
			break;
	}
}
void Interface::processKeys(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_KEYDOWN:
			this->capKeyDown(wParam);
			break;
		case WM_KEYUP:
			this->capKeyUp(wParam);
			break;
	}
}
void Interface::processMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_LBUTTONDOWN:
			this->capMouseDown(lParam,wParam,LEFT);
			break;
		case WM_LBUTTONUP:
			this->capMouseUp(lParam,wParam,LEFT);
			break;
		case WM_MBUTTONDOWN:
			this->capMouseDown(lParam,wParam,MID);
			break;
		case WM_MBUTTONUP:
			this->capMouseUp(lParam,wParam,MID);
			break;
		case WM_RBUTTONDOWN:
			this->capMouseDown(lParam,wParam,RIGHT);
			break;
		case WM_RBUTTONUP:
			this->capMouseUp(lParam,wParam,RIGHT);
			break;
		case WM_MOUSEWHEEL:
			this->capMouseWheel(lParam,wParam);
			break;
		case WM_MOUSEMOVE:
			this->capMouseMove(lParam,wParam);
			break;
	}
}

void Interface::capKeyDown(WPARAM wParam)
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	if( drawer != NULL )
		layer = drawer->getTopmost();

	switch(wParam){
		case VK_UP:
			if( layer != NULL )
				layer->scrollVer(SCROLLSTEP);
			break;
		case VK_DOWN:
			if( layer != NULL )
				layer->scrollVer(-SCROLLSTEP);
			break;
		case VK_LEFT:
			if( layer != NULL )
				layer->scrollHor(SCROLLSTEP);
			break;
		case VK_RIGHT:
			if( layer != NULL )
				layer->scrollHor(-SCROLLSTEP);
			break;
		case VK_ADD:
			this->processMenu(ID_ZOOM_ZOOMIN);
			break;
		case VK_SUBTRACT:
			this->processMenu(ID_ZOOM_ZOOMOUT);
			break;
		case VK_SPACE:
		case VK_RETURN:
			if( layer != NULL ){
				layer->nextImage();
				layer->setCancel(false);
			}
			break;
		case VK_INSERT:
		case VK_NUMPAD0:
			if( layer != NULL )
				layer->prevImage();
			break;
		case VK_PRIOR:
		case VK_NUMPAD9:
			if( layer != NULL )
				layer->scrollSet(-layer->getMaxrollHor(),layer->getMaxrollVer());
			break;
		case VK_NUMPAD8:
			if( layer != NULL )
				layer->scrollSet(0,layer->getMaxrollVer());
			break;
		case VK_HOME:
		case VK_NUMPAD7:
			if( layer != NULL )
				layer->scrollSet(layer->getMaxrollHor(),layer->getMaxrollVer());
			break;
		case VK_NUMPAD6:
			if( layer != NULL )
				layer->scrollSet(-layer->getMaxrollHor(),0);
			break;
		case VK_NUMPAD5:
			if( layer != NULL )
				layer->scrollSet(0,0);
			break;
		case VK_NUMPAD4:
			if( layer != NULL )
				layer->scrollSet(layer->getMaxrollHor(),0);
			break;
		case VK_NEXT:
		case VK_NUMPAD3:
			if( layer != NULL )
				layer->scrollSet(-layer->getMaxrollHor(),-layer->getMaxrollVer());
			break;
		case VK_NUMPAD2:
			if( layer != NULL )
				layer->scrollSet(0,-layer->getMaxrollVer());
			break;
		case VK_END:
		case VK_NUMPAD1:
			if( layer != NULL )
				layer->scrollSet(layer->getMaxrollHor(),-layer->getMaxrollVer());
			break;
		case VK_ESCAPE:
			if( this->fullscreen == true ){
				this->setFullscreen(NO);
				this->updateMenu();
			}
			else if( drawer != NULL ){
				if( layer != NULL )
					layer->setCancel();

				drawer->hideOverlay();
				this->updateMenu();
			}
			break;
		case VK_MULTIPLY:
			this->processMenu(ID_VIEW_FULLSCREEN);
			break;
		case VK_DELETE:
		case VK_DECIMAL:
		case VK_OEM_COMMA:
		case VK_OEM_PERIOD:
			this->processMenu(ID_FITTO_SCREENIFOVERSIZED);
			break;
		case VK_SHIFT:
				this->setFullpath();
			break;
		case VK_DIVIDE:
			if( layer != NULL ){
				layer->rotate(1);
				this->updateText();
			}
			break;
		case VK_BACK:
			if( layer != NULL ){
				layer->repos();
				this->update();
			}
			break;
		case VK_O:
			if( layer != NULL )
				layer->setGifDir();
			break;
		case VK_P:
			if( layer != NULL )
				layer->setGifDir(NULL);
			break;
		case VK_OEM_4:
			this->processMenu(ID_FITTO_LEFT);
			break;
		case VK_OEM_6:
			this->processMenu(ID_FITTO_RIGHT);
			break;
	}
}
void Interface::capKeyUp(WPARAM wParam)
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	if( drawer != NULL )
		layer = drawer->getTopmost();

	switch(wParam){
		case VK_ADD:
		case VK_SUBTRACT:
			if( layer != NULL )
				layer->zoomend();
			break;
	}
}

void Interface::capMouseDown(LPARAM lParam, WPARAM wParam, int button)
{
	if( this->mWH == true )
		return;

	if( this->mState == HTCLIENT && this->mBB == false ){
		this->mButton = button;
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if( button == MID ){
			this->setCursor(CURSOR_SCROLL);
			this->mX = x;
			this->mY = y;

			if( this->mMM == false ){
				//this->setCapture(true);
				this->mMM = true;
			}
			else {
				this->mMM = false;
				this->dX = this->dY = 0;
				this->setCursor(CURSOR_CLIENT);
				KillTimer(this->core->getWindowHandle(),TIMER_MMM);
			}
		}
	}
}
void Interface::capMouseMove(LPARAM lParam, WPARAM wParam)
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	Overlay *overlay = NULL;
	if( drawer != NULL ){
		layer = drawer->getTopmost();
		overlay = drawer->getOverlay();
	}

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);

	this->showCursor();

	if( overlay != NULL ){
		RECT lay = overlay->getOverlayRect();
		if( x > lay.left && x < lay.right &&
			y > lay.top && y < lay.bottom )
			this->setCursor(CURSOR_HAND);
		else
			this->setCursor(CURSOR_CLIENT);
	}
	if( this->fullscreen == true ){
		if( y < this->menuheight )
			this->showMenu();
		else
			this->hideMenu();
	}
	if( this->mButton == MID ){
		if( layer != NULL )
			layer->scroll(x - this->mX,y - this->mY);

		this->mX = x;
		this->mY = y;

		this->mMM = false;
	}
	if( this->mMM == true ){
		this->dX = (int)((this->mX - x)/SCROLL_DIV);
		this->dY = (int)((this->mY - y)/SCROLL_DIV);
	}
	if( this->fullscreen == true && this->menuvisible == false )
		SetTimer(this->core->getWindowHandle(),TIMER_MCH,CURSOR_HIDE,NULL);
}
void Interface::capMouseUp(LPARAM lParam, WPARAM wParam, int button)
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	if( drawer != NULL )
		layer = drawer->getTopmost();

	if( this->mState == HTCLIENT && this->mBB == false && this->mWH == false ){
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if( button == LEFT )
			if( layer != NULL )
				layer->prevImage(x,y);
		if( button == RIGHT )
			if( layer != NULL )
				layer->nextImage(x,y);
	}
	if( button == MID ){
		this->mBB = false;

		if( this->mMM == true ){
			//this->setCapture(false);
			SetTimer(this->core->getWindowHandle(),TIMER_MMM,SCROLL_TOUT,NULL);
		}
		else {
			this->setCursor(CURSOR_CLIENT);
		}
	}
	if( this->mButton == button )
		this->mButton = 0;
	if( this->mWH == true ){
		if( button == RIGHT )
			if( layer != NULL )
				layer->zoomend();
		this->mWH = false;
	}
}
void Interface::capMouseWheel(LPARAM lParam, WPARAM wParam)
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	if( drawer != NULL )
		layer = drawer->getTopmost();

	int dir = GET_WHEEL_DELTA_WPARAM(wParam);
	int key = GET_KEYSTATE_WPARAM(wParam);

	if( dir > 0 ){
		if( key == MK_RBUTTON ){
			this->mWH = true;
			this->processMenu(ID_ZOOM_ZOOMIN);
		}
		else if( key == MK_LBUTTON ){
			this->mWH = true;
			if( layer != NULL )
				layer->scrollHor(SCROLLSTEP);
		}
		else
			if( layer != NULL )
				layer->scrollVer(SCROLLSTEP);
	}
	else {
		if( key == MK_RBUTTON ){
			this->mWH = true;
			this->processMenu(ID_ZOOM_ZOOMOUT);
		}
		else if( key == MK_LBUTTON ){
			this->mWH = true;
			if( layer != NULL )
				layer->scrollHor(-SCROLLSTEP);
		}
		else
			if( layer != NULL )
				layer->scrollVer(-SCROLLSTEP);
	}
}
LRESULT Interface::processMouseState(LPARAM lParam, LRESULT lResult)
{
	this->mState = lResult;
	return lResult;
}

void Interface::setCapture(bool state)
{
	if( state == true )
		SetCapture(this->core->getWindowHandle());
	else
		ReleaseCapture();
}
/*
Mouse cursor management
*/
void Interface::showCursor()
{
	KillTimer(this->core->getWindowHandle(),TIMER_MCH);
	this->setCursor(this->cursor);
}
HCURSOR Interface::setCursor(LPCWSTR name)
{
	HCURSOR lastCursor = LoadCursor(NULL,IDC_ARROW);

	if( name == NULL )
		lastCursor = this->setCursor(lastCursor);
	else
		lastCursor = this->setCursor(LoadCursor(NULL,name));

	return lastCursor;
}
HCURSOR Interface::setCursor(HCURSOR cursor)
{
	if( cursor != NULL )
		this->cursor = cursor;

	return SetCursor(cursor);
}
/*
Not used
*/
void Interface::setFullpath()
{
	if( this->fullpath == true )
		this->fullpath = false;
	else
		this->fullpath = true;
	this->updateText();
}
/*
Toggle fullscreen mode
*/
void Interface::setFullscreen(int mode){
	HWND window = this->core->getWindowHandle();

	if( this->fullscreen == false && (mode == AUTO || mode == YES) ){

		this->windowInfo.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(window,&this->windowInfo);
		this->windowLong = GetWindowLong(window,GWL_STYLE);

		if( SetWindowLong(window,GWL_STYLE,WS_POPUP | WS_VISIBLE) == false ){
			this->core->messageBox_Error(MESSAGE_CANNOTFULLSCR);
			return;
		}
		if( SetWindowPos(window,HWND_NOTOPMOST,0,0,
							GetDeviceCaps(GetDC(NULL),HORZRES),
							GetDeviceCaps(GetDC(NULL),VERTRES),
							SWP_FRAMECHANGED) == false ){
			this->core->messageBox_Error(MESSAGE_CANNOTFULLSCR);
			return;
		}
		RECT client;
		GetClientRect(this->core->getWindowHandle(),&client);

		this->menuheight = GetDeviceCaps(GetDC(NULL),VERTRES) - client.bottom;
		this->fullscreen = true;
		//this->hideMenu();
	}
	else if( this->fullscreen == true && (mode == AUTO || mode == NO) ){
		int x, y, w, h;
		x = this->windowInfo.rcWindow.left;
		y = this->windowInfo.rcWindow.top;
		w = this->windowInfo.rcWindow.right - this->windowInfo.rcWindow.left;
		h = this->windowInfo.rcWindow.bottom - this->windowInfo.rcWindow.top;
		
		//this->showMenu();
		this->menuheight = 0;
		this->fullscreen = false;		

		SetWindowLong(window,GWL_STYLE,this->windowLong);
		SetWindowPos(window,HWND_NOTOPMOST,x,y,w,h,SWP_FRAMECHANGED);

		//this->core->getDrawer()->setMenuheight();
	}
	this->update();
}
/*
Block mouse buttons when neccessary
*/
void Interface::blockMBB()
{
	this->mBB = true;
}
void Interface::timerMBB()
{
	SetTimer(this->core->getWindowHandle(),TIMER_MBB,MOUSE_BLOCK,NULL);
}
void Interface::unblockMBB()
{
	this->mBB = false;
}

void Interface::movemMM()
{
	Drawer *drawer = this->core->getDrawer();
	Layer *layer = NULL;
	if( drawer != NULL )
		layer = this->core->getDrawer()->getTopmost();
	if( layer != NULL )
		layer->scroll(this->dX,this->dY);
}