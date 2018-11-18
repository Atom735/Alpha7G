FT_F26Dot6 g7Tex_nDpiHorz = 96;
FT_F26Dot6 g7Tex_nDpiVert = 96;

VOID A7TexFree ( S7Tex *p ) {
    if ( p == NULL ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return;
    }
    switch ( p -> iType ) {
        case DT7_TEX_GDI_BGR24:
        case DT7_TEX_GDI_BGRA32:
            if ( ( ( S7TexGdi* ) p ) -> hBMP != NULL ) {
                DeleteObject ( ( ( S7TexGdi* ) p ) -> hBMP );
                ( ( S7TexGdi* ) p ) -> hBMP = NULL;
            }
            if ( ( ( S7TexGdi* ) p ) -> hDC != NULL ) {
                DeleteDC ( ( ( S7TexGdi* ) p ) -> hDC );
                ( ( S7TexGdi* ) p ) -> hDC = NULL;
            }
            break;
    }
    if ( p -> bAllocated ) {
        free ( p );
    }
}

S7TexGdi* A7TexCreate_GDI ( S7TexGdi *pDst, HDC hDC, UINT nWidth, UINT nHeight, BOOL bAlpha ) {
    BITMAPINFO bmi;
    ZeroMemory ( &bmi, sizeof ( BITMAPINFO ) );
    bmi.bmiHeader.biSize        = sizeof ( BITMAPINFOHEADER ) ;
    bmi.bmiHeader.biWidth       = nWidth;
    bmi.bmiHeader.biHeight      = -nHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = bAlpha ? 32 : 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = bAlpha ? nWidth * nHeight * 4 : 0;
    if ( pDst == NULL ) {
        pDst = ( S7TexGdi* ) malloc ( sizeof ( S7TexGdi ) );
        if ( pDst == NULL ) {
            SetLastError ( ERROR_OUTOFMEMORY );
            D7ERR_WINAPI ( malloc );
            return NULL;
        }
        ZeroMemory ( pDst, sizeof ( S7TexGdi ) );
        pDst -> _tex . bAllocated = TRUE;
    }
    pDst -> _tex . iType    = bAlpha ? DT7_TEX_GDI_BGRA32 : DT7_TEX_GDI_BGR24;
    pDst -> _tex . nWidth   = nWidth;
    pDst -> _tex . nHeight  = nHeight;
    pDst -> _tex . nStride  = ( bAlpha ? ( nWidth * 4 ) : ( nWidth * 3 ) );
    if ( ( pDst -> _tex . nStride & 3 ) != 0 ) {
        pDst -> _tex . nStride += 4 - ( ( pDst -> _tex . nStride ) & 3 );
    }
    if ( pDst -> hDC == NULL ) {
        pDst -> hDC  = CreateCompatibleDC ( hDC );
    }
    if ( pDst -> hBMP != NULL ) {
        DeleteObject ( pDst -> hBMP );
    }
    pDst -> hBMP = CreateDIBSection ( pDst -> hDC, &bmi, DIB_RGB_COLORS, ( VOID** ) &( pDst -> _tex . pData ), NULL, 0x0 );
    SelectObject ( pDst -> hDC, pDst -> hBMP );
    return pDst;
}
S7TexGlyph* A7TexCreate_Glyph ( S7TexGlyph *pDst, FT_Face ftFace, FT_UInt iUnicode, FT_F26Dot6 nHeight, FT_F26Dot6 nOffsetX, FT_F26Dot6 nOffsetY ) {
    D7ERR_FREETYPE ( FT_Set_Char_Size, ftFace, 0, nHeight, g7Tex_nDpiHorz, g7Tex_nDpiVert );
    FT_Vector pen = { .x = nOffsetX, .y = nOffsetY, };
    if ( pDst == NULL ) {
        pDst = ( S7TexGlyph* ) malloc ( sizeof ( S7TexGlyph ) );
        if ( pDst == NULL ) {
            SetLastError ( ERROR_OUTOFMEMORY );
            D7ERR_WINAPI ( malloc );
            return NULL;
        }
        ZeroMemory ( pDst, sizeof ( S7TexGlyph ) );
        pDst -> _tex . bAllocated = TRUE;
    }
    FT_UInt iGlyphIndex = FT_Get_Char_Index ( ftFace, iUnicode );
    if ( ( FT_HAS_KERNING ( ftFace ) ) && ( pDst -> iGlyphIndex != 0 ) && ( iGlyphIndex != 0 ) ) {
        FT_Vector delta;
        D7ERR_FREETYPE ( FT_Get_Kerning, ftFace, pDst -> iGlyphIndex, iGlyphIndex, FT_KERNING_DEFAULT, &delta );
        pen.x += delta.x;
    }
    FT_Set_Transform ( ftFace, NULL, &pen );
    D7ERR_FREETYPE ( FT_Load_Glyph, ftFace, iGlyphIndex, FT_LOAD_RENDER ) {
        return pDst;
    }
    {
        FT_Bitmap *bmp = &ftFace -> glyph -> bitmap;
        pDst -> _tex . iType    = DT7_TEX_GLYPH_A8;
        pDst -> _tex . nWidth   = bmp -> width;
        pDst -> _tex . nHeight  = bmp -> rows;
        pDst -> _tex . nStride  = bmp -> pitch;
        pDst -> _tex . pData    = bmp -> buffer;
    }
    pDst -> nAdvance    = ftFace -> glyph -> advance.x;
    pDst -> iGlyphIndex = iGlyphIndex;
    pDst -> nOffsetLeft = ftFace -> glyph -> bitmap_left;
    pDst -> nOffsetTop  =-ftFace -> glyph -> bitmap_top;
    return pDst;
}

VOID A7TexDraw_GDI ( HDC hDC, UINT nDX, UINT nDY, S7TexGdi * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH ) {
    switch ( pSrc -> _tex . iType ) {
        case DT7_TEX_GDI_BGR24:
            BitBlt ( hDC, nDX, nDY, nW, nH, pSrc -> hDC, nSX, nSY, SRCCOPY );
            break;
        case DT7_TEX_GDI_BGRA32: {
            BLENDFUNCTION ftn = {
                .BlendOp                = AC_SRC_OVER,
                .BlendFlags             = 0,
                .SourceConstantAlpha    = 0xff,
                .AlphaFormat            = AC_SRC_ALPHA,
            };
            GdiAlphaBlend ( hDC, nDX, nDY, nW, nH, pSrc -> hDC, nSX, nSY, nW, nH, ftn );
            break;
        }
        default:
            SetLastError ( ERROR_INVALID_DATA );
            D7ERR_WINAPI ( __FUNCTION__ );
            break;
    }
}


VOID A7TexFillRect ( S7Tex * pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB ) {
    BYTE * CONST pv = pDst -> pData;
    CONST BYTE cA = ( iARGB >> 030 ) & 0xff;
    CONST BYTE cR = ( iARGB >> 020 ) & 0xff;
    CONST BYTE cG = ( iARGB >> 010 ) & 0xff;
    CONST BYTE cB = ( iARGB >> 000 ) & 0xff;
    switch ( pDst -> iType ) {
        case DT7_TEX_GDI_BGR24: {
            for ( UINT iy = 0; iy < nH; ++iy ) {
                CONST UINT _iy = nY + iy;
                if ( _iy >= pDst -> nHeight ) break;
                for ( UINT ix = 0; ix < nW; ++ix ) {
                    CONST UINT _ix = nX + ix;
                    if ( _ix >= pDst -> nWidth ) break;
                    CONST UINT _ii = _iy * pDst -> nStride + _ix * 3;
                    pv [ _ii + 0 ] = cB;
                    pv [ _ii + 1 ] = cG;
                    pv [ _ii + 2 ] = cR;
                }
            }
            break;
        }
        case DT7_TEX_GDI_BGRA32: {
            for ( UINT iy = 0; iy < nH; ++iy ) {
                CONST UINT _iy = nY + iy;
                if ( _iy >= pDst -> nHeight ) break;
                for ( UINT ix = 0; ix < nW; ++ix ) {
                    CONST UINT _ix = nX + ix;
                    if ( _ix >= pDst -> nWidth ) break;
                    CONST UINT _ii = _iy * pDst -> nStride + _ix * 4;
                    pv [ _ii + 0 ] = cB;
                    pv [ _ii + 1 ] = cG;
                    pv [ _ii + 2 ] = cR;
                    pv [ _ii + 3 ] = cA;
                }
            }
            break;
        }
        default:
            SetLastError ( ERROR_INVALID_DATA );
            D7ERR_WINAPI ( __FUNCTION__ );
            break;
    }
}
