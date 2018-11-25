#ifndef _H_A7ERR_H_
#define _H_A7ERR_H_

/* Буффер для хранения текста последней сгенерироанной ошибки */
extern WCHAR g7Err_buf[];

/* Функция выхода из приложения с ошибкой */
BOOL A7Err ( LPCWSTR szTtile, INT iError, LPCWSTR szTypeName, LPCWSTR szFileName, INT nFileLine, LPCWSTR szText );
BOOL A7Err_WinAPI ( LPCWSTR szFuncName, LPCWSTR szFileName, INT szFileLine );
BOOL A7Err_FreeType ( LPCWSTR szFuncName, LPCWSTR szFileName, INT szFileLine, FT_Error iError );

#define __D7WTEXT(_txt) L##_txt
#define D7WTEXT(_txt) __D7WTEXT(_txt)
#define D7ERR_WINAPI(_funcName,_eexpr) (void) ( !!(_eexpr) && A7Err_WinAPI ( D7WTEXT ( #_funcName ), D7WTEXT ( __FILE__ ), __LINE__ ) )
#define D7ERR_FREETYPE(_funcName,...) (void) ( A7Err_FreeType (  D7WTEXT ( #_funcName ), D7WTEXT (__FILE__ ), __LINE__, _funcName (__VA_ARGS__ ) ) )


#define D7ERR_OTHER(_funcName,_eexpr,_text) ( !!(_eexpr) && A7Err ( D7WTEXT ( #_funcName ), 1, L"Other", D7WTEXT ( __FILE__ ), __LINE__, _text ) )
#define D7ERR_MALLOC(_funcName,_eexpr) if ( D7ERR_OTHER ( _funcName, _eexpr, L"Ошибка выделения памяти" ) )


#endif /* _H_A7ERR_H_ */


