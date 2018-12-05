#include "header.h"

HINSTANCE       g_hInstance;


static LRESULT CALLBACK
_7SysWndProc (
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {
    static UINT _nSysWndCount = 0;
    S7Window *p = ( S7Window* ) GetWindowLongPtrW ( hWnd, GWLP_USERDATA );
    S7WindowVTbl *pVTbl = p ? p -> pVTbl : NULL;
    SetLastError ( 0 );
    F7ERR_WINAPI ( GetWindowLongPtrW, p == NULL );
    switch ( uMsg ) {
        case WM_CREATE: {
            ++_nSysWndCount;
            p = ( S7Window* ) malloc ( sizeof ( S7Window ) );
            F7ERR_MALLOC ( p, "Под структуру S7Window внутри функции _7SysWndProc" );
            p -> hWnd = hWnd;
            SetLastError ( 0 );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( p ) ) == 0 ) {
                F7ERR_WINAPI ( SetWindowLongPtrW, TRUE );
            }
            CREATESTRUCT *pCTS = ( CREATESTRUCT* ) ( lParam );
            p -> pVTbl = ( S7WindowVTbl* ) pCTS -> lpCreateParams;
            if ( p -> pVTbl && p -> pVTbl -> rOnCreate ) {
                return p -> pVTbl -> rOnCreate ( p );
            }
            return 0;
        }
        case WM_SIZE: {
            if ( pVTbl && pVTbl -> rOnSize ) {
                return pVTbl -> rOnSize ( p, wParam, LOWORD ( lParam ), HIWORD ( lParam ) );
            }
            return 0;
        }
        case WM_DESTROY: {
            UINT uCode = 0;
            if ( pVTbl && pVTbl -> rOnDestroy ) {
                uCode = pVTbl -> rOnDestroy ( p );
            }
            free ( p );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( NULL ) ) == 0 ) {
                F7ERR_WINAPI ( SetWindowLongPtrW, TRUE );
            }
            --_nSysWndCount;
            if ( _nSysWndCount == 0 ) {
                PostQuitMessage ( uCode );
            }
            return 0;
        }
        case WM_PAINT: {
            if ( pVTbl && pVTbl -> rOnPaint ) {
                return pVTbl -> rOnPaint ( p );
            }
            return 0;
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}

static UINT
_7SysInit (
    UINT iState,
    VOID *pvCtx
) {
    static CONST LPCWSTR kwClassName = L"WCN-Alpha7-Universal";
    switch ( iState ) {
        case K7FS_PUSH: {
            /* Регистрация класса главного окна */
            WNDCLASSEXW wc = {
                .cbSize        = sizeof ( WNDCLASSEXW ),
                .style         = 0x0,
                .lpfnWndProc   = _7SysWndProc,
                .cbClsExtra    = 0,
                .cbWndExtra    = 0,
                .hInstance     = g_hInstance,
                .hIcon         = NULL,
                .hCursor       = NULL,
                .hbrBackground = NULL,
                .lpszMenuName  = NULL,
                .lpszClassName = kwClassName,
                .hIconSm       = NULL,
            };
            CONST ATOM iClassAtom = RegisterClassExW ( &wc );
            F7ERR_WINAPI ( RegisterClassExW, iClassAtom == 0 );
            CONST HWND hWnd = CreateWindowExW ( 0, kwClassName, NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, NULL );
            F7ERR_WINAPI ( CreateWindowExW, hWnd == NULL );
            return 0;
        }
        case K7FS_POP: {
            if ( ! UnregisterClassW ( kwClassName, g_hInstance ) ) {
                F7ERR_WINAPI ( UnregisterClassW, TRUE );
            }
            return 0;
        }
    }
    return 0;
}


/* Точка входа в приложение */
INT APIENTRY
wWinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR wCmdLine,
    INT iShowCmd
) {
    g_hLogFile = fopen ( "log.log", "wb" );
    if ( !g_hLogFile ) return -1;
    fwide ( g_hLogFile, 1 );
    fwprintf ( g_hLogFile, L"\xFEFF" );
    FOR_U ( i, 0, 0xff ) {
        g_wLogSpaces [ i ] = L' ';
    }
    g_wLogSpaces [ 0xff ] = 0;
    LOG_ENTER       ( );
    LOG_ARG_HANDLE  ( hInstance );
    LOG_ARG_HANDLE  ( hPrevInstance );
    LOG_ARG_WSTR    ( wCmdLine );
    LOG_ARG_NUM     ( iShowCmd );

    g_hInstance = hInstance;
    S7FuncStack *pFuncStack = A7FS_PushNew ( NULL, _7SysInit, NULL );
    MSG msg = { };
    if ( pFuncStack ) {
        while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }
        while ( ( pFuncStack = A7FS_Pop ( pFuncStack ) ) );
    }

    LOG_RET_NUM ( msg.wParam );
    LOG_LEAVE ( );
    fclose ( g_hLogFile );

    return msg.wParam;
}


