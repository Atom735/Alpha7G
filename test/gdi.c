#include <WinSock2.h>
#include <Windows.h>

#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>

int main ( int argc, char const *argv[] ) {

    HWND hWnd = GetDesktopWindow();
    HDC hDC = GetWindowDC ( hWnd );

    HDC hBackBufferDC = CreateCompatibleDC ( hDC );

    BITMAPINFO bi;
    memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biWidth = 256;
    bi.bmiHeader.biHeight = 256;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biPlanes = 1;



    BYTE *buf;
    HBITMAP hBackBuffer = CreateDIBSection ( hBackBufferDC, &bi, DIB_RGB_COLORS, (void**)&buf, NULL, 0 );

    for (int i = 0; i < 256*256; ++i ) {
        const int x = (i%256);
        const int y = (i/256)%256;
        buf[i*3+0]=x;
        buf[i*3+1]=y;
        buf[i*3+2]=x^y;
        // buf[i*4+3]=0;
    }

    SelectObject ( hBackBufferDC, hBackBuffer );



    BitBlt ( hDC, 0, 0, 256, 256, hBackBufferDC, 0, 0, SRCCOPY);


    DeleteObject ( hBackBuffer );



    DeleteDC ( hBackBufferDC );


    ReleaseDC ( hWnd, hDC );

    return 0;
}
