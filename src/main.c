#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <WS2TCPIP.h>
// #include <IPHlpAPI.h>
#include <Windows.h>

#include <OpenSSL/SSL.h>
#include <OpenSSL/ERR.h>

#include <StdLib.h>
#include <StdIO.h>
#include <String.h>
#include <TChar.h>

#include "log.h"
#include "help_wsa.c"

FILE       *g_logfile = NULL;
#define CLASSNAME_ALPHA7 "CLN-Alpha7G-IP/NET"

HINSTANCE   g_hInstance = NULL;
WSADATA     g_WSAData = {0};

typedef struct _NETWNDDATA {
    CHAR host[32];
    u_short port;
    CHAR *hstn;
    DWORD btSent;
    DWORD btRecv;
    DWORD flRecv;
    WSABUF req;
    WSABUF res;
} NETWNDDATA;

INT         g_wndcount = 0;

HWND a7CreateAsyncHttpConnection( LPCSTR i_host, LPCSTR i_path ) {
    NETWNDDATA *p = (NETWNDDATA*)malloc(sizeof(NETWNDDATA));
    ASSERT_ALLOC(p);
    strcpy( p->host, i_host );
    p->port = 80;
    p->req.len = 2047;
    p->req.buf = malloc(p->req.len+1);
    p->res.len = 2047;
    p->res.buf = malloc(p->res.len+1);
    if(i_path == NULL) i_path = "/";
    int sz = snprintf(p->req.buf, p->req.len+1, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",
        i_path, i_host );
    p->req.len = sz;

    return CreateWindowEx( WS_EX_OVERLAPPEDWINDOW, TEXT(CLASSNAME_ALPHA7), NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, p );
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch(uMsg) {
        case WM_CREATE: {
            CREATESTRUCT *c = (CREATESTRUCT*)lParam;
            NETWNDDATA *p = (NETWNDDATA*)c->lpCreateParams;

            SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)p );
            if( p == NULL ) {
                LOG_ERR( "SetWindowLongPtr()", hWnd );
            }

            p->hstn = (CHAR*)malloc(MAXGETHOSTSTRUCT);
            ASSERT_ALLOC(p->hstn);
            HANDLE h = WSAAsyncGetHostByName( hWnd, WM_WSAASYNCGETHOSTBYNAME, p->host, p->hstn, MAXGETHOSTSTRUCT );
            if( h == 0 ) {
                int err = WSAGetLastError();
                LOG_ERR_WSA( "WSAAsyncGetHostByName", err );
                return -1;
            }

            ++g_wndcount;

            return 0;
        }
        case WM_DESTROY: {
            NETWNDDATA *p = (NETWNDDATA*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
            if( p == NULL ) {
                LOG_ERR( "GetWindowLongPtr()", hWnd );
            }
            free( p->hstn );
            p->hstn = NULL;
            free(p);
            --g_wndcount;
            if(g_wndcount == 0)
                PostQuitMessage(0);
            return 0;
        }
        case WM_WSAASYNCGETHOSTBYNAME: {
            NETWNDDATA *p = (NETWNDDATA*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
            if( p == NULL ) {
                LOG_ERR( "GetWindowLongPtr()", hWnd );
            }
            if ( WSAGETASYNCERROR(lParam) ) {
                int err = WSAGETASYNCERROR(lParam);
                LOG_ERR_WSA( "WM_WSAASYNCGETHOSTBYNAME", err );
                DestroyWindow( hWnd );
            } else {
                HOSTENT *h = (HOSTENT*)p->hstn;
                if( h == NULL ) {
                    LOG_ERR( "HOSTENT is null", hWnd );
                }

                LOG( "Host: buflen -> %d\n", WSAGETASYNCBUFLEN(lParam) );
                LOG( "      name   -> %s\n", h->h_name );
                for ( char **ch = h->h_aliases; *ch != NULL; ++ch )
                    LOG( "      alias  -> %s\n", *ch );
                LOG( "      length -> %d\n", (int)h->h_length );
                for ( char **ch = h->h_addr_list; *ch != NULL; ++ch ) {
                    LOG( "      addr   -> %d", (int)(u_char)((*ch)[0]) );
                    for ( int i=1; i<(int)h->h_length; ++i )
                        LOG( ".%d", (int)(u_char)((*ch)[i]) );
                    LOG( "\n" );
                }

                SOCKET s = INVALID_SOCKET;
                if ( ( s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 ) ) == INVALID_SOCKET ) {
                    int err = WSAGetLastError();
                    LOG_ERR_WSA( "WSASocket()", err );
                    DestroyWindow( hWnd );
                    return 0;
                }
                LOG( "SOCKET(%d) CREATED BY (%s)\n", (int)s, h->h_name );
                if ( WSAAsyncSelect( s, hWnd, WM_WSAASYNCSELECT, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE ) == SOCKET_ERROR ) {
                    int err = WSAGetLastError();
                    LOG_ERR_WSA( "WSAAsyncSelect()", err );
                    closesocket(s);
                    DestroyWindow( hWnd );
                    return 0;
                }

                SOCKADDR_IN sa = {
                    .sin_family = AF_INET,
                    .sin_port = htons(p->port),
                    .sin_addr.s_addr = *((UINT32*)(h->h_addr_list[0]))
                };


                if( WSAConnect( s, (LPSOCKADDR)(&sa), sizeof(sa), NULL, NULL, NULL, NULL ) == SOCKET_ERROR ) {
                    int err = WSAGetLastError();
                    if(err != WSAEWOULDBLOCK) {
                        LOG_ERR_WSA( "WSAConnect()", err );
                        closesocket(s);
                        DestroyWindow( hWnd );
                        return 0;
                    } else {
                        LOG_ERR_WSA( "WSAConnect() not ", err );
                    }
                }
            }
            return 0;
        }
        case WM_WSAASYNCSELECT: {
            NETWNDDATA *p = (NETWNDDATA*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
            if( p == NULL ) {
                LOG_ERR( "GetWindowLongPtr()", hWnd );
            }
            SOCKET s = (SOCKET)wParam;
            if (WSAGETSELECTERROR(lParam)) {
                int err = WSAGETSELECTERROR(lParam);
                LOG_ERR_WSA( "WM_WSAASYNCSELECT", err );
                closesocket( s );
                DestroyWindow( hWnd );
            } else {
                switch(WSAGETSELECTEVENT(lParam)){
                    case FD_READ:   LOG( "SOCKET(%d) FD_READ\n",   (int)s); break;
                    case FD_WRITE:  LOG( "SOCKET(%d) FD_WRITE\n",  (int)s); break;
                    case FD_OOB:    LOG( "SOCKET(%d) FD_OOB\n",    (int)s); break;
                    case FD_ACCEPT: LOG( "SOCKET(%d) FD_ACCEPT\n", (int)s); break;
                    case FD_CONNECT:LOG( "SOCKET(%d) FD_CONNECT\n",(int)s); break;
                    case FD_CLOSE:  LOG( "SOCKET(%d) FD_CLOSE\n",  (int)s); break;
                }
                switch(WSAGETSELECTEVENT(lParam)){
                    case FD_CONNECT: {
                        if ( WSASend( s, &p->req, 1, &p->btSent, 0, NULL, NULL ) == SOCKET_ERROR ) {
                            int err = WSAGetLastError();
                            if(err != WSAEWOULDBLOCK) {
                                LOG_ERR_WSA( "WSASend()", err );
                                closesocket(s);
                                DestroyWindow( hWnd );
                                return 0;
                            } else {
                                LOG_ERR_WSA( "WSASend() not ", err );
                            }
                        }
                        break;
                    }
                    case FD_READ: {
                        p->flRecv = 0;
                        if ( WSARecv( s, &p->res, 1, &p->btRecv, &p->flRecv, NULL, NULL ) == SOCKET_ERROR ) {
                            int err = WSAGetLastError();
                            if(err != WSAEWOULDBLOCK) {
                                LOG_ERR_WSA( "WSARecv()", err );
                                closesocket(s);
                                DestroyWindow( hWnd );
                                return 0;
                            } else {
                                LOG_ERR_WSA( "WSARecv() not ", err );
                            }
                        }
                        p->res.buf[p->btRecv] = '\0';
                        LOG( "SOCKET(%d) RECV %d >>> \n%s\n ========= END =========\n",(int)s, (int)p->btRecv, p->res.buf );
                        shutdown(s, SD_SEND);
                        break;
                    }
                    case FD_CLOSE: {
                        LOG( "SOCKET(%d) CLOSE", s );
                        closesocket(s);
                        DestroyWindow( hWnd );
                        break;
                    }
                }
            }
            return 0;
        }
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}



#ifndef UNICODE
int APIENTRY WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
#else
int APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd )
#endif
{
    g_hInstance = hInstance;
    LOG_OPEN();
    A7_WSAStartup(&g_WSAData);

    {
        WNDCLASSEXW wcex;
        wcex.cbSize         = sizeof(WNDCLASSEX);
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = WndProc;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = hInstance;
        wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
        wcex.lpszMenuName   = NULL;
        wcex.lpszClassName  = TEXT(CLASSNAME_ALPHA7);
        wcex.hIconSm        = NULL;
        if ( RegisterClassEx( &wcex ) == 0 ) {
            DWORD err = GetLastError();
            LOG_ERR( "RegisterClassEx()", err );
            return (-1);
        }
    }

    a7CreateAsyncHttpConnection( "example.com", NULL );
    a7CreateAsyncHttpConnection( "vk.com", NULL );
    a7CreateAsyncHttpConnection( "yandex.ru", NULL );

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    A7_WSACleanup();
    LOG_CLOSE();
    return (int) msg.wParam;
}

