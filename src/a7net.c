#include <WinSock2.h>
#include <WS2TCPIP.h>
#include <Windows.h>

#include <OpenSSL/SSL.h>
#include <OpenSSL/ERR.h>

#include <Assert.h>
#include <StdLib.h>
#include <String.h>
#include <Time.h>

#include "a7err.h"
#include "a7net.h"

#define _A7NET_WND_CLASS_NAME "CLN-Alpha7G-IP/NET"

static WSADATA g_WSAData = {0};
static int g_A7NetErr = 0;
static HINSTANCE g_A7NetInstance = NULL;
static SSL_CTX *g_A7NetCtx = NULL;

int g_A7NetConnectsCount = 0;

typedef struct _S7NETWNDCREATE {
    int port;
    char const *hostname;
    int tls;
} S7NETWNDCREATE;

#define A7LOGNET_ERR_WSA( _fn, _err ) {\
        long c_clock = clock() - nwd->c_clock;\
        A7LOGFINFO( nwd->log, "clock % 8ld", c_clock );\
        A7LOGFERR_WSA( nwd->log, _fn, _err );\
    }

#define A7LOGNET_WARN_WSA( _fn, _err ) {\
        long c_clock = clock() - nwd->c_clock;\
        A7LOGFINFO( nwd->log, "clock % 8ld", c_clock );\
        A7LOGFWARN_WSA( nwd->log, _fn, _err );\
    }

#define A7LOGNET_INFO( ... ) {\
        long c_clock = clock() - nwd->c_clock;\
        A7LOGFINFO( nwd->log, "clock % 8ld", c_clock );\
        A7LOGFINFO( nwd->log, __VA_ARGS__ );\
    }
#define A7LOGNET_ERRCS( _fn, _err, _errs ) {\
        long c_clock = clock() - nwd->c_clock;\
        A7LOGFINFO( nwd->log, "clock % 8ld", c_clock );\
        A7LOGFERRCS( nwd->log, _fn, _err, _errs );\
    }



typedef struct _S7NETWNDDATA {
    int port;
    char *hostname;
    char *_host;
    FILE *log;
    long c_clock;
    SSL *ssl;
    BIO *app, *net;

} S7NETWNDDATA;

static void A7NetWndDataFree( S7NETWNDDATA *nwd ) {
    assert ( nwd != NULL );
    assert ( nwd->hostname != NULL );
    free ( nwd->hostname );
    if ( nwd->_host != NULL ) free ( nwd->_host );
    if ( nwd->log != NULL ) {
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        fprintf ( nwd->log, " ====== %s", asctime ( timeinfo ) );
        fclose ( nwd->log );
    }
    free ( nwd );
}


HWND A7NetConnect ( const char *hostname, int port, int tls ) {
    S7NETWNDCREATE _nwc = {
        .port = port,
        .hostname = hostname,
        .tls = tls,
    };
    return CreateWindowEx ( WS_EX_OVERLAPPEDWINDOW, TEXT ( _A7NET_WND_CLASS_NAME ), NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_A7NetInstance, &_nwc );
}

static void A7NetErrSslStack ( void ) {
    int err;
    while ( ( err = ERR_get_error() ) ) {
        char *str = ERR_error_string ( err, 0);
        if (!str) return;
        A7LOGERRCS ( "A7NetErrSslStack", err, str );
    }
}

static LRESULT A7NetWndPCreate ( HWND hWnd, S7NETWNDCREATE *nwc ) {
    assert ( nwc != NULL );
    assert ( nwc->hostname != NULL );
    S7NETWNDDATA *nwd = ( S7NETWNDDATA* ) malloc ( sizeof ( S7NETWNDDATA ) );
    SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)nwd );
    assert ( nwd != NULL );
    nwd->hostname = ( char* ) malloc ( strlen ( nwc->hostname ) + 1 );
    assert ( nwd->hostname != NULL );
    nwd->port = nwc->port;
    strcpy ( nwd->hostname, nwc->hostname );
    nwd->_host = ( char* ) malloc ( MAXGETHOSTSTRUCT );
    assert ( nwd->_host != NULL );

    {
        char str[128];
        int i=0;
        nwd->log = NULL;
        while ( 1 ) {
            snprintf( str, 127, "log %d %s %05d.log", nwd->port, nwd->hostname, i );
            ++i;
            nwd->log = fopen ( str, "r" );
            if ( nwd->log != NULL ) fclose( nwd->log );
            else break;
        }
        nwd->log = fopen ( str, "w" );
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        fprintf ( nwd->log, " ====== %s", asctime ( timeinfo ) );
        nwd->c_clock = clock();
    }

    ++g_A7NetConnectsCount;

    if ( WSAAsyncGetHostByName ( hWnd, WM_WSA_GETHOSTBYNAME, nwd->hostname, nwd->_host, MAXGETHOSTSTRUCT ) == NULL ) {
        int err = WSAGetLastError();
        A7LOGERR_WSA ( "WSAAsyncGetHostByName", err );
        A7LOGNET_ERR_WSA ( "WSAAsyncGetHostByName", err );
        return (-1);
    }

    A7LOGNET_INFO ( "created window WND(%p)", hWnd );
    return (0);
}

static LRESULT A7NetWndPDestroy ( HWND hWnd, S7NETWNDDATA *nwd ) {
    assert ( nwd != NULL );

    A7LOGNET_INFO ( "destroyed window WND(%p)", hWnd );
    A7NetWndDataFree ( nwd );
    --g_A7NetConnectsCount;
    return 0;
}

static LRESULT A7NetWndPGetHostByName ( HWND hWnd, S7NETWNDDATA *nwd, HANDLE _h, int err, int buflen ) {
    assert ( nwd != NULL );

    if( err ) {
        A7LOGERR_WSA ( "WSAAsyncGetHostByName IN EVENT", err );
        A7LOGNET_ERR_WSA ( "WSAAsyncGetHostByName IN EVENT", err );
        DestroyWindow( hWnd );
        return 0;
    }

    HOSTENT *host = ( HOSTENT* ) nwd->_host;
    if( host == NULL ) {
        A7LOGERRCS ( "hostent is null", 0, "NULL" );
        A7LOGNET_ERRCS ( "hostent is null", 0, "NULL" );
        DestroyWindow ( hWnd );
    } else {
        char info[512];
        int sz=0;
        sz += snprintf ( info, 511-sz, "      name   -> %s\n", host->h_name );
        sz += snprintf ( info, 511-sz, "      buflen -> %d\n", buflen );
        for ( char **ch = host->h_aliases; *ch != NULL; ++ch )
            sz += snprintf ( info, 511-sz, "      alias  -> %s\n", *ch );
        sz += snprintf ( info, 511-sz, "      length -> %d\n", (int)host->h_length );
        for ( char **ch = host->h_addr_list; *ch != NULL; ++ch ) {
            sz += snprintf ( info, 511-sz, "      addr   -> %d", (int)(u_char)((*ch)[0]) );
            for ( int i=1; i<(int)host->h_length; ++i )
                sz += snprintf ( info, 511-sz, ".%d", (int)(u_char)((*ch)[i]) );
            sz += snprintf ( info, 511-sz, "\n" );
        }
        A7LOGNET_INFO ( "%d\n%s", sz, info );
        /// Creation soket
        SOCKET s = INVALID_SOCKET;
        #ifdef DEF_FUNC_WSA
        if ( ( s = WSASocket ( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 ) ) == INVALID_SOCKET )
        #else
        if ( ( s = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET )
        #endif
        {
            err = WSAGetLastError();
            A7LOGERR_WSA ( "WSASocket", err );
            A7LOGNET_ERR_WSA ( "WSASocket", err );
            DestroyWindow ( hWnd );
            return 0;
        }
        /// Set socket to async non-block
        if ( WSAAsyncSelect ( s, hWnd, WM_WSA_SELECT, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE ) == SOCKET_ERROR ) {
            err = WSAGetLastError();
            A7LOGERR_WSA ( "WSAAsyncSelect", err );
            A7LOGNET_ERR_WSA ( "WSAAsyncSelect", err );
            closesocket ( s );
            DestroyWindow ( hWnd );
            return 0;
        }
        /// Socket connection to host
        SOCKADDR_IN sa = {
            .sin_family = AF_INET,
            .sin_port = htons ( nwd->port ),
            .sin_addr.s_addr = *( ( UINT32* ) ( host->h_addr ) )
        };
        #ifdef DEF_FUNC_WSA
        if( WSAConnect ( s, (LPSOCKADDR)(&sa), sizeof(sa), NULL, NULL, NULL, NULL ) == SOCKET_ERROR )
        #else
        if( connect ( s, (LPSOCKADDR)(&sa), sizeof(sa) ) == SOCKET_ERROR )
        #endif
        {
            err = WSAGetLastError();
            if ( err != WSAEWOULDBLOCK ) {
                A7LOGERR_WSA ( "WSAConnect", err );
                A7LOGNET_ERR_WSA ( "WSAConnect", err );
                closesocket( s );
                DestroyWindow( hWnd );
                return 0;
            } else {
                A7LOGWARN_WSA ( "NonBlock WSAConnect", err );
                A7LOGNET_WARN_WSA ( "NonBlock WSAConnect", err );
            }
        }
    }
    return 0;
}

static LRESULT A7NetWndPSelect ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s, int err, int event ) {

    closesocket( s );
    DestroyWindow( hWnd );
    return 0;
}

static LRESULT CALLBACK A7NetWndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch ( uMsg ) {
        case WM_CREATE:
            assert( ( ( CREATESTRUCT* ) lParam ) != NULL );
            assert( ( ( CREATESTRUCT* ) lParam ) ->lpCreateParams != NULL );
            return A7NetWndPCreate( hWnd, ( S7NETWNDCREATE* )( ( ( CREATESTRUCT* ) lParam )->lpCreateParams ) );
        case WM_DESTROY: return A7NetWndPDestroy( hWnd, ( S7NETWNDDATA* ) GetWindowLongPtr ( hWnd, GWLP_USERDATA ) );
        case WM_WSA_GETHOSTBYNAME: return A7NetWndPGetHostByName( hWnd, ( S7NETWNDDATA* ) GetWindowLongPtr ( hWnd, GWLP_USERDATA ), ( HANDLE ) wParam, WSAWM_ERROR ( lParam ), WSAWM_BUFLEN ( lParam ) );
        case WM_WSA_SELECT: return A7NetWndPSelect( hWnd, ( S7NETWNDDATA* ) GetWindowLongPtr ( hWnd, GWLP_USERDATA ), ( SOCKET ) wParam, WSAWM_ERROR ( lParam ), WSAWM_EVENT ( lParam ) );
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

int A7NetInit ( HINSTANCE hInstance ) {
    g_A7NetInstance = hInstance;

    WNDCLASSEXW wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = A7NetWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = g_A7NetInstance;
    wcex.hIcon          = LoadIcon ( NULL, IDI_APPLICATION );
    wcex.hCursor        = LoadCursor ( NULL, IDC_ARROW );
    wcex.hbrBackground  = (HBRUSH) GetStockObject ( WHITE_BRUSH );
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = TEXT ( _A7NET_WND_CLASS_NAME );
    wcex.hIconSm        = NULL;
    if ( RegisterClassEx ( &wcex ) == 0 ) {
        g_A7NetErr = GetLastError();
        A7LOGERRC ( "RegisterClassEx", g_A7NetErr );
        g_A7NetErr = -1;
        return (-1);
    }
    ;
    if ( ( g_A7NetErr = WSAStartup ( WINSOCK_VERSION, &g_WSAData ) ) != 0 ) {
        A7LOGERR_WSA ( "WSAStartup", g_A7NetErr );
        g_A7NetErr = 1;
        return (-1);
    }

    SSL_load_error_strings();
    if ( ( g_A7NetErr = SSL_library_init() ) != 1 ) {
        A7LOGWARNC ( "SSL_library_init", g_A7NetErr );
        A7NetErrSslStack();
    }

    SSL_METHOD const * method = TLSv1_2_client_method();
    if ( method == NULL ) {
        A7LOGWARNCS ( "TLSv1_2_client_method", 0, "NULL" );
        method = TLSv1_1_client_method();
        if ( method == NULL ) {
            A7LOGWARNCS ( "TLSv1_1_client_method", 0, "NULL" );
            method = TLSv1_client_method();
            if ( method == NULL ) {
                A7LOGWARNCS ( "TLSv1_client_method", 0, "NULL" );
                method = SSLv23_client_method();
                if ( method == NULL ) {
                    A7LOGWARNCS ( "SSLv23_client_method", 0, "NULL" );
                }
            }
        }
    }
    if ( method == NULL ) {
        A7LOGERRCS ( "can't get SSL_METHOD", 0, "NULL" );
        g_A7NetErr = 1;
        return (-1);
    } else {
        g_A7NetCtx = SSL_CTX_new ( method );
        if ( g_A7NetCtx == NULL ) {
            A7LOGERRCS ( "SSL_CTX_new", 0, "NULL" );
            A7NetErrSslStack();
            g_A7NetErr = 1;
            return (-1);
        }
    }
    g_A7NetErr = 0;
    return (0);
}

void A7NetRelease ( void ) {
    int err = g_A7NetErr;
    if( g_A7NetCtx != NULL ) {
        SSL_CTX_free( g_A7NetCtx );
        A7NetErrSslStack();
    }
    switch ( err ) {

        case 1:
            if ( WSACleanup() == SOCKET_ERROR ) {
                g_A7NetErr = WSAGetLastError();
                A7LOGERR_WSA ( "WSACleanup", g_A7NetErr );
            }
        case -1:
        break;
    }
}
