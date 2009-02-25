/*
Fedit.cpp
The standard template code of a window application
*/

#include "stdafx.h"
#include "Core.h"

using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"comctl32.lib");

HINSTANCE hInst;
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

/*
Application Core Object
*/
Core *Miracle;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	MSG msg;
	HACCEL hAccelTable;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	MyRegisterClass(hInstance);

	/*
	Application Core creation
	*/
	Miracle = new Core((WCHAR *)lpCmdLine,hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATORS));

	while(GetMessage(&msg, NULL, 0, 0))
    {
		if (!TranslateMDISysAccel(Miracle->getMdiclientHandle(), &msg) &&
			!TranslateAccelerator(Miracle->getWindowHandle(), hAccelTable, &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	delete Miracle;

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIEW));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= IDCL_FEDIT;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Control Center
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwCC::processMessages;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_CC;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Control Center Popup
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwCC::processPopups;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_CCP;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Dock
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwDock::processMessages;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_DOCK;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Tool in Dock
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwDock::processTools;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_DKTL;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Box
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwBox::processMessages;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_BOX;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Box Contents
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwBox::processContents;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_BXCNT;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Scroll Frame
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwBoxScroll::processMessages;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_BXSCRL;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Scroll Content
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwBoxScroll::processContents;
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE + 1;
	wcex.lpszClassName	= IDCL_TOOLW_BXPANCNT;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Toolbar - Scroll Content
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ToolwBoxScroll::processContents;
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNSHADOW + 1);
	wcex.lpszClassName	= IDCL_TOOLW_BXSCRLCNT;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the Tooltip
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) Toolw::tooltipLabelProc;
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNSHADOW + 1);
	wcex.lpszClassName	= IDCL_FAKETOOLTIP;

	if( RegisterClassEx(&wcex) == NULL )
		return NULL;

	// Register the MDI child window class. 
 
	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) ChildCore::processMessages; 
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIEW)); 
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
    wcex.lpszMenuName	= NULL; 
    wcex.cbWndExtra		= MDI_EXTRADATA; 
    wcex.lpszClassName	= IDCL_MDICHILD; 

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindowEx(WS_EX_ACCEPTFILES,
						 IDCL_FEDIT, APP_TITLE,
						 WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
						 CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
						 NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow | SW_SHOWMAXIMIZED);
   UpdateWindow(hWnd);

   return TRUE;
}
/*
Window procedures redirected to respective procedures of the Core object
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Miracle->processMessages(hWnd,message,wParam,lParam);
}

LRESULT CALLBACK MdiProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Miracle->processMdiMessages(hWnd,message,wParam,lParam);
}