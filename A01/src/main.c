#include <windows.h>
#include <stdlib.h>

typedef float T_REAL;

/* Количество узлов сетки */
static UINT nNodesCount = 128U;

/* Массивы данных температур узлов сетки для последнего временного слоя и предыдущего */
static T_REAL * pDataLast = NULL;
static T_REAL * pDataOld = NULL;

/* Максимальное значение ошибки вычесления */
static T_REAL fMaxError = 0.01L;
/* Расстояние между слоями по времени */
static T_REAL fTimeStep = 0.001L;
/* Расстояние между узлами сетки */
static T_REAL fGridStep;
/* Краевые условия */
static T_REAL fTempInitial = 0.0L;
static T_REAL fTempLeft = 0.0L;
static T_REAL fTempRight = 1.0L;
static T_REAL fTempTop = 0.0L;
static T_REAL fTempBottom = 0.0L;

/* Время временного слоя */
static T_REAL fTime;

/* Константы которые помогут при расчётах */
static T_REAL fConst1;
static T_REAL fConst2;

/* Подготовка данных для вычеслений */
static void rInit() {
    const UINT nNCW = (nNodesCount+2);
    const UINT nNCWLE = nNCW-1;
    const UINT nNCFull = nNCW*nNCW;

    fGridStep = ((T_REAL)(1))/((T_REAL)(nNodesCount-1));
    fTime = 0;

    fConst1 = fGridStep*fGridStep/(fGridStep*fGridStep+4.0L*fTimeStep);
    fConst2 = fTimeStep/(fGridStep*fGridStep+4.0L*fTimeStep);

    pDataLast = (T_REAL*) malloc ( sizeof (T_REAL) * nNCFull * 2 );
    pDataOld = pDataLast + nNCFull;

    /* Initial Temp */
    for ( UINT i = 0; i < nNCFull; ++i ) pDataOld[i] = pDataLast[i] = fTempInitial;

    for ( UINT i = 0; i < nNCWLE; ++i ) {
        /* Left Edge */
        pDataOld[(i)*nNCW] = pDataLast[(i)*nNCW] = fTempLeft;
        /* Right Edge */
        pDataOld[(i)*nNCW+nNCWLE] = pDataLast[(i)*nNCW+nNCWLE] = fTempRight;
        /* Bottom Edge */
        pDataOld[(i)] = pDataLast[(i)] = fTempBottom;
        /* Top Edge */
        pDataOld[(nNCFull-nNCW)+(i)] = pDataLast[(nNCFull-nNCW)+(i)] = fTempTop;
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

/* Расчитываем след временной шаг */
static void rSolveStep() {
    const UINT nNCW = (nNodesCount+2);
    const UINT nNCWEL = nNCW-1;
    const UINT nNCFull = nNCW*nNCW;
    const UINT nNCFullM = nNCFull-nNCW;

    fTime += fTimeStep;
    /* Swap Buffers */
    {
        T_REAL *pBuf = pDataLast;
        pDataLast = pDataOld;
        pDataOld = pBuf;
    }
    T_REAL fErr;
    do {
        fErr = 0;
        for ( UINT i = nNCW+1; i < nNCFullM; ++i ) {
            T_REAL fBuf = pDataLast[i];
            pDataLast[i] = ( fConst1 * pDataOld[i] ) + ( fConst2 * (
                    + pDataLast[i-1] /* Left */
                    + pDataLast[i+1] /* Right */
                    + pDataLast[i-nNCW] /* Bottom */
                    + pDataLast[i+nNCW] /* Top */
                ) );
            fErr += fBuf > pDataLast[i] ? fBuf - pDataLast[i] : pDataLast[i] - fBuf;
            if ( i%nNCW==nNCWEL-1 ) i+=2;
        }
    } while ( fErr > fMaxError );
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
    static UINT nScreenWidth = 0, nScreenHeight = 0;
    static UINT nScreenMin = 0;

    void _RePaint () {
        const UINT nNCW = (nNodesCount+2);
        const UINT nNCFull = nNCW*nNCW;

        const UINT nMul = nScreenMin / nNCW;

        for ( UINT iy = 0; iy < nNCW; ++iy )
        for ( UINT ix = 0; ix < nNCW; ++ix ) {
            const UINT j = ix*nMul+iy*nMul*4096;
            const T_REAL f = pDataLast[ix+iy*nNCW]*(T_REAL)6;
            if ( f < (T_REAL)1 ) {
                pBuf[j*3+0]=(BYTE)((f)*(T_REAL)255);
                pBuf[j*3+1]=0x00;
                pBuf[j*3+2]=0x00;
            } else
            if ( f < (T_REAL)2 ) {
                pBuf[j*3+0]=0xff;
                pBuf[j*3+1]=(BYTE)((f-(T_REAL)1)*(T_REAL)255);
                pBuf[j*3+2]=0x00;
            } else
            if ( f < (T_REAL)3 ) {
                pBuf[j*3+0]=(BYTE)(((T_REAL)3-f)*(T_REAL)255);
                pBuf[j*3+1]=0xff;
                pBuf[j*3+2]=0x00;
            } else
            if ( f < (T_REAL)4 ) {
                pBuf[j*3+0]=0x00;
                pBuf[j*3+1]=0xff;
                pBuf[j*3+2]=(BYTE)((f-(T_REAL)3)*(T_REAL)255);
            } else
            if ( f < (T_REAL)5 ) {
                pBuf[j*3+0]=0x00;
                pBuf[j*3+1]=(BYTE)(((T_REAL)5-f)*(T_REAL)255);
                pBuf[j*3+2]=0xff;
            } else
            {
                pBuf[j*3+0]=(BYTE)((f-(T_REAL)5)*(T_REAL)255);
                pBuf[j*3+1]=(BYTE)((f-(T_REAL)5)*(T_REAL)255);
                pBuf[j*3+2]=0xff;
            }
            if ( nMul > 1 ) {
                for ( UINT ky = 0; ky < nMul; ++ky )
                for ( UINT kx = 0; kx < nMul; ++kx ) {
                    pBuf[(j+kx+ky*4096)*3+0] = pBuf[j*3+0];
                    pBuf[(j+kx+ky*4096)*3+1] = pBuf[j*3+1];
                    pBuf[(j+kx+ky*4096)*3+2] = pBuf[j*3+2];
                }
            }
        }
    }

    void _RePaintBg () {
        for ( UINT iy = 0; iy < 4096; ++iy )
        for ( UINT ix = 0; ix < 4096; ++ix ) {
            const UINT j = ix+iy*4096;
            const UINT k = (((ix/8)^(iy/8))&1)?0xaf:0x70;
            pBuf[j*3+0]=k;
            pBuf[j*3+1]=k;
            pBuf[j*3+2]=k;
        }
    }

    switch ( uMsg ) {
        case WM_CREATE: {
            rInit();
            _hDC = GetWindowDC ( hWnd );
            hBackBufferDC = CreateCompatibleDC ( _hDC );
            BITMAPINFO bi;
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biBitCount = 24;
            bi.bmiHeader.biWidth = 4096;
            bi.bmiHeader.biHeight = -4096;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biPlanes = 1;
            hBackBuffer = CreateDIBSection ( hBackBufferDC, &bi, DIB_RGB_COLORS, (void**)&pBuf, NULL, 0 );
            return 0;
        }
        case WM_SIZE: {
            nScreenWidth = LOWORD(lParam);
            nScreenHeight = HIWORD(lParam);
            nScreenMin = nScreenWidth > nScreenHeight ? nScreenHeight : nScreenWidth;
            _RePaintBg();
            return 0;
        }
        case WM_DESTROY: {
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
            _RePaint();
            BitBlt ( hDC, 0, 0, nScreenWidth, nScreenHeight, hBackBufferDC, 0, 0, SRCCOPY);
            EndPaint ( hWnd, &ps );
            InvalidateRect ( hWnd, NULL, FALSE );
            Sleep ( 0 );
            rSolveStep();
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
