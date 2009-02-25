class Core;
class Gridlay;
class ToolFilter;

struct CMYKCOLOR { UINT C,M,Y,K; };
struct HSVCOLOR { UINT H,S,V; };
struct LabCOLOR { UINT L,a,b; };

class Dialogs
{
public:
	static Core *core;

	Dialogs(Core *core);
	~Dialogs();

	int showDialog(LPCTSTR resource, DLGPROC dlgproc, LPARAM lParam = NULL);

	static LRESULT CALLBACK processDlg_New(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processDlg_Clr(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processDlg_Res(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK processDlg_Fil(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static void limitSliderInt(HWND hInput, HWND hWnd, int min, int max, int val);
	static void limitInputInt(HWND hWnd, int min, int max);
	static void limitInputText(HWND hWnd, int min, int max);
	static LRESULT CALLBACK procSliderInt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK procInputInt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK procInputText(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static void editStaticText(HWND hWnd, int x, int y);
	static LRESULT CALLBACK procStaticEditText(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK procStaticEditEdit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static HWND ownerdrawButton(HWND hOwner, WCHAR *icon, int x, int y, int w, int h, int id, BOOL autocheck);
	static HWND ownerdrawButton(HWND hWnd, HWND hOwner, BOOL autocheck);
	static void paintButton(LPDRAWITEMSTRUCT button);
	static LRESULT CALLBACK processButton(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static HWND colorPicker(HWND hOwner, Color color, int x, int y, int id, int w = NULL, int h = NULL);
	static LRESULT CALLBACK processPicker(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

class DialogHelper_Clr
{
protected:
	static const int intFieldWidth = 256;
	static const int intSlidWidth = 20;
	static const int intCmpWidth = 70;

	static DialogHelper_Clr *that;

	HWND windowHandle, hField, hSlid, hCmp;
	WNDPROC procField, procSlid;

	Bitmap *bmpField, *bmpSlid, *bmpCmp;
	Color clrCurrent, clrNew;
	CMYKCOLOR clrCmyk;
	HSVCOLOR clrHSV;
	LabCOLOR clrLab;

public:
	DialogHelper_Clr(HWND hWnd, Color current);
	~DialogHelper_Clr();

	Color getNewColor();
	Color getOldColor();

	void valueChange(int ctrlId);
	void radioChange();

	static LRESULT CALLBACK procNewField(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK procNewSlid(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void renderField();
	void renderSlid();
	void renderCmp();
	void render();

	void setField();
	void setSlid();
	void setCmp();
	void setCtrls(int except = NULL);

	void setBitmap(Bitmap *bmp, int itemId);

	int getCheckedRadio();
};

class DialogHelper_Fil
{
protected:
	static DialogHelper_Fil *that;

	HWND windowHandle, hThumb;
	ToolFilter::Info *fi, fiThumb;
	Rect clip;

	Gridlay *grid;

	int **customMatrixIds;
	bool isMoving, isTracking;
	POINTS lastMouse;

	WNDPROC procThumb;

public:
	DialogHelper_Fil(HWND hWnd, ToolFilter::Info *fi);
	~DialogHelper_Fil();

	void valueChange(int ctrlId);
	void checkChange(int ctrlId);

	void setTextMatrix();
	void clearTextMatrix();
	ToolFilter::Matrix getTextMatrix();

	static LRESULT CALLBACK procNewThumb(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void renderThumb();
	void setThumb(Bitmap *bmp);
};