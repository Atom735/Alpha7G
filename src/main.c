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
#include <Time.h>

#define DEF_FUNC_WSA /* Use WSA functionst */

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
    FILE *pLog;
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
    p->pLog = NULL;
    if(i_path == NULL) i_path = "/";
    int sz = snprintf(p->req.buf, p->req.len+1, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",
        i_path, i_host );
    p->req.len = sz;

    return CreateWindowEx( WS_EX_OVERLAPPEDWINDOW, TEXT(CLASSNAME_ALPHA7), NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, p );
}

LRESULT CALLBACK WndProcWmCreate( HWND hWnd, CREATESTRUCT *cs, NETWNDDATA *nwd ) {
    SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)nwd );
    nwd->hstn = (CHAR*)malloc( MAXGETHOSTSTRUCT );
    ASSERT_ALLOC( nwd->hstn );
    HANDLE h = WSAAsyncGetHostByName( hWnd, WM_WSAASYNCGETHOSTBYNAME, nwd->host, nwd->hstn, MAXGETHOSTSTRUCT );
    if( h == NULL ) {
        int err = WSAGetLastError();
        LOG_ERR_WSA( "WSAAsyncGetHostByName()", err );
        if( nwd->hstn ) free( nwd->hstn );
        return -1;
    }
    ++g_wndcount;
    return 0;
}

LRESULT CALLBACK WndProcWmDestroy( HWND hWnd, NETWNDDATA *nwd ) {
    if( nwd == NULL ) {
        LOG_ERR( "GetWindowLongPtr", hWnd );
    } else {
        if( nwd->pLog ) fclose( nwd->pLog );
        if( nwd->hstn ) free( nwd->hstn );
        free( nwd );
    }
    --g_wndcount;
    if(g_wndcount == 0)
        PostQuitMessage(0);
    return 0;
}

LRESULT CALLBACK WndProcWmWSAAsyncGethostByName( HWND hWnd, NETWNDDATA *nwd, HANDLE _h, int err, int bufLen ){
    if( nwd == NULL ) {
        LOG_ERR( "GetWindowLongPtr", hWnd );
        DestroyWindow( hWnd );
        return 0;
    }
    if( err ) {
        LOG_ERR_WSA( "WndProcWmWSAAsyncGethostByName()", err );
        DestroyWindow( hWnd );
        return 0;
    }

    HOSTENT *h = (HOSTENT*)nwd->hstn;
    if( h == NULL ) {
        LOG_ERR( "HOSTENT is null", hWnd );
    } else {
        LOG( "Host: buflen -> %d\n", bufLen );
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
        /// Creation soket
        SOCKET s = INVALID_SOCKET;
        #ifdef DEF_FUNC_WSA
        if ( ( s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 ) ) == INVALID_SOCKET ) {
        #else
        if ( ( s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
        #endif
            err = WSAGetLastError();
            LOG_ERR_WSA( "WSASocket()", err );
            DestroyWindow( hWnd );
            return 0;
        }
        {
            char str[128];
            snprintf( str, 127, "log_%i_%s_%i.log", (int)s, nwd->host, (int)nwd->port );
            nwd->pLog = fopen( str, "w" );
            LOG( "SOCKET(%d \"%s\") CREATED AND LOG TO >>> (%s)\n", (int)s, nwd->host, str );
            time_t rawtime;
            struct tm * timeinfo;
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            LOG_WSA_SOCKET( "TIME: %s\n", asctime(timeinfo) );
        }
        /// Set socket to async non-block
        if ( WSAAsyncSelect( s, hWnd, WM_WSAASYNCSELECT, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE ) == SOCKET_ERROR ) {
            err = WSAGetLastError();
            LOG_ERR_WSA( "WSAAsyncSelect()", err );
            LOG_WSA_SOCKET_ERR_WSA( "WSAAsyncSelect()", err );
            closesocket( s );
            DestroyWindow( hWnd );
            return 0;
        }
        /// Socket connection to host
        SOCKADDR_IN sa = {
            .sin_family = AF_INET,
            .sin_port = htons( nwd->port ),
            .sin_addr.s_addr = *((UINT32*)(h->h_addr_list[0]))
        };
        #ifdef DEF_FUNC_WSA
        if( WSAConnect( s, (LPSOCKADDR)(&sa), sizeof(sa), NULL, NULL, NULL, NULL ) == SOCKET_ERROR ) {
        #else
        if( connect( s, (LPSOCKADDR)(&sa), sizeof(sa) ) == SOCKET_ERROR ) {
        #endif
            err = WSAGetLastError();
            if ( err != WSAEWOULDBLOCK ) {
                LOG_ERR_WSA( "WSAConnect()", err );
                LOG_WSA_SOCKET_ERR_WSA( "WSAConnect()", err );
                closesocket( s );
                DestroyWindow( hWnd );
                return 0;
            } else {
                LOG_ERR_WSA_ASYNC( "WSAConnect()", err );
                LOG_WSA_SOCKET_ERR_WSA_ASYNC( "WSAConnect()", err );
            }
        }
    }
    return 0;
}

LRESULT CALLBACK WndProcWmWSAAsyncSelect( HWND hWnd, NETWNDDATA *nwd, SOCKET s, int err, int type ) {
    if( s == INVALID_SOCKET ) {
        LOG_ERR( "WndProcWmWSAAsyncSelect INVALID_SOCKET", hWnd );
        DestroyWindow( hWnd );
        return 0;
    }
    if( nwd == NULL ) {
        LOG_ERR( "GetWindowLongPtr", hWnd );
        closesocket( s );
        DestroyWindow( hWnd );
        return 0;
    }
    LOG( "SOCKET(%d \"%s\")", (int)s, nwd->host );
    LOG_WSA_SOCKET( ">>>" );
    if( type&FD_READ ) { LOG(" FD_READ"); LOG_WSA_SOCKET(" FD_READ"); }
    if( type&FD_WRITE ) { LOG(" FD_WRITE"); LOG_WSA_SOCKET(" FD_WRITE"); }
    if( type&FD_OOB ) { LOG(" FD_OOB"); LOG_WSA_SOCKET(" FD_OOB"); }
    if( type&FD_ACCEPT ) { LOG(" FD_ACCEPT"); LOG_WSA_SOCKET(" FD_ACCEPT"); }
    if( type&FD_CONNECT ) { LOG(" FD_CONNECT"); LOG_WSA_SOCKET(" FD_CONNECT"); }
    if( type&FD_CLOSE ) { LOG(" FD_CLOSE"); LOG_WSA_SOCKET(" FD_CLOSE"); }
    if( type&FD_QOS ) { LOG(" FD_QOS"); LOG_WSA_SOCKET(" FD_QOS"); }
    if( type&FD_GROUP_QOS ) { LOG(" FD_GROUP_QOS"); LOG_WSA_SOCKET(" FD_GROUP_QOS"); }
    if( type&FD_ROUTING_INTERFACE_CHANGE ) { LOG(" FD_ROUTING_INTERFACE_CHANGE"); LOG_WSA_SOCKET(" FD_ROUTING_INTERFACE_CHANGE"); }
    if( type&FD_ADDRESS_LIST_CHANGE ) { LOG(" FD_ADDRESS_LIST_CHANGE"); LOG_WSA_SOCKET(" FD_ADDRESS_LIST_CHANGE"); }
    LOG("\n");
    LOG_WSA_SOCKET(" >>>\n");
    if( err ) {
        LOG_ERR_WSA( "WndProcWmWSAAsyncSelect()", err );
        LOG_WSA_SOCKET_ERR_WSA( "WndProcWmWSAAsyncSelect()", err );
        closesocket( s );
        DestroyWindow( hWnd );
        return 0;
    }
    switch( type ) {
        case FD_CONNECT: {
            #ifdef DEF_FUNC_WSA
            if ( WSASend( s, &nwd->req, 1, &nwd->btSent, 0, NULL, NULL ) == SOCKET_ERROR ) {
            #else
            if ( ( nwd->btSent = send( s, nwd->req.buf, nwd->req.len, 0 ) ) == SOCKET_ERROR ) {
            #endif
                err = WSAGetLastError();
                if(err != WSAEWOULDBLOCK) {
                    LOG_ERR_WSA( "WSASend()", err );
                    LOG_WSA_SOCKET_ERR_WSA( "WSASend()", err );
                    closesocket(s);
                    DestroyWindow( hWnd );
                    return 0;
                } else {
                    LOG_ERR_WSA_ASYNC( "WSASend()", err );
                    LOG_WSA_SOCKET_ERR_WSA_ASYNC( "WSASend()", err );
                }
            }
            return 0;
        }
        case FD_READ: {
            nwd->flRecv = 0;
            #ifdef DEF_FUNC_WSA
            if ( WSARecv( s, &nwd->res, 1, &nwd->btRecv, &nwd->flRecv, NULL, NULL ) == SOCKET_ERROR ) {
            #else
            if ( ( nwd->flRecv = recv( s, nwd->res.buf, nwd->res.len, nwd->flRecv ) ) == SOCKET_ERROR ) {
            #endif
                err = WSAGetLastError();
                if(err != WSAEWOULDBLOCK) {
                    LOG_ERR_WSA( "WSARecv()", err );
                    LOG_WSA_SOCKET_ERR_WSA( "WSARecv()", err );
                    closesocket(s);
                    DestroyWindow( hWnd );
                    return 0;
                } else {
                    LOG_ERR_WSA_ASYNC( "WSARecv()", err );
                    LOG_WSA_SOCKET_ERR_WSA_ASYNC( "WSARecv()", err );
                }
            }
            nwd->res.buf[nwd->btRecv] = '\0';
            LOG( "SOCKET(%d \"%s\") RECV %d >>> \n%s\n ========= END =========\n",(int)s, nwd->host, (int)nwd->btRecv, nwd->res.buf );
            LOG_WSA_SOCKET( ">>> RECV %d >>> \n%s\n ========= END =========\n", (int)nwd->btRecv, nwd->res.buf );
            shutdown( s, SD_SEND );
            return 0;
        }
        case FD_CLOSE: {
            LOG( "SOCKET(%d \"%s\") CLOSED", (int)s, nwd->host );
            LOG_WSA_SOCKET( ">>> CLOSED >>>" );
            closesocket( s );
            DestroyWindow( hWnd );
            return 0;
        }
    }
    return 0;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch(uMsg) {
        case WM_CREATE: {
            CREATESTRUCT *c = (CREATESTRUCT*)lParam;
            return WndProcWmCreate( hWnd, c, (NETWNDDATA*)c->lpCreateParams );
        }
        case WM_DESTROY:
            return WndProcWmDestroy( hWnd, (NETWNDDATA*)GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
        case WM_WSAASYNCGETHOSTBYNAME:
            return WndProcWmWSAAsyncGethostByName( hWnd, (NETWNDDATA*)GetWindowLongPtr( hWnd, GWLP_USERDATA ), (HANDLE)wParam, WSAGETASYNCERROR( lParam ), WSAGETASYNCBUFLEN( lParam ) );
        case WM_WSAASYNCSELECT:
            return WndProcWmWSAAsyncSelect( hWnd, (NETWNDDATA*)GetWindowLongPtr( hWnd, GWLP_USERDATA ), (SOCKET)wParam, WSAGETSELECTERROR( lParam ), WSAGETSELECTEVENT( lParam ) );
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

