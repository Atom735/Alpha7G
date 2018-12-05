#include "header.h"


FILE           *g_hLogFile;
WCHAR           g_wLogBuf [ 2048 ] ;
WCHAR           g_wLogSpaces [ 256 ];
CONST UINT      g_knLogStackIdent = 4;
UINT            g_nLogStackSz = 0xff;

VOID A7ErrFatal (
    LPCSTR sFuncName,
    LPCSTR sFileName,
    INT nFileLine,
    INT iError,
    ...
) {
    LOG_ENTER ( );
    if ( iError == E7_OK ) return;
    UINT i = swprintf ( g_wLogBuf, 2047, L"Отметка времени: %ums\nФункция: %hs\nФайл: %hs:%i\nНомер ошибки: %i\n\n", clock ( ), sFuncName, sFileName, nFileLine, iError );
    va_list args;
    va_start ( args, iError );
    switch ( iError ) {
        case E7_IN_WINAPI:
            swprintf ( g_wLogBuf + i, 2047 - i, L"Ошибка в модуле WinAPI\n%ls", va_arg ( args, LPCWSTR ) );
            break;
        case E7_MALLOC:
            if ( va_arg ( args, VOID* ) ) {
                va_end ( args );
                LOG_LEAVE ( );
                return;
            }
            swprintf ( g_wLogBuf + i, 2047 - i, L"Неудалось выделить память с помощью malloc\n%ls", va_arg ( args, LPCWSTR ) );
            break;
        default:
            swprintf ( g_wLogBuf + i, 2047 - i, L"Неизвестный код ошибки %i", iError );
    }
    va_end ( args );

    LOG_ARG ( L"=== Критическая ошибка! ===\n%ls", g_wLogBuf );
    if ( MessageBoxW ( NULL, g_wLogBuf, L"Критическая ошибка!", MB_YESNO | MB_ICONSTOP ) == IDNO ) {
        fclose ( g_hLogFile );
        exit ( iError );
    }
    LOG_LEAVE ( );
}

VOID A7Err_WinAPI ( LPCSTR sFuncName, LPCSTR sFileName, INT sFileLine ) {
    DWORD iError = GetLastError ( );
    if ( iError == 0 ) return;
    LPWSTR wText = NULL;
    FormatMessageW ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, iError, MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPWSTR ) ( &wText ), 0, NULL );
    A7ErrFatal ( sFuncName, sFileName, sFileLine, E7_IN_WINAPI, wText );
    if ( wText != NULL ) LocalFree ( wText );
    return;
}
