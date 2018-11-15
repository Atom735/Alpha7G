
#include <Windows.h>


#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>
#include <WChar.h>


#include "a7err.c"
#include "a7bmp.h"

#if 0
S7Bmp *A7BmpCreateTab ( S7Bmp *pTab, FT_Face face, CONST CHAR *pStr ) {

    S7Bmp *buf = A7BmpCreate ( D7BMP_A7_RGBA, 2048, 2048, 0 );
    memset ( buf -> pData, 0, buf -> nHeight * buf -> nStride );

    S7Bmp *glyph = NULL;

    UINT x = 0;
    pTab -> nTop = 0;
    pTab -> nLeft = 0;
    A7BmpDrawAlphaMap ( buf, pTab, 0x1f, 0x1f, 0x1f, 0xff, FALSE );
    x += pTab -> nWidth * 64;
    CONST UINT h = pTab -> nHeight * 32;

    for ( CHAR CONST *ch = pStr; *ch != 0; ++ch ) {
        glyph = A7BmpLoad_Symbol ( glyph, face, *ch, h, x, h+h / 2 );
        A7BmpDrawRect ( buf, x / 64, 0, glyph -> nAdvance / 64, pTab -> nHeight, 0x1f, 0x1f, 0x1f, 0xff );
        A7BmpDrawAlphaMap ( buf, glyph, 0xef, 0xef, 0xef, 0xff, FALSE );
        x += glyph -> nAdvance;
    }

    pTab -> nTop = 0;
    pTab -> nLeft = x / 64;
    A7BmpDrawAlphaMap ( buf, pTab, 0x1f, 0x1f, 0x1f, 0xff, TRUE );

    if ( glyph ) A7BmpFree ( glyph );

    return buf;
}
#endif


/* Хендл приложения */
HINSTANCE       g_hInstance;
/* Имя класса главного окна */
CONST LPCWSTR   kg_szMainClassName      = L"CWN-A7Main";
/* ATOM класса главного окна */
ATOM            g_iMainClassAtom;
/* Handle главного окна */
HWND            g_hWndMain;

S7Bmp           *g_bmpWallPapper;

S7Bmp           *g_bmpGDI_Layer_BackGround;
S7Bmp           *g_bmpGDI_Layer_Tabs;

S7Bmp           *g_bmp_Tab;

// S7Bmp           *g_bmpTab;


FT_Library      g_ftLibrary;
FT_Face         g_ftFace;

UINT            g_nInputSz = 0;
CHAR            g_szInput[512];

/* Название процедуры главного окна */
LRESULT CALLBACK A7MainWinProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch ( uMsg ) {
        case WM_CHAR: {
            g_szInput [ g_nInputSz ] = wParam;
            ++g_nInputSz;
            InvalidateRect ( hWnd, NULL, FALSE );
            break;
        }
        case WM_KEYDOWN: {
            switch ( wParam ) {
                case VK_F1:
                    // Resampler = A7BmpResamplePoint;
                    // InvalidateRect ( hWnd, NULL, FALSE );
                    break;
                case VK_F2:
                    // Resampler = A7BmpResampleLinear;
                    // InvalidateRect ( hWnd, NULL, FALSE );
                    break;
            }
            return 0;
        }
        case WM_CREATE: {
            g_bmpWallPapper = A7BmpCreateByJpegFileA ( "data/mountain_river_snow_winter_93245_1920x1080.jpg" );
            g_bmp_Tab = A7BmpGen_Tab ( 32, 1.3f, 12.0f, 12.0f );
            // g_bmp_Tab = A7BmpGen_Symbol ( g_ftFace, '@', 64*73, 64*73, 64*73 );
            // g_bmpTab = A7BmpCreateTab ( g_bmp_Tab, g_ftFace, "Hello World!" );

            g_bmpGDI_Layer_BackGround = NULL;
            g_bmpGDI_Layer_Tabs = NULL;
            return 0;
        }
        case WM_SIZE: {
            HDC hDC = GetDC ( hWnd );
            if ( g_bmpGDI_Layer_BackGround != NULL ) A7BmpFree ( g_bmpGDI_Layer_BackGround );
            g_bmpGDI_Layer_BackGround = A7BmpCreateByGDI ( hDC, LOWORD ( lParam ), HIWORD ( lParam ), FALSE );
            A7BmpCopyFull ( g_bmpGDI_Layer_BackGround, 0, 0, g_bmpWallPapper );


            if ( g_bmpGDI_Layer_Tabs != NULL ) A7BmpFree ( g_bmpGDI_Layer_Tabs );
            g_bmpGDI_Layer_Tabs = A7BmpCreateByGDI ( hDC, LOWORD ( lParam ), HIWORD ( lParam ), TRUE );
            // A7BmpCopyFull ( g_bmpGDI_Layer_Tabs, 0, 0, g_bmpTab );

            ReleaseDC ( hWnd, hDC );
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage ( 0 );
            A7BmpFree ( g_bmp_Tab );
            A7BmpFree ( g_bmpWallPapper );
            // A7BmpFree ( g_bmpTab );
            if ( g_bmpGDI_Layer_BackGround != NULL ) A7BmpFree ( g_bmpGDI_Layer_BackGround );
            if ( g_bmpGDI_Layer_Tabs != NULL ) A7BmpFree ( g_bmpGDI_Layer_Tabs );
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            // HBRUSH  hbrBlack = CreateSolidBrush ( RGB ( 0x00, 0x00, 0x00 ) );
            HDC hDC = BeginPaint ( hWnd, &ps );
            // FillRect ( hDC, &ps.rcPaint, hbrBlack );
            UINT nCWidth;
            UINT nCHeight;
            {
                RECT rt;
                GetClientRect ( hWnd, &rt );
                nCWidth = rt.right - rt.left;
                nCHeight = rt.bottom - rt.top;
            }

            CONST UINT h = 16;
            CONST UINT _i = nCHeight * 4 / 5 / h;
            for ( UINT i = 0; i < _i; ++i ) {
                A7BmpDrawTextA ( g_bmpGDI_Layer_Tabs, g_ftFace, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ Hellow World!))) What's your name? How old are You?",
                    8, h+i*h*5/4, h*64, 0, (_i/2-i)*4, 0xff, 0x7f, 0x00, 0xff );
                A7BmpDrawTextA ( g_bmpGDI_Layer_Tabs, g_ftFace, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ Hellow World!))) What's your name? How old are You?",
                    8, h+i*h*5/4+1, h*64, 64, (_i/2-i)*4, 0xff, 0xff, 0xff, 0xff );
            }


            A7BmpDrawFull_GDI ( hDC, 0, 0, g_bmpGDI_Layer_BackGround );
            A7BmpDrawFull_GDI ( hDC, 0, 0, g_bmpGDI_Layer_Tabs );

            // A7BmpGDI_Draw ( hDC, 0, 0, g_bmpGDI_Layer_BackGround, 0, 0, nCWidth, nCHeight );
            // A7BmpGDI_Draw ( hDC, 0, 0, g_bmpGDI_Layer_Tabs, 0, 0, nCWidth, nCHeight );


            EndPaint ( hWnd, &ps );
            // DeleteObject ( hbrBlack );
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}



/* Точка входа в приложение */
INT APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd ) {

    FT_Init_FreeType ( &g_ftLibrary );
    FT_New_Face ( g_ftLibrary, "C:\\Windows\\Fonts\\RobotoSlab-Regular.ttf", 0, &g_ftFace );

    g_hInstance = hInstance;
    /* Регистрация класса главного окна */
    {
        WNDCLASSEXW wc = {
            .cbSize        = sizeof ( WNDCLASSEXW ),
            .style         = CS_VREDRAW | CS_HREDRAW,
            .lpfnWndProc   = A7MainWinProc,
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = g_hInstance,
            .hIcon         = NULL,
            .hCursor       = NULL,
            .hbrBackground = NULL,
            .lpszMenuName  = NULL,
            .lpszClassName = kg_szMainClassName,
            .hIconSm       = NULL,
        };
        if ( ( g_iMainClassAtom = RegisterClassExW ( &wc ) ) == 0 )
            D7ERROREXIT ( RegisterClassExW );
    }

    /* Создание главного окна */
    if ( ( g_hWndMain = CreateWindowExW ( 0, ( ( LPCWSTR ) ( g_iMainClassAtom ) ), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, NULL ) ) == NULL )
        D7ERROREXIT ( CreateWindowExW );

    // ShowWindow ( g_hWndMain, nCmdShow );

    /* Входим в цикл обработки сообщений */
    {
        MSG msg = { };
        while ( GetMessage( &msg, NULL, 0, 0 ) ) {
            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }
    }

    /* Освобождение класса главного окна */
    if ( ! UnregisterClassW ( kg_szMainClassName, g_hInstance ) )
        D7ERROREXIT ( UnregisterClassW );

    FT_Done_Face ( g_ftFace );
    FT_Done_FreeType ( g_ftLibrary );

    return 0;
}
