
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

    A7NetConnect ( "vk.com", 80, 0 );
    A7NetConnect ( "example.com", 80, 0 );
    A7NetConnect ( "yandex.com", 80, 0 );

    A7NetConnect ( "vk.com", 443, 1 );
    A7NetConnect ( "example.com", 443, 1 );
    A7NetConnect ( "yandex.com", 443, 1 );

    while ( ( g_A7NetConnectsCount != 0 ) && ( GetMessage ( &msg, NULL, 0, 0) ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    end:
    A7NetRelease ( );
    A7LogClose ( );
    return (int) msg.wParam;
}
