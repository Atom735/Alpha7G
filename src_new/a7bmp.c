#include <Windows.h>

#include <Math.h>
#include <StdLib.h>
#include <StdIO.h>

#include <JpegLib.h>

#include "a7bmp.h"

/* Возвращает количество байт на тексель от типа текстуры */
static inline UINT A7BmpBpp ( UINT iType ) {
    switch ( iType ) {
        case D7BMP_A7_R:        return 1;
        case D7BMP_A7_RG:       return 2;
        case D7BMP_A7_RGB:      return 3;
        case D7BMP_A7_RGBA:     return 4;
        case D7BMP_GDI_RGB:     return 3;
        case D7BMP_GDI_RGBA:    return 4;
        case D7BMP_GLYPH:       return 1;
    }
    return 0;
}

/* Копирование части текстуры в другую текстуру */
VOID A7BmpCopy ( S7Bmp * pDst, UINT nDX, UINT nDY, S7Bmp * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH ) {
    CONST UINT iBppDT = A7BmpBpp ( pDst -> iType );
    CONST UINT iBppST = A7BmpBpp ( pSrc -> iType );
    /* Если подобная / одинаковая структура */
    if ( iBppDT == iBppST ) {
        for ( UINT iy = 0; iy < nH; ++iy ) {
            memcpy ( ( BYTE* ) ( pDst -> pData ) + ( ( nDY + iy ) * pDst -> nStride ) + ( nDX * iBppDT ), ( BYTE* ) ( pSrc -> pData ) + ( ( nSY + iy ) * pSrc -> nStride ) + ( nSX * iBppST ), nW * iBppDT );
        }
    }
}
/* Копирование максимально большей части текстуры в другую текстуру */
VOID A7BmpCopyFull ( S7Bmp * pDst, UINT nDX, UINT nDY, S7Bmp * pSrc ) {
    A7BmpCopy ( pDst, nDX, nDY, pSrc, 0, 0, __min ( pSrc -> nWidth, pDst -> nWidth - nDX ), __min ( pSrc -> nHeight, pDst -> nHeight - nDY ) );
}

/* Создание новой текстуры */
S7Bmp *A7BmpCreate ( UINT iType, UINT nWidth, UINT nHeight, UINT nStride, VOID *pData ) {
    if ( nStride == 0 ) {
        nStride = nWidth * A7BmpBpp ( iType );
        if ( nStride & 3 ) {
            nStride += 4 - ( nStride & 3 );
        }
    }
    S7Bmp *o = ( S7Bmp* ) malloc ( ( pData == NULL ) ? sizeof ( S7Bmp ) + ( nStride * nHeight ) : sizeof ( S7Bmp ) );
    memset ( o, 0, sizeof ( S7Bmp ) );
    o -> iType   = iType;
    o -> nWidth  = nWidth;
    o -> nHeight = nHeight;
    o -> nStride = nStride;
    o -> pData   = ( pData == NULL ) ? ( BYTE* ) ( o ) + sizeof ( S7Bmp ) : pData;
    return o;
}
/* Освобождение ресурсов текстуры */
VOID A7BmpFree ( S7Bmp * p ) {
    if ( p -> hBMP != NULL ) DeleteObject ( p -> hBMP );
    if ( p -> hDC  != NULL ) DeleteDC ( p -> hDC );
    free ( p );
}

/* Загрузка текстуры из Jpeg файла */
S7Bmp *A7BmpCreateByJpegFileA ( LPCSTR szFileName ) {
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
    S7Bmp *o = A7BmpCreate ( D7BMP_A7_RGB, ow, oh, 0, NULL );
    while ( cinfo.output_scanline < oh ) {
        BYTE *pV = ( BYTE* ) ( o -> pData ) + ( cinfo.output_scanline * o -> nStride );
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
S7Bmp *A7BmpCreateByGDI ( HDC hDC, UINT nWidth, UINT nHeight, BOOL bAlpha ) {
    BITMAPINFO bmi;
    ZeroMemory ( &bmi, sizeof ( BITMAPINFO ) );
    bmi.bmiHeader.biSize        = sizeof ( BITMAPINFOHEADER ) ;
    bmi.bmiHeader.biWidth       = nWidth;
    bmi.bmiHeader.biHeight      = -nHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = bAlpha ? 32 : 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = bAlpha ? nWidth * nHeight * 4 : 0;
    S7Bmp *o = A7BmpCreate ( bAlpha ? D7BMP_GDI_RGBA : D7BMP_GDI_RGB, nWidth, nHeight, 0, ( VOID* ) 0x100 );
    o -> hDC  = CreateCompatibleDC ( hDC );
    o -> hBMP = CreateDIBSection ( o -> hDC, &bmi, DIB_RGB_COLORS, &( o -> pData ), NULL, 0x0 );
    SelectObject ( o -> hDC, o -> hBMP );
    return o;
}
/* Создание маски символа ( является временной ссылкой ) */
S7Bmp *A7BmpCreateByFreeType ( S7Bmp *o, FT_Face ftFace, UINT iUnicode, UINT nGlyphHeight, UINT nGlyphOffsetX ) {
    FT_Set_Char_Size ( ftFace, 0, nGlyphHeight, 96, 96 );
    FT_Vector pen = { .x = nGlyphOffsetX, .y = 0, };
    FT_Set_Transform ( ftFace, NULL, &pen );
    int err = FT_Load_Char ( ftFace, iUnicode, FT_LOAD_RENDER );
    if ( err ) return NULL;
    FT_Bitmap *bmp = &ftFace -> glyph -> bitmap;
    if ( o == NULL ) {
        o = A7BmpCreate ( D7BMP_GLYPH, bmp -> width, bmp -> rows, bmp -> pitch, bmp -> buffer );
    } else {
        o -> pData      = bmp -> buffer;
        o -> nWidth     = bmp -> width;
        o -> nHeight    = bmp -> rows;
        o -> nStride    = bmp -> pitch;
    }
    o -> nAdvance   = ftFace -> glyph -> advance.x;
    o -> nLeft      = ftFace -> glyph -> bitmap_left;
    o -> nTop       =-ftFace -> glyph -> bitmap_top;
    return o;
}
/* Создание копии текстуры */
S7Bmp *A7BmpCreateByCopy ( S7Bmp *pSrc, UINT iType ) {
    S7Bmp *o = A7BmpCreate ( iType, pSrc -> nWidth, pSrc -> nHeight, pSrc -> nStride, NULL );
    A7BmpCopyFull ( o, 0, 0, pSrc );
    return o;
}

/* Рисует часть GDI текстуры на контекст */
VOID A7BmpDraw_GDI ( HDC hDC, UINT nDX, UINT nDY, S7Bmp * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH ) {
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
/* Рисует максимально наибольшую часть GDI текстуры на контекст */
VOID A7BmpDrawFull_GDI ( HDC hDC, UINT nDX, UINT nDY, S7Bmp * pSrc ) {
    A7BmpDraw_GDI ( hDC, nDX, nDY, pSrc, 0, 0, pSrc -> nWidth, pSrc -> nHeight );
}

/* Генерация маски вкладки */
S7Bmp *A7BmpGen_Tab ( CONST UINT nH, CONST FLOAT _an, CONST FLOAT _r0, CONST FLOAT _r1 ) {
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
    CONST FLOAT _y1 = ( FLOAT ) ( nH ) -_r1;
    CONST FLOAT _yb = _y1 + _r1 * _cx;
    CONST FLOAT _xb = ( _yb - _ya ) / _cy * _cx + _xa;
    CONST FLOAT _x1 = _xb + _r1 * _cy;
    CONST FLOAT _cb = _cx * _xb + _cy * _yb;
    CONST FLOAT _cc = _cx * _yb - _cy * _xb;
    CONST UINT nW = ( UINT ) ( _x1 );
    S7Bmp *o = A7BmpCreate ( D7BMP_A7_R, nW, nH, 0, NULL );
    BYTE *pb = ( BYTE* ) o -> pData;
    for ( UINT iy = 0; iy < nH; ++iy ) {
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT i = iy * o -> nStride + ix;
            CONST FLOAT x = (FLOAT)(ix)+0.5f;
            CONST FLOAT y = (FLOAT)(nH-iy)-0.5f;
            CONST FLOAT la = _cx * x + _cy * y;
            if ( la < _ca ) {
                /* Segment 1 - Circle [0] */
                CONST FLOAT x0 = x - _x0;
                CONST FLOAT y0 = y - _y0;
                CONST FLOAT r0 = sqrtf ( x0 * x0 + y0 * y0 ) - _r0 + 0.5f;
                pb [ i ] = r0 < 0.0f ? 0x00 : r0 < 1.0f ? ( BYTE ) ( r0 * 255.0f ) : 0xff;
            } else if ( la > _cb ) {
                /* Segment 3 - Circle [1] */
                CONST FLOAT x0 = x - _x1;
                CONST FLOAT y0 = y - _y1;
                CONST FLOAT r0 = _r1 - sqrtf ( x0 * x0 + y0 * y0 ) + 0.5f;
                pb [ i ] = r0 < 0.0f ? 0x00 : r0 < 1.0f ? ( BYTE ) ( r0 * 255.0f ) : 0xff;
            } else {
                /* Segment 2 - Line */
                CONST FLOAT lb = _cy * x - _cx * y + _cc + 0.5f;
                pb [ i ] = lb < 0.0f ? 0x00 : lb < 1.0f ? ( BYTE ) ( lb * 255.0f ) : 0xff;
            }
        }
    }
    return o;
}


/* Рисует прямоугольник с указаным цветом на цветовую карту */
VOID A7BmpDrawRect ( S7Bmp *pDst, UINT nX, UINT nY, UINT nW, UINT nH, BYTE cR, BYTE cG, BYTE cB ) {
    CONST UINT bpp = A7BmpBpp ( pDst -> iType );
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = ( iy + nY );
        if ( idy >= 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ( ix + nX );
            if ( idx >= 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * bpp;
            if ( bpp >= 1 ) pbd [ id + 0 ] = cB;
            if ( bpp >= 2 ) pbd [ id + 1 ] = cG;
            if ( bpp >= 3 ) pbd [ id + 2 ] = cR;
            if ( bpp >= 4 ) pbd [ id + 3 ] = 0xff;
        }
    }
}
VOID A7BmpDrawRectA ( S7Bmp *pDst, UINT nX, UINT nY, UINT nW, UINT nH, BYTE cR, BYTE cG, BYTE cB, BYTE cA ) {
    CONST UINT bpp = A7BmpBpp ( pDst -> iType );
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = ( iy + nY );
        if ( idy >= 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ( ix + nX );
            if ( idx >= 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * bpp;
            if ( bpp >= 1 ) pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - cA ) + cB * cA ) / 0xff;
            if ( bpp >= 2 ) pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - cA ) + cG * cA ) / 0xff;
            if ( bpp >= 3 ) pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - cA ) + cR * cA ) / 0xff;
            if ( bpp >= 4 ) pbd [ id + 3 ] = ( pbd [ id + 3 ] * ( 0xff - cA ) + cA * 0xff ) / 0xff;
        }
    }
}

/* Рисует маску с указаным цветом на цветовую карту */
VOID A7BmpDrawAlphaMap ( S7Bmp *pDst, S7Bmp *pSrc, UINT nX, UINT nY, BYTE cR, BYTE cG, BYTE cB, BOOL bMirrorX ) {
    CONST UINT bpp = A7BmpBpp ( pDst -> iType );
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    BYTE *pbs = ( BYTE* ) pSrc -> pData;
    for ( UINT iy = 0; iy < pSrc -> nHeight; ++iy ) {
        CONST UINT idy = ( iy + nY + pSrc -> nTop );
        if ( idy >= 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < pSrc -> nWidth; ++ix ) {
            CONST UINT idx = ( ix + nX + pSrc -> nLeft );
            if ( idx >= 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 4;
            CONST UINT is = ( iy ) * pSrc -> nStride + ( bMirrorX ? ( pSrc -> nWidth - ix - 1 ) : ix );
            if ( bpp >= 1 ) pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( ( 0xff ) - pbs [ is ] ) + cB * pbs [ is ] ) / ( 0xff );
            if ( bpp >= 2 ) pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( ( 0xff ) - pbs [ is ] ) + cG * pbs [ is ] ) / ( 0xff );
            if ( bpp >= 3 ) pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( ( 0xff ) - pbs [ is ] ) + cR * pbs [ is ] ) / ( 0xff );
            if ( bpp >= 4 ) pbd [ id + 3 ] = ( pbd [ id + 3 ] * ( ( 0xff ) - pbs [ is ] ) + 0xff * pbs [ is ] ) / ( 0xff );
        }
    }
}
VOID A7BmpDrawAlphaMapA ( S7Bmp *pDst, S7Bmp *pSrc, UINT nX, UINT nY, BYTE cR, BYTE cG, BYTE cB, BYTE cA, BOOL bMirrorX ) {
    CONST UINT bpp = A7BmpBpp ( pDst -> iType );
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    BYTE *pbs = ( BYTE* ) pSrc -> pData;
    for ( UINT iy = 0; iy < pSrc -> nHeight; ++iy ) {
        CONST UINT idy = ( iy + nY + pSrc -> nTop );
        if ( idy >= 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < pSrc -> nWidth; ++ix ) {
            CONST UINT idx = ( ix + nX + pSrc -> nLeft );
            if ( idx >= 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 4;
            CONST UINT is = ( iy ) * pSrc -> nStride + ( bMirrorX ? ( pSrc -> nWidth - ix - 1 ) : ix );
            if ( bpp >= 1 ) pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( ( 0xff * 0xff ) - pbs [ is ] * cA ) + cB * pbs [ is ] * cA ) / ( 0xff * 0xff );
            if ( bpp >= 2 ) pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( ( 0xff * 0xff ) - pbs [ is ] * cA ) + cG * pbs [ is ] * cA ) / ( 0xff * 0xff );
            if ( bpp >= 3 ) pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( ( 0xff * 0xff ) - pbs [ is ] * cA ) + cR * pbs [ is ] * cA ) / ( 0xff * 0xff );
            if ( bpp >= 4 ) pbd [ id + 3 ] = ( pbd [ id + 3 ] * ( ( 0xff * 0xff ) - pbs [ is ] * cA ) + 0xff * pbs [ is ] * cA ) / ( 0xff * 0xff );
        }
    }
}

/* Получает длину строки в текселях */
UINT A7BmpGetStringWidth ( FT_Face ftFace, CONST CHAR *pStr, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking ) {
    FT_Set_Char_Size ( ftFace, 0, nGlyphHeight, 96, 96 );
    UINT x = nGlyphOffsetX;
    CHAR CONST *ch = pStr;
    while ( *ch ) {
        FT_Vector pen = { .x = x, .y = 0, };
        FT_Set_Transform ( ftFace, NULL, &pen );
        int err = FT_Load_Char ( ftFace, *ch, FT_LOAD_DEFAULT );
        if ( err ) return 0;
        x += ftFace -> glyph -> advance.x + nTracking;
        ++ch;
    }
    return x;
}
UINT A7BmpGetStringWidthW ( FT_Face ftFace, CONST WCHAR *pStr, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking ) {
    FT_Set_Char_Size ( ftFace, 0, nGlyphHeight, 96, 96 );
    UINT x = nGlyphOffsetX;
    WCHAR CONST *ch = pStr;
    while ( *ch ) {
        FT_Vector pen = { .x = x, .y = 0, };
        FT_Set_Transform ( ftFace, NULL, &pen );
        int err = FT_Load_Char ( ftFace, *ch, FT_LOAD_DEFAULT );
        if ( err ) return 0;
        x += ftFace -> glyph -> advance.x + nTracking;
        ++ch;
    }
    return x;
}

UINT A7BmpGetStringWidthU8 ( FT_Face ftFace, CONST BYTE *pStr, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking ) {
    FT_Set_Char_Size ( ftFace, 0, nGlyphHeight, 96, 96 );
    UINT x = nGlyphOffsetX;
    UINT iUnicode = 0;
    BYTE CONST *ch = pStr;
    while ( ( ch = A7UnicodeByUTF8 ( &iUnicode, ch ) ) != NULL ) {
        FT_Vector pen = { .x = x, .y = 0, };
        FT_Set_Transform ( ftFace, NULL, &pen );
        int err = FT_Load_Char ( ftFace, iUnicode, FT_LOAD_DEFAULT );
        if ( err ) return 0;
        x += ftFace -> glyph -> advance.x + nTracking;
    }
    return x;
}

/* Рисует строку на текстуру */
VOID A7BmpDrawText ( S7Bmp *pDst, FT_Face ftFace, CONST CHAR *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB ) {
    S7Bmp *bmpGlyph = NULL;
    UINT x = nGlyphOffsetX;
    CHAR CONST *ch = pStr;
    while ( *ch ) {
        bmpGlyph = A7BmpCreateByFreeType ( bmpGlyph, ftFace, *ch, nGlyphHeight, x );
        A7BmpDrawAlphaMap ( pDst, bmpGlyph, nX, nY, cR, cG, cB, FALSE );
        x += bmpGlyph -> nAdvance + nTracking;
        ++ch;
    }
}
VOID A7BmpDrawTextA ( S7Bmp *pDst, FT_Face ftFace, CONST CHAR *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB, BYTE cA ) {
    S7Bmp *bmpGlyph = NULL;
    UINT x = nGlyphOffsetX;
    CHAR CONST *ch = pStr;
    while ( *ch ) {
        bmpGlyph = A7BmpCreateByFreeType ( bmpGlyph, ftFace, *ch, nGlyphHeight, x );
        A7BmpDrawAlphaMapA ( pDst, bmpGlyph, nX, nY, cR, cG, cB, cA, FALSE );
        x += bmpGlyph -> nAdvance + nTracking;
        ++ch;
    }
}

VOID A7BmpDrawTextW ( S7Bmp *pDst, FT_Face ftFace, CONST WCHAR *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB ) {
    S7Bmp *bmpGlyph = NULL;
    UINT x = nGlyphOffsetX;
    WCHAR CONST *ch = pStr;
    while ( *ch ) {
        bmpGlyph = A7BmpCreateByFreeType ( bmpGlyph, ftFace, *ch, nGlyphHeight, x );
        A7BmpDrawAlphaMap ( pDst, bmpGlyph, nX, nY, cR, cG, cB, FALSE );
        x += bmpGlyph -> nAdvance + nTracking;
        ++ch;
    }
}
VOID A7BmpDrawTextWA ( S7Bmp *pDst, FT_Face ftFace, CONST WCHAR *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB, BYTE cA ) {
    S7Bmp *bmpGlyph = NULL;
    UINT x = nGlyphOffsetX;
    WCHAR CONST *ch = pStr;
    while ( *ch ) {
        bmpGlyph = A7BmpCreateByFreeType ( bmpGlyph, ftFace, *ch, nGlyphHeight, x );
        A7BmpDrawAlphaMapA ( pDst, bmpGlyph, nX, nY, cR, cG, cB, cA, FALSE );
        x += bmpGlyph -> nAdvance + nTracking;
        ++ch;
    }
}
VOID A7BmpDrawTextU8 ( S7Bmp *pDst, FT_Face ftFace, CONST BYTE *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB ) {
    S7Bmp *bmpGlyph = NULL;
    UINT x = nGlyphOffsetX;
    UINT iUnicode = 0;
    BYTE CONST *ch = pStr;
    while ( ( ch = A7UnicodeByUTF8 ( &iUnicode, ch ) ) != NULL ) {
        bmpGlyph = A7BmpCreateByFreeType ( bmpGlyph, ftFace, iUnicode, nGlyphHeight, x );
        A7BmpDrawAlphaMap ( pDst, bmpGlyph, nX, nY, cR, cG, cB, FALSE );
        x += bmpGlyph -> nAdvance + nTracking;
    }
}
VOID A7BmpDrawTextU8A ( S7Bmp *pDst, FT_Face ftFace, CONST BYTE *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB, BYTE cA ) {
    S7Bmp *bmpGlyph = NULL;
    UINT x = nGlyphOffsetX;
    UINT iUnicode = 0;
    BYTE CONST *ch = pStr;
    while ( ( ch = A7UnicodeByUTF8 ( &iUnicode, ch ) ) != NULL ) {
        bmpGlyph = A7BmpCreateByFreeType ( bmpGlyph, ftFace, iUnicode, nGlyphHeight, x );
        A7BmpDrawAlphaMapA ( pDst, bmpGlyph, nX, nY, cR, cG, cB, cA, FALSE );
        x += bmpGlyph -> nAdvance + nTracking;
    }
}



VOID A7BmpClearRect ( S7Bmp *pDst, UINT nX, UINT nY, UINT nW, UINT nH ) {
    CONST UINT bpp = A7BmpBpp ( pDst -> iType );
    BYTE *pbd = ( BYTE* ) pDst -> pData;
    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = ( iy + nY );
        if ( idy >= 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ( ix + nX );
            if ( idx >= 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * bpp;
            if ( bpp >= 1 ) pbd [ id + 0 ] = 0;
            if ( bpp >= 2 ) pbd [ id + 1 ] = 0;
            if ( bpp >= 3 ) pbd [ id + 2 ] = 0;
            if ( bpp >= 4 ) pbd [ id + 3 ] = 0;
        }
    }
}

VOID A7BmpDrawTab ( S7Bmp *pDst, UINT nX, UINT nY, S7Bmp *pTab, BYTE cBgR, BYTE cBgG, BYTE cBgB, UINT nMarginLeft, UINT nMarginMiddle, UINT nMarginRight, FT_Face ftFace, CONST CHAR *pStr, UINT nGlyphHeight, UINT nTracking, BYTE cTxtR, BYTE cTxtG, BYTE cTxtB, FT_Face ftFaceIcon, UINT iUnicodeIcon, UINT nGlyphIconHeight, BYTE cIcoR, BYTE cIcoG, BYTE cIcoB ) {

    A7BmpDrawAlphaMap ( pDst, pTab, nX, nY, cBgR, cBgG, cBgB, FALSE );
    CONST UINT nX0 = pTab -> nWidth * 64 + nMarginLeft;
    CONST UINT nX1 = A7BmpGetStringWidth ( ftFace, pStr, nGlyphHeight, nX0, nTracking );
    CONST UINT nX2 = nX1 + nMarginMiddle;
    CONST WCHAR pStrIco[2] = { iUnicodeIcon, 0 };
    CONST UINT nX3 = A7BmpGetStringWidthW ( ftFaceIcon, pStrIco, nGlyphIconHeight, nX2, 0 );
    CONST UINT nX4 = nX3 + nMarginRight;
    CONST UINT nW = nX4 / 64 - pTab -> nWidth;
    A7BmpDrawRect ( pDst, nX + pTab -> nWidth, nY, nW, pTab -> nHeight, cBgR, cBgG, cBgB );
    A7BmpDrawAlphaMap ( pDst, pTab, nX + nW + pTab -> nWidth, nY, cBgR, cBgG, cBgB, TRUE );

    CONST UINT nY0 = pTab -> nHeight / 2 + nGlyphHeight / 128;
    A7BmpDrawText ( pDst, ftFace, pStr, nX, nY + nY0, nGlyphHeight, nX0, nTracking, cTxtR, cTxtG, cTxtB );
    CONST UINT nY1 = pTab -> nHeight / 2 + nGlyphIconHeight / 128;
    A7BmpDrawTextW ( pDst, ftFaceIcon, pStrIco, nX, nY + nY1, nGlyphIconHeight, nX2, 0, cIcoR, cIcoG, cIcoB );
}
VOID A7BmpDrawTabA ( S7Bmp *pDst, UINT nX, UINT nY, S7Bmp *pTab, BYTE cBgR, BYTE cBgG, BYTE cBgB, BYTE cBgA, UINT nMarginLeft, UINT nMarginMiddle, UINT nMarginRight, FT_Face ftFace, CONST CHAR *pStr, UINT nGlyphHeight, UINT nTracking, BYTE cTxtR, BYTE cTxtG, BYTE cTxtB, BYTE cTxtA, FT_Face ftFaceIcon, UINT iUnicodeIcon, UINT nGlyphIconHeight, BYTE cIcoR, BYTE cIcoG, BYTE cIcoB, BYTE cIcoA ) {

    A7BmpDrawAlphaMapA ( pDst, pTab, nX, nY, cBgR, cBgG, cBgB, cBgA, FALSE );
    CONST UINT nX0 = pTab -> nWidth * 64 + nMarginLeft;
    CONST UINT nX1 = A7BmpGetStringWidth ( ftFace, pStr, nGlyphHeight, nX0, nTracking );
    CONST UINT nX2 = nX1 + nMarginMiddle;
    CONST WCHAR pStrIco[2] = { iUnicodeIcon, 0 };
    CONST UINT nX3 = A7BmpGetStringWidthW ( ftFaceIcon, pStrIco, nGlyphIconHeight, nX2, 0 );
    CONST UINT nX4 = nX3 + nMarginRight;
    CONST UINT nW = nX4 / 64 - pTab -> nWidth;
    A7BmpDrawRectA ( pDst, nX + pTab -> nWidth, nY, nW, pTab -> nHeight, cBgR, cBgG, cBgB, cBgA );
    A7BmpDrawAlphaMapA ( pDst, pTab, nX + nW + pTab -> nWidth, nY, cBgR, cBgG, cBgB, cBgA, TRUE );

    CONST UINT nY0 = pTab -> nHeight / 2 + nGlyphHeight / 128;
    A7BmpDrawTextA ( pDst, ftFace, pStr, nX, nY + nY0, nGlyphHeight, nX0, nTracking, cTxtR, cTxtG, cTxtB, cTxtA );
    CONST UINT nY1 = pTab -> nHeight / 2 + nGlyphIconHeight / 128;
    A7BmpDrawTextWA ( pDst, ftFaceIcon, pStrIco, nX, nY + nY1, nGlyphIconHeight, nX2, 0, cIcoR, cIcoG, cIcoB, cIcoA );
}
VOID A7BmpDrawTabW ( S7Bmp *pDst, UINT nX, UINT nY, S7Bmp *pTab, BYTE cBgR, BYTE cBgG, BYTE cBgB, UINT nMarginLeft, UINT nMarginMiddle, UINT nMarginRight, FT_Face ftFace, CONST WCHAR *pStr, UINT nGlyphHeight, UINT nTracking, BYTE cTxtR, BYTE cTxtG, BYTE cTxtB, FT_Face ftFaceIcon, UINT iUnicodeIcon, UINT nGlyphIconHeight, BYTE cIcoR, BYTE cIcoG, BYTE cIcoB ) {

    A7BmpDrawAlphaMap ( pDst, pTab, nX, nY, cBgR, cBgG, cBgB, FALSE );
    CONST UINT nX0 = pTab -> nWidth * 64 + nMarginLeft;
    CONST UINT nX1 = A7BmpGetStringWidthW ( ftFace, pStr, nGlyphHeight, nX0, nTracking );
    CONST UINT nX2 = nX1 + nMarginMiddle;
    CONST WCHAR pStrIco[2] = { iUnicodeIcon, 0 };
    CONST UINT nX3 = A7BmpGetStringWidthW ( ftFaceIcon, pStrIco, nGlyphIconHeight, nX2, 0 );
    CONST UINT nX4 = nX3 + nMarginRight;
    CONST UINT nW = nX4 / 64 - pTab -> nWidth;
    A7BmpDrawRect ( pDst, nX + pTab -> nWidth, nY, nW, pTab -> nHeight, cBgR, cBgG, cBgB );
    A7BmpDrawAlphaMap ( pDst, pTab, nX + nW + pTab -> nWidth, nY, cBgR, cBgG, cBgB, TRUE );

    CONST UINT nY0 = pTab -> nHeight / 2 + nGlyphHeight / 128;
    A7BmpDrawTextW ( pDst, ftFace, pStr, nX, nY + nY0, nGlyphHeight, nX0, nTracking, cTxtR, cTxtG, cTxtB );
    CONST UINT nY1 = pTab -> nHeight / 2 + nGlyphIconHeight / 128;
    A7BmpDrawTextW ( pDst, ftFaceIcon, pStrIco, nX, nY + nY1, nGlyphIconHeight, nX2, 0, cIcoR, cIcoG, cIcoB );
}
VOID A7BmpDrawTabWA ( S7Bmp *pDst, UINT nX, UINT nY, S7Bmp *pTab, BYTE cBgR, BYTE cBgG, BYTE cBgB, BYTE cBgA, UINT nMarginLeft, UINT nMarginMiddle, UINT nMarginRight, FT_Face ftFace, CONST WCHAR *pStr, UINT nGlyphHeight, UINT nTracking, BYTE cTxtR, BYTE cTxtG, BYTE cTxtB, BYTE cTxtA, FT_Face ftFaceIcon, UINT iUnicodeIcon, UINT nGlyphIconHeight, BYTE cIcoR, BYTE cIcoG, BYTE cIcoB, BYTE cIcoA ) {

    A7BmpDrawAlphaMapA ( pDst, pTab, nX, nY, cBgR, cBgG, cBgB, cBgA, FALSE );
    CONST UINT nX0 = pTab -> nWidth * 64 + nMarginLeft;
    CONST UINT nX1 = A7BmpGetStringWidthW ( ftFace, pStr, nGlyphHeight, nX0, nTracking );
    CONST UINT nX2 = nX1 + nMarginMiddle;
    CONST WCHAR pStrIco[2] = { iUnicodeIcon, 0 };
    CONST UINT nX3 = A7BmpGetStringWidthW ( ftFaceIcon, pStrIco, nGlyphIconHeight, nX2, 0 );
    CONST UINT nX4 = nX3 + nMarginRight;
    CONST UINT nW = nX4 / 64 - pTab -> nWidth;
    A7BmpDrawRectA ( pDst, nX + pTab -> nWidth, nY, nW, pTab -> nHeight, cBgR, cBgG, cBgB, cBgA );
    A7BmpDrawAlphaMapA ( pDst, pTab, nX + nW + pTab -> nWidth, nY, cBgR, cBgG, cBgB, cBgA, TRUE );

    CONST UINT nY0 = pTab -> nHeight / 2 + nGlyphHeight / 128;
    A7BmpDrawTextWA ( pDst, ftFace, pStr, nX, nY + nY0, nGlyphHeight, nX0, nTracking, cTxtR, cTxtG, cTxtB, cTxtA );
    CONST UINT nY1 = pTab -> nHeight / 2 + nGlyphIconHeight / 128;
    A7BmpDrawTextWA ( pDst, ftFaceIcon, pStrIco, nX, nY + nY1, nGlyphIconHeight, nX2, 0, cIcoR, cIcoG, cIcoB, cIcoA );
}
