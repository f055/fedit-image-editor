/*
FeditCofigs.h
Contains all configurable variables, that is they can be changed
without interfering the workflow of the application
*/

/* confs */
#define DEBUG									// unused
//#define DISKCACHING							// force Cacher to copy all data to memory instead of reading from disk files

#define FONT				L"Arial"			// unused
#define FILE_EXT			L".fed"				// default Fedit Document extension
#define FONT_STATIC			L"MS Shell Dlg"		// default interface font
#define FONTSIZE			11					// unused
#define FONTSIZE_STATIC		8					// default interface font size
#define DIRMERGE			L"\\*"				// dorectory browse all symbol

#define STGWSPC				L"0"				// object identifiers of compound file
#define STGINFO				L"I"				// that is used to create a .fed file
#define STGDATA				L"D"				// D - layer data
#define STGI				'I'					// I - layer info
#define STGD				'D'

#define TIPTOUT				600					// tooltip timeout in milliseconds

#define HISTHUMBSIZE		22					// History Window icon size

#define GAUSSMAX			2					// Gaussian Blur maximal value
#define SHARPENMAX			10					// Sharpen filter maximal value
#define FILTERMAX			30					// Defined filter matrix maximal value

#define MATRIXMIN			-50					// Custom filter matrix limit values
#define MATRIXMAX			50

#define LASTITEMS			7					// number of recently opened files in menu

#define ADD					1
#define REMOVE				2

#define LAYNAMEMARGIN		10					// Layers Window margin
#define LAYTHUMBSIZE		40					// Layers Window thumbnail size

#define IDJUMP				10					// identifier jump for list windows
												// like Layers of History,
												// each controls in one row of the list
												// are identified in the following way:
												// first id is a multiplication of IDJUMP
												// next ones are: first id + i, where i > 0 && i < IDJUMP
												// thus the row number of any processed control
												// can be obtaines by control id % IDJUMP

#define MAXFONTSIZE			1000				// maximum font size of ToolText
#define MAXOPACITY			100					// maximum layer opacity

#define MAX_WIDTH			10000				// maximum document width
#define MAX_HEIGHT			10000				// maximum document height

#define TW_BX_MINW			200					// TW_ define various Tool Windows
#define TW_BX_MINH			150					// properties of display

#define TW_CC_POPTOUT		500					// time needed to hold the button on Control Center
												// in order to a switcher popup to appear

#define TW_CC_SEP			2					// thickness of Control Center separator
#define TW_CC_BUTW			25					// witdth of Control Center buttons
#define TW_CC_BUTH			21					// height of Control Center buttons

#define TW_DOCK_TOPMARGIN	5					// margin for contents in the Dock Window
#define TW_DOCK_BOTMARGIN	5
#define TW_DOCK_LEFTMARGIN	5

#define TW_DOCK_W			800					// Dock Window default width
#define TW_DOCK_H			35					// Dock Window default height
#define TW_DOCK_DKGRAB		10					// Dock Window grabbing distance needed to undock
#define TW_DOCK_UDKGRAB		15					// Dock Window docking distance need to dock

#define TWPOS_CC			100					// Default Control Center position
#define TWPOS_DOCK			75					// Default Dock Window position when initialised undocked

#define MINCHISIZE			150					// minimal child size

#define CHIPADDING			50					// child windows top-left padding on screen
#define CHIPADMULTI			15					// child windows padding modificator for concurrent children
#define CHICTRLIMIT			10					// child windows limit of padding reset

#define INIT_TOUT			10					// initial timeout

#define RC_DLL				L"DLL"				// resource identifier
#define RC_PNG				L"PNG"				// resource identifier
#define RARDLL				L"unrar.dll"		// resource on-disk name

#define CACHE_SIZE			20					// Cacher object size
#define CACHE_LIMIT			10					// Cacher limit - at any moment, stores only LIMIT number
												// of objects before and after the current object

#define MARGIN				20					// default margin
#define FRAME				2					// default frame thickness

#define SCROLLSTEP			10					// scrolling step

#define GRDBGBLOCK			10					// transparency grid block size

#define DRZOOMINIT			100					// initial zoom in integer values
#define DRZOOMLOSTEP		20					// zoom step below initial zoom
#define DRZOOMHISTEP		100					// zoom step above initial zoom

#define ZOOMHISTEP			1.0					// initial zoom in double values
#define ZOOMLOSTEP			0.2					// zoom step below initial zoom
#define ZOOMINIT			1.0					// zoom step above initial zoom

#define ZOOMTOOLIMIT		3					// miniaml size of zoom tool slecting rectangle

#define MINDELAY			50					// minimal thread delay

#define CURSOR_CLIENT		IDC_ARROW
#define CURSOR_SCROLL		IDC_SIZEALL
#define CURSOR_WAIT			IDC_WAIT
#define CURSOR_ARRWAIT		IDC_APPSTARTING
#define CURSOR_HAND			IDC_HAND

#define CURSOR_HIDE			2000				// cursor hide timout
#define MOUSE_BLOCK			200					// mouse buttons block timeout
#define SCROLL_TOUT			50					// scroll timeout
#define OVL_HIDEOUT			100					// unused

#define SCROLL_DIV			3.0

#define OVL_SIZE			650					// overlay size
#define OVL_MARGIN			30					// overlay margin

#define THB_SIZE			165					// thumbnail view selected thumbnail size
#define THB_SMSIZE			110					// thumbnail view thumbnail size
#define THB_SPACE			10					// thumbnail view separation distnace
#define THB_ROW				5					// thumbnail view number of rows
#define THB_COUNT			12					// thumbnail view number of thumbnails
#define THB_TOUT			500					// thumbnail view progress indicator timeout

#define LST_X				5					// values for list view (unused)
#define LST_Y				30
#define LST_MAXLEN			96
#define LST_MOD				5

// application defined colors

#define MAKECLR(dword) (Color(GetRValue(dword),GetGValue(dword),GetBValue(dword)))
#define MAKEALPHA(alpha,dword) (Color(alpha,GetRValue(dword),GetGValue(dword),GetBValue(dword)))

#define CLR_BLACK			Color(255,0,0,0)
#define CLR_WHITE			Color(255,255,255,255)
#define CLR_WHITE_ALPHA		Color(175,255,255,255)
#define CLR_FRAME_LIGHT		MAKECLR(GetSysColor(COLOR_BTNFACE))
#define CLR_FRAME_DARK		MAKECLR(GetSysColor(COLOR_BTNSHADOW))
#define CLR_LITE			MAKEALPHA(200,GetSysColor(COLOR_BTNFACE))
#define CLR_DIRK			MAKEALPHA(200,GetSysColor(COLOR_BTNSHADOW))
#define CLR_CAPTION_ACTIV	MAKECLR(GetSysColor(COLOR_ACTIVECAPTION))
#define CLR_CAPTION_GRAD	MAKECLR(GetSysColor(COLOR_GRADIENTACTIVECAPTION))
