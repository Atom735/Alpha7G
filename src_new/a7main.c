
#include <Windows.h>


#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>
#include <WChar.h>
#include <Math.h>

#include <JpegLib.h>

#include "a7err.c"
#include "a7bmp.c"

/* Хендл приложения */
HINSTANCE       g_hInstance;
/* Имя класса главного окна */
CONST LPCWSTR   kg_szMainClassName      = L"CWN-A7Main";
/* ATOM класса главного окна */
ATOM            g_iMainClassAtom;
/* Handle главного окна */
HWND            g_hWndMain;

S7BMP           *g_bmpWallPapper;

S7BMP           *g_bmpGDI_Layer0;
S7BMP           *g_bmpGDI_Layer1;

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
            g_bmpGDI_Layer0 = NULL;
            return 0;
        }
        case WM_SIZE: {
            HDC hDC = GetDC ( hWnd );
            if ( g_bmpGDI_Layer0 != NULL ) A7BmpFree ( g_bmpGDI_Layer0 );
            g_bmpGDI_Layer0 = A7BmpCreateByGDI ( hDC, LOWORD ( lParam ), HIWORD ( lParam ), FALSE, FALSE );
            A7BmpCopy ( g_bmpGDI_Layer0, 0, 0, g_bmpWallPapper, 0, 0, __min ( g_bmpGDI_Layer0 -> nWidth, g_bmpWallPapper -> nWidth ), __min ( g_bmpGDI_Layer0 -> nHeight, g_bmpWallPapper -> nHeight ) );
            ReleaseDC ( hWnd, hDC );
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage ( 0 );
            A7BmpFree ( g_bmpWallPapper );
            if ( g_bmpGDI_Layer0 != NULL ) A7BmpFree ( g_bmpGDI_Layer0 );
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
            A7BmpGDI_Draw ( hDC, 0, 0, g_bmpGDI_Layer0, 0, 0, nCWidth, nCHeight );
            EndPaint ( hWnd, &ps );
            // DeleteObject ( hbrBlack );
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}



/* Точка входа в приложение */
INT APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd ) {

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
    if ( ( g_hWndMain = CreateWindowExW ( 0, ( LPCWSTR ) g_iMainClassAtom, NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, NULL ) ) == NULL )
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


    return 0;
}
