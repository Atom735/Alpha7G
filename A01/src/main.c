#include <windows.h>
#include <stdlib.h>

typedef float T_REAL;

#define D_GRID_SIZE 128

static T_REAL * pArray = NULL;
static T_REAL * pArrayOld = NULL;
static const T_REAL fTimeStep = 0.000001L;
static const T_REAL fGridStep = 1.0L/D_GRID_SIZE;
static const T_REAL fTempInitial = 0.0L;
static const T_REAL fTempLeft = 1.0L;
static const T_REAL fTempRight = 0.25L;
static const T_REAL fTempTop = 0.5L;
static const T_REAL fTempBottom = 0.75L;


static T_REAL fTime = 0;




static void SOLV_Init() {
    /* Initial Temp */
    for ( UINT i = 0; i < (D_GRID_SIZE+2)*(D_GRID_SIZE+2); ++i ) {
        pArray[i] = fTempInitial;
    }
    /* Left Edge */
    for ( UINT i = 0; i < D_GRID_SIZE+1; ++i ) {
        pArray[i*(D_GRID_SIZE+2)] = fTempLeft;
    }
    /* Right Edge */
    for ( UINT i = 0; i < D_GRID_SIZE+1; ++i ) {
        pArray[i*(D_GRID_SIZE+2)+D_GRID_SIZE+1] = fTempRight;
    }
    /* Bottom Edge */
    for ( UINT i = 0; i < D_GRID_SIZE+1; ++i ) {
        pArray[i] = fTempBottom;
    }
    /* Top Edge */
    for ( UINT i = 0; i < D_GRID_SIZE+1; ++i ) {
        pArray[i+(D_GRID_SIZE+1)*(D_GRID_SIZE+2)] = fTempTop;
    }
}

/*
    T[ k - индекс по временному слою, i,j - индексы по пространственной сетке ]
    T[0,i,j] - начальные значения
    T[k,0,j] - левая грань
    T[k,Ni,j] - правая грань
    T[k,i,0] - нижняя грань
    T[k,i,Nj] - верхняя грань

 == Дискретный аналог исходного уравнения:
    ( T[k+1,*,*] - T[k,*,*] ) / t =
        ( T[*,i-1,j] - 2 T[*,i,j] + T[*,i+1,j] ) / h^2 +
        ( T[*,i,j-1] - 2 T[*,i,j] + T[*,i,j+1] ) / h^2

 == Неявная схема:
    ( T[k+1,i,j] - T[k,i,j] ) / t =
        ( T[k+1,i-1,j] - 2 T[k+1,i,j] + T[k+1,i+1,j] ) / h^2 +
        ( T[k+1,i,j-1] - 2 T[k+1,i,j] + T[k+1,i,j+1] ) / h^2
    >> изменим немного индекс k
    ( T[k,i,j] - T[k-1,i,j] ) / t =
        ( T[k,i-1,j] - 2 T[k,i,j] + T[k,i+1,j] ) / h^2 +
        ( T[k,i,j-1] - 2 T[k,i,j] + T[k,i,j+1] ) / h^2
    >> T[k,i,j] - расчитывыемый узел
    >> T[k-1,i,j] - расчитанное значения в узле в пред момент времени - константа
    >> Умножим обе части уравнения на t и h
    h^2 * ( T[k,i,j] - T[k-1,i,j] ) =
        t * ( T[k,i-1,j] - 2 T[k,i,j] + T[k,i+1,j] + T[k,i,j-1] - 2 T[k,i,j] + T[k,i,j+1] )
    >> Вынесем неизвестные части влево, а известные вправо
    h^2 T[k,i,j] + 2t T[k,i,j] + 2t T[k,i,j]
        -t * ( T[k,i-1,j] + T[k,i+1,j] + T[k,i,j-1] + T[k,i,j+1] )
        = h^2 T[k-1,i,j]
    >> Для наглядности сделаем замену
        T[k,i,j] == This
        T[k-1,i,j] == Old
        T[k,i-1,j] == Left
        T[k,i+1,j] == Right
        T[k,i,j-1] == Bottom
        T[k,i,j+1] == Top
    (h^2+4t)*This - t*(Left+Right+Bottom+Top) = h^2 * Old
    >> Считаем итерационнм методом, считая что нам известны:
        Left, Bottom - из пред расчётов или краевых условий
        Old - из пред временногослоя
        Bottom, Top - известны из пред итерации
    This = [h^2/(h^2+4t)] * Old + [t/(h^2+4t)] * (Left+Right+Bottom+Top)
    >> Заменим константы
        Const1 == [h^2/(h^2+4t)]
        Const2 == [t/(h^2+4t)]
    This = Const1 * Old + Const2 * (Left+Right+Bottom+Top)
*/
static const T_REAL fConst1 = fGridStep*fGridStep/(fGridStep*fGridStep+4.0L*fTimeStep);
static const T_REAL fConst2 = fTimeStep/(fGridStep*fGridStep+4.0L*fTimeStep);

static void SOLV_STEP() {
    for ( UINT i = 0; i<D_GRID_SIZE*D_GRID_SIZE; ++i) {
        const UINT j = i+3+D_GRID_SIZE+(2*(i/D_GRID_SIZE));
        pArray[j] = ( fConst1 * pArray[j] ) + ( fConst2 * (
            pArray[j-1] + pArray[j+1] +
            pArray[j-(D_GRID_SIZE+2)] +
            pArray[j+(D_GRID_SIZE+2)] ) );
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
            pArray = (T_REAL*) malloc ( sizeof (T_REAL) * ((D_GRID_SIZE+2) * (D_GRID_SIZE+2)) );
            pArrayOld = (T_REAL*) malloc ( sizeof (T_REAL) * ((D_GRID_SIZE+2) * (D_GRID_SIZE+2)) );
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
            SOLV_Init();

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
            for ( UINT i = 0; i < D_GRID_SIZE*D_GRID_SIZE; ++i ) {
                const UINT j = i+3+D_GRID_SIZE+(2*(i/D_GRID_SIZE));
                const T_REAL f = pArray[j]*(T_REAL)6;
                if ( f < (T_REAL)1 ) {
                    pBuf[i*3+0]=(BYTE)((f)*(T_REAL)255);
                    pBuf[i*3+1]=0x00;
                    pBuf[i*3+2]=0x00;
                } else
                if ( f < (T_REAL)2 ) {
                    pBuf[i*3+0]=0xff;
                    pBuf[i*3+1]=(BYTE)((f-(T_REAL)1)*(T_REAL)255);
                    pBuf[i*3+2]=0x00;
                } else
                if ( f < (T_REAL)3 ) {
                    pBuf[i*3+0]=(BYTE)(((T_REAL)3-f)*(T_REAL)255);
                    pBuf[i*3+1]=0xff;
                    pBuf[i*3+2]=0x00;
                } else
                if ( f < (T_REAL)4 ) {
                    pBuf[i*3+0]=0x00;
                    pBuf[i*3+1]=0xff;
                    pBuf[i*3+2]=(BYTE)((f-(T_REAL)3)*(T_REAL)255);
                } else
                if ( f < (T_REAL)5 ) {
                    pBuf[i*3+0]=0x00;
                    pBuf[i*3+1]=(BYTE)(((T_REAL)5-f)*(T_REAL)255);
                    pBuf[i*3+2]=0xff;
                } else
                if ( f < (T_REAL)6 ) {
                    pBuf[i*3+0]=(BYTE)((f-(T_REAL)5)*(T_REAL)255);
                    pBuf[i*3+1]=(BYTE)((f-(T_REAL)5)*(T_REAL)255);
                    pBuf[i*3+2]=0xff;
                }
            }
            for ( UINT i = 0; i < 25; ++i ) {
                SOLV_STEP();
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
    CreateWindowExW ( 0, L"SOME_CLASS_NAME", NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

    MSG msg = { };
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }

    return msg.wParam;
}
