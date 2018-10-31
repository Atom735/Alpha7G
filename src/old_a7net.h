#ifndef _H_A7NET_H_
#define _H_A7NET_H_

#include <Windows.h>

#define WSAWM_BUFLEN(lParam)    LOWORD(lParam)
#define WSAWM_EVENT(lParam)     LOWORD(lParam)
#define WSAWM_ERROR(lParam)     HIWORD(lParam)

#define WM_WSA_GETHOSTBYNAME    (WM_USER + 1)
#define WM_WSA_SELECT           (WM_USER + 2)

extern int A7NetInit ( HINSTANCE hInstance );
extern void A7NetRelease ( void );

extern HWND A7NetConnect ( const char *hostname, int port, int tls );

extern int g_A7NetConnectsCount;

#endif
