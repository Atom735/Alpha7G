#include <Windows.h>

HINSTANCE g_hInstance;
CONST LPCWSTR g_ksMainClassName = L"WCNA7_MAIN";

/* Процедура главного окна */
LRESULT CALLBACK rMsgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    static WCHAR sBuf[1024] = L"Hello World!";
    static UINT nBufSz = 12;
    switch ( uMsg ) {
        case WM_CREATE: {
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
        case WM_CHAR: {
            if ( wParam >= 0x20 ) {
                sBuf [ nBufSz ] = wParam;
                ++nBufSz;
            } else
            if ( wParam == 0x08 ) {
                --nBufSz;
            } else
            if ( wParam == 0x0D ) {
                nBufSz = 0;
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
