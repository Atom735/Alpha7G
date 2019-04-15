#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Точность вычеслений */
typedef double T_REAL;
#define REAL_PRINT_TYPE "lf"
/* Количество узлов сетки */
static UINT nNodesCountX = 128U;
static UINT nNodesCountY = 128U;
static UINT nNodesCountT = 4U;
/* Массивы данных температур узлов сетки для последнего временного слоя и предыдущего */
static T_REAL * pDataLast;
static T_REAL * pDataOld;
/* Максимальное значение ошибки вычесления 10^(-6) */
static T_REAL fMaxError = 0.000001L * 0.000001L;
/* Расстояние между слоями по времени */
static T_REAL fTimeStep;
/* Расстояние между узлами сетки */
static T_REAL fGridStepX;
static T_REAL fGridStepY;
/* Краевые условия */
static T_REAL fTempInitial ( T_REAL x, T_REAL y ) { return 0.0; }
static T_REAL fTempLeft ( T_REAL y ) { return 0.0; }
static T_REAL fTempRight ( T_REAL y ) { return 0.0; }
static T_REAL fTempTop ( T_REAL x ) { return sin(M_PI*x); }
static T_REAL fTempBottom ( T_REAL x ) { return 0.0; }
/* Отношение сторон пластины X к Y */
static T_REAL fAspect = 1.0L;
/* Количество узлов в памяти */
static UINT nNCX, nNCY, nNCF, nNCM;
/* Константы которые помогут при расчётах */
static T_REAL fConstX, fConstY, fConstZ;
static UINT nTimeStep;
/* Подготовка данных для вычеслений */
static void rInit ( ) {
    fGridStepX = ((T_REAL)(1)) / ((T_REAL)(nNodesCountX-1));
    fGridStepY = ((T_REAL)(1)) / ( fAspect * ((T_REAL)(nNodesCountY-1)) );
    fTimeStep = ((T_REAL)(1)) / ((T_REAL)(nNodesCountT-1));
    nNCX = nNodesCountX + 2;
    nNCY = nNodesCountY + 2;
    nNCF = nNCX * nNCY;
    nNCM = nNCF - nNCX;
    nTimeStep = 0;
    pDataLast = (T_REAL*) malloc ( sizeof (T_REAL) * nNCF * 2 );
    pDataOld = pDataLast + nNCF;
    /* Initial Temp */
    {
        T_REAL *pD1 = pDataLast, *pD2;
        for ( UINT j = 0; j < nNCY; ++j )
        for ( UINT i = 0; i < nNCX; ++i ) {
            *pD1 = fTempInitial ( i*fGridStepX, j*fGridStepY ); ++pD1;
        }
        pD1 = pDataLast; pD2 = pDataLast + (nNCY-1)*nNCX;
        for ( UINT i = 0; i < nNCX; ++i ) {
            *pD1 = fTempBottom ( i*fGridStepX ); ++pD1;
            *pD2 = fTempTop ( i*fGridStepX ); ++pD2;
        }
        pD1 = pDataLast; pD2 = pDataLast + (nNCX-1);
        for ( UINT j = 0; j < nNCY; ++j ) {
            *pD1 = fTempLeft ( j*fGridStepY ); pD1+=nNCX;
            *pD2 = fTempRight ( j*fGridStepY ); pD2+=nNCX;
        }
        memcpy ( pDataOld, pDataLast, sizeof (T_REAL) * nNCF );
    }
    fConstX = ((T_REAL)(1)) / ( fGridStepX * fGridStepX );
    fConstY = ( fAspect * fAspect ) / ( fGridStepY * fGridStepY );
    fConstZ = (((T_REAL)(1)) / fTimeStep ) + (((T_REAL)(2)) * ( fConstX + fConstY ));
    fConstX /= fConstZ;
    fConstY /= fConstZ;
    fConstZ = ((T_REAL)(1)) / ( fTimeStep * fConstZ );
}


static T_REAL fDeltaNorma;
static T_REAL fDeltaAbs;
/* Расчёт временного слоя */
static void rSolveStep ( HDC hDC ) {
    /* Swap Buffers */
    { T_REAL *pBuf = pDataLast; pDataLast = pDataOld; pDataOld = pBuf; }
    T_REAL fErr;
    T_REAL fBuf;
    do {
        fErr = 0;
        for ( UINT ij = nNCX+1; ij < nNCM; ++ij ) {
            fBuf = pDataLast[ij];
            pDataLast[ij] = ( fConstZ * pDataOld[ij] ) +
                ( fConstX * (
                    + pDataLast[ij-1] /* Left */
                    + pDataLast[ij+1] /* Right */
                ) + fConstY * (
                    + pDataLast[ij-nNCX] /* Bottom */
                    + pDataLast[ij+nNCX] /* Top */
                ) );
            fBuf -= pDataLast[ij];
            fErr += fBuf * fBuf;
            if ( ij%nNCX==nNodesCountX ) ij+=2;
        }
        CHAR pBuf[512];
        TextOutA ( hDC, 0, 16, pBuf, sprintf ( pBuf, "fError: %.13" REAL_PRINT_TYPE, fErr ) );
        Sleep ( 0 );
    } while ( fErr > fMaxError );

    fDeltaNorma = 0;
    for ( UINT ij = nNCX+1; ij < nNCM; ++ij ) {
        fBuf = pDataLast[ij] - pDataOld[ij];
        fDeltaNorma += fBuf * fBuf;
        if ( ij%nNCX==nNodesCountX ) ij+=2;
    }

    fDeltaAbs = 0;
    for ( UINT ij = nNCX+1; ij < nNCM; ++ij ) {
        T_REAL x = (ij%nNCX)/(T_REAL)(nNCX-1);
        T_REAL y = (ij/nNCX)/(T_REAL)(nNCY-1);
        fBuf = (exp(M_PI-M_PI*y)*(-1+exp(2*M_PI*y))/(-1+exp(2*M_PI))*sin(M_PI*x));
        fBuf -= pDataLast[ij];
        fDeltaAbs += fBuf * fBuf;
        if ( ij%nNCX==nNodesCountX ) ij+=2;
    }
}



/*
fd = mopen ( "F:\projects\Alpha7G-master\A01\data_128x128_4__1.bin", 'rb' );
data = matrix ( mget ( 128*128, 'd', fd ), 128, 128 );
data_x = 1:128;
clf()
gcf().color_map = jetcolormap ( 64 );
colorbar (0,1)
Sgrayplot ( data_x, data_x, data, strf="041" )

fd = mopen ( "F:\projects\Alpha7G-master\A01\data_128x128_4__33.bin", 'rb' );
data = matrix ( mget ( 128*128, 'd', fd ), 128, 128 );
data_x = 1:128;
clf()
gcf().color_map = jetcolormap ( 64 );
colorbar (0,1)
Sgrayplot ( data_x, data_x, data, strf="041" )

fd = mopen ( "F:\projects\Alpha7G-master\A01\data_128x128__end.bin", 'rb' );
data_end = matrix ( mget ( 128*128, 'd', fd ), 128, 128 );
data_delta = data_end - data;
clf()
gcf().color_map = jetcolormap ( 64 );
colorbar (0,1)
Sgrayplot ( data_x, data_x, data_delta, strf="041" )

contour2d (  data_x, data_x, data_delta, 10 )

*/
static void rSolveSave () {
    CHAR f_name[512];
    sprintf ( f_name, "data_%ux%u_%u__%u.bin", nNodesCountX, nNodesCountY, nNodesCountT, nTimeStep );
    FILE *pF = fopen ( f_name, "wb" );
    for ( UINT i = 1; i < nNCY-1; ++i )
        fwrite ( pDataLast+(i*nNCX+1), sizeof ( T_REAL ), nNodesCountX, pF );
    fclose ( pF );
}

#define D_BMP_WIDTH 4096

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
        const UINT nMul = nScreenMin > nNCX ? nScreenMin / nNCX : 1;
        for ( UINT iy = 0; iy < nNCY; ++iy )
        for ( UINT ix = 0; ix < nNCX; ++ix ) {
            const UINT j = ix*nMul+iy*nMul*D_BMP_WIDTH;
            const T_REAL f = pDataLast[ix+iy*nNCX]*(T_REAL)6;
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
                    const UINT _ = (j+kx+ky*D_BMP_WIDTH)*3;
                    pBuf[_+0] = pBuf[j*3+0];
                    pBuf[_+1] = pBuf[j*3+1];
                    pBuf[_+2] = pBuf[j*3+2];
                }
            }
        }
    }

    void _RePaintBg () {
        for ( UINT iy = 0; iy < D_BMP_WIDTH; ++iy )
        for ( UINT ix = 0; ix < D_BMP_WIDTH; ++ix ) {
            const UINT _ = (ix+iy*D_BMP_WIDTH)*3;
            const UINT k = (((ix/8)^(iy/8))&1)?0xaf:0x70;
            pBuf[_+0]=k;
            pBuf[_+1]=k;
            pBuf[_+2]=k;
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
            bi.bmiHeader.biWidth = D_BMP_WIDTH;
            bi.bmiHeader.biHeight = -D_BMP_WIDTH;
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
            CHAR pBuf[512];
            TextOutA ( hDC, 0, 0, pBuf, sprintf ( pBuf, "fDeltaNorma: %.13" REAL_PRINT_TYPE, fDeltaNorma ) );
            TextOutA ( hDC, 0, 32, pBuf, sprintf ( pBuf, "nTimeStep: %d", nTimeStep ) );
            TextOutA ( hDC, 0, 48, pBuf, sprintf ( pBuf, "fTime: %.13" REAL_PRINT_TYPE, nTimeStep*fTimeStep ) );
            TextOutA ( hDC, 0, 64, pBuf, sprintf ( pBuf, "fDeltaAbs: %.13" REAL_PRINT_TYPE, fDeltaAbs ) );
            ++nTimeStep;
            rSolveStep ( hDC );
            rSolveSave();
            EndPaint ( hWnd, &ps );
            InvalidateRect ( hWnd, NULL, FALSE );
            Sleep ( 0 );
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
        CW_USEDEFAULT, CW_USEDEFAULT, 9999, 9999, NULL, NULL, hInstance, NULL );

    CHAR f_name[512];
    sprintf ( f_name, "data_%ux%u__end.bin", nNodesCountX, nNodesCountY );
    FILE *pF = fopen ( f_name, "wb" );
    T_REAL fBuf;
    for ( UINT ij = nNCX+1; ij < nNCM; ++ij ) {
        T_REAL x = (ij%nNCX)/(T_REAL)(nNCX-1);
        T_REAL y = (ij/nNCX)/(T_REAL)(nNCY-1);
        fBuf = (exp(M_PI-M_PI*y)*(-1+exp(2*M_PI*y))/(-1+exp(2*M_PI))*sin(M_PI*x));
        fwrite ( &fBuf, sizeof ( T_REAL ), 1, pF );
        if ( ij%nNCX==nNodesCountX ) ij+=2;
    }
    fclose ( pF );



    MSG msg = { };
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }

    return msg.wParam;
}

