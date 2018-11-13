
/* Структура текстур */
typedef struct _S7BMP {
    /* Тип структуры */
    UINT    iType;
    /* Размеры текстуры */
    UINT    nWidth, nHeight;
    /* Количество байт на строку */
    UINT    nStride;
    /* Указатель на данные */
    VOID   *pData;

    /* Поля для GDI текстуры */
    HDC     hDC;
    HBITMAP hBMP;
} S7BMP;

enum {
    D7BMP_NULL = 0,
    /* Обычные A7 карты */
    D7BMP_A7_R,
    D7BMP_A7_RG,
    D7BMP_A7_RGB,
    D7BMP_A7_RGBA,
    /* GDI карты */
    D7BMP_GDI_RGB,
    D7BMP_GDI_RGBA,
};

/* Создание новой текстуры */
S7BMP *A7BmpCreate ( UINT iType, UINT nWidth, UINT nHeight, UINT nStride ) {
    if ( nStride == 0 ) {
        switch ( iType ) {
            case D7BMP_A7_R:        nStride = nWidth;       break;
            case D7BMP_A7_RG:       nStride = nWidth * 2;   break;
            case D7BMP_A7_RGB:      nStride = nWidth * 3;   break;
            case D7BMP_A7_RGBA:     nStride = nWidth * 4;   break;
            case D7BMP_GDI_RGB:     nStride = nWidth * 3;   break;
            case D7BMP_GDI_RGBA:    nStride = nWidth * 4;   break;
        }
        if ( nStride & 3 ) {
            nStride += 4 - ( nStride & 3 );
        }
    }
    /* Выделять ли память под сами данные */
    BOOL bMemData = FALSE;
    switch ( iType ) {
        case D7BMP_A7_R:        bMemData = TRUE;    break;
        case D7BMP_A7_RG:       bMemData = TRUE;    break;
        case D7BMP_A7_RGB:      bMemData = TRUE;    break;
        case D7BMP_A7_RGBA:     bMemData = TRUE;    break;
        case D7BMP_GDI_RGB:     bMemData = FALSE;   break;
        case D7BMP_GDI_RGBA:    bMemData = FALSE;   break;
    }

    S7BMP *o = ( S7BMP* ) malloc ( bMemData ? sizeof ( S7BMP ) + ( nStride * nHeight ) : sizeof ( S7BMP ) );
    memset ( o, 0, sizeof ( S7BMP ) );
    o -> iType   = iType;
    o -> nWidth  = nWidth;
    o -> nHeight = nHeight;
    o -> nStride = nStride;
    o -> pData   = ( BYTE* ) ( o ) + sizeof ( S7BMP );
    // o -> hDC     = NULL;
    // o -> hBMP    = NULL;
    return o;
}
/* Освобождение ресурсов текстуры */
VOID A7BmpFree ( S7BMP * p ) {
    if ( p -> hBMP != NULL ) DeleteObject ( p -> hBMP );
    if ( p -> hDC != NULL ) DeleteDC ( p -> hDC );
    free ( p );
}

/* Загрузка текстуры из Jpeg файла */
S7BMP *A7BmpCreateByJpegFileA ( LPCSTR szFileName, BOOL bBottomUp ) {
    FILE * infile = fopen ( szFileName, "rb" );
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error ( &jerr );
    jpeg_create_decompress ( &cinfo );
    jpeg_stdio_src ( &cinfo, infile );
    jpeg_read_header ( &cinfo, TRUE );
    cinfo.scale_num   = 1;
    cinfo.scale_denom = 1;
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress ( &cinfo );
    CONST UINT ow = cinfo.output_width;
    CONST UINT oh = cinfo.output_height;
    S7BMP *o = A7BmpCreate ( D7BMP_A7_RGB, ow, oh, 0 );
    while ( cinfo.output_scanline < oh ) {
        BYTE *pV = ( BYTE* ) ( o -> pData ) + ( ( bBottomUp ? ( oh - cinfo.output_scanline - 1 ) : cinfo.output_scanline ) * o -> nStride );
        jpeg_read_scanlines( &cinfo, ( JSAMPARRAY ) &pV, 1 );
        for ( UINT i = 0; i < ow; ++i ) {
            BYTE _ = pV [ i * 3 ];
            pV [ i * 3 ] = pV [ i * 3 + 2 ];
            pV [ i * 3 + 2 ] = _;
        }
    }
    jpeg_finish_decompress ( &cinfo );
    fclose ( infile );
    return o;
}
/* Создание текстуры в контексте GDI */
S7BMP *A7BmpCreateByGDI ( HDC hDC, UINT nWidth, UINT nHeight, BOOL bAlpha, BOOL bBottomUp ) {
    BITMAPINFO bmi;
    ZeroMemory ( &bmi, sizeof ( BITMAPINFO ) );
    bmi.bmiHeader.biSize        = sizeof ( BITMAPINFOHEADER ) ;
    bmi.bmiHeader.biWidth       = nWidth;
    bmi.bmiHeader.biHeight      = bBottomUp ? nHeight : ( -nHeight );
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = bAlpha ? 32 : 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = bAlpha ? nWidth * nHeight * 4 : 0;
    S7BMP *o  = A7BmpCreate ( bAlpha ? D7BMP_GDI_RGBA : D7BMP_GDI_RGB, nWidth, nHeight, 0 );
    o -> hDC  = CreateCompatibleDC ( hDC );
    o -> hBMP = CreateDIBSection ( o -> hDC, &bmi, DIB_RGB_COLORS, &( o -> pData ), NULL, 0x0 );
    SelectObject ( o -> hDC, o -> hBMP );
    return o;
}

/* Копирование части текстуры в другую текстуру */
VOID A7BmpCopy ( S7BMP * pDst, UINT nDX, UINT nDY, S7BMP * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH ) {
    UINT iBppDT = 0;
    switch ( pDst -> iType ) {
        case D7BMP_A7_R:     iBppDT = 1; break;
        case D7BMP_A7_RG:    iBppDT = 2; break;
        case D7BMP_A7_RGB:   iBppDT = 3; break;
        case D7BMP_A7_RGBA:  iBppDT = 4; break;
        case D7BMP_GDI_RGB:  iBppDT = 3; break;
        case D7BMP_GDI_RGBA: iBppDT = 4; break;
    }
    UINT iBppST = 0;
    switch ( pDst -> iType ) {
        case D7BMP_A7_R:     iBppST = 1; break;
        case D7BMP_A7_RG:    iBppST = 2; break;
        case D7BMP_A7_RGB:   iBppST = 3; break;
        case D7BMP_A7_RGBA:  iBppST = 4; break;
        case D7BMP_GDI_RGB:  iBppST = 3; break;
        case D7BMP_GDI_RGBA: iBppST = 4; break;
    }
    /* Если подобная / одинаковая структура */
    if ( iBppDT == iBppST ) {
        for ( UINT iy = 0; iy < nH; ++iy ) {
            memcpy ( ( BYTE* ) ( pDst -> pData ) + ( ( nDY + iy ) * pDst -> nStride ) + ( nDX * iBppDT ), ( BYTE* ) ( pSrc -> pData ) + ( ( nSY + iy ) * pSrc -> nStride ) + ( nSX * iBppST ), nW * iBppDT );
        }
    }
}

/* Рисует GDI текстуру на контекст */
VOID A7BmpGDI_Draw ( HDC hDC, UINT nDX, UINT nDY, S7BMP * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH ) {
    switch ( pSrc -> iType ) {
        case D7BMP_GDI_RGB:
            BitBlt ( hDC, nDX, nDY, nW, nH, pSrc -> hDC, nSX, nSY, SRCCOPY );
            break;
        case D7BMP_GDI_RGBA: {
            BLENDFUNCTION ftn = {
                .BlendOp                = AC_SRC_OVER,
                .BlendFlags             = 0,
                .SourceConstantAlpha    = 0xff,
                .AlphaFormat            = AC_SRC_ALPHA,
            };
            GdiAlphaBlend ( hDC, nDX, nDY, nW, nH, pSrc -> hDC, nSX, nSY, nW, nH, ftn );
            break;
        }
    }

}
