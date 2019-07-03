#include <Windows.h>

INT APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd ) {

    MSG msg = { };

    return (INT) msg.wParam;
}
