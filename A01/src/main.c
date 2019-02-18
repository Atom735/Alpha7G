#include <windows.h>
#include <stdlib.h>

typedef double T_REAL;

#define D_GRID_SIZE 1024

static T_REAL * pArray = NULL;
static T_REAL * pArrayOld = NULL;
static const T_REAL fTimeStep = 0.000001L;
static const T_REAL fGridStep = 1.0L/D_GRID_SIZE;
static const T_REAL fTempInitial = 0.0L;
static const T_REAL fTempLeft = 0.0L;
static const T_REAL fTempRight = 0.0L;
static const T_REAL fTempTop = 1.0L;
static const T_REAL fTempBottom = 0.0L;

static const T_REAL fConst1 = fTimeStep/fGridStep/fGridStep;
static const T_REAL fConst2 = 1.0L/(1.0L+(4.0L*fConst1));

static T_REAL fTime = 0;

static void SOLV_STEP() {
    UINT i = 0;
    {
        pArray[0] = (( fTempLeft + pArrayOld[1] + fTempBottom + pArrayOld[D_GRID_SIZE] ) * fConst1 - pArrayOld) * fConst2;
        for (; i<D_GRID_SIZE; ++i)
        {
            /* code */
        }
    }
    for ( UINT ii = 1; ii < 1000; ++ii ) {

    }
}

static LRESULT CALLBACK
WndProc (
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {
    static HDC _hDC = NULL;
    static HDC hBackBufferDC = NULL;
    static HBITMAP hBackBuffer = NULL;
    static BYTE * pBuf = NULL;
    switch ( uMsg ) {
        case WM_CREATE: {
            pArray = (T_REAL*) malloc ( sizeof (T_REAL) * D_GRID_SIZE * D_GRID_SIZE );
            pArrayOld = (T_REAL*) malloc ( sizeof (T_REAL) * D_GRID_SIZE * D_GRID_SIZE );
            _hDC = GetWindowDC ( hWnd );
            hBackBufferDC = CreateCompatibleDC ( _hDC );
            BITMAPINFO bi;
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biBitCount = 24;
            bi.bmiHeader.biWidth = D_GRID_SIZE;
            bi.bmiHeader.biHeight = D_GRID_SIZE;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biPlanes = 1;
            hBackBuffer = CreateDIBSection ( hBackBufferDC, &bi, DIB_RGB_COLORS, (void**)&pBuf, NULL, 0 );
            for ( UINT i = 0; i < D_GRID_SIZE*D_GRID_SIZE; ++i ) {
                pArrayOld[i] = fTempInitial;
                const int x = (i%256);
                const int y = (i/D_GRID_SIZE)%256;
                pBuf[i*3+0]=x;
                pBuf[i*3+1]=y;
                pBuf[i*3+2]=x^y;
            }
            return 0;
        }
        case WM_SIZE: {
            return 0;
        }
        case WM_DESTROY: {
            free ( pArrayOld );
            free ( pArray );
            DeleteObject ( hBackBuffer );
            DeleteDC ( hBackBufferDC );
            ReleaseDC ( hWnd, _hDC );
            PostQuitMessage ( 0 );
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );
            SelectObject ( hBackBufferDC, hBackBuffer );
            SOLV_STEP();
            for ( UINT i = 0; i < D_GRID_SIZE*D_GRID_SIZE; ++i ) {
                const BYTE b = (BYTE)(pArray[i]*((T_REAL)255));
                pBuf[i*3+0]=b;
                pBuf[i*3+1]=b;
                pBuf[i*3+2]=b;
            }
            BitBlt ( hDC, 0, 0, D_GRID_SIZE, D_GRID_SIZE, hBackBufferDC, 0, 0, SRCCOPY);
            EndPaint ( hWnd, &ps );
            InvalidateRect ( hWnd, NULL, FALSE );
            Sleep ( 1 );
            return 0;
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}


INT APIENTRY wWinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    INT nShowCmd )
{
    {
        WNDCLASSEXW wc = {
            .cbSize        = sizeof ( WNDCLASSEXW ),
            .style         = CS_VREDRAW|CS_HREDRAW|CS_OWNDC,
            .lpfnWndProc   = WndProc,
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = hInstance,
            .hIcon         = NULL,
            .hCursor       = NULL,
            .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
            .lpszMenuName  = NULL,
            .lpszClassName = L"SOME_CLASS_NAME",
            .hIconSm       = NULL,
        };
        RegisterClassExW ( &wc );
    }
    CONST HWND hWnd = CreateWindowExW ( 0, L"SOME_CLASS_NAME", NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

    MSG msg = { };
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }

    return msg.wParam;
}
