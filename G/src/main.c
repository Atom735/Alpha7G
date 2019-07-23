#include <Windows.h>
#include <commdlg.h>

#include <stdio.h>

HINSTANCE g_hInstance;
CONST LPCWSTR g_ksMainClassName = L"WCNA7_MAIN";

struct SFileBufferLine {
    LPWSTR sBuf;
    UINT nSize;
    UINT nSizeMax;
};

struct SFileBuffer {
    struct SFileBufferLine * pLines;
    UINT nSize;
    UINT nLines;
    UINT nLinesMax;
};

struct SCarret {
    struct SFileBuffer * pFB;
    UINT nLine;
    UINT nOldCol;
    UINT nCol;
};

struct SSettigs {
    HFONT hFont;
    HBRUSH hBrushCarret;
    HBRUSH hBrushBackground;

    LPCWSTR sFontName;

    UINT nOverScrollTop;
    UINT nOverScrollBottom;
    UINT nOverScrollLeft;
    UINT nOverScrollRight;

    UINT nOverLineTop;
    UINT nOverLineBottom;
    UINT nOverLineBetween;

    UINT nCarretTop;
    UINT nCarretBottom;
    UINT nCarretWidth;

    UINT nFontWidth;
    UINT nFontHeight;

    COLORREF iColorFont;
    COLORREF iColorCarret;
    COLORREF iColorBackground;
};

struct SSettigs g_Settings = {
    .sFontName           = L"Fira Code",
    .nOverScrollTop      = 32,
    .nOverScrollBottom   = 32,
    .nOverScrollLeft     = 32,
    .nOverScrollRight    = 32,
    .nOverLineTop        = 3,
    .nOverLineBottom     = 6,
    .nOverLineBetween    = 2,
    .nCarretTop          = 2,
    .nCarretBottom       = 2,
    .nCarretWidth        = 3,
    .nFontWidth          = 0,
    .nFontHeight         = 16,
    .iColorFont          = RGB ( 0xEE, 0xEE, 0xEE ),
    .iColorCarret        = RGB ( 0xFF, 0xAA, 0x55 ),
    .iColorBackground    = RGB ( 0x11, 0x13, 0x15 ),
};

FILE *_pF = NULL;

INT CALLBACK rSettigsSetUp_EnumFontFamExProc ( CONST LOGFONTW * lpelfe, CONST TEXTMETRIC * lpntme, DWORD FontType, LPARAM lParam ) {
    LPCSTR _CharSet ( BYTE i ) {
        switch ( i ) {
            case ANSI_CHARSET: return "ANSI_CHARSET";
            case DEFAULT_CHARSET: return "DEFAULT_CHARSET";
            case SYMBOL_CHARSET: return "SYMBOL_CHARSET";
            case SHIFTJIS_CHARSET: return "SHIFTJIS_CHARSET";
            case HANGEUL_CHARSET: return "HANGEUL_CHARSET";
            case GB2312_CHARSET: return "GB2312_CHARSET";
            case CHINESEBIG5_CHARSET: return "CHINESEBIG5_CHARSET";
            case OEM_CHARSET: return "OEM_CHARSET";
            case JOHAB_CHARSET: return "JOHAB_CHARSET";
            case HEBREW_CHARSET: return "HEBREW_CHARSET";
            case ARABIC_CHARSET: return "ARABIC_CHARSET";
            case GREEK_CHARSET: return "GREEK_CHARSET";
            case TURKISH_CHARSET: return "TURKISH_CHARSET";
            case VIETNAMESE_CHARSET: return "VIETNAMESE_CHARSET";
            case THAI_CHARSET: return "THAI_CHARSET";
            case EASTEUROPE_CHARSET: return "EASTEUROPE_CHARSET";
            case RUSSIAN_CHARSET: return "RUSSIAN_CHARSET";
            case MAC_CHARSET: return "MAC_CHARSET";
            case BALTIC_CHARSET: return "BALTIC_CHARSET";
        }
        return "NULL";
    }
    LPCSTR _OutPrecision ( BYTE i ) {
        switch ( i ) {
            case OUT_DEFAULT_PRECIS: return "OUT_DEFAULT_PRECIS";
            case OUT_STRING_PRECIS: return "OUT_STRING_PRECIS";
            case OUT_CHARACTER_PRECIS: return "OUT_CHARACTER_PRECIS";
            case OUT_STROKE_PRECIS: return "OUT_STROKE_PRECIS";
            case OUT_TT_PRECIS: return "OUT_TT_PRECIS";
            case OUT_DEVICE_PRECIS: return "OUT_DEVICE_PRECIS";
            case OUT_RASTER_PRECIS: return "OUT_RASTER_PRECIS";
            case OUT_TT_ONLY_PRECIS: return "OUT_TT_ONLY_PRECIS";
            case OUT_OUTLINE_PRECIS: return "OUT_OUTLINE_PRECIS";
            case OUT_SCREEN_OUTLINE_PRECIS: return "OUT_SCREEN_OUTLINE_PRECIS";
            case OUT_PS_ONLY_PRECIS: return "OUT_PS_ONLY_PRECIS";
        }
        return "NULL";
    }
    LPCSTR _ClipPrecision ( BYTE i ) {
        switch ( i ) {
            case CLIP_DEFAULT_PRECIS: return "CLIP_DEFAULT_PRECIS";
            case CLIP_CHARACTER_PRECIS: return "CLIP_CHARACTER_PRECIS";
            case CLIP_STROKE_PRECIS: return "CLIP_STROKE_PRECIS";
            case CLIP_MASK: return "CLIP_MASK";
            case CLIP_LH_ANGLES: return "CLIP_LH_ANGLES";
            case CLIP_TT_ALWAYS: return "CLIP_TT_ALWAYS";
            case CLIP_DFA_DISABLE: return "CLIP_DFA_DISABLE";
            case CLIP_EMBEDDED: return "CLIP_EMBEDDED";
        }
        return "NULL";
    }
    LPCSTR _Quality ( BYTE i ) {
        switch ( i ) {
            case DEFAULT_QUALITY: return "DEFAULT_QUALITY";
            case DRAFT_QUALITY: return "DRAFT_QUALITY";
            case PROOF_QUALITY: return "PROOF_QUALITY";
            case NONANTIALIASED_QUALITY: return "NONANTIALIASED_QUALITY";
            case ANTIALIASED_QUALITY: return "ANTIALIASED_QUALITY";
            case CLEARTYPE_QUALITY: return "CLEARTYPE_QUALITY";
            case CLEARTYPE_NATURAL_QUALITY: return "CLEARTYPE_NATURAL_QUALITY";
        }
        return "NULL";
    }
    LPCSTR _PitchAndFamily ( BYTE i ) {
        switch ( i ) {
            case FIXED_PITCH: return "FIXED_PITCH";
            case VARIABLE_PITCH: return "VARIABLE_PITCH";
            case MONO_FONT: return "MONO_FONT";
            case FF_ROMAN: return "FF_ROMAN";
            case FF_SWISS: return "FF_SWISS";
            case FF_MODERN: return "FF_MODERN";
            case FF_SCRIPT: return "FF_SCRIPT";
            case FF_DECORATIVE: return "FF_DECORATIVE";
        }
        return "";
    }
    static UINT i = 0; ++i;
    fprintf ( _pF, "% 4u %ls\n", i, lpelfe -> lfFaceName );

    fprintf ( _pF, "    Sizes       %ld x %ld : %ld\n", lpelfe -> lfHeight, lpelfe -> lfWidth, lpelfe -> lfWeight );
    fprintf ( _pF, "    Orientation %ld x %ld\n", lpelfe -> lfEscapement, lpelfe -> lfOrientation );
    fprintf ( _pF, "    IUS         %hd : %hd : %hd\n", lpelfe -> lfItalic, lpelfe -> lfUnderline, lpelfe -> lfStrikeOut );
    fprintf ( _pF, "    CharSet        %hu %s\n", lpelfe -> lfCharSet, _CharSet ( lpelfe -> lfCharSet ) );
    fprintf ( _pF, "    OutPrecision   %hu %s\n", lpelfe -> lfOutPrecision, _OutPrecision ( lpelfe -> lfOutPrecision ) );
    fprintf ( _pF, "    ClipPrecision  %hu %s\n", lpelfe -> lfClipPrecision, _ClipPrecision ( lpelfe -> lfClipPrecision ) );
    fprintf ( _pF, "    Quality        %hu %s\n", lpelfe -> lfQuality, _Quality ( lpelfe -> lfQuality ) );
    fprintf ( _pF, "    PitchAndFamily 0x%hx", lpelfe -> lfPitchAndFamily );
    for ( UINT i = 0; i < 8; ++i ) {
        fprintf ( _pF, " %s", _PitchAndFamily ( lpelfe -> lfPitchAndFamily & (1<<i) ) );
    }
    fprintf ( _pF, "\n\n" );

    return 1;
}

VOID rSettigsSetUp ( struct SSettigs * p, HDC hDC ) {
    LOGFONTW _lf = { 0 };
    _lf.lfPitchAndFamily = FIXED_PITCH | MONO_FONT;
    // memcpy ( _lf.lfFaceName, p -> sFontName, (wcslen ( p -> sFontName ) + 1) * 2 );
    _pF = fopen ( "fonts2.txt", "w" );
    EnumFontFamiliesExW ( hDC, &_lf, (FONTENUMPROCW)(rSettigsSetUp_EnumFontFamExProc), (LPARAM)(0), 0 );
    fclose ( _pF );
    _lf.lfHeight = p -> nFontHeight;
    _lf.lfWidth = 0;
    p -> hFont = CreateFontIndirectW (&_lf );
    p -> hBrushCarret = CreateSolidBrush ( p -> iColorCarret );
    p -> hBrushBackground = CreateSolidBrush ( p -> iColorBackground );

    CHOOSEFONTW cf;
    LOGFONTW lf;
    HFONT hfont;

    // Initialize members of the CHOOSEFONT structure.

    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = (HWND)NULL;
    cf.hDC = (HDC)NULL;
    cf.lpLogFont = &lf;
    cf.iPointSize = 0;
    cf.Flags = CF_SCREENFONTS;
    cf.rgbColors = RGB(0,0,0);
    cf.lCustData = 0L;
    cf.lpfnHook = (LPCFHOOKPROC)NULL;
    cf.lpTemplateName = (LPSTR)NULL;
    cf.hInstance = (HINSTANCE) NULL;
    cf.lpszStyle = (LPWSTR)NULL;
    cf.nFontType = SCREEN_FONTTYPE;
    cf.nSizeMin = 0;
    cf.nSizeMax = 0;

    // Display the CHOOSEFONT common-dialog box.

    ChooseFontW(&cf);

    // Create a logical font based on the user's
    // selection and return a handle identifying
    // that font.

    FILE *pF = fopen ( "font.txt", "w" );

    fprintf ( pF, "L_lfHeight         %ld\n", lf.lfHeight );
    fprintf ( pF, "L_lfWidth          %ld\n", lf.lfWidth );
    fprintf ( pF, "L_lfEscapement     %ld\n", lf.lfEscapement );
    fprintf ( pF, "L_lfOrientation    %ld\n", lf.lfOrientation );
    fprintf ( pF, "L_lfWeight         %ld\n", lf.lfWeight );
    fprintf ( pF, "B_lfItalic         %hd\n", lf.lfItalic );
    fprintf ( pF, "B_lfUnderline      %hd\n", lf.lfUnderline );
    fprintf ( pF, "B_lfStrikeOut      %hd\n", lf.lfStrikeOut );
    fprintf ( pF, "B_lfCharSet        %hd\n", lf.lfCharSet );
    fprintf ( pF, "B_lfOutPrecision   %hd\n", lf.lfOutPrecision );
    fprintf ( pF, "B_lfClipPrecision  %hd\n", lf.lfClipPrecision );
    fprintf ( pF, "B_lfQuality        %hd\n", lf.lfQuality );
    fprintf ( pF, "B_lfPitchAndFamily %hd\n", lf.lfPitchAndFamily );
    fprintf ( pF, "W_lfFaceName[32]   %ls\n", lf.lfFaceName );

    fclose ( pF );

    p -> hFont = CreateFontIndirectW(cf.lpLogFont);
}

struct SViewer {
    struct SFileBuffer * pFB;
    struct SCarret * pCarret;
    UINT nFontSize;
    UINT nHeight;
    UINT nPosY;
    UINT nPosX;
};

VOID rViewerRender ( struct SViewer * p, HDC hDC ) {
    if ( g_Settings.hFont == NULL ) {
        rSettigsSetUp ( &g_Settings, hDC );
        p -> nFontSize = g_Settings.nFontHeight;
    }
    SetBkMode ( hDC, TRANSPARENT );
    SelectObject ( hDC, g_Settings.hFont );
    SetTextColor ( hDC, g_Settings.iColorFont );

    if ( p -> nFontSize == 0 ) {
        SIZE sz;
        GetTextExtentPoint32W ( hDC, L"A", 1, &sz );
        p -> nFontSize = sz.cy;
    }
    for ( UINT i = 0; i < p -> pFB -> nLines; ++ i ) {
        if ( p -> pFB -> pLines [ i ] . nSize > 0 ) {
            TextOutW ( hDC, g_Settings.nOverScrollLeft - p -> nPosX, ( p -> nFontSize + g_Settings.nOverLineTop + g_Settings.nOverLineBottom ) * ( i + 1 ) + g_Settings.nOverLineTop + g_Settings.nOverScrollTop - p -> nPosY, p -> pFB -> pLines [ i ] . sBuf, p -> pFB -> pLines [ i ] . nSize );
        }
    }
    if ( p -> pCarret != NULL ) {
        SIZE sz = { };
        if ( p -> pCarret -> nCol > 0 ) {
            GetTextExtentPoint32W ( hDC, p -> pFB -> pLines [ p -> pCarret -> nLine ] . sBuf, p -> pCarret -> nCol, &sz );
        } else {
            sz.cx = 0;
        }
        RECT rc;
        rc.left = sz.cx + g_Settings.nOverScrollLeft - p -> nPosX;
        rc.right = rc.left + g_Settings.nCarretWidth;
        rc.top = ( p -> nFontSize + g_Settings.nOverLineTop + g_Settings.nOverLineBottom ) * ( p -> pCarret -> nLine + 1 ) + g_Settings.nOverLineTop + g_Settings.nOverScrollTop - p -> nPosY - g_Settings.nCarretTop;
        rc.bottom = rc.top + p -> nFontSize + g_Settings.nCarretTop + g_Settings.nCarretBottom;
        FillRect ( hDC, &rc, g_Settings.hBrushCarret );
    }
}



VOID rCarretMoveLeft ( struct SCarret * p ) {
    if ( p -> nCol > 0 ) {
        -- p -> nCol;
    } else if ( p -> nLine > 0 ) {
        -- p -> nLine;
        p -> nCol = p -> pFB -> pLines [ p -> nLine ] . nSize - 1;
    }
    p -> nOldCol = p -> nCol;
}

VOID rCarretMoveRight ( struct SCarret * p ) {
    if ( p -> nCol < p -> pFB -> pLines [ p -> nLine ] . nSize ) {
        ++ p -> nCol;
    } else if ( p -> nLine < p -> pFB -> nLines ) {
        ++ p -> nLine;
        p -> nCol = 0;
    }
    p -> nOldCol = p -> nCol;
}

VOID rCarretMoveUp ( struct SCarret * p ) {
    if ( p -> nLine > 0 ) {
        -- p -> nLine;
        p -> nCol = p -> pFB -> pLines [ p -> nLine ] . nSize - 1;
        if ( p -> nCol > p -> nOldCol ) p -> nCol = p -> nOldCol;
    }
}

VOID rCarretMoveDown ( struct SCarret * p ) {
    if ( p -> nLine < p -> pFB -> nLines ) {
        ++ p -> nLine;
        p -> nCol = p -> pFB -> pLines [ p -> nLine ] . nSize - 1;
        if ( p -> nCol > p -> nOldCol ) p -> nCol = p -> nOldCol;
    }
}

VOID rCarretInsertChar ( struct SCarret * p, WCHAR ch ) {
    struct SFileBufferLine * pLine = p -> pFB -> pLines + p -> nLine;
    if ( pLine -> nSize + 1 > pLine -> nSizeMax ) {
        pLine -> nSizeMax = ( ( ( pLine -> nSizeMax >> 4 ) + 1 ) << 4 );
        LPWSTR sBufOld = pLine -> sBuf;
        pLine -> sBuf = ( LPWSTR ) malloc ( sizeof ( WCHAR ) * pLine -> nSizeMax );
        if ( sBufOld ) {
            memcpy ( pLine -> sBuf, sBufOld, sizeof ( WCHAR ) * pLine -> nSize );
            free ( sBufOld );
        }
    }
    for ( UINT i = pLine -> nSize; i > p -> nCol; -- i ) {
        pLine -> sBuf [ i ] = pLine -> sBuf [ i - 1 ];
    }
    pLine -> sBuf [ p -> nCol ] = ch;
    ++ pLine -> nSize;
}

struct SFileBuffer * rFB_OpenFile ( LPCWSTR sFileName ) {
    FILE * pF = _wfopen ( sFileName, L"rb" );
    fseek ( pF, 0, SEEK_END );
    CONST UINT nSize = ftell ( pF );
    fseek ( pF, 0, SEEK_SET );

    BYTE * CONST pFileBuf = ( BYTE * ) malloc ( nSize + 1 );
    UINT nSizeReaded = 0;
    while ( nSizeReaded < nSize ) {
        nSizeReaded += fread ( pFileBuf + nSizeReaded, 1, nSize - nSizeReaded, pF );
    }
    pFileBuf [ nSize ] = 0x00;
    fclose ( pF );

    UINT nLines = 1;
    for ( UINT i = 0; i < nSize; ++ i ) {
        if ( pFileBuf [ i ] == '\n' ) ++ nLines;
    }

    struct SFileBuffer * pFB = ( struct SFileBuffer * ) malloc ( sizeof ( struct SFileBuffer ) );
    pFB -> nLines = nLines;
    pFB -> nLinesMax = ( ( ( nLines >> 4 ) + 1 ) << 4 );
    pFB -> pLines = ( struct SFileBufferLine * ) malloc ( sizeof ( struct SFileBufferLine ) * pFB -> nLinesMax );
    memset ( pFB -> pLines, 0, sizeof ( struct SFileBufferLine ) * pFB -> nLinesMax );

    struct SFileBufferLine * pLine = pFB -> pLines;

    for ( UINT i = 0; i < nSize; ++ i ) {
        if ( pFileBuf [ i ] == '\n' ) {
            ++ pLine;
        } else {
            if ( pLine -> nSize + 1 > pLine -> nSizeMax ) {
                pLine -> nSizeMax = ( ( ( pLine -> nSizeMax >> 4 ) + 1 ) << 4 );
                LPWSTR sBufOld = pLine -> sBuf;
                pLine -> sBuf = ( LPWSTR ) malloc ( sizeof ( WCHAR ) * pLine -> nSizeMax );
                if ( sBufOld ) {
                    memcpy ( pLine -> sBuf, sBufOld, sizeof ( WCHAR ) * pLine -> nSize );
                    free ( sBufOld );
                }
            }
            pLine -> sBuf [ pLine -> nSize ] = pFileBuf [ i ];
            ++ pLine -> nSize;
        }
    }

    return pFB;
}



/* Процедура главного окна */
LRESULT CALLBACK rMsgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    static WCHAR sBuf[1024] = L"Hello World!";
    static UINT nBufSz = 12;
    static UINT nCarret = 0;
    static struct SFileBuffer * pFB;
    static struct SCarret ACarret = { };
    static struct SViewer AViewer = { };
    switch ( uMsg ) {
        case WM_CREATE: {
            pFB = rFB_OpenFile ( L"src\\main.c" );
            ACarret . pFB = pFB;
            AViewer . pFB = pFB;
            AViewer . pCarret = &ACarret;



            return 0;
        }
        case WM_SIZE: {
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage ( 0 );
            return 0;
        }
        case WM_MOUSEMOVE: {
            return 0;
        }
        case WM_COMMAND: {
            return 0;
        }
        case WM_KEYDOWN: {
            switch ( wParam ) {
                case VK_LEFT:
                    rCarretMoveLeft ( &ACarret );
                    break;
                case VK_RIGHT:
                    rCarretMoveRight ( &ACarret );
                    break;
                case VK_UP:
                    rCarretMoveUp ( &ACarret );
                    break;
                case VK_DOWN:
                    rCarretMoveDown ( &ACarret );
                    break;
            }

            if ( wParam == VK_LEFT && nCarret ) {
                --nCarret;
            } else
            if ( wParam == VK_RIGHT && nCarret < nBufSz ) {
                ++nCarret;
            }
            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
        case WM_CHAR: {
            if ( wParam >= 0x20 ) {

                rCarretInsertChar ( &ACarret, wParam );
                rCarretMoveRight ( &ACarret );

                for ( UINT i = nBufSz; i > nCarret; --i ) {
                    sBuf [ i ] = sBuf [ i - 1 ];
                }
                sBuf [ nCarret ] = wParam;
                ++nBufSz;
                ++nCarret;
            } else
            if ( wParam == 0x08 && nCarret ) {
                for ( UINT i = nCarret-1; i < nBufSz; ++i ) {
                    sBuf [ i ] = sBuf [ i + 1 ];
                }
                --nBufSz;
                --nCarret;
            } else
            if ( wParam == 0x0D ) {
                nBufSz = 0;
                nCarret = 0;
            }
            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );
            FillRect ( hDC, &ps.rcPaint, GetSysColorBrush ( 1 ) );
            SetBkMode ( hDC, TRANSPARENT );
            SelectObject ( hDC, (HFONT) GetStockObject ( SYSTEM_FONT ) );
            SetTextColor ( hDC, 0x00FF7700 );
            TextOutW ( hDC, 0, 0, sBuf, nBufSz );

            SIZE sz;
            GetTextExtentPoint32W ( hDC, sBuf, nCarret, &sz );

            SetTextColor ( hDC, 0x000077FF );
            TextOutW ( hDC, sz.cx, 0, L"|", 1 );

            SetTextColor ( hDC, 0x00AAAAAA );
            rViewerRender ( &AViewer, hDC );

            EndPaint ( hWnd, &ps );
            return 0;
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}

/* Точка входа в приложение */
INT APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd ) {
    MSG msg = { };
    /* Регистрация класса главного окна */
    WNDCLASSEXW wc = {
        .cbSize        = sizeof ( WNDCLASSEXW ),
        .style         = CS_VREDRAW | CS_HREDRAW,
        .lpfnWndProc   = rMsgProc,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = hInstance,
        .hIcon         = NULL,
        .hCursor       = NULL,
        .hbrBackground = NULL,
        .lpszMenuName  = NULL,
        .lpszClassName = g_ksMainClassName,
        .hIconSm       = NULL,
    };
    RegisterClassExW ( &wc );
    /* Создание главного окна */
    CreateWindowExW ( 0, g_ksMainClassName, NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );
    /* Входим в цикл обработки сообщений */
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }
    /* Освобождение класса главного окна */
    UnregisterClassW ( g_ksMainClassName, hInstance );
    return (INT) msg.wParam;
}
