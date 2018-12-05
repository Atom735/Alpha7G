#ifndef _H7_HEADER_H_
#define _H7_HEADER_H_

#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT _WIN32_WINNT_WIN7
#endif /*_WIN32_WINNT*/
#ifndef UNICODE
    #define UNICODE
#endif /*UNICODE*/

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <time.h>


#define FOR_U(_i,_a,_b) for ( UINT _i = _a; _i < _b; ++_i )

typedef struct tagS7Window S7Window;
typedef struct tagS7WindowVTbl S7WindowVTbl;

typedef
UINT
(*P7Wnd_WND) (
    S7Window* pWnd
);

typedef
UINT
(*P7Wnd_SIZE) (
    S7Window* pWnd,
    UINT iFlags,
    UINT nWidth,
    UINT nHeight
);


struct tagS7WindowVTbl {
    P7Wnd_WND    rOnCreate;
    P7Wnd_WND    rOnDestroy;
    P7Wnd_SIZE   rOnSize;
    P7Wnd_WND    rOnPaint;
};

struct tagS7Window {
    HWND hWnd;
    S7WindowVTbl *pVTbl;
};

typedef struct tagS7FuncStack S7FuncStack;

typedef
UINT
(*P7FuncStackProc) (
    UINT iState,
    VOID *pvCtx
);

enum {
    K7FS_NONE = 0,
    K7FS_PUSH,
    K7FS_POP,
};

struct tagS7FuncStack {
    BOOL            bAllocated;
    P7FuncStackProc rFunc;
    S7FuncStack    *pPrev;
    VOID           *pvCtx;
};

S7FuncStack *A7FS_New ( P7FuncStackProc rFunc, VOID *pvCtx );
S7FuncStack *A7FS_Push ( S7FuncStack *pThis, S7FuncStack *pPush );
S7FuncStack *A7FS_PushNew ( S7FuncStack *pThis, P7FuncStackProc rFunc, VOID *pvCtx );
S7FuncStack *A7FS_Pop ( S7FuncStack *pThis );



extern FILE           *g_hLogFile;
extern WCHAR           g_wLogBuf [ ] ;
extern WCHAR           g_wLogSpaces [ ];
extern CONST UINT      g_knLogStackIdent;
extern UINT            g_nLogStackSz;

#define LOG_ENTER() fwprintf ( g_hLogFile, L"%ls>>% 8u to [%hs] in (%hs:%i)\n", g_wLogSpaces + ( g_nLogStackSz & 0xff ), ( UINT ) clock ( ), __FUNCTION__, __FILE__, __LINE__ ), g_nLogStackSz -= g_knLogStackIdent
#define LOG_LEAVE() fwprintf ( g_hLogFile, L"%ls<<% 8u to [%hs] in (%hs:%i)\n", g_wLogSpaces + ( ( g_nLogStackSz += g_knLogStackIdent ) & 0xff ), ( UINT ) clock ( ), __FUNCTION__, __FILE__, __LINE__ )
#define LOG_ARG(_s,...) fwprintf ( g_hLogFile, L"%ls" _s L"\n", g_wLogSpaces + g_nLogStackSz, __VA_ARGS__ )
#define LOG_ARG_END() fwprintf ( g_hLogFile, L"\n" )
#define LOG_ARG_HANDLE(_h) LOG_ARG ( TEXT( #_h ) L" = %p HANDLE", _h )
#define LOG_ARG_ASTR(_p) LOG_ARG ( TEXT( #_p ) L" = %p \"%-.32hs\"", _p, _p )
#define LOG_ARG_WSTR(_p) LOG_ARG ( TEXT( #_p ) L" = %p \"%-.32ls\"", _p, _p )
#define LOG_ARG_NUM(_i) LOG_ARG ( TEXT( #_i ) L" = %i", _i )
#define LOG_RET_NUM(_i) LOG_ARG ( L"return = %i", _i )
#define LOG_RET_HANDLE(_h) LOG_ARG ( L"return = %p HANDLE", _h )




VOID A7ErrFatal (
    LPCSTR sFuncName,
    LPCSTR sFileName,
    INT nFileLine,
    INT iError,
    ...
);

enum {
    E7_OK = 0,
    E7_IN_WINAPI, /* LPCWSTR - описание ошибки */
    E7_MALLOC = 0xE701, /* Ошибка malloc */

};
#define F7ERR_FATAL(...) A7ErrFatal ( __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__ )
VOID A7Err_WinAPI ( LPCSTR sFuncName, LPCSTR sFileName, INT sFileLine );
#define F7ERR_WINAPI(_funcName,_eexpr) (void) ( _eexpr ? A7Err_WinAPI ( #_funcName, __FILE__, __LINE__ ) : 0 )

#define F7ERR_MALLOC(_pv,_wtxt) (void) ( _pv ? 0 : F7ERR_FATAL ( E7_MALLOC, _pv, _wtxt ) )


#endif /*_H7_HEADER_H_*/
