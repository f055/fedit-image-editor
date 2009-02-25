/*
FeditEssentials.h
Contains all non-configurable variables strictly important for
the workflow of the application
*/

/* essentials */

#define APP_TITLE		L"Fedit"
#define MAX_LOADSTRING	100
#define FERROR			-1
#define DEFAULT			666
#define ENGLISH_LOCALE	"English_US.1252"

#define ENCBMP L"image/bmp"
#define ENCJPG L"image/jpeg"
#define ENCGIF L"image/gif"
#define ENCTIF L"image/tiff"
#define ENCPNG L"image/png"

/* window classes */
#define IDCL_FEDIT		L"FEDIT"

#define IDCL_EDIT		L"EDIT"
#define IDCL_REDIT		RICHEDIT_CLASS
#define IDCL_REDITDLL	L"RICHED20.DLL"
#define IDCL_BUTTON		L"BUTTON"
#define IDCL_STATIC		L"STATIC"
#define IDCL_COMBO		L"COMBOBOX"
#define IDCL_SLIDER		TRACKBAR_CLASS
#define IDCL_TOOLTIP	TOOLTIPS_CLASS

#define IDCL_MDICHILD		L"MDICHILD"
#define IDCL_FAKEDLG		L"FAKEDIALOG"
#define IDCL_FAKETOOLTIP	L"TOOLTIP"

#define IDCL_TOOLW_CC		L"TOOLW_CC"
#define IDCL_TOOLW_CCB		L"TOOLW_CCBUTTON"
#define IDCL_TOOLW_CCP		L"TOOLW_CCPOPUP"
#define IDCL_TOOLW_DOCK		L"TOOLW_DOCK"
#define IDCL_TOOLW_DKTL		L"TOOLW_DOCKTOOL"
#define IDCL_TOOLW_BOX		L"TOOLW_BOX"
#define IDCL_TOOLW_BXCNT	L"TOOLW_BOXCONTENT"
#define IDCL_TOOLW_BXSCRL	L"TOOLW_BOXSCROLL"
#define IDCL_TOOLW_BXSCRLCNT L"TOOLW_BOXSCROLLCONTENT"
#define IDCL_TOOLW_BXPANCNT L"TOOLW_BOXPANECONTENT"

/* message identifiers */
#define ICC_COLORFORE	1400
#define ICC_COLORBACK	1410
#define ICC_COLORSWT	1420
#define ICC_COLORRES	1430

/* mdi essentials */
#define ID_FIRSTCHILD	60000
#define MDI_EXTRADATA	1024
#define MDICLIENT		L"MDICLIENT"

/* window properties identifiers*/
#define ATOM_ID			L"ID"
#define ATOM_OWNER		L"OWNER"
#define ATOM_THIS		L"THIS"
#define ATOM_PROC		L"WNDPROC"
#define ATOM_TTPROC		L"TOOLTIPPROC"

#define ATOM_LIMIT		L"LIMIT"
#define ATOM_POPLIST	L"POPLIST"
#define ATOM_PARENT		L"PARENT"
#define ATOM_CHILD		L"CHILD"
#define ATOM_POINT		L"POINT"
#define ATOM_BOOL		L"BOOL"
#define ATOM_STL_AC		L"STYLE_AUTOCHECK"
#define ATOM_COLOR		L"COLOR"
#define ATOM_SLIDER		L"SLIDER"
#define ATOM_VAL		L"VALUE"
#define ATOM_TTL		L"TOOLTIP"

/* custom messages */
#define EN_UPDATED			4401

/* identifiers */
#define FRM_NULL		0
#define FRM_TEXT		1
#define FRM_VECT		2

#define WSPCSIZE		10
#define WSPC_			0
#define WSPC_W			1
#define WSPC_H			2
#define WSPC_RES		3
#define WSPC_PXW		4
#define WSPC_PXH		5
#define WSPC_SUNIT		6
#define WSPC_RUNIT		7
#define WSPC_DMODE		8
#define WSPC_DUNIT		9

#define FRMDSIZE		9
#define FRMD_TYPE		0
#define FRMD_X			1
#define FRMD_Y			2
#define FRMD_OPCT		3
#define FRMD_LOCK		4
#define FRMD_VIS		5
#define FRMD_EMPT		6
#define FRMD_NAMELEN	7
#define FRMD_NAMEPTR	8

#define FRMTSIZE		10
#define FRMT_TXTLEN		0
#define FRMT_

#define INVDEF				0
#define INVSCENE			1
#define INVSCENEXOR			2
#define INVSCENEXORNOSCALE	3

#define FILTER			2222

#define COPCOP			1111
#define COPCPM			1112
#define COPCUT			1113
#define COPPAS			1114
#define COPCLR			1115

#define PENPEN			1
#define PENRUB			2

#define SELRECT			1
#define SELCIRC			2
#define SELHOR			3
#define SELVER			4

#define CROP			0
#define CROPTL			1
#define CROPTM			2
#define CROPTR			3
#define CROPML			4
#define CROPMR			5
#define CROPBL			6
#define CROPBM			7
#define CROPBR			8

#define GRID_BKGND		1
#define GRID_SNAPD		2

#define INFLATE			1
#define STRICTY			2

#define HEADSORT		-1
#define TAILSORT		1

#define LEFT			1
#define MID				2
#define RIGHT			3

#define TOP				-1
#define BOT				1
#define UPDATE			777

#define CLIENT			1
#define NONCLIENT		2

#define NO				0
#define YES				1
#define AUTO			2
#define NOTHUMBS		3

#define FITSCREEN		10
#define FITSCREENOV		15
#define FITNUMPAD		20
#define FITWIDTH		30
#define FITHEIGHT		40
#define FITLEFT			50
#define FITRIGHT		60

#define ROT_0			0
#define ROT_90			1
#define ROT_180			2
#define ROT_270			3

#define VK_O			0x4F
#define VK_P			0x50

#define TIMER_MBB		333
#define TIMER_MCH		325
#define TIMER_MMM		111
#define TIMER_OVL		847
#define TIMER_THB		783
#define TIMER_TCC		888
#define TIMER_TIP		999

#define TIMER_INT		777

#define TICKER_OFF		-1
#define TICKER_STEPS	4
#define TICKER_SIZE		5
#define TICKER_INDENT	6

/* file recognition */

/* mime */
#define MIME_NONE		0
#define MIME_IMAGE		1
#define MIME_ARCHIVE	2
#define MIME_DIR		3

/* type */
#define TYPE_NONE			0
#define TYPE_TIFF			111
#define TYPE_TIFFINTEL		1
#define TYPE_TIFFMOTOROLA	2
#define TYPE_GIF			333
#define TYPE_GIF87a			3
#define TYPE_GIF89a			4
#define TYPE_PNG			5
#define TYPE_JPG			666
#define TYPE_JPEGJFIF		6
#define TYPE_JPEGEXIF		7
#define TYPE_JPEGAPP2		8
#define TYPE_JPEGAPP3		9
#define TYPE_JPEGAPP4		10
#define TYPE_JPEGAPP5		11
#define TYPE_JPEGAPP6		12
#define TYPE_JPEGAPP7		13
#define TYPE_JPEGAPP8		14
#define TYPE_JPEGAPP9		15
#define TYPE_JPEGAPPA		16
#define TYPE_JPEGAPPB		17
#define TYPE_JPEGAPPC		18
#define TYPE_JPEGAPPD		19
#define TYPE_JPEGAPPE		20
#define TYPE_JPEGAPPF		21
#define TYPE_BITMAP			22

#define TYPE_ZIP			23
#define TYPE_RAR			24

#define TYPE_DIR			25

#define TYPE_FED			26

#define TYPE_ICO			27