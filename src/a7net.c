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

/* Порт DNS сервера */
#define _A7PORT_DNS 53

/* Имя класса окна A7NET */
#define _A7NET_WND_CLASS_NAME "WCN-Alpha7G-A7NET"
/* Макросы получения типа события и ошибки из lParam */
#define GET_WM_WSA_EVENT( lParam )  LOWORD ( lParam )
#define GET_WM_WSA_ERROR( lParam )  HIWORD ( lParam )
/* Макрос номера сообщения A7NET */
#define WM_WSA_SELECT   ( WM_USER + 1 )


/* Данные WSA */
static WSADATA      g_A7NetWsaData  = {0};
/* Instance приложения */
static HINSTANCE    g_A7NetInstance = NULL;
/* SSL контекст библиотеки OpenSSL */
static SSL_CTX     *g_A7NetCtx      = NULL;

int     g_A7NetConnectsCount    = 0;

static int A7NetLogV ( S7WDA7NET *p, const char * fmt, va_list args ) {
    if ( p == NULL ) return -1;
    if ( p -> fLog == NULL ) return -1;
    int n = vfprintf ( p -> fLog, fmt, args );
    fflush ( p -> fLog );
    return n;
}

static int A7NetLog ( S7WDA7NET *p, const char * fmt, ... ) {
    if ( p == NULL ) return -1;
    if ( p -> fLog == NULL ) return -1;
    va_list args;
    va_start ( args, fmt );
    int n = A7NetLogV ( p, fmt, args );
    va_end ( args );
    fflush ( p -> fLog );
    return n;
}

#include "a7net_log.c"

/* Функция обарботчик сообщения WM_CREATE */
static LRESULT A7NetOnCreate ( HWND hWnd, S7WDA7NETCRT *pCRT ) {
    ++g_A7NetConnectsCount;

    S7WDA7NET *p = ( S7WDA7NET* ) malloc ( sizeof ( S7WDA7NET ) );

    p -> hWnd   = hWnd;
    p -> iSock  = INVALID_SOCKET;
    p -> bTls   = 0;
    p -> pCBS   = pCRT -> pCBS;
    p -> szText = NULL;
    p -> fLog   = NULL;
    p -> nClock = 0;

    SetWindowLongPtr( hWnd, GWLP_USERDATA, ( LONG_PTR ) p );

    BYTE *pIP   = pCRT -> aIp;
    BOOL bIPv4  = (!(pCRT -> bForceV6)) &&
        (pIP[000]==0x00) && (pIP[001]==0x00) &&
        (pIP[002]==0x00) && (pIP[003]==0x00) &&
        (pIP[004]==0x00) && (pIP[005]==0x00) &&
        (pIP[006]==0x00) && (pIP[007]==0x00) &&
        (pIP[010]==0x00) && (pIP[011]==0x00) &&
        (pIP[012]==0xff) && (pIP[013]==0xff);

    /// Create name and open log file
    {
        CHAR str[256];
        INT i = 0;
        if ( bIPv4 ) {
            i = snprintf ( str, 255, "A7NETv4 "
                "%d.%d.%d.%d:%d",
                pIP[014], pIP[015], pIP[016], pIP[017], pCRT -> nPort );
        } else {
            UINT16 *pIP16 = ( UINT16* ) pIP;
            i = snprintf ( str, 255, "A7NETv6 "
                "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
                htons(pIP16[0]), htons(pIP16[1]), htons(pIP16[2]), htons(pIP16[3]),
                htons(pIP16[4]), htons(pIP16[5]), htons(pIP16[6]), htons(pIP16[7]),
                pCRT -> nPort );
        }
        p -> szText = ( CHAR* ) malloc ( i + 1 );
        memcpy ( p -> szText, str, i + 1 );
        int j = 0;
        while (1) {
            if ( bIPv4 ) {
                i = snprintf ( str, 255, "A7NETv4 %d-%d_%d_%d_%d.%d.a7-log",
                    pCRT -> nPort, pIP[014], pIP[015], pIP[016], pIP[017], j );
            } else {
                UINT16 *pIP16 = ( UINT16* ) pIP;
                i = snprintf ( str, 255, "A7NETv6 %d-"
                    "%04x_%04x_%04x_%04x_%04x_%04x_%04x_%04x.%d.a7-log",
                    pCRT -> nPort,
                    htons(pIP16[0]), htons(pIP16[1]), htons(pIP16[2]), htons(pIP16[3]),
                    htons(pIP16[4]), htons(pIP16[5]), htons(pIP16[6]), htons(pIP16[7]),
                    j );
            }
            p -> fLog = fopen ( str, "r" );
            if ( p -> fLog == NULL ) break;
            fclose ( p -> fLog );
            ++j;
        }
        p -> fLog = fopen ( str, "w" );
        time_t rawtime;
        time ( &rawtime );
        A7NetLog ( p, "@^ %s", asctime ( localtime ( &rawtime ) ) );
        p -> nClock = clock();
        A7NetLog ( p, "    Clock of start: %lu\n", p -> nClock );
        A7NetLog ( p, "    Name: %s\n", p -> szText );
    }

    if ( bIPv4 ) {
        SOCKADDR_IN sa;
        memset ( &sa, 0, sizeof ( sa ) );
        sa.sin_family     = AF_INET,
        sa.sin_port       = htons ( pCRT -> nPort ),
        memcpy ( &sa.sin_addr, pIP+014, 4 );
        if ( ( p -> iSock = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
            A7NetLog_socket ( p, WSAGetLastError(), TRUE );
            return (-1);
        } else {
            A7NetLog_socket ( p, 0, TRUE );
        }
        /// Set socket to async non-block
        if ( WSAAsyncSelect ( p -> iSock, hWnd, WM_WSA_SELECT, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE ) != 0 ) {
            A7NetLog_WSAAsyncSelect ( p, WSAGetLastError() );
            return (-1);
        } else {
            A7NetLog_WSAAsyncSelect ( p, 0 );
        }
        /// Socket connection to server
        if( connect ( p -> iSock, ( LPSOCKADDR )( &sa ), sizeof ( sa ) ) != 0 ) {
            int err = WSAGetLastError();
            A7NetLog_connect( p, err );
            if ( err != WSAEWOULDBLOCK ) {
                return (-1);
            }
        } else {
            A7NetLog_connect( p, 0 );
        }
    } else {
        SOCKADDR_IN6 sa;
        memset ( &sa, 0, sizeof ( sa ) );
        sa.sin6_family    = AF_INET6,
        sa.sin6_port      = htons ( pCRT -> nPort ),
        memcpy ( &sa.sin6_addr, pIP, 16 );
        if ( ( p -> iSock = socket ( AF_INET6, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
            A7NetLog_socket ( p, WSAGetLastError(), TRUE );
            return (-1);
        } else {
            A7NetLog_socket ( p, 0, TRUE );
        }
        /// Set socket to async non-block
        if ( WSAAsyncSelect ( p -> iSock, hWnd, WM_WSA_SELECT, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE ) != 0 ) {
            A7NetLog_WSAAsyncSelect ( p, WSAGetLastError() );
            return (-1);
        } else {
            A7NetLog_WSAAsyncSelect ( p, 0 );
        }
        /// Socket connection to server
        if( connect ( p -> iSock, ( LPSOCKADDR )( &sa ), sizeof ( sa ) ) != 0 ) {
            int err = WSAGetLastError();
            A7NetLog_connect( p, err );
            if ( err != WSAEWOULDBLOCK ) {
                return (-1);
            }
        } else {
            A7NetLog_connect( p, 0 );
        }
    }
    return (0);
}

/* Функция обарботчик сообщения WM_DESTROY */
static LRESULT A7NetOnDestroy ( HWND hWnd, S7WDA7NET *p ) {
    if ( p != NULL ) {
        if ( p -> fLog != NULL ) {
            time_t rawtime;
            time ( &rawtime );
            long unsigned c = clock();
            A7NetLog ( p, "@$ %s", asctime ( localtime ( &rawtime ) ) );
            A7NetLog ( p, "    Clock of end: %lu\n", c );
            A7NetLog ( p, "    Name: %s\n", p -> szText );
            A7NetLog ( p, "    Socket: %i\n", p -> iSock );
            A7NetLog ( p, "    Clock of run: %lu\n", c - p -> nClock );
            fclose ( p -> fLog );
        }
        if ( p -> iSock != INVALID_SOCKET ) {
            closesocket ( p -> iSock );
        }
        if ( p -> szText != NULL ) {
            free ( p -> szText );
        }
        free ( p );
    }
    --g_A7NetConnectsCount;
    return (0);
}


/* Функция обработчик сообщений окна */
static LRESULT CALLBACK A7NetWndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch ( uMsg ) {
        case WM_CREATE:
            return A7NetOnCreate ( hWnd, ( S7WDA7NETCRT* ) ( ( ( CREATESTRUCT* ) lParam ) ->lpCreateParams ) );
        case WM_DESTROY: {
            return A7NetOnDestroy ( hWnd, A7NetGetWndData ( hWnd ) );
        }

        case WM_WSA_SELECT: {
            int err = GET_WM_WSA_ERROR ( lParam );
            int etp = GET_WM_WSA_EVENT ( lParam );
            S7WDA7NET *p = A7NetGetWndData ( hWnd );
            S7WDA7NETCB *cb = p->pCBS;
            A7NetLog_WSAAsyncSelect_WM ( p, err, etp );
            if ( err != 0 ) {
                if ( cb -> OnError ( p, etp, err ) ) {
                    DestroyWindow( hWnd );
                }
            } else {
                switch ( etp ) {
                    case FD_CONNECT:
                        if ( cb -> OnConnect ( p ) ) {
                            closesocket ( p -> iSock );
                        }
                        break;
                    case FD_CLOSE:
                        if ( cb -> OnClose ( p ) ) {
                            DestroyWindow( hWnd );
                        }
                        break;
                    case FD_WRITE:
                        if ( cb -> OnWrite ( p ) ) {
                            closesocket ( p -> iSock );
                        }
                        break;
                    case FD_READ:
                        if ( cb -> OnRead ( p ) ) {
                            closesocket ( p -> iSock );
                        }
                        break;
                }
            }
            return 0;
        }
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

int     A7NetInit           ( HINSTANCE hInstance ) {
    g_A7NetInstance = hInstance;

    WNDCLASSEXW wcex;
    wcex.cbSize         = sizeof ( WNDCLASSEX );
    wcex.style          = 0;
    wcex.lpfnWndProc    = A7NetWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = g_A7NetInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = NULL;
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = TEXT ( _A7NET_WND_CLASS_NAME );
    wcex.hIconSm        = NULL;
    if ( RegisterClassEx ( &wcex ) == 0 ) {
        // TODO:FATAL
        return (-1);
    }
    if ( ( WSAStartup ( WINSOCK_VERSION, &g_A7NetWsaData ) ) != 0 ) {
        // TODO:FATAL
        return (-1);
    }

    SSL_load_error_strings();
    if ( ( SSL_library_init() ) != 1 ) {
        // TODO:ERROR
    }

    SSL_METHOD const * method = TLSv1_2_client_method();
    if ( method == NULL ) {
        // TODO:WARN
        method = TLSv1_1_client_method();
        if ( method == NULL ) {
            // TODO:WARN
            method = TLSv1_client_method();
            if ( method == NULL ) {
                // TODO:WARN
                method = SSLv23_client_method();
                if ( method == NULL ) {
                    // TODO:WARN
                }
            }
        }
    }
    if ( method == NULL ) {
        // TODO:ERROR
    } else {
        g_A7NetCtx = SSL_CTX_new ( method );
        if ( g_A7NetCtx == NULL ) {
            // TODO:ERROR
        }
    }
    return (0);
}

void    A7NetRelease        ( void ) {
    if( g_A7NetCtx != NULL ) {
        SSL_CTX_free ( g_A7NetCtx );
    }
    if ( WSACleanup() == SOCKET_ERROR ) {
        // TODO:ERROR
    }
}

HWND    A7NetNewConnect     ( S7WDA7NETCRT *pCRT ) {
    return ( pCRT == NULL ) ? NULL : CreateWindowEx ( 0, TEXT ( _A7NET_WND_CLASS_NAME ), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_A7NetInstance, pCRT );
}


S7WDA7NET * A7NetGetWndData ( HWND hWnd ) {
    return ( S7WDA7NET* ) GetWindowLongPtr ( hWnd, GWLP_USERDATA );
}

HWND    A7NetNewConnect4    ( BYTE *pIp, UINT16 nPort, BOOL bTls,
    S7WDA7NETCB *pCBS ) {
    S7WDA7NETCRT crt = {
        // .aIp    = {
        //         0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        //         0x00,0x00,0xff,0xff,
        //         pIp[0],pIp[1],pIp[2],pIp[3] },
        .nPort  = nPort,
        .bTls   = bTls,
        .pCBS   = pCBS,
        .bForceV6   = FALSE,
    };
    memset ( crt.aIp, 0, 10 );
    crt.aIp[012] = 0xff;
    crt.aIp[013] = 0xff;
    memcpy ( crt.aIp+014, pIp, 4 );
    return A7NetNewConnect ( &crt );
}

HWND    A7NetNewConnect6    ( UINT16 *pIp, UINT16 nPort, BOOL bTls,
    S7WDA7NETCB *pCBS ) {
    S7WDA7NETCRT crt = {
        // .aIp    = {
        //         pIp[000],pIp[001],pIp[002],pIp[003],
        //         pIp[004],pIp[005],pIp[006],pIp[007],
        //         pIp[010],pIp[011],pIp[012],pIp[013],
        //         pIp[014],pIp[015],pIp[016],pIp[017] },
        .nPort  = nPort,
        .bTls   = bTls,
        .pCBS   = pCBS,
        .bForceV6   = TRUE,
    };

    memcpy ( crt.aIp, pIp, 16 );

    UINT16 *pIp1 = ( UINT16* ) ( crt.aIp );
    pIp1[0] = htons ( pIp[0] );
    pIp1[1] = htons ( pIp[1] );
    pIp1[2] = htons ( pIp[2] );
    pIp1[3] = htons ( pIp[3] );
    pIp1[4] = htons ( pIp[4] );
    pIp1[5] = htons ( pIp[5] );
    pIp1[6] = htons ( pIp[6] );
    pIp1[7] = htons ( pIp[7] );
    return A7NetNewConnect ( &crt );
}

/*  Callback вызывемый в случае успешной установки подключения */
BOOL CALLBACK A7CbDnsOnConnect ( S7WDA7NET *p ) {
    A7Log ( "%p CONNECT\n", p -> hWnd );

    p -> send = 0;

    // UINT16 pb = (UINT16*) p -> buf;
    // pb[0] = htons ( 0xAAAA ); /* ID */
    // pb[1] = htons ( 0x0100 ); /* Параметры запроса */
    // pb[2] = htons ( 0x0001 ); /* Количество вопросов */
    // pb[3] = htons ( 0x0000 ); /* Количество ответов */
    // pb[4] = htons ( 0x0000 ); /* Количество записей об уполномоченных серверах */
    // pb[5] = htons ( 0x0000 ); /* Количество дополнительных записей */
    // p -> buf [12] = 7;
    // memcpy ( p -> buf + 13, "example", 7 );
    // p -> buf [20] = 3;
    // memcpy ( p -> buf + 21, "com", 3 );
    // p -> buf [24] = 0;
    // pb = (UINT16*) ( p -> buf + 25);
    // pb[0] = htons ( 0x0001 ); /* QType */
    // pb[1] = htons ( 0x0001 ); /* QClass */


    return FALSE;
}
/*  Callback вызывемый в случае закрытия соединения */
BOOL CALLBACK A7CbDnsOnClose ( S7WDA7NET *p ) {
    A7Log ( "%p CLOSE\n", p -> hWnd );
    return TRUE;
}
/*  Callback вызывемый в случае пояления места для записи */
BOOL CALLBACK A7CbDnsOnWrite ( S7WDA7NET *p ) {
    A7Log ( "%p WRITE\n", p -> hWnd );
    return FALSE;
}
/*  Callback вызывемый в случае пояления данных для чтения */
BOOL CALLBACK A7CbDnsOnRead ( S7WDA7NET *p ) {
    A7Log ( "%p READ\n", p -> hWnd );
    return FALSE;
}
/*  Callback вызывемый в случае ошибки */
BOOL CALLBACK A7CbDnsOnError ( S7WDA7NET *p, int etp, int err ) {
    switch ( etp ) {
        case FD_CONNECT:
            switch ( err ) {
                case WSAEAFNOSUPPORT:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "Addresses in the specified family cannot be used with this socket." );
                    break;
                case WSAECONNREFUSED:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "The attempt to connect was rejected." );
                    break;
                case WSAENETUNREACH:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "The network cannot be reached from this host at this time." );
                    break;
                case WSAEFAULT:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "The namelen parameter is invalid." );
                    break;
                case WSAEINVAL:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "The socket is already bound to an address." );
                    break;
                case WSAEISCONN:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "The socket is already connected." );
                    break;
                case WSAEMFILE:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "No more file descriptors are available." );
                    break;
                case WSAENOBUFS:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "No buffer space is available. The socket cannot be connected." );
                    break;
                case WSAENOTCONN:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "The socket is not connected." );
                    break;
                case WSAETIMEDOUT:
                    A7Log ( "%p ERROR %s\n", p -> hWnd, "Attempt to connect timed out without establishing a connection." );
                    break;
            }
            break;
    }
    A7Log ( "%p ERROR\n", p -> hWnd );

    return TRUE;
}

static S7WDA7NETCB g_A7NetCbDns = {
        .OnConnect  = A7CbDnsOnConnect,
        .OnClose    = A7CbDnsOnClose,
        .OnWrite    = A7CbDnsOnWrite,
        .OnRead     = A7CbDnsOnRead,
        .OnError    = A7CbDnsOnError,
    };

HWND    A7NetNewConnectDns4 ( BYTE *pIp ) {
    return A7NetNewConnect4 ( pIp, _A7PORT_DNS, FALSE, &g_A7NetCbDns );
}

/*  Подключение к DNS серверу по TCP/IPv6
    @pIp        > IPv4 сервера (16байт)
    @<          >   HWND окна, который обрабатывает подключения
                    NULL в случае ошибки
*/
HWND    A7NetNewConnectDns6 ( UINT16 *pIp ) {
    return A7NetNewConnect6 ( pIp, _A7PORT_DNS, FALSE, &g_A7NetCbDns );
}
