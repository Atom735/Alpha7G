


/* Функция выхода из приложения с ошибкой */
VOID A7ErrExit ( LPCWSTR szFuncName, LPCWSTR szFileName, INT szFileLine ) {
    DWORD err = GetLastError ( );
    static WCHAR erstr[2048];
    LPWSTR errText = NULL;
    FormatMessageW ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPWSTR ) ( &errText ), 0, NULL );
    snwprintf ( erstr, 2047, L"Fatal Error: %d (0x%x)\n%s:%d\n%s", ( INT ) err, err, szFileName, szFileLine, errText != NULL ? errText : L"NULL");
    if ( errText != NULL ) LocalFree ( errText );
    MessageBoxW ( NULL, erstr, szFuncName, MB_OK | MB_ICONSTOP );
    exit ( err );
}
#define D7ERROREXIT(a) A7ErrExit ( TEXT(#a), TEXT(__FILE__), __LINE__ )
