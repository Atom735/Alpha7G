#include <Windows.h>

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


struct SViewer {
    struct SFileBuffer * pFB;
    struct SCarret * pCarret;
    UINT nFontSize;
    UINT nHeight;
    UINT nPosY;
};


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

VOID rViewerRender ( struct SViewer * p, HDC hDC ) {
    if ( p -> nFontSize == 0 ) {
        SIZE sz;
        GetTextExtentPoint32W ( hDC, L"A", 1, &sz );
        p -> nFontSize = sz.cy;
    }
    for ( UINT i = 0; i < p -> pFB -> nLines; ++ i ) {
        if ( p -> pFB -> pLines [ i ] . nSize > 0 ) {
            TextOutW ( hDC, 0, p -> nFontSize * ( i + 1 ) - p -> nPosY, p -> pFB -> pLines [ i ] . sBuf, p -> pFB -> pLines [ i ] . nSize );
        }
    }
    if ( p -> pCarret != NULL ) {
        SIZE sz = { };
        if ( p -> pCarret -> nCol > 0 ) {
            GetTextExtentPoint32W ( hDC, p -> pFB -> pLines [ p -> pCarret -> nLine ] . sBuf, p -> pCarret -> nCol, &sz );
        } else {
            sz.cx = 0;
        }
        RECT rc = {
            .left = sz.cx,
            .top = p -> nFontSize * ( p -> pCarret -> nLine + 1 ) - p -> nPosY,
            .right = sz.cx + 1,
            .bottom = p -> nFontSize * ( p -> pCarret -> nLine + 2 ) - p -> nPosY - 1,
        };
        FillRect ( hDC, &rc, GetSysColorBrush ( 3 ) );
    }
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
