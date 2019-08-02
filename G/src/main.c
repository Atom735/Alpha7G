#if 0
Показуха всех лигатур шрифта Fira Code

    .= ..= .- := ::= =:= __
     == != === !== =/= =!=

<-< <<- <-- <- <-> -> --> ->> >->
<=< <<= <==    <=> => ==> =>> >=>
    >>= >>- >- <~> -< -<< =<<
        <~~ <~ ~~ ~> ~~>

     <<< << <= <>  >= >> >>>
   {. {| [| <:  ✓  :> |] |} .}
   <||| <|| <| <|> |> ||> |||>

            <$ <$> $>
            <+ <+> +>
            <* <*> *>

       \  \\  /* */  /// //
      </ <!--  </>  --> />
      0xF 9:45 m-x *ptr www

       ;; :: ::: !! ?? %% &&
      || .. ... ..< .? ?. ?:
       -- --- ++ +++ ** ***

          ~= ~- -~ ~@
          ^= ?= /= /==
        -| _|_ |- |= ||=
        #! #= ## ### ####
      #{ #[ ]# #( #? #_ #_(


a*b a*A B*b A*B *a *A a* A*
a-b a-A B-b A-B -a -A a- A-
a+b a+A B+b A+B +a +A a+ A+
a:b a:A B:b A:B :a :A a: A:

#endif

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdio.h>
#include <assert.h>

CONST WCHAR     g_ksFontFace[32] = L"Fira Code";
CONST UINT      g_knFontSize = 32;
CONST UINT32    g_kiColorForeground = 0xE9DED8;
CONST UINT32    g_kiColorBackground = 0x463D34;
CONST UINT32    g_kiColorCarret = 0x007fff;

HINSTANCE       g_hInstance;
CONST LPCWSTR   g_ksMainClassName = L"WCNA7_MAIN";

HFONT           g_hFontText = NULL;
HBRUSH          g_hBrushBackground = NULL;
HBRUSH          g_hBrushCarret = NULL;


/* Переносит строку в количестве [nCount] символов из [s + nPos] в [s] */
VOID rWcharMoveToLeft ( CONST LPWSTR s, CONST UINT nCount, CONST UINT nPos ) {
    for ( UINT i = 0; i < nCount; ++i ) s [ i ] = s [ i + nPos ];
}
/* Переносит строку в количестве [nCount] символов из [s] в [s + nPos] */
VOID rWcharMoveToRight ( CONST LPWSTR s, CONST UINT nCount, CONST UINT nPos ) {
    for ( UINT i = nCount - 1; i < nCount; --i ) s [ i + nPos ] = s [ i ];
}

struct SFileBufferLine {
    LPWSTR sBuf;
    UINT nSize;
    UINT nSizeMax;
};

/* Добавляет символы в линию [pLine] для новых символов в количестве [nCount] в позиции [nPos] */
LPWSTR rFBL_InsertSpace ( struct SFileBufferLine * CONST pLine, CONST UINT nCount, CONST UINT nPos ) {
    /* Проверка влезают ли новые символы в память */
    if ( pLine -> nSize + nCount > pLine -> nSizeMax ) {
        pLine -> nSizeMax += ( ( ( nCount >> 4 ) + 1 ) << 4 );
        LPWSTR sBuf = ( LPWSTR ) malloc ( sizeof ( WCHAR ) * pLine -> nSizeMax );
        if ( pLine -> sBuf != NULL ) {
            memcpy ( sBuf, pLine -> sBuf, sizeof ( WCHAR ) * pLine -> nSize );
            free ( pLine -> sBuf );
        }
        pLine -> sBuf = sBuf;
    }
    rWcharMoveToRight ( pLine -> sBuf + nPos, pLine -> nSize - nPos, nCount );
    pLine -> nSize += nCount;
    return pLine -> sBuf + nPos;
}
LPWSTR rFBL_InsertSpaceR ( struct SFileBufferLine * CONST pLine, CONST UINT nCount, CONST UINT nPos ) {
    return rFBL_InsertSpace ( pLine, nCount, pLine -> nSize - nPos );
}

/* Удаляет символы в линии [pLine] в количестве [nCount] начиная с позиции [nPos] */
VOID rFBL_DeleteChars ( struct SFileBufferLine * CONST pLine, CONST UINT nCount, CONST UINT nPos ) {
    assert ( nCount + nPos <= pLine -> nSize );
    rWcharMoveToLeft ( pLine -> sBuf + nPos, pLine -> nSize - nPos - nCount, nCount );
    pLine -> nSize -= nCount;
}
/* Удаляет символы в линии [pLine] в количестве [nCount] начиная с позиции [nPos] с конца */
VOID rFBL_DeleteCharsR ( struct SFileBufferLine * CONST pLine, CONST UINT nCount, CONST UINT nPos ) {
    rFBL_DeleteChars ( pLine, nCount, pLine -> nSize - nPos - nCount );
}



struct SFileBuffer {
    struct SFileBufferLine * aLines;
    UINT nLines;
    UINT nLinesMax;
};


VOID rFB_Close ( struct SFileBuffer * pFB ) {
    if ( pFB -> aLines != NULL ) {
        struct SFileBufferLine * pLine = pFB -> aLines;
        for ( UINT i = 0; i < pFB -> nLines; ++i ) {
            free ( pLine -> sBuf );
            ++pLine;
        }
    }
    memset ( pFB, 0, sizeof ( struct SFileBuffer ) );
}

struct SFileBuffer * rFB_OpenFile ( struct SFileBuffer * pFB, LPCWSTR sFileName ) {
    rFB_Close ( pFB );
    /* Копируем название файла */
    FILE * pF = _wfopen ( sFileName, L"rb" );
    assert ( pF != NULL );
    /* Получаем размеры файла */
    fseek ( pF, 0, SEEK_END );
    CONST UINT nSize = ftell ( pF );
    fseek ( pF, 0, SEEK_SET );
    /* Копируем данные файла в память */
    BYTE * CONST pFileBuf = (BYTE *) malloc ( nSize );
    UINT nSizeReaded = 0;
    while ( nSizeReaded < nSize ) {
        nSizeReaded += fread ( pFileBuf + nSizeReaded, 1, nSize - nSizeReaded, pF );
    }
    fclose ( pF );
    /* Подсчитываем количество линий в файле */
    UINT nLines = 1;
    for ( UINT i = 0; i < nSize; ++i ) {
        if ( pFileBuf [ i ] == '\n' ) ++nLines;
    }
    /* Выделяем память под массивы линий данных */
    pFB -> nLines = nLines;
    pFB -> nLinesMax = ( ( ( nLines >> 4 ) + 1 ) << 4 );
    struct SFileBufferLine * pLine = pFB -> aLines = ( struct SFileBufferLine * ) malloc ( sizeof ( struct SFileBufferLine ) * pFB -> nLinesMax );
    memset ( pLine, 0, sizeof ( struct SFileBufferLine ) * pFB -> nLinesMax );
    /* Переносим байты */
    for ( UINT i = 0; i < nSize; ++ i ) {
        if ( pFileBuf [ i ] == '\n' ) {
            ++pLine;
        } else {
            LPWSTR s = rFBL_InsertSpaceR ( pLine, 1, 0 );
            if ( ( pFileBuf [ i ] & 0x80 ) ) {
                /* UTF-8 */
                if ( ( pFileBuf [ i ] & 0xE0 ) == 0xC0 ) {
                    *s =
                    ( ( pFileBuf [ i + 0 ] & 0x1F ) << 6 ) +
                    ( pFileBuf [ i + 1 ] & 0x3F );
                    i += 1;
                } else if ( ( pFileBuf [ i ] & 0xF0 ) == 0xE0 ) {
                    *s =
                    ( ( pFileBuf [ i + 0 ] & 0x0F ) << 12 ) +
                    ( ( pFileBuf [ i + 1 ] & 0x3F ) << 6 ) +
                    ( pFileBuf [ i + 2 ] & 0x3F );
                    i += 2;
                } else if ( ( pFileBuf [ i ] & 0xF8 ) == 0xF0 ) {
                    *s =
                    ( ( pFileBuf [ i + 0 ] & 0x07 ) << 18 ) +
                    ( ( pFileBuf [ i + 1 ] & 0x3F ) << 12 ) +
                    ( ( pFileBuf [ i + 2 ] & 0x3F ) << 6 ) +
                    ( pFileBuf [ i + 3 ] & 0x3F );
                    i += 3;
                }
            } else {
                /* ASCII */
                *s = pFileBuf [ i ];
            }
        }
    }
    return pFB;
}

struct SCarret {
    struct SFileBufferLine * pFB;
    UINT nLine;
    UINT nColumn;
};

/* Процедура главного окна */
LRESULT CALLBACK rMsgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    static struct SFileBuffer _FB = { };

    static HDC bmp_hDC = NULL;
    static HBITMAP bmp_hBMP;
    static BYTE * bmp_buf;
    static struct SCarret * pCarret = NULL;
    static UINT iMouseX = 0;
    static UINT iMouseY = 0;
    static UINT iMouseLine = 0;
    static UINT iMouseColumn = 0;

    void redraw ( ) {

        RECT rc = { .left = 0, .top = 0, .right = 1024, .bottom = 4096 };
        FillRect ( bmp_hDC, &rc, g_hBrushBackground );
        for ( UINT i = 0; i < _FB.nLines; ++i ) {
            TextOutW ( bmp_hDC, 0, i * g_knFontSize,
                _FB.aLines[i].sBuf, _FB.aLines[i].nSize );;
        }
    }

    switch ( uMsg ) {
        case WM_CREATE: {
            LOGFONTW lf = { 0 };
            memcpy ( lf.lfFaceName, g_ksFontFace, 32 );
            lf.lfHeight = g_knFontSize;
            lf.lfWeight = 300;
            lf.lfCharSet = ANSI_CHARSET;
            lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
            lf.lfQuality = DEFAULT_QUALITY;
            g_hFontText = CreateFontIndirectW ( &lf );
            g_hBrushBackground = CreateSolidBrush ( g_kiColorBackground );
            g_hBrushCarret = CreateSolidBrush ( g_kiColorCarret );
            rFB_OpenFile ( &_FB, L"src\\main.c" );

            HDC hDC = GetWindowDC ( hWnd );
            bmp_hDC = CreateCompatibleDC ( hDC );
            ReleaseDC ( hWnd, hDC );

            BITMAPINFO bi;
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biBitCount = 24;
            bi.bmiHeader.biWidth = 1024;
            bi.bmiHeader.biHeight = 4096;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biSizeImage = 0;
            bmp_hBMP = CreateDIBSection ( bmp_hDC, &bi, DIB_RGB_COLORS, (void**)&bmp_buf, NULL, 0 );
            SelectObject ( bmp_hDC, bmp_hBMP );
            SetBkMode ( bmp_hDC, TRANSPARENT );
            SelectObject ( bmp_hDC, g_hFontText );
            SetTextColor ( bmp_hDC, g_kiColorForeground );
            redraw ( );

            return 0;
        }
        case WM_SIZE: {
            return 0;
        }
        case WM_DESTROY: {
            DeleteBrush ( g_hBrushCarret );
            DeleteBrush ( g_hBrushBackground );
            DeleteFont ( g_hFontText );
            DeleteBitmap ( bmp_hBMP );
            DeleteDC ( bmp_hDC );
            PostQuitMessage ( 0 );
            return 0;
        }
        case WM_MOUSEMOVE: {
            iMouseX = GET_X_LPARAM(lParam);
            iMouseY = GET_Y_LPARAM(lParam);
            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
        case WM_LBUTTONDOWN: {
            iMouseX = GET_X_LPARAM(lParam);
            iMouseY = GET_Y_LPARAM(lParam);
            if ( pCarret == NULL ) {
                pCarret = (struct SCarret *) malloc ( sizeof ( struct SCarret ) );
                pCarret -> pFB = &_FB;
            }
            pCarret -> nColumn = iMouseColumn;
            pCarret -> nLine = iMouseLine;
            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
        case WM_COMMAND: {
            return 0;
        }
        case WM_KEYDOWN: {
            if ( pCarret == NULL ) return 0;
            switch ( wParam ) {
                case VK_BACK:
                    --pCarret -> nColumn;
                    rFBL_DeleteChars ( _FB.aLines + pCarret -> nLine, 1, pCarret -> nColumn );
                    redraw ( );
                    break;
                case VK_DELETE:
                    rFBL_DeleteChars ( _FB.aLines + pCarret -> nLine, 1, pCarret -> nColumn );
                    redraw ( );
                    break;
            }
            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
        case WM_CHAR: {
            if ( pCarret == NULL ) return 0;
            if ( wParam >= 0x20 ) {
                * ( rFBL_InsertSpace ( _FB.aLines + pCarret -> nLine, 1, pCarret -> nColumn ) ) = wParam;
                redraw ( );
                InvalidateRect ( hWnd, NULL, FALSE );
                ++pCarret -> nColumn;
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );

            BitBlt ( hDC, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, bmp_hDC, 0, 0, SRCCOPY );
            iMouseLine = iMouseY / g_knFontSize;
            iMouseColumn = 1;
            SetBkMode ( hDC, TRANSPARENT );
            SelectObject ( hDC, g_hFontText );
            SetTextColor ( hDC, g_kiColorForeground );

            SIZE sz;
            GetTextExtentExPointW ( hDC, _FB.aLines[iMouseLine].sBuf, _FB.aLines[iMouseLine].nSize, iMouseX, &iMouseColumn, NULL, &sz );



            if ( pCarret != NULL ) {
                RECT rc;
                rc.top = pCarret -> nLine * g_knFontSize;
                rc.bottom = rc.top + g_knFontSize;
                rc.left = 0;
                if ( pCarret -> nColumn > 0 ) {
                    GetTextExtentPoint32W ( hDC, _FB.aLines[ pCarret -> nLine ].sBuf, pCarret -> nColumn, &sz );
                    rc.left = sz.cx;
                }
                rc.right = rc.left + 1;
                FillRect ( hDC, &rc, g_hBrushCarret );
            }


            WCHAR s[128];
            TextOutW ( hDC, 256, 0, s, snwprintf ( s, 127, L"Line %u, Column %u", iMouseLine+1, iMouseColumn+1 ) );


            EndPaint ( hWnd, &ps );
            return 0;
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}

/* Точка входа в приложение */
INT APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd ) {
    /* Регистрация класса главного окна */
    WNDCLASSEXW wc = {
        .cbSize        = sizeof ( WNDCLASSEXW ),
        .style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW,
        .lpfnWndProc   = rMsgProc,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = hInstance,
        .hIcon         = NULL,
        .hCursor       = LoadCursorW ( NULL, IDC_IBEAM ),
        .hbrBackground = NULL,
        .lpszMenuName  = NULL,
        .lpszClassName = g_ksMainClassName,
        .hIconSm       = NULL,
    };
    RegisterClassExW ( &wc );
    /* Создание главного окна */
    CreateWindowExW ( 0, g_ksMainClassName, NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );
    /* Входим в цикл обработки сообщений */
    MSG msg = { };
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }
    /* Освобождение класса главного окна */
    UnregisterClassW ( g_ksMainClassName, hInstance );
    return (INT) msg.wParam;
}
