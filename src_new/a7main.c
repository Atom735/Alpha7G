
#include <Windows.h>


#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>
#include <WChar.h>
#include <Math.h>

#include <JpegLib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "a7err.c"
#include "a7bmp.c"

/* Рисует маску с указаным цветом на цветовую карту */
VOID A7BmpDrawAlphaMap ( S7BMP *pDst, S7BMP *pSrc, BYTE r, BYTE g, BYTE b, BYTE a, BOOL bMirror ) {
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    BYTE *pbs = ( BYTE* ) pSrc -> pData;
    for ( UINT iy = 0; iy < pSrc -> nHeight; ++iy ) {
        CONST UINT idy = ( iy + pSrc -> nTop );
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < pSrc -> nWidth; ++ix ) {
            CONST UINT idx = ( ix + pSrc -> nLeft );
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 4;
            CONST UINT is = ( iy ) * pSrc -> nStride + ( bMirror ? pSrc -> nWidth - ix - 1 : ix );
            pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - pbs [ is ] ) + b * pbs [ is ] ) / 0xff;
            pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - pbs [ is ] ) + g * pbs [ is ] ) / 0xff;
            pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - pbs [ is ] ) + r * pbs [ is ] ) / 0xff;
            pbd [ id + 3 ] = ( pbd [ id + 3 ] * ( 0xff - pbs [ is ] ) + a * pbs [ is ] ) / 0xff;
        }
    }
}
/* Рисует прямоугольник с указаным цветом на цветовую карту */
VOID A7BmpDrawRect ( S7BMP *pDst, UINT nX, UINT nY, UINT nW, UINT nH, BYTE r, BYTE g, BYTE b, BYTE a ) {
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = ( iy + nY );
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ( ix + nX );
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 4;
            pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + b * a ) / 0xff;
            pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + g * a ) / 0xff;
            pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + r * a ) / 0xff;
            pbd [ id + 3 ] = ( pbd [ id + 3 ] * ( 0xff - a ) + a * 0xff ) / 0xff;
        }
    }
}

S7BMP *A7BmpCreateTab ( S7BMP *pTab, FT_Face face, CONST CHAR *pStr ) {

    S7BMP *buf = A7BmpCreate ( D7BMP_A7_RGBA, 512, 512, 0 );
    memset ( buf -> pData, 0, buf -> nHeight * buf -> nStride );

    S7BMP *glyph = NULL;

    UINT x = 0;
    pTab -> nTop = 0;
    pTab -> nLeft = 0;
    A7BmpDrawAlphaMap ( buf, pTab, 0xff, 0x7f, 0x1f, 0xff, FALSE );
    x += pTab -> nWidth * 64;
    CONST UINT h = pTab -> nHeight * 32;
    UINT x0 = pTab -> nWidth;
    UINT x1 = 0;

    for ( CHAR CONST *ch = pStr; *ch != 0; ++ch ) {
        glyph = A7BmpLoad_Symbol ( glyph, face, *ch, h, x, h+h / 2 );
        A7BmpDrawRect ( buf, x / 64, 0, glyph -> nAdvance / 64, pTab -> nHeight, 0xff, 0x7f, 0x1f, 0xff );
        A7BmpDrawAlphaMap ( buf, glyph, 0x1f, 0x7f, 0xff, 0xff, FALSE );
        x += glyph -> nAdvance;
    }

    pTab -> nTop = 0;
    pTab -> nLeft = x / 64;
    A7BmpDrawAlphaMap ( buf, pTab, 0xff, 0x7f, 0x1f, 0xff, TRUE );

    if ( glyph ) A7BmpFree ( glyph );

    return buf;
}


/* Хендл приложения */
HINSTANCE       g_hInstance;
/* Имя класса главного окна */
CONST LPCWSTR   kg_szMainClassName      = L"CWN-A7Main";
/* ATOM класса главного окна */
ATOM            g_iMainClassAtom;
/* Handle главного окна */
HWND            g_hWndMain;

S7BMP           *g_bmpWallPapper;

S7BMP           *g_bmpGDI_Layer_BackGround;
S7BMP           *g_bmpGDI_Layer_Tabs;

S7BMP           *g_bmp_Tab;

S7BMP           *g_bmpTab;

typedef struct _S7SETTINGS {

} S7SETTINGS;


FT_Library      g_ftLibrary;
FT_Face         g_ftFace;

/* Название процедуры главного окна */
LRESULT CALLBACK A7MainWinProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch ( uMsg ) {
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
            g_bmpWallPapper = A7BmpCreateByJpegFileA ( "data/mountain_river_snow_winter_93245_1920x1080.jpg", FALSE );
            g_bmp_Tab = A7BmpGen_Tab ( 32, 1.0f, 12.0f, 12.0f );
            // g_bmp_Tab = A7BmpGen_Symbol ( g_ftFace, '@', 64*73, 64*73, 64*73 );

            g_bmpTab = A7BmpCreateTab ( g_bmp_Tab, g_ftFace, "Hello World!" );


            g_bmpGDI_Layer_BackGround = NULL;
            g_bmpGDI_Layer_Tabs = NULL;
            return 0;
        }
        case WM_SIZE: {
            HDC hDC = GetDC ( hWnd );
            if ( g_bmpGDI_Layer_BackGround != NULL ) A7BmpFree ( g_bmpGDI_Layer_BackGround );
            g_bmpGDI_Layer_BackGround = A7BmpCreateByGDI ( hDC, LOWORD ( lParam ), HIWORD ( lParam ), FALSE, FALSE );
            A7BmpCopy ( g_bmpGDI_Layer_BackGround, 0, 0, g_bmpWallPapper, 0, 0, __min ( g_bmpGDI_Layer_BackGround -> nWidth, g_bmpWallPapper -> nWidth ), __min ( g_bmpGDI_Layer_BackGround -> nHeight, g_bmpWallPapper -> nHeight ) );

            if ( g_bmpGDI_Layer_Tabs != NULL ) A7BmpFree ( g_bmpGDI_Layer_Tabs );
            g_bmpGDI_Layer_Tabs = A7BmpCreateByGDI ( hDC, LOWORD ( lParam ), HIWORD ( lParam ), TRUE, FALSE );

            A7BmpCopy ( g_bmpGDI_Layer_Tabs, 0, 0, g_bmpTab, 0, 0, __min ( g_bmpGDI_Layer_Tabs -> nWidth, g_bmpTab -> nWidth ), __min ( g_bmpGDI_Layer_Tabs -> nHeight, g_bmpTab -> nHeight ) );

            ReleaseDC ( hWnd, hDC );
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage ( 0 );
            A7BmpFree ( g_bmp_Tab );
            A7BmpFree ( g_bmpWallPapper );
            A7BmpFree ( g_bmpTab );
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
            A7BmpGDI_Draw ( hDC, 0, 0, g_bmpGDI_Layer_BackGround, 0, 0, nCWidth, nCHeight );
            A7BmpGDI_Draw ( hDC, 0, 0, g_bmpGDI_Layer_Tabs, 0, 0, nCWidth, nCHeight );
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
