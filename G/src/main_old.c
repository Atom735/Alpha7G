#include <Windows.h>
#include <commdlg.h>
/* Привет мир)))

->
=> */
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

struct SColorShemeGlobal {
    UINT32 foreground;
    UINT32 background;
    UINT32 caret;
    UINT32 block_caret;
    UINT32 invisibles;
    UINT32 line_highlight;
    UINT32 selection;
    UINT32 selection_border;
    UINT32 misspelling;
    UINT32 active_guide;
    UINT32 find_highlight_foreground;
    UINT32 find_highlight;
    UINT32 brackets_options;
    UINT32 brackets_foreground;
    UINT32 bracket_contents_options;
    UINT32 bracket_contents_foreground;
    UINT32 tags_options;
};

struct SSettigs {
    HFONT hFont;
    HBRUSH hBrushCarret;
    HBRUSH hBrushBackground;

    LPCWSTR sFontName;

    UINT nFontHeight;
    UINT nFontWeight;
    UINT nFontWidth; /* if Fix Width */

    UINT32 iColorFont;
    UINT32 iColorCarret;
    UINT32 iColorBackground;
};

struct SSettigs g_Settings = {
    .sFontName           = L"Fira Code",
    .nFontHeight         = 24,
    .nFontWeight         = 400,
    .nFontWidth          = 0,
    .iColorFont          = RGB ( 0xEE, 0xEE, 0xEE ),
    .iColorCarret        = RGB ( 0xFF, 0xAA, 0x55 ),
    .iColorBackground    = RGB ( 0x11, 0x13, 0x15 ),
};

VOID rSettigsSetUp ( struct SSettigs * p, HDC hDC ) {

    LOGFONTW lf = { 0 };
    memcpy ( lf.lfFaceName, p -> sFontName, ( wcslen ( p -> sFontName ) + 1 ) * 2 );
    lf.lfHeight    = p -> nFontHeight;
    lf.lfWidth     = 0;
    lf.lfWeight    = p -> nFontWeight;
    lf.lfCharSet   = ANSI_CHARSET;
    p -> hFont = CreateFontIndirectW ( &lf );
    p -> hBrushCarret = CreateSolidBrush ( p -> iColorCarret );
    p -> hBrushBackground = CreateSolidBrush ( p -> iColorBackground );


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
    fprintf ( pF, "W_lfFaceName[32]   %ls\n\n\n", lf.lfFaceName );

    SelectObject ( hDC, p -> hFont );
    TEXTMETRICW tm;
    GetTextMetricsW ( hDC, &tm );

    fprintf ( pF, "tmHeight            %ld\n", (LONG)tm.tmHeight );
    fprintf ( pF, "tmAscent            %ld\n", (LONG)tm.tmAscent );
    fprintf ( pF, "tmDescent           %ld\n", (LONG)tm.tmDescent );
    fprintf ( pF, "tmInternalLeading   %ld\n", (LONG)tm.tmInternalLeading );
    fprintf ( pF, "tmExternalLeading   %ld\n", (LONG)tm.tmExternalLeading );
    fprintf ( pF, "tmAveCharWidth      %ld\n", (LONG)tm.tmAveCharWidth );
    fprintf ( pF, "tmMaxCharWidth      %ld\n", (LONG)tm.tmMaxCharWidth );
    fprintf ( pF, "tmWeight            %ld\n", (LONG)tm.tmWeight );
    fprintf ( pF, "tmOverhang          %ld\n", (LONG)tm.tmOverhang );
    fprintf ( pF, "tmDigitizedAspectX  %ld\n", (LONG)tm.tmDigitizedAspectX );
    fprintf ( pF, "tmDigitizedAspectY  %ld\n", (LONG)tm.tmDigitizedAspectY );
    fprintf ( pF, "tmFirstChar         %ld\n", (LONG)tm.tmFirstChar );
    fprintf ( pF, "tmLastChar          %ld\n", (LONG)tm.tmLastChar );
    fprintf ( pF, "tmDefaultChar       %ld\n", (LONG)tm.tmDefaultChar );
    fprintf ( pF, "tmBreakChar         %ld\n", (LONG)tm.tmBreakChar );
    fprintf ( pF, "tmItalic            %ld\n", (LONG)tm.tmItalic );
    fprintf ( pF, "tmUnderlined        %ld\n", (LONG)tm.tmUnderlined );
    fprintf ( pF, "tmStruckOut         %ld\n", (LONG)tm.tmStruckOut );
    fprintf ( pF, "tmPitchAndFamily    %ld\n", (LONG)tm.tmPitchAndFamily );
    fprintf ( pF, "tmCharSet           %ld\n", (LONG)tm.tmCharSet );
    fclose ( pF );

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

    for ( UINT i = 0; i < p -> pFB -> nLines; ++ i ) {
        if ( p -> pFB -> pLines [ i ] . nSize > 0 ) {
            {
                SIZE szOld = { };
                SIZE sz = { };
                RECT rc;
                for ( UINT j = 1; j < p -> pFB -> pLines [ i ] . nSize; ++j ) {
                    GetTextExtentPoint32W ( hDC, p -> pFB -> pLines [ i ] . sBuf, j, &sz );
                    rc.left = szOld.cx - p -> nPosX;
                    rc.right = rc.left + sz.cx - szOld.cx - 1;
                    rc.top = ( p -> nFontSize ) * ( i + 1 ) - p -> nPosY;
                    rc.bottom = rc.top + p -> nFontSize - 1;
                    szOld.cx = sz.cx;
                    FillRect ( hDC, &rc, g_Settings.hBrushCarret );
                }
            }

            TextOutW ( hDC, 0 - p -> nPosX, ( p -> nFontSize ) * ( i + 1 ) - p -> nPosY, p -> pFB -> pLines [ i ] . sBuf, p -> pFB -> pLines [ i ] . nSize );

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
        rc.left = sz.cx  - p -> nPosX;
        rc.right = rc.left + 1;
        rc.top = ( p -> nFontSize ) * ( p -> pCarret -> nLine + 1 ) - p -> nPosY;
        rc.bottom = rc.top + p -> nFontSize;
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
            if ( ( pFileBuf [ i ] & 0x80 ) ) {
                if ( ( pFileBuf [ i ] & 0xE0 ) == 0xC0 ) {
                    pLine -> sBuf [ pLine -> nSize ] =
                    ( ( pFileBuf [ i + 0 ] & 0x1F ) << 6 ) +
                    ( pFileBuf [ i + 1 ] & 0x3F );
                    i += 1;
                } else if ( ( pFileBuf [ i ] & 0xF0 ) == 0xE0 ) {
                    pLine -> sBuf [ pLine -> nSize ] =
                    ( ( pFileBuf [ i + 0 ] & 0x0F ) << 12 ) +
                    ( ( pFileBuf [ i + 1 ] & 0x3F ) << 6 ) +
                    ( pFileBuf [ i + 2 ] & 0x3F );
                    i += 2;
                } else if ( ( pFileBuf [ i ] & 0xF8 ) == 0xF0 ) {
                    pLine -> sBuf [ pLine -> nSize ] =
                    ( ( pFileBuf [ i + 0 ] & 0x07 ) << 18 ) +
                    ( ( pFileBuf [ i + 1 ] & 0x3F ) << 12 ) +
                    ( ( pFileBuf [ i + 2 ] & 0x3F ) << 6 ) +
                    ( pFileBuf [ i + 3 ] & 0x3F );
                    i += 3;
                }
            } else {
                pLine -> sBuf [ pLine -> nSize ] = pFileBuf [ i ];
            }
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
