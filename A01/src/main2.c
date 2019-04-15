#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/* Хендл окна */
static HWND g_hWnd;
/* Точность вычеслений */
typedef double T_REAL;
#define REAL_PRINT_TYPE "lf"
/* Количество узлов сетки */
static UINT nNCX = 128U, nNCY = 128U, nNCT = 4U, nNCF, nNCM;
/* Массивы данных температур узлов сетки для последнего временного слоя и предыдущего */
static T_REAL * pDL, * pDO, * pDN;
/* Квадрат максимальной ошибки вычесления 10^(-6) */
static T_REAL fEQ = 0.000001L * 0.000001L;
/* Расстояние между слоями по времени и узлами сетки */
static T_REAL fST, fSX, fSY;
/* Краевые усля */
static T_REAL rUI ( T_REAL x, T_REAL y ) { return 0.0; }
static T_REAL rUX0 ( T_REAL y ) { return 0.0; }
static T_REAL rUX1 ( T_REAL y ) { return 0.0; }
static T_REAL rUY0 ( T_REAL x ) { return 0.0; }
static T_REAL rUY1 ( T_REAL x ) { return sin ( M_PI * x ); }
/* Отношение сторон пластины X к Y */
static T_REAL fA = 1;
/* Константы которые помогут при расчётах */
static T_REAL fCX, fCY, fCZ, fCKX, fCKY, fCKZ;
/* Номер временного интервала и номер итерации */
static UINT nCT, cCI;

/* Подготовка данных для вычеслений */
static void rInit ( ) {
    fSX = ((T_REAL)(1)) / ((T_REAL)(nNCX-1));
    fSY = ((T_REAL)(1)) / ((T_REAL)(nNCY-1)) / fA;
    fST = ((T_REAL)(1)) / ((T_REAL)(nNCT-1));
    nNCF = nNCX * nNCY;
    nNCM = nNCF - nNCX;
    cCI = nCT = 0;
    pDL = (T_REAL*) malloc ( sizeof (T_REAL) * nNCF * 3 );
    pDO = pDL + nNCF;
    pDN = pDO + nNCF;
    /* Initial Temp */
    {
        T_REAL *pD1 = pDL, *pD2;
        for ( UINT j = 0; j < nNCY; ++j )
        for ( UINT i = 0; i < nNCX; ++i ) {
            *pD1 = rUI ( i*fSX, j*fSY ); ++pD1;
        }
        pD1 = pDL; pD2 = pDL + nNCM;
        for ( UINT i = 0; i < nNCX; ++i ) {
            *pD1 = rUY0 ( i*fSX ); ++pD1;
            *pD2 = rUY1 ( i*fSX ); ++pD2;
        }
        pD1 = pDL; pD2 = pDL + (nNCX-1);
        for ( UINT j = 0; j < nNCY; ++j ) {
            *pD1 = rUX0 ( j*fSY ); pD1+=nNCX;
            *pD2 = rUX1 ( j*fSY ); pD2+=nNCX;
        }
        memcpy ( pDO, pDL, sizeof (T_REAL) * nNCF );
    }
    fCKX = 1 / ( fSX * fSX );
    fCKY = ( fA * fA ) / ( fSY * fSY );
    fCKZ = ( 1 / fST ) + ( 2 * ( fCKX + fCKY ) );
    fCX = fCKX / fCKZ;
    fCY = fCKY / fCKZ;
    fCZ = 1 / ( fST * fCKZ );
}


static void rS0 ( ) {
    static T_REAL fErr = 0.0;

    if ( fErr < fEQ ) {
        /* Swap Buffers */
        { T_REAL *pBuf = pDL; pDL = pDO; pDO = pBuf; }
        memcpy ( pDL, pDO, sizeof (T_REAL) * nNCF );
        ++nCT;
        cCI = 0;
        InvalidateRect ( g_hWnd, NULL, FALSE );
    }

    fErr = 0;
    T_REAL fBuf;
    for ( UINT ij = nNCX+1; ij < nNCM; ++ij ) {
        fBuf = pDL[ij];
        pDL[ij] = ( fCZ * pDO[ij] ) +
            ( fCX * (
                + pDL[ij-1] /* Left */
                + pDL[ij+1] /* Right */
            ) + fCY * (
                + pDL[ij-nNCX] /* Bottom */
                + pDL[ij+nNCX] /* Top */
            ) );
        if(cCI%16==0) {
            pDN[ij] = ( pDL[ij] * fCKZ - fBuf / fST ) -
                ( fCKX * (
                    + pDL[ij-1] /* Left */
                    + pDL[ij+1] /* Right */
                ) + fCKY * (
                    + pDL[ij-nNCX] /* Bottom */
                    + pDL[ij+nNCX] /* Top */
                ) );
            InvalidateRect ( g_hWnd, NULL, FALSE );
        }
        fBuf -= pDL[ij];
        fErr += fBuf * fBuf;
        if ( ij%nNCX==nNCX-1 ) ij+=2;
    }
    ++cCI;
    CHAR pBuf[512];
    HDC hDC = GetWindowDC ( g_hWnd );
    TextOutA ( hDC, 0, 32, pBuf, sprintf ( pBuf, "fError: %.13" REAL_PRINT_TYPE, fErr ) );
    TextOutA ( hDC, 0, 48, pBuf, sprintf ( pBuf, "cCI: %u", cCI ) );
    ReleaseDC ( g_hWnd, hDC );
}


#define D_BMP_WIDTH 4096

static LRESULT CALLBACK
WndProc (
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {
    static HDC hBackBufferDC = NULL;
    static HBITMAP hBackBuffer = NULL;
    static BYTE * pBuf = NULL;
    static UINT nScreenWidth = 0, nScreenHeight = 0;
    static UINT nScreenMin = 0;
    UINT32 _GetColor ( T_REAL f ) { f *= 6;
        f<0?f=-f:0;
        return f<1?(((UINT32)(f*0xff))):
        f<2?(((UINT32)((f-1)*0xff))<<8)|0x000000ffU:
        f<3?(((UINT32)((3-f)*0xff))<<0)|0x0000ff00U:
        f<4?(((UINT32)((f-3)*0xff))<<16)|0x0000ff00U:
        f<5?(((UINT32)((5-f)*0xff))<<8)|0x00ff0000U:
        (((UINT32)((f-5)*0xff))*0x00000101U)|0x00ff0000U;
    }
    void _RP() {
        const UINT nMul = nScreenMin > nNCX ? nScreenMin / nNCX : 1;
        T_REAL fMin = 0, fMax = 0;
        T_REAL *p = pDN;
        for ( UINT iy = 0; iy < nNCY; ++iy )
        for ( UINT ix = 0; ix < nNCX; ++ix ) {
            fMin>p[ix+iy*nNCX]?fMin=p[ix+iy*nNCX]:
            fMax<p[ix+iy*nNCX]?fMax=p[ix+iy*nNCX]:0;
        }

        {
            CHAR pBuf[512];
            HDC hDC = GetWindowDC ( g_hWnd );
            TextOutA ( hDC, 0, 128, pBuf, sprintf ( pBuf, "fMin: %.13" REAL_PRINT_TYPE, fMin ) );
            TextOutA ( hDC, 0, 128+16, pBuf, sprintf ( pBuf, "fMax: %.13" REAL_PRINT_TYPE, fMax ) );
            ReleaseDC ( g_hWnd, hDC );
        }


        for ( UINT iy = 0; iy < nNCY; ++iy )
        for ( UINT ix = 0; ix < nNCX; ++ix ) {
            const UINT _ = (ix*nMul+(D_BMP_WIDTH-iy*nMul-1)*D_BMP_WIDTH);
            ((UINT32*)pBuf)[_]=_GetColor((p[ix+iy*nNCX]-fMin)/(fMax-fMin));
            if ( nMul > 1 ) {
                for ( UINT ky = 0; ky < nMul; ++ky )
                for ( UINT kx = 0; kx < nMul; ++kx ) {
                    const UINT __ = _+kx-ky*D_BMP_WIDTH;
                    ((UINT32*)pBuf)[__]=((UINT32*)pBuf)[_];
                }
            }
        }
    }

    void _RePaintBg () {
        for ( UINT iy = 0; iy < D_BMP_WIDTH; ++iy )
        for ( UINT ix = 0; ix < D_BMP_WIDTH; ++ix ) {
            const UINT _ = (ix+iy*D_BMP_WIDTH);
            const UINT32 k = (((ix/8)^(iy/8))&1)?0xafafbf:0x807070;
            ((UINT32*)pBuf)[_]=k;
        }
    }

    switch ( uMsg ) {
        case WM_CREATE: {
            rInit();
            HDC hDC = GetWindowDC ( hWnd );
            hBackBufferDC = CreateCompatibleDC ( hDC );
            ReleaseDC ( hWnd, hDC );
            BITMAPINFO bi;
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biWidth = D_BMP_WIDTH;
            bi.bmiHeader.biHeight = D_BMP_WIDTH;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biSizeImage = 4*D_BMP_WIDTH*D_BMP_WIDTH;
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
            PostQuitMessage ( 0 );
            return 0;
        }
        case WM_PAINT: {
            _RP();
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );
            SelectObject ( hBackBufferDC, hBackBuffer );
            BitBlt ( hDC, 0, 0, nScreenWidth, nScreenHeight, hBackBufferDC, 0, 0, SRCCOPY );
            EndPaint ( hWnd, &ps );
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
        WNDCLASSEX wc = {
            .cbSize        = sizeof ( WNDCLASSEX ),
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
    g_hWnd = CreateWindowEx ( 0, L"SOME_CLASS_NAME", NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

    {
        CHAR str[512];
        sprintf ( str, "data/_%ux%u__end.bin", nNCX, nNCY );
        FILE *pF = fopen ( str, "wb" );
        T_REAL fBuf;

        for ( UINT ij = 0; ij < nNCF; ++ij ) {
            CONST T_REAL x = (ij%nNCX)*fSX;
            CONST T_REAL y = (ij/nNCX)*fSY;
            fBuf = ( exp(M_PI-M_PI*y)*(-1+exp(2*M_PI*y))/(-1+exp(2*M_PI))*sin(M_PI*x));
            fwrite ( &fBuf, sizeof ( T_REAL ), 1, pF );
        }
        fclose ( pF );
    }


    MSG msg = { };
    while ( TRUE ) {
        while ( !PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
            rS0();
        }
        if ( !GetMessage ( &msg, NULL, 0, 0 ) ) break;
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }
    return msg.wParam;
}

