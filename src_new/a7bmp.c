
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
    /* Поля для глифа */
    INT     nAdvance;
    INT     nLeft;
    INT     nTop;

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
    /* Glyph Alpha карта */
    D7BMP_GLYPH,
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
        case D7BMP_GLYPH:       bMemData = FALSE;   break;
    }

    S7BMP *o = ( S7BMP* ) malloc ( bMemData ? sizeof ( S7BMP ) + ( nStride * nHeight ) : sizeof ( S7BMP ) );
    memset ( o, 0, sizeof ( S7BMP ) );
    o -> iType   = iType;
    o -> nWidth  = nWidth;
    o -> nHeight = nHeight;
    o -> nStride = nStride;
    o -> pData   = ( BYTE* ) ( o ) + sizeof ( S7BMP );
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
        case D7BMP_GLYPH:    iBppDT = 1; break;
    }
    UINT iBppST = 0;
    switch ( pDst -> iType ) {
        case D7BMP_A7_R:     iBppST = 1; break;
        case D7BMP_A7_RG:    iBppST = 2; break;
        case D7BMP_A7_RGB:   iBppST = 3; break;
        case D7BMP_A7_RGBA:  iBppST = 4; break;
        case D7BMP_GDI_RGB:  iBppST = 3; break;
        case D7BMP_GDI_RGBA: iBppST = 4; break;
        case D7BMP_GLYPH:    iBppST = 1; break;
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


/* Генерация маски вкладки */
S7BMP *A7BmpGen_Tab ( CONST UINT nH, CONST FLOAT _an, CONST FLOAT _r0, CONST FLOAT _r1 ) {

    // CONST UINT nH = 768;

    // CONST FLOAT _an = 1.0f;
    CONST FLOAT _cx = cosf ( _an );
    CONST FLOAT _cy = sinf ( _an );

    // CONST FLOAT _r0 = 333.0f;
    CONST FLOAT _x0 = 0.0f;
    CONST FLOAT _y0 = _r0;

    CONST FLOAT _xa = _x0 + _r0 * _cy;
    CONST FLOAT _ya = _y0 - _r0 * _cx;
    CONST FLOAT _ca = _cx * _xa + _cy * _ya;

    // CONST FLOAT _r1 = 123.0f;
    CONST FLOAT _y1 = (FLOAT)nH-_r1;

    CONST FLOAT _yb = _y1 + _r1 * _cx;
    CONST FLOAT _xb = ( _yb - _ya ) / _cy * _cx + _xa;

    CONST FLOAT _x1 = _xb + _r1 * _cy;
    CONST FLOAT _cb = _cx * _xb + _cy * _yb;

    CONST FLOAT _cc = _cx * _yb - _cy * _xb;

    CONST UINT nW = (UINT)_x1;



    S7BMP *o = A7BmpCreate ( D7BMP_A7_R, nW, nH, 0 );
    BYTE *pb = ( BYTE* ) o -> pData;
    for ( UINT iy = 0; iy < nH; ++iy ) {
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT i = iy * o -> nStride + ix;
            CONST FLOAT x = (FLOAT)(ix)+0.5f;
            CONST FLOAT y = (FLOAT)(nH-iy)-0.5f;


            CONST FLOAT la = _cx * x + _cy * y;

            /* Segment 1 - Circle [0] */
            if ( la < _ca ) {
                CONST FLOAT x0 = x - _x0;
                CONST FLOAT y0 = y - _y0;
                CONST FLOAT r0 = sqrtf ( x0 * x0 + y0 * y0 ) - _r0 + 0.5f;
                pb [ i ] = r0 < 0.0f ? 0x00 : r0 < 1.0f ? r0 * 255.0f : 0xff;
            } else
            /* Segment 3 - Circle [1] */
            if ( la > _cb ) {
                CONST FLOAT x0 = x - _x1;
                CONST FLOAT y0 = y - _y1;
                CONST FLOAT r0 = _r1 - sqrtf ( x0 * x0 + y0 * y0 ) + 0.5f;
                pb [ i ] = r0 < 0.0f ? 0x00 : r0 < 1.0f ? r0 * 255.0f : 0xff;
            } else
            /* Segment 2 - Line */
            {
                CONST FLOAT lb = _cy * x - _cx * y + _cc + 0.5f;
                pb [ i ] = lb < 0.0f ? 0x00 : lb < 1.0f ? lb * 255.0f : 0xff;

            }
        }
    }
    return o;
}
/* Генерация маски символа */
S7BMP *A7BmpLoad_Symbol ( S7BMP *o, FT_Face face, UINT symbol, UINT h, UINT x, UINT y ) {
    FT_Set_Char_Size ( face, 0, h, 96, 96 );
    FT_Vector pen = { .x = x, .y =-y, };
    FT_Set_Transform ( face, NULL, &pen );
    int err = FT_Load_Char ( face, symbol, FT_LOAD_RENDER );
    if ( err ) return NULL;
    FT_Bitmap *bmp = &face -> glyph -> bitmap;
    if ( o == NULL ) {
        o = A7BmpCreate ( D7BMP_GLYPH, bmp -> width, bmp -> rows, bmp -> pitch );
    } else {
        o -> nWidth     = bmp -> width;
        o -> nHeight    = bmp -> rows;
        o -> nStride    = bmp -> pitch;
    }
    o -> pData      = bmp -> buffer;
    o -> nAdvance   = face -> glyph -> advance.x;
    o -> nLeft      = face -> glyph -> bitmap_left;
    o -> nTop       =-face -> glyph -> bitmap_top;
    return o;
}








