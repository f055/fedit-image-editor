class FwCHAR;
class Core;

class Interface
{
private:
	Core *core;				// owner

	/* fiew */
	LRESULT mState;			// mouse state
	UINT mButton;			// mouse button
	int mX, mY, dX, dY;		// mouse position and difference
	bool mWH,				// mouse wheel flag
		mMM;				// mouse move flag

	FwCHAR *lastFolder;
	List<Point> *lastItemIds;
	List<FwCHAR> *lastItems;

	bool fullscreen, menuvisible, fullpath;
	WCHAR *textmessage;
	int menuheight;

	WINDOWINFO windowInfo;
	HMENU windowMenu, menuFile, menuView, menuHelp;
	LONG windowLong;
	HCURSOR cursor;

public:
	bool mBB;

	Interface(Core *core);
	~Interface();

	void update();
	void updateMenu();
	void updateText();

	void newFile();
	void openFile();
	void openFolder();
	void openDropFile(WPARAM wParam);

	void addLastItem(WCHAR *item);
	void callLastItem(int id);

	bool saveFile();
	bool saveFileAs();

	static int ofnToExt(int filterId);
	static int extToFilter(int ext);
	static WCHAR *extToOfn(int ext);

	void close();
	void closeAll();

	void undo();
	void redo();

	void cut();
	void copy();
	void copyMerged();
	void paste();
	void clear();

	void filter(int id);

	void toolToMenu(Tool *tool);

	void resizeImage();

	bool isFullscreen();

	void setMessage(WCHAR *message);

	void showCursor();
	HCURSOR setCursor(LPCWSTR name = NULL);
	HCURSOR setCursor(HCURSOR cursor);

	void blockMBB();
	void timerMBB();
	void unblockMBB();

	void movemMM();

	void processMenu(int id);
	void processKeys(UINT message, WPARAM wParam, LPARAM lParam);
	void processMouse(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT processMouseState(LPARAM lParam, LRESULT lResult);

	static int CALLBACK BFFCallback(HWND hWnd, UINT message, LPARAM lParam, LPARAM lpData);

private:
	void initialize();
	void initLocale();

	void showMenu();
	void hideMenu();

	void capKeyDown(WPARAM wParam);
	void capKeyUp(WPARAM wParam);

	void capMouseDown(LPARAM lParam, WPARAM wParam, int button);
	void capMouseUp(LPARAM lParam, WPARAM wParam, int button);
	void capMouseWheel(LPARAM lParam, WPARAM wParam);
	void capMouseMove(LPARAM lParam, WPARAM wParam);

	void setCapture(bool state);
	void setFullscreen(int mode);
	void setFullpath();

	static UINT_PTR CALLBACK saveHookProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);

};