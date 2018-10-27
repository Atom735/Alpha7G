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
#define _A7NEW_WND_BUF_SIZE (64*1024)

static WSADATA g_WSAData = {0};
static int g_A7NetErr = 0;
static HINSTANCE g_A7NetInstance = NULL;
static SSL_CTX *g_A7NetCtx = NULL;

int g_A7NetConnectsCount = 0;

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
#define A7LOGNET_ERR( ... ) {\
        long c_clock = clock() - nwd->c_clock;\
        A7LOGFINFO( nwd->log, "clock % 8ld", c_clock );\
        A7LOGFERR( nwd->log, __VA_ARGS__ );\
    }

typedef struct _S7NETWNDCREATE {
    int port;
    char const *hostname;
    int tls;
} S7NETWNDCREATE;

enum {
    A7NWDS_NULL = 0, /* Когда нет сокета */
    A7NWDS_CREATED, /* Когда сокет создан и ждет подключения */
    A7NWDS_CONNECTED, /* Когда сокет подключен */
    A7NWDS_READY, /* Готов к передаче */

    A7NWDS_TLS_CREATED, /* TLS создан */
    A7NWDS_TLS_CONNECT, /* TLS пытаеться подключиться */
    A7NWDS_TLS_READY, /* TLS готов к передаче */
};

typedef struct _S7NETWNDDATA {
    int port;
    char *hostname;
    char *buf;
    int bufS, bufR;
    FILE *log;
    long c_clock;
    int state;
    SSL *ssl;
    BIO *app, *net;

} S7NETWNDDATA;

static void A7NetWndDataFree( S7NETWNDDATA *nwd ) {
    assert ( nwd != NULL );
    assert ( nwd->hostname != NULL );
    free ( nwd->hostname );
    if ( nwd->buf != NULL ) free ( nwd->buf );
    if ( nwd->ssl ) SSL_free ( nwd->ssl );
    if ( nwd->net ) BIO_free ( nwd->net );
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
    nwd->buf = ( char* ) malloc ( ( nwc->tls != 0 ) ? _A7NEW_WND_BUF_SIZE : MAXGETHOSTSTRUCT );
    assert ( nwd->buf != NULL );

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

    nwd->state = A7NWDS_NULL;
    nwd->app = NULL;
    nwd->net = NULL;
    nwd->ssl = ( nwc->tls != 0 ) ? ( void* )( 1 ) : NULL;

    nwd->bufS = 0;
    nwd->bufR = 0;

    ++g_A7NetConnectsCount;

    if ( WSAAsyncGetHostByName ( hWnd, WM_WSA_GETHOSTBYNAME, nwd->hostname, nwd->buf, MAXGETHOSTSTRUCT ) == NULL ) {
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

    if( err != 0 ) {
        A7LOGERR_WSA ( "WSAAsyncGetHostByName IN EVENT", err );
        A7LOGNET_ERR_WSA ( "WSAAsyncGetHostByName IN EVENT", err );
        DestroyWindow( hWnd );
        return 0;
    }

    HOSTENT *host = ( HOSTENT* ) nwd->buf;
    if( host == NULL ) {
        A7LOGERRCS ( "hostent is null", 0, "NULL" );
        A7LOGNET_ERRCS ( "hostent is null", 0, "NULL" );
        DestroyWindow ( hWnd );
    } else {
        char info[512];
        int sz=0;
        sz += snprintf ( info+sz, 511-sz, "      name   -> %s\n", host->h_name );
        sz += snprintf ( info+sz, 511-sz, "      buflen -> %d\n", buflen );
        for ( char **ch = host->h_aliases; *ch != NULL; ++ch )
            sz += snprintf ( info+sz, 511-sz, "      alias  -> %s\n", *ch );
        sz += snprintf ( info+sz, 511-sz, "      length -> %d\n", (int)host->h_length );
        for ( char **ch = host->h_addr_list; *ch != NULL; ++ch ) {
            sz += snprintf ( info+sz, 511-sz, "      addr   -> %d", (int)(u_char)((*ch)[0]) );
            for ( int i=1; i<(int)host->h_length; ++i )
                sz += snprintf ( info+sz, 511-sz, ".%d", (int)(u_char)((*ch)[i]) );
            sz += snprintf ( info+sz, 511-sz, "\n" );
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


        nwd->state = A7NWDS_CREATED;
        A7LOGINFO ( "socket created SOCKET(%p)", (void*)s );
        A7LOGNET_INFO ( "socket created SOCKET(%p)", (void*)s );

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
static LRESULT A7NetWndPSelectConnect ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    assert ( nwd->state == A7NWDS_CREATED );
    nwd->state = A7NWDS_CONNECTED;
    A7LOGINFO ( "socket connected SOCKET(%p)", (void*)s );
    A7LOGNET_INFO ( "socket connected SOCKET(%p)", (void*)s );
    if ( nwd->ssl != NULL ) {
        nwd->ssl = SSL_new ( g_A7NetCtx );
        if ( nwd->ssl == NULL ) {
            A7LOGERRCS ( "SSL_new", 0, "NULL" );
            A7LOGNET_ERRCS ( "SSL_new", 0, "NULL" );
            A7NetErrSslStack();
            shutdown ( s, SD_SEND );
            return 0;
        }
        int err = BIO_new_bio_pair ( &nwd->app, 0, &nwd->net, 0);
        if ( err != 1 ) {
            A7LOGERRCS ( "BIO_new_bio_pair", err, "NULL" );
            A7LOGNET_ERRCS ( "BIO_new_bio_pair", err, "NULL" );
            A7NetErrSslStack();
            shutdown ( s, SD_SEND );
            return 0;
        }
        SSL_set_bio ( nwd->ssl, nwd->app, nwd->app );

        nwd->state = A7NWDS_TLS_CREATED;
        A7LOGINFO ( "TLS created SOCKET(%p) SSL(%p)", (void*)s, nwd->ssl );
        A7LOGNET_INFO ( "TLS created SOCKET(%p) SSL(%p)", (void*)s, nwd->ssl );
    } else {
        assert ( 0!=/*TODO*/0+0 );
    }

    return 0;
}
static LRESULT A7NetWndPSelectClose ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    nwd->state = A7NWDS_NULL;
    A7LOGINFO ( "socket closed SOCKET(%p)", (void*)s );
    A7LOGNET_INFO ( "socket closed SOCKET(%p)", (void*)s );
    closesocket ( s );
    DestroyWindow ( hWnd );
    return 0;
}

static void A7NetWndPSelect_ssl2net ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    char d[4096];
    int l;
    while ( ( l = BIO_read( nwd->net, d, 4096 ) ) > 0 ) {
        int k = 0;
        int r = l;
        while ( ( k < r ) && ( ( l = send ( s, d + k, r - k, 0 ) ) != SOCKET_ERROR  ) ) k += l;

        A7LOGINFO ( "ssl2net %d >>> %d SOCKET(%p) SSL(%p)", r, k, (void*)s, nwd->ssl );
        A7LOGNET_INFO ( "ssl2net %d >>> %d SOCKET(%p) SSL(%p)", r, k, (void*)s, nwd->ssl );

        if ( l == SOCKET_ERROR ) {
            int err = WSAGetLastError();
            A7LOGERR_WSA ( "send", err );
            A7LOGNET_ERR_WSA ( "send", err );
        }
    }
}
static void A7NetWndPSelect_net2ssl ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    char d[4096];
    int l;
    while ( ( l = recv ( s, d, 4096, 0 ) ) != SOCKET_ERROR ) {
        int k = 0;
        int r = l;
        while ( ( k < r ) && ( ( l = BIO_write ( nwd->net, d + k, r - k ) ) > 0 ) ) k += l;

        A7LOGINFO ( "net2ssl %d >>> %d SOCKET(%p) SSL(%p)", r, k, (void*)s, nwd->ssl );
        A7LOGNET_INFO ( "net2ssl %d >>> %d SOCKET(%p) SSL(%p)", r, k, (void*)s, nwd->ssl );
    }
    if ( l == SOCKET_ERROR ) {
        int err = WSAGetLastError();
        if ( err == WSAEWOULDBLOCK ) {
            A7LOGWARN_WSA ( "recv", err );
            A7LOGNET_WARN_WSA ( "recv", err );
        } else {
            A7LOGERR_WSA ( "recv", err );
            A7LOGNET_ERR_WSA ( "recv", err );
        }
    }
}

static void A7NetWndPSelect_trySslConnect ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    int err = SSL_connect ( nwd->ssl );
    if ( err == 1 ) {
        nwd->state = A7NWDS_TLS_CONNECT;
        A7LOGINFO ( "tls connected SOCKET(%p) SSL(%p)", (void*)s, nwd->ssl );
        A7LOGNET_INFO ( "tls connected SOCKET(%p) SSL(%p)", (void*)s, nwd->ssl );
        return;
    }
    if ( err == 0 )  {
        shutdown ( s, SD_SEND );
    }
    err = SSL_get_error ( nwd->ssl, err );
    A7LOGERRCS ( "SSL_connect", err, A7Err_SSL_get_error ( err ) );
    A7LOGNET_ERRCS ( "SSL_connect", err, A7Err_SSL_get_error ( err ) );
    A7NetErrSslStack();
    A7NetWndPSelect_ssl2net ( hWnd, nwd, s );
}

static LRESULT A7NetWndPSelectRead ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    if ( nwd->state == A7NWDS_TLS_CREATED ) {
        A7NetWndPSelect_net2ssl ( hWnd, nwd, s );
        A7NetWndPSelect_trySslConnect ( hWnd, nwd, s );
    }
    if ( nwd->state == A7NWDS_TLS_CONNECT ) {
        shutdown ( s, SD_SEND );
        // char str[] = "GEt"
        // SSL_write ( nwd->ssl,  );
    }
    return 0;
}
static LRESULT A7NetWndPSelectWrite ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s ) {
    if ( nwd->state == A7NWDS_TLS_CREATED )
        A7NetWndPSelect_trySslConnect ( hWnd, nwd, s );
    return 0;
}

static LRESULT A7NetWndPSelect ( HWND hWnd, S7NETWNDDATA *nwd, SOCKET s, int err, int event ) {
    assert ( nwd != NULL );

    if( err != 0 ) {
        A7LOGERR_WSA ( "WSAAsyncSelect IN EVENT", err );
        A7LOGNET_ERR_WSA ( "WSAAsyncSelect IN EVENT", err );
        closesocket( s );
        DestroyWindow( hWnd );
        return 0;
    }

    if( s == INVALID_SOCKET ) {
        A7LOGERR_WSA ( "INVALID_SOCKET WSAAsyncSelect IN EVENT", 0 );
        A7LOGNET_ERR_WSA ( "INVALID_SOCKET WSAAsyncSelect IN EVENT", 0 );
        DestroyWindow( hWnd );
        return 0;
    }

    switch ( event ) {
        #ifdef FD_READ
        case FD_READ:
            A7LOGINFO ( "SOCKET(%p) FD_READ", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_READ", (void*)s );
            return A7NetWndPSelectRead ( hWnd, nwd, s );
        #endif
        #ifdef FD_WRITE
        case FD_WRITE:
            A7LOGINFO ( "SOCKET(%p) FD_WRITE", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_WRITE", (void*)s );
            return A7NetWndPSelectWrite ( hWnd, nwd, s );
        #endif
        #ifdef FD_OOB
        case FD_OOB:
            A7LOGINFO ( "SOCKET(%p) FD_OOB", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_OOB", (void*)s );
            break;
        #endif
        #ifdef FD_ACCEPT
        case FD_ACCEPT:
            A7LOGINFO ( "SOCKET(%p) FD_ACCEPT", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_ACCEPT", (void*)s );
            break;
        #endif
        #ifdef FD_CONNECT
        case FD_CONNECT:
            A7LOGINFO ( "SOCKET(%p) FD_CONNECT", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_CONNECT", (void*)s );
            return A7NetWndPSelectConnect ( hWnd, nwd, s );
        #endif
        #ifdef FD_CLOSE
        case FD_CLOSE:
            A7LOGINFO ( "SOCKET(%p) FD_CLOSE", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_CLOSE", (void*)s );
            return A7NetWndPSelectClose ( hWnd, nwd, s );
        #endif
        #ifdef FD_QOS
        case FD_QOS:
            A7LOGINFO ( "SOCKET(%p) FD_QOS", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_QOS", (void*)s );
            break;
        #endif
        #ifdef FD_GROUP_QOS
        case FD_GROUP_QOS:
            A7LOGINFO ( "SOCKET(%p) FD_GROUP_QOS", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_GROUP_QOS", (void*)s );
            break;
        #endif
        #ifdef FD_ROUTING_INTERFACE_CHANGE
        case FD_ROUTING_INTERFACE_CHANGE:
            A7LOGINFO ( "SOCKET(%p) FD_ROUTING_INTERFACE_CHANGE", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_ROUTING_INTERFACE_CHANGE", (void*)s );
            break;
        #endif
        #ifdef FD_ADDRESS_LIST_CHANGE
        case FD_ADDRESS_LIST_CHANGE:
            A7LOGINFO ( "SOCKET(%p) FD_ADDRESS_LIST_CHANGE", (void*)s );
            A7LOGNET_INFO ( "SOCKET(%p) FD_ADDRESS_LIST_CHANGE", (void*)s );
            break;
        #endif
    }
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
