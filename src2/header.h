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
#include <wchar.h>
#include <time.h>


#define FOR_U(_i,_a,_b) for ( UINT _i = _a; _i < _b; ++_i )





extern FILE           *g_hLogFile;
extern WCHAR           g_wLogBuf [ ] ;
extern WCHAR           g_wLogSpaces [ ];
extern CONST UINT      g_knLogStackIdent;
extern UINT            g_nLogStackSz;

#define LOG_ENTER() fwprintf ( g_hLogFile, L"%ls%u Enter to [%hs] in (%hs:%i)\n", g_wLogSpaces + ( g_nLogStackSz & 0xff ), ( UINT ) clock ( ), __FUNCTION__, __FILE__, __LINE__ ), g_nLogStackSz -= g_knLogStackIdent
#define LOG_LEAVE() fwprintf ( g_hLogFile, L"%ls%u Leave to [%hs] in (%hs:%i)\n", g_wLogSpaces + ( ( g_nLogStackSz += g_knLogStackIdent ) & 0xff ), ( UINT ) clock ( ), __FUNCTION__, __FILE__, __LINE__ )
#define LOG_ARG(_s,...) fwprintf ( g_hLogFile, L"%ls" _s L"\n", g_wLogSpaces + g_nLogStackSz, __VA_ARGS__ )
#define LOG_ARG_END() fwprintf ( g_hLogFile, L"\n" )
#define LOG_ARG_HANDLE(_h) LOG_ARG ( TEXT( #_h ) L" = %p HANDLE", _h )
#define LOG_ARG_ASTR(_p) LOG_ARG ( TEXT( #_p ) L" = %p \"%-.32hs\"", _p, _p )
#define LOG_ARG_WSTR(_p) LOG_ARG ( TEXT( #_p ) L" = %p \"%-.32ls\"", _p, _p )
#define LOG_ARG_NUM(_i) LOG_ARG ( TEXT( #_i ) L" = %i", _i )

#endif /*_H7_HEADER_H_*/
