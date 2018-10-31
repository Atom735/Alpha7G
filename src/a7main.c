
#include <Windows.h>

#include "a7log.h"
#include "a7net.h"



#ifndef UNICODE
int APIENTRY WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
#else
int APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd )
#endif
{
    MSG msg = {0};
    msg.wParam = -1;
    if ( A7LogOpen ( NULL ) != 0 ) return (-1);
    if ( A7NetInit ( hInstance ) != 0 ) goto end;
    msg.wParam = 0;

    BYTE IPv4[4] = { 8,8,8,8 };
    BYTE IPv4_[4] = { 8,8,4,4 };
    UINT16 IPv6[8] = {
        0x2001,0x4860,0x4860,0x0000,
        0x0000,0x0000,0x0000,0x8888
    };
    UINT16 IPv6_[8] = {

        0x0000,0x0000,0x0000,0x0000,
        0x0000,0xffff,0x0808,0x0808
    };



    A7NetNewConnectDns4 ( IPv4 );
    A7NetNewConnectDns6 ( IPv6 );
    A7NetNewConnectDns6 ( IPv6_ );

    while ( ( g_A7NetConnectsCount != 0 ) && ( GetMessage ( &msg, NULL, 0, 0) ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    end:
    A7NetRelease();
    A7LogClose();
    return (int) msg.wParam;
}
