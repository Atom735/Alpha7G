#include "Header.h"

HINSTANCE       g_hInstance;

FILE           *g_hLogFile;
WCHAR           g_wLogBuf [ 512 ] ;
WCHAR           g_wLogSpaces [ 256 ];
CONST UINT      g_knLogStackIdent = 4;
UINT            g_nLogStackSz = 0xff;

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

    if ( wmemcmp ( wCmdLine, L"test/", 5 ) == 0 ) {
        goto P_leave;
    }

    P_leave:
    LOG_LEAVE ( ) ;
    fclose ( g_hLogFile );
    return 0;
}
