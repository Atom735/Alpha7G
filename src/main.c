#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <WS2TCPIP.h>
// #include <IPHlpAPI.h>
#include <Windows.h>

#include <OpenSSL/BIO.h>
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

#include "ssl_ctx_meth.c"

/*
s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
connect(s, (struct sockaddr *)&sa, socklen);
ssl = SSL_new (ctx); // alloc for SSL

SSL_set_fd(ssl, s); =>
{
    bio = BIO_new(BIO_s_socket());
    BIO_set_fd(bio, s, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);
}

int err = SSL_connect(ssl); =>
{
    (ssl->method->internal->ssl_connect(ssl)); == (ctx->method->internal->ssl_connect(ssl)); == (ctx_method->internal->ssl_connect(ssl)); == (ssl3_connect(ssl));

}
SSL_write(ssl, buf, strlen(buf)); => (ssl->method->internal->ssl_write_bytes(s,
        SSL3_RT_APPLICATION_DATA, buf, len));
SSL_read(ssl, buf, 100); => ret = (ssl->method->internal->ssl_read_bytes(s,
        SSL3_RT_APPLICATION_DATA, buf, len, peek));


*/

FILE       *g_logfile = NULL;
#define CLASSNAME_ALPHA7 "CLN-Alpha7G-IP/NET"

HINSTANCE   g_hInstance = NULL;
WSADATA     g_WSAData = {0};
SSL_CTX    *g_ssl_ctx = NULL;

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
    SSL *ssl;
    BIO *web, *out;
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
    int sz = snprintf(p->req.buf, p->req.len, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",
        i_path, i_host );
    p->req.len = sz;
    p->ssl = NULL;
    p->web = NULL;
    p->out = NULL;

    return CreateWindowEx( WS_EX_OVERLAPPEDWINDOW, TEXT(CLASSNAME_ALPHA7), NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, p );
}


static int my_null_write(BIO *h, const char *buf, int num);
static int my_null_read(BIO *h, char *buf, int size);
static int my_null_puts(BIO *h, const char *str);
static int my_null_gets(BIO *h, char *str, int size);
static long my_null_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int my_null_new(BIO *h);
static int my_null_free(BIO *data);

static const BIO_METHOD my_null_method = {
    .type = BIO_TYPE_NULL,
    .name = "NULL",
    .bwrite = my_null_write,
    .bread = my_null_read,
    .bputs = my_null_puts,
    .bgets = my_null_gets,
    .ctrl = my_null_ctrl,
    .create = my_null_new,
    .destroy = my_null_free
};

const BIO_METHOD * my_BIO_s_null(void)
{
    return (&my_null_method);
}

static int my_null_new(BIO *bi)
{
    LOG( "my_null_new(%p)\n", bi );
    bi->init = 1;
    bi->num = 0;
    bi->ptr = (NULL);
    return (1);
}

static int my_null_free(BIO *a)
{
    LOG( "my_null_free(%p)\n", a );
    if (a == NULL)
        return (0);
    return (1);
}

static int my_null_read(BIO *b, char *out, int outl)
{
    LOG( "my_null_read(%p, %p, %i)\n", b, out, outl );
    return (0);
}

static int my_null_write(BIO *b, const char *in, int inl)
{
    LOG( "my_null_write(%p, %p, %i)\n", b, in, inl );
    return (inl);
}

static long my_null_ctrl(BIO *b, int cmd, long num, void *ptr)
{
    LOG( "my_null_ctrl(%p, %i, %li, %p)\n", b, cmd, num, ptr );
    long ret = 1;

    switch (cmd) {
    case BIO_CTRL_RESET:
    case BIO_CTRL_EOF:
    case BIO_CTRL_SET:
    case BIO_CTRL_SET_CLOSE:
    case BIO_CTRL_FLUSH:
    case BIO_CTRL_DUP:
        ret = 1;
        break;
    case BIO_CTRL_GET_CLOSE:
    case BIO_CTRL_INFO:
    case BIO_CTRL_GET:
    case BIO_CTRL_PENDING:
    case BIO_CTRL_WPENDING:
    default:
        ret = 0;
        break;
    }
    return (ret);
}

static int my_null_gets(BIO *bp, char *buf, int size)
{
    LOG( "my_null_gets(%p, %p, %i)\n", bp, buf, size );
    return (0);
}

static int my_null_puts(BIO *bp, const char *str)
{
    LOG( "my_null_puts(%p, %p)\n", bp, str );
    if (str == NULL)
        return (0);
    return (strlen(str));
}

HWND a7CreateAsyncHttpsConnection( LPCSTR i_host, LPCSTR i_path ) {
    NETWNDDATA *p = (NETWNDDATA*)malloc(sizeof(NETWNDDATA));
    ASSERT_ALLOC(p);
    strcpy( p->host, i_host );
    p->port = 443;
    p->req.len = 2047;
    p->req.buf = malloc(p->req.len+1);
    p->res.len = 2047;
    p->res.buf = malloc(p->res.len+1);
    p->pLog = NULL;
    if(i_path == NULL) i_path = "/";
    int sz = snprintf(p->req.buf, p->req.len, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
        i_path, i_host );
    p->req.len = sz;
    p->ssl = NULL;
    p->web = NULL;
    p->out = NULL;


    if( g_ssl_ctx != NULL ) {

        LOG( "BIO_METHOD(%p)\n", my_BIO_s_null() );


        BIO *bio = BIO_new( my_BIO_s_null() );
        LOG( "BIO(%p)\n", bio );
        SSL *ssl = SSL_new( g_ssl_ctx );
        LOG( "SSL(%p)\n", ssl );

        LOG( "SSL_set_bio(%p,%p,%p)\n", ssl, bio, bio );
        SSL_set_bio( ssl, bio, bio );

        LOG( "SSL_connect(%p)\n", ssl );
        SSL_connect( ssl );

        LOG( "SSL_do_handshake(%p)\n", ssl );
        SSL_do_handshake( ssl );

        LOG( "SSL_free(%p)\n", ssl );
        SSL_free( ssl );
        LOG( "SSL_END()\n" );


    } else {
        LOG( "WARNING SSL CONTEXT NOT CREATED\n" );
    }

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

LRESULT WndProcHttpConnect( HWND hWnd, NETWNDDATA *nwd, SOCKET s ) {
    #ifdef DEF_FUNC_WSA
    if ( WSASend( s, &nwd->req, 1, &nwd->btSent, 0, NULL, NULL ) == SOCKET_ERROR ) {
    #else
    if ( ( nwd->btSent = send( s, nwd->req.buf, nwd->req.len, 0 ) ) == SOCKET_ERROR ) {
    #endif
        int err = WSAGetLastError();
        if( err != WSAEWOULDBLOCK ) {
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

LRESULT WndProcHttpRead( HWND hWnd, NETWNDDATA *nwd, SOCKET s ) {
    nwd->flRecv = 0;
    #ifdef DEF_FUNC_WSA
    if ( WSARecv( s, &nwd->res, 1, &nwd->btRecv, &nwd->flRecv, NULL, NULL ) == SOCKET_ERROR ) {
    #else
    if ( ( nwd->flRecv = recv( s, nwd->res.buf, nwd->res.len, nwd->flRecv ) ) == SOCKET_ERROR ) {
    #endif
        int err = WSAGetLastError();
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


LRESULT WndProcHttpClose( HWND hWnd, NETWNDDATA *nwd, SOCKET s ) {
    LOG( "SOCKET(%d \"%s\") CLOSED", (int)s, nwd->host );
    LOG_WSA_SOCKET( ">>> CLOSED >>>" );
    closesocket( s );
    DestroyWindow( hWnd );
    return 0;
}

LRESULT WndProcHttpsConnect( HWND hWnd, NETWNDDATA *nwd, SOCKET s ) {
    int sock = SSL_get_fd( nwd->ssl );
    int err;
    if( SSL_set_fd( nwd->ssl, s ) != 1 ) {
        LOG( "SOCKET(%d \"%s\") OpenSSL >>>", (int)s, nwd->host );
        LOG_ERR( "SSL_set_fd()", 0 );
        LOG_WSA_SOCKET_ERR( "SSL_set_fd()", 0 );
        while ( ( err = ERR_get_error() ) ) {
            char *str = ERR_error_string( err, 0 );
            if ( str == NULL ) break;
            LOG( "OPENSSL >>> %s\n", str );
            LOG_WSA_SOCKET( "OPENSSL >>> %s\n", str );
        }
    }
    err = SSL_connect( nwd->ssl );
    printf ("SSL connection using %s\n", SSL_get_cipher ( nwd->ssl ));

    err = SSL_write( nwd->ssl, nwd->req.buf, nwd->req.len );
    if ( err < 0 ) {
        err = SSL_get_error( nwd->ssl, err );
        shutdown( s, SD_SEND );
        switch (err) {
            case SSL_ERROR_WANT_WRITE:
                LOG_WSA_SOCKET( "OPENSSL ERR WRITE >>> SSL_ERROR_WANT_WRITE\n" );
                return 0;
            case SSL_ERROR_WANT_READ:
                LOG_WSA_SOCKET( "OPENSSL ERR WRITE >>> SSL_ERROR_WANT_READ\n" );
                return 0;
            case SSL_ERROR_ZERO_RETURN:
                LOG_WSA_SOCKET( "OPENSSL ERR WRITE >>> SSL_ERROR_ZERO_RETURN\n" );
                return -1;
            case SSL_ERROR_SYSCALL:
                LOG_WSA_SOCKET( "OPENSSL ERR WRITE >>> SSL_ERROR_SYSCALL\n" );
                return -1;
            case SSL_ERROR_SSL:
                LOG_WSA_SOCKET( "OPENSSL ERR WRITE >>> SSL_ERROR_SSL\n" );
                return -1;
            default:
                LOG_WSA_SOCKET( "OPENSSL ERR WRITE >>> UNCKNOWN\n" );
                return -1;
        }
    }
    return 0;
}

LRESULT WndProcHttpsRead( HWND hWnd, NETWNDDATA *nwd, SOCKET s ) {
    nwd->flRecv = 0;
    int err = SSL_read(nwd->ssl, nwd->res.buf, nwd->res.len);
    if( err > 0 ) nwd->btRecv = err;
    else {
        err = SSL_get_error( nwd->ssl, err );
        shutdown( s, SD_SEND );
        switch (err) {
            case SSL_ERROR_WANT_WRITE:
                LOG_WSA_SOCKET( "OPENSSL ERR READ >>> SSL_ERROR_WANT_WRITE\n" );
                return 0;
            case SSL_ERROR_WANT_READ:
                LOG_WSA_SOCKET( "OPENSSL ERR READ >>> SSL_ERROR_WANT_READ\n" );
                return 0;
            case SSL_ERROR_ZERO_RETURN:
                LOG_WSA_SOCKET( "OPENSSL ERR READ >>> SSL_ERROR_ZERO_RETURN\n" );
                return -1;
            case SSL_ERROR_SYSCALL:
                LOG_WSA_SOCKET( "OPENSSL ERR READ >>> SSL_ERROR_SYSCALL\n" );
                return -1;
            case SSL_ERROR_SSL:
                LOG_WSA_SOCKET( "OPENSSL ERR READ >>> SSL_ERROR_SSL\n" );
                return -1;
            default:
                LOG_WSA_SOCKET( "OPENSSL ERR READ >>> UNCKNOWN\n" );
                return -1;
        }
    }

    nwd->res.buf[nwd->btRecv] = '\0';
    LOG( "SOCKET(%d \"%s\") RECV %d >>> \n%s\n ========= END =========\n",(int)s, nwd->host, (int)nwd->btRecv, nwd->res.buf );
    LOG_WSA_SOCKET( ">>> RECV %d >>> \n%s\n ========= END =========\n", (int)nwd->btRecv, nwd->res.buf );
    shutdown( s, SD_SEND );
    return 0;
}


LRESULT WndProcHttpsClose( HWND hWnd, NETWNDDATA *nwd, SOCKET s ) {
    LOG( "SOCKET(%d \"%s\") CLOSED", (int)s, nwd->host );
    LOG_WSA_SOCKET( ">>> CLOSED >>>" );
    closesocket( s );
    DestroyWindow( hWnd );
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
    if( nwd->ssl == NULL ) {
        switch( type ) {
            case FD_CONNECT: return WndProcHttpConnect( hWnd, nwd, s );
            case FD_READ: return WndProcHttpRead( hWnd, nwd, s );
            case FD_CLOSE: return WndProcHttpClose( hWnd, nwd, s );
        }
    } else {
        switch( type ) {
            case FD_CONNECT: return WndProcHttpsConnect( hWnd, nwd, s );
            case FD_READ: return WndProcHttpsRead( hWnd, nwd, s );
            case FD_CLOSE: return WndProcHttpsClose( hWnd, nwd, s );
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
        SSL_load_error_strings(); /* readable error messages */
        SSL_library_init();       /* initialize library */
        const SSL_METHOD *method = A7_TLSv1_2_client_method();
        if( method == NULL ) {
            LOG_ERR( "A7_TLSv1_2_client_method()", 0 );
        } else {
            g_ssl_ctx = SSL_CTX_new ( method );
            if( g_ssl_ctx == NULL ) {
                LOG_ERR( "SSL_CTX_new()", 0 );
            }
        }
    }

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

    // a7CreateAsyncHttpConnection( "example.com", NULL );
    // a7CreateAsyncHttpConnection( "vk.com", NULL );
    // a7CreateAsyncHttpConnection( "yandex.ru", NULL );
    a7CreateAsyncHttpsConnection( "example.com", NULL );
    a7CreateAsyncHttpsConnection( "vk.com", NULL );
    a7CreateAsyncHttpsConnection( "yandex.ru", NULL );

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if( g_ssl_ctx != NULL ) {
        SSL_CTX_free( g_ssl_ctx );
    }

    A7_WSACleanup();
    LOG_CLOSE();
    return (int) msg.wParam;
}

