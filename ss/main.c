#include "header.h"

static FT_Library g_ftLib;

/*
DNS Size limits
    labels          63 octets or less
    names           255 octets or less
    TTL             positive values of a signed 32 bit number.
    UDP messages    512 octets or less

DNS Qtype [https://en.wikipedia.org/wiki/List_of_DNS_record_types]
    A       1       Address record          Returns a 32-bit IPv4 address
    AAAA    28      IPv6 address record     Returns a 128-bit IPv6 address

DNS Cache
    Массив смещения указателей на строки, оканчивающиеся нулем
    Название хоста, байт где младшие 4 бита - кол во IpV4, а 4 старших бита - колво IpV6

*/
UINT A7Pack_DnsQName ( UINT8 *pBuf8, LPCSTR sLabel ) {
    UINT o = 0;
    UINT n = 0;
    while ( TRUE ) {
        if ( sLabel [ n ] == '.' ) {
            assert ( n < 64 );
            *pBuf8 = n;
            memcpy ( pBuf8 + 1, sLabel, n );
            ++n;
            o += n;
            pBuf8 += n;
            sLabel += n;
            n = 0;
            continue;
        }
        if ( sLabel [ n ] == 0 ) {
            assert ( n < 64 );
            *pBuf8 = n;
            ++n;
            memcpy ( pBuf8 + 1, sLabel, n );
            return o + n + 1;
        }
        ++n;
    }
    return 0;
}
UINT A7Pack_DnsStdRequest ( UINT8 *pBuf8, LPCSTR sLabel ) {
    static UINT _id = 0;
    ++_id;
    UINT16 *pBuf16 = ( UINT16* ) pBuf8;
    *pBuf16 = htons ( _id ); ++pBuf16;
    *pBuf16 = htons ( 0x0100 ); ++pBuf16;
    *pBuf16 = htons ( 1 ); ++pBuf16;
    *pBuf16 = htons ( 0 ); ++pBuf16;
    *pBuf16 = htons ( 0 ); ++pBuf16;
    *pBuf16 = htons ( 0 );
    UINT n = A7Pack_DnsQName ( pBuf8 + 12, sLabel );
    pBuf16 = ( UINT16* ) ( pBuf8 + 12 + n );
    *pBuf16 = htons ( 1 ); ++pBuf16;
    *pBuf16 = htons ( 1 );
    return n + 16;
}



/* Название процедуры главного окна */
static LRESULT CALLBACK
_7WinProc (
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {
    static WCHAR _w [ 512 * 64 ] = { };
    static UINT _cw [ 64 ] = { };
    static RECT _rc = { };

    static struct sockaddr_in _dns_addr = { };
    static SOCKET _dns_socket;

    void _push_text() {
        for ( UINT i = 63; i > 0; --i ) {
            _cw [ i ] = _cw [ i - 1 ];
            wmemcpy ( _w + ( i * 512 ), _w + ( ( i - 1 ) * 512 ), _cw [ i ] * sizeof ( WCHAR ) );
        }
        InvalidateRect ( hWnd, NULL, FALSE );
    }
    #define _print(...) _push_text ( ); _cw [ 0 ] = swprintf ( _w, 511, __VA_ARGS__ )
    void _print_dns_head ( UINT8 *pBuf ) {
        UINT16 *pBuf16      = ( UINT16* ) pBuf;
        CONST UINT ID       = ntohs ( *pBuf16 ); ++pBuf16;
        CONST UINT FLAGS    = ntohs ( *pBuf16 ); ++pBuf16;
        CONST UINT QD       = ntohs ( *pBuf16 ); ++pBuf16;
        CONST UINT AN       = ntohs ( *pBuf16 ); ++pBuf16;
        CONST UINT NS       = ntohs ( *pBuf16 ); ++pBuf16;
        CONST UINT AR       = ntohs ( *pBuf16 ); ++pBuf16;
        CONST UINT QR       = (FLAGS>>0xf)&0x1;
        CONST UINT OPCODE   = (FLAGS>>0xb)&0xf;
        CONST UINT AA       = (FLAGS>>0xa)&0x1;
        CONST UINT TC       = (FLAGS>>0x9)&0x1;
        CONST UINT RD       = (FLAGS>>0x8)&0x1;
        CONST UINT RA       = (FLAGS>>0x7)&0x1;
        CONST UINT Z        = (FLAGS>>0x4)&0x7;
        CONST UINT RCODE    = (FLAGS>>0x0)&0xf;
        _print ( L"DNS %hs ID = %i", ID, (FLAGS&0x8000?"response":"query") );
        switch ( OPCODE ) {
            case 0:
                _print ( L"a standard query (QUERY)" );
                break;
            case 1:
                _print ( L"an inverse query (IQUERY)" );
                break;
            case 2:
                _print ( L"a server status request (STATUS)" );
                break;
            default:
                _print ( L"UNDEFINED OPCODE = %i", OPCODE );
        }
        _print ( L"%hs", AA );
    }

    switch ( uMsg ) {
        case WM_CREATE: {

            _dns_addr.sin_family = AF_INET;
            _dns_addr.sin_addr.s_addr = inet_addr ( "8.8.8.8" ); /* Google Public DNS */
            // _dns_addr.sin_addr.s_addr = inet_addr ( "192.168.32.2" ); /* KPFU Local DNS server */
            _dns_addr.sin_port = htons ( 53 ); /* DNS */

            /* create UDP DataGram Socket for DNS requests */
            _dns_socket = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); /* UDP for DNS */
            _print ( L"_dns_socket = %i", _dns_socket );
            /* Set socket to async non-block */
            WSAAsyncSelect ( _dns_socket, hWnd, WM_SOCK_DNS, FD_READ | FD_WRITE );
            return 0;
        }
        case WM_SOCK_DNS: {
            SOCKET s = ( SOCKET ) wParam;
            CONST UINT ev = LOWORD ( lParam );
            // CONST UINT er = HIWORD ( lParam );
            switch ( ev ) {
                case FD_WRITE: {
                    _print ( L"FD_WRITE" );
                    UINT8 pBuf [ 512 ];
                    UINT nBuf = A7Pack_DnsStdRequest ( pBuf, "example.com" );
                    _print ( L"packed = %i", nBuf );
                    int err = sendto ( s, ( VOID* ) pBuf, nBuf, 0, ( struct sockaddr* ) &_dns_addr, sizeof ( _dns_addr ) );
                    _print ( L"sendto = %i", err );
                    if ( err == SOCKET_ERROR ) {
                        err = WSAGetLastError ();
                        _print ( L"last error = %i", err );
                    }
                    break;
                }
                case FD_READ: {
                    _print ( L"FD_READ" );
                    UINT8 pBuf [ 512 ];
                    UINT nBuf = recv ( s, ( VOID* ) pBuf, 512, 0 );
                    _print ( L"recved = %i", nBuf );
                    _print_dns_head ( pBuf );

                    break;
                }
            }
            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
        case WM_SIZE: {
            _rc . right = LOWORD ( lParam );
            _rc . bottom = HIWORD ( lParam );
            return 0;
        }
        case WM_DESTROY: {
            closesocket ( _dns_socket );
            PostQuitMessage ( 0 );
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );
            FillRect( hDC, &_rc, GetStockObject ( GRAY_BRUSH ) );
            for ( UINT i = 0; i < 64; ++i) {
                TextOutW ( hDC, 0, _rc . bottom - 16*i - 16, _w + ( 512 * i ), _cw [ i ] );
            }
            EndPaint ( hWnd, &ps );
            return 0;
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}



/* Точка входа в приложение */
INT APIENTRY
wWinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    INT nShowCmd
) {
    WSADATA _wsadata = { 0 };
    WSAStartup ( WINSOCK_VERSION, &_wsadata );

    FT_Init_FreeType( &g_ftLib );

    CONST LPCWSTR kwClassName = L"WCN-A7Main";
    WNDCLASSEXW wc = {
        .cbSize        = sizeof ( WNDCLASSEXW ),
        .style         = CS_VREDRAW | CS_HREDRAW,
        .lpfnWndProc   = _7WinProc,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = hInstance,
        .hIcon         = NULL,
        .hCursor       = NULL,
        .hbrBackground = NULL,
        .lpszMenuName  = NULL,
        .lpszClassName = kwClassName,
        .hIconSm       = NULL,
    };
    if ( RegisterClassExW ( &wc ) == 0 ) {
        return -1;
    }
    /* Создание главного окна */
    if ( CreateWindowExW ( 0, kwClassName, NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL ) == NULL ) {
        return -1;
    }

    /* Входим в цикл обработки сообщений */
    MSG msg = { };
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }

    /* Освобождение класса главного окна */
    if ( ! UnregisterClassW ( kwClassName, hInstance ) ) {
        return -1;
    }

    FT_Done_FreeType ( g_ftLib );

    WSACleanup ( );

    return 0;
}













