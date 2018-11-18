#include "A7Err.h"

/* Буффер для хранения текста последней сгенерироанной ошибки */
WCHAR g7Err_buf[2048];

/* Функция выхода из приложения с ошибкой */
BOOL A7Err ( LPCWSTR szTtile, INT iError, LPCWSTR szTypeName, LPCWSTR szFileName, INT nFileLine, LPCWSTR szText ) {
    if ( iError == 0 ) return FALSE;
    snwprintf ( g7Err_buf, 2047, L"Error!\n%s: %d (0x%x)\n%s:%d\n%s\n\nПродолжить?", szTypeName, ( INT ) iError, ( UINT ) iError, szFileName, nFileLine, szText != NULL ? szText : L"NULL");
    if ( MessageBoxW ( NULL, g7Err_buf, szTtile, MB_YESNO | MB_ICONSTOP ) == IDNO ) {
        exit ( iError );
    }
    return TRUE;
}
BOOL A7Err_WinAPI ( LPCWSTR szFuncName, LPCWSTR szFileName, INT nFileLine ) {
    DWORD iError = GetLastError ( );
    if ( iError == 0 ) return FALSE;
    LPWSTR szText = NULL;
    FormatMessageW ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, iError, MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPWSTR ) ( &szText ), 0, NULL );
    BOOL o = A7Err ( szFuncName, iError, L"WinAPI", szFileName, nFileLine, szText );
    if ( szText != NULL ) LocalFree ( szText );
    return o;
}
BOOL A7Err_FreeType ( LPCWSTR szFuncName, LPCWSTR szFileName, INT nFileLine, FT_Error iError ) {
    if ( iError == 0 ) return FALSE;
    LPCWSTR szText = L"unknown";
    switch ( iError ) {
        #define FT_ERRORDEF_(_e,_v,_s) case _v: szText = L##_s; break;
        #define FT_NOERRORDEF_(_e,_v,_s) FT_ERRORDEF_(_e,_v,_s)
        #include FT_ERROR_DEFINITIONS_H
    }
    return A7Err ( szFuncName, iError, L"FreeType", szFileName, nFileLine, szText );;
}
