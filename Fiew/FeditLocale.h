/*
FeditLocale.h
Contains bilingual of all text info used in the application
*/

/* locales */
#define LANG_EN 44
#ifndef LANG_EN
#define LANG_PL 48
#endif

// messages
#define GUI_ARCHSPLIT L"~"
#define GUI_OF L"/"
#define GUI_SEP L"\n\n"

#ifdef LANG_EN

#define FRAME_NAME L"Layer"

#define BUT_NEWLAY L"New Layer"
#define BUT_DELLAY L"Delete Layer"
#define BUT_DELHIS L"Delete History"
#define BUT_UNDO L"Undo"
#define BUT_REDO L"Redo"
#define BUT_LAYUP L"Move Up"
#define BUT_LAYDOWN L"Move Down"
#define BUT_COLSWAP L"Swap Colors"
#define BUT_COLRES L"Reset Colors"
#define BUT_COLCHOOSE L"Choose Text Color"

#define FIL_GENERIC L"Matrix Filter"
#define FIL_BLUR L"Blur Filter"
#define FIL_BLURGAUSS L"Gaussian Blur Filter"
#define FIL_SHARPEN L"Sharepen Filter"
#define FIL_EDGETRACE L"Edge Trace Filter"
#define FIL_EMBOSS_E L"East Emboss Filter"
#define FIL_EMBOSS_S L"South Emboss Filter"
#define FIL_EMBOSS_SE L"South-East Emboss Filter"
#define FIL_HIGHLIGHT L"Highlight Filter"
#define FIL_DEFOCUS L"Defocus Filter"
#define FIL_OLDSTONE L"Old Stone Filter"
#define FIL_CUSTOM L"Custom Matrix Filter"

#define TOOLWBOX_INFO_COLOR L"Color Data"
#define TOOLWBOX_INFO_POS L"Cursor"
#define TOOLWBOX_INFO_RECT L"Rectangle"

#define TOOLWBOX_HISTORY_ORDER L"Ordered Layer"
#define TOOLWBOX_HISTORY_RESIZE L"Resize Image"
#define TOOLWBOX_HISTORY_FRAME_TEXT L"New Text"
#define TOOLWBOX_HISTORY_FRAME_REMOVE L"Delete Layer"
#define TOOLWBOX_HISTORY_FRAME_ADD L"New Layer"
#define TOOLWBOX_HISTORY_DESELEE L"Clear Selection"
#define TOOLWBOX_HISTORY_ON L" on layer: "
#define TOOLWBOX_HISTORY_FIRST L"Begin Editing"

#define TOOLWBOX_LAYERS_LOCK L"Lock Layer"
#define TOOLWBOX_LAYERS_OPACITY L"Opacity: "
#define TOOLWBOX_LAYERS_NEW L"new"
#define TOOLWBOX_LAYERS_DEL L"delete"

#define TOOLWBOX_HISTORY L"History"
#define TOOLWBOX_INFO L"Info"
#define TOOLWBOX_LAYERS L"Layers"

#define TOOL_ZOOM_ALL L"Zoom All Windows"
#define TOOL_ZOOM_RESET L"Reset Zoom"
#define TOOL_ZOOM_FITTO L"Fit to Window"

#define TOOL_HAND_ALL L"Scroll All Windows"
#define TOOL_HAND_RESET L"Reset Position"

#define TOOL_COPY_MESSAGE L"No area selected."

#define TOOL_RASTER L"Rasterize Layer"
#define TOOL_MERGE L"Merge Layer"
#define TOOL_FILTER L"Filter"

#define TOOL_COPY L"Copy"
#define TOOL_COPM L"Copy Merged"
#define TOOL_CUT L"Cut"
#define TOOL_PASTE L"Paste"
#define TOOL_CLEAR L"Clear"

#define TOOL_CROP L"Crop Tool"
#define TOOL_ERASER L"Eraser Tool"
#define TOOL_TEXT L"Text Tool"
#define TOOL_FILBUC L"Bucket Fill Tool"
#define TOOL_MAG L"Magic Wand Tool"
#define TOOL_SMPCOL L"Color Picker Tool"
#define TOOL_SELMOS L"Free Selection Tool"
#define TOOL_SELPOLY L"Polygonal Selection Tool"
#define TOOL_SELVER L"Vertical Selection Tool"
#define TOOL_SELHOR L"Horizontal Selection Tool"
#define TOOL_SELCIRC L"Ellipse Selection Tool"
#define TOOL_SELRECT L"Rectangle Selection Tool"
#define TOOL_MOVE L"Move Tool"
#define TOOL_RREC L"Draw Rounded Rectangle Tool"
#define TOOL_ELLI L"Draw Ellipse Tool"
#define TOOL_RECT L"Draw Rectangle Tool"
#define TOOL_LINE L"Draw Line Tool"
#define TOOL_PENCIL L"Pencil Tool"
#define TOOL_ZOOM L"Zoom Tool"
#define TOOL_HAND L"Hand Tool"

#define UNTITLED L"Untitled"
#define SUNIT_PERCENT L"percents"
#define SUNIT_PIXEL L"pixels"
#define SUNIT_MM L"mm"
#define SUNIT_INCH L"inch"
#define RUNIT_PIXOINCH L"pixel/inch"
#define RUNIT_PIXOCM L"pixel/cm"

#define IM_BICUBIC_HQ L"HQ Bicubic"
#define IM_BILINEAR L"Bilinear"
#define IM_BICUBIC L"Bicubic"
#define IM_NN L"Nearest Neighbor"

#define BKGND_TRANS L"Transparent"
#define BKGND_FORCOL L"Foreground Color"
#define BKGND_WHITE L"White"

#define MENU_FILE L"&File"
#define MENU_FILE_OPEN L"&Open...\tCtrl+O"
#define MENU_FILE_OPENFOLDER L"Open &Folder...\tCtrl+Shift+O"
#define MENU_FILE_EXTRACT L"&Extract\tCtrl+E"
#define MENU_FILE_SETWALL L"Set as &Wallpaper\tCtrl+W"
#define MENU_FILE_CLOSE L"&Close\tCtrl+C"
#define MENU_FILE_EXIT L"E&xit\tCtrl+Q"

#define MENU_VIEW L"&View"
#define MENU_VIEW_FULLSCREEN L"Full&screen\t*"
#define MENU_VIEW_THUMBNAILS L"Thum&bnails\tCtrl+M"
#define MENU_VIEW_LIST L"&List\tCtrl+L"
#define MENU_VIEW_FITTO L"Fi&t to"
#define MENU_VIEW_ZOOM L"&Zoom"
#define MENU_VIEW_ROTATE L"&Rotate"

#define MENU_VIEW_FITTO_SCREENOV L"Sc&reen if Oversized\t."
#define MENU_VIEW_FITTO_SCREEN L"Sc&reen"
#define MENU_VIEW_FITTO_HEIGHT L"&Height\tCtrl+F"
#define MENU_VIEW_FITTO_WIDTH L"&Width\tCtrl+Shift+F"
#define MENU_VIEW_FITTO_NUMPAD L"&Numpad\tAlt+F"
#define MENU_VIEW_FITTO_LEFT L"&Left\t["
#define MENU_VIEW_FITTO_RIGHT L"&Right\t]"

#define MENU_VIEW_ZOOM_ZOOMIN L"Zoom &in\t+"
#define MENU_VIEW_ZOOM_ZOOMOUT L"Zoom &out\t-"

#define MENU_VIEW_ROTATE_RESET L"&Reset"

#define MENU_VIEW_FLOWSCROLL L"&Flow Scroll\tCtrl+S"

#define MENU_HELP L"&Help"
#define MENU_HELP_ABOUT L"&About...\tCtrl+?"
#define MENU_HELP_MANUAL L"&Manual...\tCtrl+I"

#define DOCKDLG_PXTOLER L"px tolerance"
#define DOCKDLG_TOLERANCE L"Tolerance: "
#define DOCKDLG_FONTSIZE L"Size: "
#define DOCKDLG_FONT L"Font Family: "
#define DOCKDLG_SIZE L"px width"
#define DOCKDLG_AA L"use antialiasing"

#define MESSAGE_FOLDERBROWSER L"Choose a folder to begin browsing its image contents."

#define MESSAGE_SAVE L"Do you want to save your work?"

#define MESSAGE_NODOCK L"Tool settings are currently hidden."

#define MESSAGE_LIMIT1 L"The input value has to be between "
#define MESSAGE_LIMIT2 L" and "
#define MESSAGE_LIMIT3 L"."

#define MESSAGE_ERROR L"Error"
#define MESSAGE_WARNING L"Warning"
#define MESSAGE_INFO L"Notice"
#define MESSAGE_CLOSE L"Close"

#define MESSAGE_PROCESS L"Processing... Please wait."
#define MESSAGE_EXTRACT L"Extracting... Please wait."

#define MESSAGE_CROP L"Do you want to apply the cropping?"

#define MESSAGE_FAKEIMG L"This file is not a proper image and cannot be read."
#define MESSAGE_NORAR L"Unable to load unrar.dll! Reading RAR Archives disabled."
#define MESSAGE_NORARFUNC L"Unable to load unrar.dll functions! Reading RAR Archives disabled."
#define MESSAGE_CANNOTSAVE L"The file cannot be saved."
#define MESSAGE_CANNOTOPEN L"The file cannot be opened."
#define MESSAGE_CANNOTFULLSCR L"Could not change to fullscreen display."
#define MESSAGE_CLOSETEXT L"Do you want to close the image?"

#define MESSAGE_EXTRACTEDIMAGE L"Image successfully extracted."
#define MESSAGE_EXTRACTERROR L"The extraction was unsuccessful."

#define MESSAGE_SETWALLEDIMAGE L"Image successfully set as wallpaper."
#define MESSAGE_SETWALLERROR L"Placing the wallpaper was unsuccessful."

#define MESSAGE_RAR_NO_MEMORY L"Not enough memory to initialize data structures."
#define MESSAGE_RAR_BAD_DATA L"Archive header broken."
#define MESSAGE_RAR_BAD_ARCHIVE L"File is not valid RAR archive."
#define MESSAGE_RAR_UNKNOWN_FORMAT L"Unknown encryption used for archive headers."
#define MESSAGE_RAR_EOPEN L"File open error."
#define MESSAGE_RAR_BAD_PASS L"Incorrect archive password."

#define MESSAGE_EMPTY L"No image files in this collection."

#define MESSAGE_ZIP_CONST L"Unknown zip result code."
#define MESSAGE_ZIP_ZR_OK L"Success."
#define MESSAGE_ZIP_ZR_FAIL L"File is not valid ZIP archive."
#define MESSAGE_ZIP_ZR_NODUPH L"Couldn't duplicate handle."
#define MESSAGE_ZIP_ZR_NOFILE L"Couldn't create/open file."
#define MESSAGE_ZIP_ZR_NOALLOC L"Failed to allocate memory."
#define MESSAGE_ZIP_ZR_WRITE L"Error writing to file."
#define MESSAGE_ZIP_ZR_NOTFOUND L"File not found in the zipfile."
#define MESSAGE_ZIP_ZR_MORE L"Still more data to unzip."
#define MESSAGE_ZIP_ZR_CORRUPT L"Zipfile is corrupt or not a zipfile."
#define MESSAGE_ZIP_ZR_READ L"Error reading file."
#define MESSAGE_ZIP_ZR_ARGS L"Caller: faulty arguments."
#define MESSAGE_ZIP_ZR_PARTIALUNZ L"Caller: the file had already been partially unzipped."
#define MESSAGE_ZIP_ZR_NOTMMAP L"Caller: can only get memory of a memory zipfile."
#define MESSAGE_ZIP_ZR_MEMSIZE L"Caller: not enough space allocated for memory zipfile."
#define MESSAGE_ZIP_ZR_FAILED L"Caller: there was a previous error."
#define MESSAGE_ZIP_ZR_ENDED L"Caller: additions to the zip have already been ended."
#define MESSAGE_ZIP_ZR_ZMODE L"Caller: mixing creation and opening of zip."
#define MESSAGE_ZIP_ZR_NOTINITED L"Zip-bug: internal initialisation not completed."
#define MESSAGE_ZIP_ZR_SEEK L"Zip-bug: trying to seek the unseekable."
#define MESSAGE_ZIP_ZR_MISSIZE L"Zip-bug: the anticipated size turned out wrong."
#define MESSAGE_ZIP_ZR_NOCHANGE L"Zip-bug: tried to change mind, but not allowed."
#define MESSAGE_ZIP_ZR_FLATE L"Zip-bug: an internal error during flation."

#define DIALOG_PASSWORD L"Archive password"

#define OPENFILEFILTER L"All Readable\0*.fed;*.jpg;*.jpeg;*.png;*.gif;*.tif;*.tiff;*.bmp;*.ico;.rar;*.cbr;*.zip;*.cbz\0FED\0*.fed\0Jpeg\0*.jpg;*.jpeg\0Png\0*.png\0Gif\0*.gif\0Tif\0*.tif;*.tiff\0Bmp\0*.bmp\0Icon\0*.ico\0Rar\0*.rar\0Cbr\0*.cbr\0Zip\0*.zip\0Cbz\0*.cbz\0\0";
#define SAVEFILEFILTER L"FED\0*.fed\0Jpeg\0*.jpg;*.jpeg\0Png\0*.png\0Gif\0*.gif\0Tif\0*.tif;*.tiff\0Bmp\0*.bmp\0Icon\0*.ico\0\0";

#endif
#ifdef LANG_PL
#define MENU_FILE L"&Plik"
#define MENU_FILE_OPEN L"&Otwórz...\tCtrl+O"
#define MENU_FILE_OPENFOLDER L"Otwórz &Folder...\tCtrl+Shift+O"
#define MENU_FILE_EXTRACT L"&Rozpakuj\tCtrl+E"
#define MENU_FILE_SETWALL L"Ustaw jako &Tapetê\tCtrl+W"
#define MENU_FILE_CLOSE L"&Zamknij\tCtrl+C"
#define MENU_FILE_EXIT L"&WyjdŸ\tCtrl+Q"

#define MENU_VIEW L"&Widok"
#define MENU_VIEW_FULLSCREEN L"Pe³ny &Ekran\t*"
#define MENU_VIEW_THUMBNAILS L"&Miniaturki\tCtrl+M"
#define MENU_VIEW_LIST L"&Lista\tCtrl+L"
#define MENU_VIEW_FITTO L"&Dopasuj do"
#define MENU_VIEW_ZOOM L"&Skaluj"
#define MENU_VIEW_ROTATE L"&Obróæ"

#define MENU_VIEW_FITTO_SCREENOV L"&Ekranu gdy Wiêkszy\t."
#define MENU_VIEW_FITTO_SCREEN L"&Ekranu"
#define MENU_VIEW_FITTO_HEIGHT L"&Wysokoœci\tCtrl+F"
#define MENU_VIEW_FITTO_WIDTH L"&Szerokoœci\tCtrl+Shift+F"
#define MENU_VIEW_FITTO_NUMPAD L"&Numpada\tAlt+F"
#define MENU_VIEW_FITTO_LEFT L"&Lewej\t["
#define MENU_VIEW_FITTO_RIGHT L"&Prawej\t]"

#define MENU_VIEW_ZOOM_ZOOMIN L"Po&wiêksz\t+"
#define MENU_VIEW_ZOOM_ZOOMOUT L"Po&mniejsz\t-"

#define MENU_VIEW_ROTATE_RESET L"&Resetuj"

#define MENU_VIEW_FLOWSCROLL L"&P³ynne Przewijanie\tCtrl+S"

#define MENU_HELP L"&Pomoc"
#define MENU_HELP_ABOUT L"&O Programie...\tCtrl+?"
#define MENU_HELP_MANUAL L"&Interfejs...\tCtrl+I"

#define MESSAGE_ERROR L"B³¹d"
#define MESSAGE_WARNING L"Ostrze¿enie"
#define MESSAGE_INFO L"Informacja"
#define MESSAGE_CLOSE L"Zamknij"

#define MESSAGE_PROCESS L"Przetwarzanie... Proszê czekaæ."
#define MESSAGE_EXTRACT L"Rozpakowywanie... Proszê czekaæ."

#define MESSAGE_FAKEIMG L"Plik nie jest w³aœciwym obrazem i nie mo¿e byæ odczytany."
#define MESSAGE_NORAR L"Za³adowanie biblioteki unrar.dll nie powiod³o siê! Odczyt archiwów RAR wy³¹czony."
#define MESSAGE_NORARFUNC L"Za³adowanie funkcji biblioteki unrar.dll nie powiod³o siê! Odczyt archiwów RAR wy³¹czony."
#define MESSAGE_CANNOTOPEN L"Plik nie mo¿e zostaæ otwarty."
#define MESSAGE_CANNOTFULLSCR L"Zmiana na tryb pe³noekranowy nie powiod³a siê."
#define MESSAGE_CLOSETEXT L"Czy chcesz zamkn¹c obraz?"

#define MESSAGE_EXTRACTEDIMAGE L"Obraz zosta³ rozpakowany."
#define MESSAGE_EXTRACTERROR L"Rozpakowanie nie powiod³o siê."

#define MESSAGE_SETWALLEDIMAGE L"Obraz umieszczony na pulpicie."
#define MESSAGE_SETWALLERROR L"Umieszczenie na pulpicie nie powiod³o siê."

#define MESSAGE_RAR_NO_MEMORY L"Za ma³o pamiêci by zainicjalizowaæ struktury."
#define MESSAGE_RAR_BAD_DATA L"Uszkodzony nag³ówek archiwum."
#define MESSAGE_RAR_BAD_ARCHIVE L"Plik nie jest prawid³owym archiwem RAR."
#define MESSAGE_RAR_UNKNOWN_FORMAT L"Nieznane kodowanie nag³ówka archiwum."
#define MESSAGE_RAR_EOPEN L"B³¹d odczytu pliku."
#define MESSAGE_RAR_BAD_PASS L"B³êdne has³o do archiwum."

#define MESSAGE_EMPTY L"Brak obrazów w tym zbiorze."

#define MESSAGE_ZIP_CONST L"Nieznany kod rezultatu."
#define MESSAGE_ZIP_ZR_OK L"Sukces."
#define MESSAGE_ZIP_ZR_FAIL L"Plik nie jest prawid³owym archiwem ZIP."
#define MESSAGE_ZIP_ZR_NODUPH L"B³ad duplikacji handlera."
#define MESSAGE_ZIP_ZR_NOFILE L"B³ad otwarcia/utworzenia pliku."
#define MESSAGE_ZIP_ZR_NOALLOC L"B³ad alokacji pamiêci."
#define MESSAGE_ZIP_ZR_WRITE L"B³ad zapisu pliku."
#define MESSAGE_ZIP_ZR_NOTFOUND L"Plik nie zosta³ znaleziony w archiwum."
#define MESSAGE_ZIP_ZR_MORE L"Wiêcej danych do rozpakowania."
#define MESSAGE_ZIP_ZR_CORRUPT L"Archiwum jest uszkodzone lub nie jest archiwem ZIP."
#define MESSAGE_ZIP_ZR_READ L"B³¹d odczytu pliku."
#define MESSAGE_ZIP_ZR_ARGS L"Caller: b³êdne argumenty."
#define MESSAGE_ZIP_ZR_PARTIALUNZ L"Caller: plik ju¿ zosta³ czêœciowo rozpakowany."
#define MESSAGE_ZIP_ZR_NOTMMAP L"Caller: mo¿na pobraæ pamiêæ tylko archiwum w pamiêci."
#define MESSAGE_ZIP_ZR_MEMSIZE L"Caller: za ma³o pamiêci zaalokowanej dla archiwum w pamiêci."
#define MESSAGE_ZIP_ZR_FAILED L"Caller: wczeœniej wyst¹pi³ b³¹d."
#define MESSAGE_ZIP_ZR_ENDED L"Caller: dodawanie do archiwum ju¿ zosta³o wstrzymane."
#define MESSAGE_ZIP_ZR_ZMODE L"Caller: mieszanie tworzenia i dodawania do archiwum."
#define MESSAGE_ZIP_ZR_NOTINITED L"Zip-bug: wewnêtrzna inicjalizacja przerwana."
#define MESSAGE_ZIP_ZR_SEEK L"Zip-bug: próba poszukiwania nieistniej¹cego."
#define MESSAGE_ZIP_ZR_MISSIZE L"Zip-bug: zak³adana wielkoœæ okaza³a siê b³êdna."
#define MESSAGE_ZIP_ZR_NOCHANGE L"Zip-bug: chcia³em zmieniæ w³asne zdanie, ale mi nie pozwolili."
#define MESSAGE_ZIP_ZR_FLATE L"Zip-bug: wewnêtrzny b³¹d podczas operacji 'flation'."

#define DIALOG_PASSWORD L"Podaj has³o do archiwum"

#define OPENFILEFILTER L"Wszystkie\0*.JPG;*.JPEG;*.PNG;*.GIF;*.TIF;*.TIFF;*.BMP;*.RAR;*.CBR;*.ZIP;*.CBZ\0Jpeg\0*.JPG;*.JPEG\0Png\0*.PNG\0Gif\0*.GIF\0Tif\0*.TIF;*.TIFF\0Bmp\0*.BMP\0Rar\0*.RAR\0Cbr\0*.CBR\0Zip\0*.ZIP\0Cbz\0*.CBZ\0\0";
#endif