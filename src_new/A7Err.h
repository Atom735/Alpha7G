#ifndef _H_A7ERR_H_
#define _H_A7ERR_H_

/* Буффер для хранения текста последней сгенерироанной ошибки */
extern WCHAR g7Err_buf[];

/* Функция выхода из приложения с ошибкой */
BOOL A7Err ( LPCWSTR szTtile, INT iError, LPCWSTR szTypeName, LPCWSTR szFileName, INT nFileLine, LPCWSTR szText );
BOOL A7Err_WinAPI ( LPCWSTR szFuncName, LPCWSTR szFileName, INT szFileLine );
BOOL A7Err_FreeType ( LPCWSTR szFuncName, LPCWSTR szFileName, INT szFileLine, FT_Error iError );


#define D7ERR_WINAPI(a) if ( A7Err_WinAPI ( TEXT(#a), TEXT(__FILE__), __LINE__ ) )
#define D7ERR_FREETYPE(a,...) if ( A7Err_FreeType ( TEXT(#a), TEXT(__FILE__), __LINE__, a (__VA_ARGS__ ) ) )

#endif /* _H_A7ERR_H_ */


