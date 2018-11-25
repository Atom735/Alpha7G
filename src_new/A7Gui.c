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
S7TexGlyph* A7TexCreate_Glyph ( S7TexGlyph *pDst, FT_Face ftFace, FT_UInt iUnicode, FT_F26Dot6 nHeight ) {
    D7ERR_FREETYPE ( FT_Set_Char_Size, ftFace, 0, nHeight, g7Tex_nDpiHorz, g7Tex_nDpiVert );
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
        pDst -> nAdvance += delta.x;
    }
    FT_Vector pen = { .x = pDst -> nOffsetX + pDst -> nAdvance, .y = pDst -> nOffsetY, };
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
    pDst -> nAdvance   += ftFace -> glyph -> advance.x;
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

VOID A7TexDrawAlphaMap ( S7Tex *pDst, S7Tex *pSrc, UINT nX, UINT nY, UINT32 iARGB, UINT iFlags ) {
    CONST BYTE cA = ( iARGB >> 030 ) & 0xff;
    CONST BYTE cR = ( iARGB >> 020 ) & 0xff;
    CONST BYTE cG = ( iARGB >> 010 ) & 0xff;
    CONST BYTE cB = ( iARGB >> 000 ) & 0xff;
    BYTE *pbd = pDst -> pData;
    BYTE *pbs = pSrc -> pData;
    for ( UINT iy = 0; iy < pSrc -> nHeight; ++iy ) {
        // CONST UINT idy = pSrc -> iType == DT7_TEX_GLYPH_A8 ? iy + nY + ( ( S7TexGlyph* ) pSrc ) -> nOffsetTop : iy + nY;
        CONST UINT idy = iy + nY;
        if ( idy & 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < pSrc -> nWidth; ++ix ) {
            // CONST UINT idx = pSrc -> iType == DT7_TEX_GLYPH_A8 ? ix + nX + ( ( S7TexGlyph* ) pSrc ) -> nOffsetLeft : ix + nX;
            CONST UINT idx = ix + nX;
            if ( idx & 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;

            CONST UINT is = ( iFlags & D7TEXDAM_MIRROR_VERT ? pSrc -> nHeight - iy - 1 : iy ) * pSrc -> nStride + ( iFlags & D7TEXDAM_MIRROR_HORI ? ( pSrc -> nWidth - ix - 1 ) : ix );
            CONST UINT _cA = pbs [ is ] * cA;
            CONST UINT _cR = cR * _cA;
            CONST UINT _cG = cG * _cA;
            CONST UINT _cB = cB * _cA;

            switch ( pDst -> iType ) {
                case DT7_TEX_GDI_BGR24: {
                    CONST UINT id = idy * pDst -> nStride + idx * 3;
                    pbd [ id + 0 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 0 ] + _cB ) / ( 0xFE01 );
                    pbd [ id + 1 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 1 ] + _cG ) / ( 0xFE01 );
                    pbd [ id + 2 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 2 ] + _cR ) / ( 0xFE01 );

                    break;
                }
                case DT7_TEX_GDI_BGRA32: {
                    CONST UINT id = idy * pDst -> nStride + idx * 4;
                    pbd [ id + 0 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 0 ] + _cB ) / ( 0xFE01 );
                    pbd [ id + 1 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 1 ] + _cG ) / ( 0xFE01 );
                    pbd [ id + 2 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 2 ] + _cR ) / ( 0xFE01 );
                    pbd [ id + 3 ] = ( ( ( 0xFE01 ) - _cA ) * pbd [ id + 3 ] + _cA ) / ( 0xFE01 );
                    break;
                }

            }
        }
    }
}

struct _S7Shape {
    BYTE        *pData;
    UINT        nX, nW, nY, nH;
};

struct _S7SetsButton {
    UINT        iType;

    FT_Face     txt_ftFace;
    UINT32      txt_iARGB;
    UINT32      txt_iFlags;
    FT_F26Dot6  txt_nHeight;
    FT_F26Dot6  txt_nKerning;
    FT_F26Dot6  txt_nOblique;

    UINT32      bg_iARGB;
    FT_F26Dot6  bg_nHeight;


};

struct _S7Button {
    UINT        iType;
    S7SetsButton *pSets;
    LPCWSTR     pText;
};
extern FT_Face g_ftFace;
VOID A7TexDraw_Button ( S7Tex *pDst ) {
    A7TexFillRect_FULL ( pDst, 0x00000000 );

    S7GuiTextSets sets = {
        .iType  = 0,
        .ftFace = g_ftFace,
        .iARGB  = 0xffff0000,
        .iFlags = 0,
        .nHeight    = 18 * 64,
        .nOffsetX   = 0,
        .nOffsetY   = 0,
        .nLineHeight= 24 * 64,
        .nLineWidth = 0,
        .nTracking  = 0,
        .nOblique   = 0,
    };
    // A7GuiDraw_ShapeRoundWithRipple ( pDst, 0, 0, 1024, 1024, 0xff7fff00, 256.0f, 512, 512, 512.0f, 0x7fffffff );

    // A7GuiDraw_ShapeOutlinedRound ( pDst, 128, 128, 256, 128, 0xff7f00ff, 16.0f, 8.0f );

    A7GuiDraw_TextWide ( pDst, 256, 128*1, L"Привет мир, дремучий!\nКак житуха?)))\n☺☻♥♦♣♠", &sets );
    sets . iFlags ^= D7GUITEXT_AA_LCD;
    A7GuiDraw_TextWide ( pDst, 256, 128*2, L"Привет мир, дремучий!\nКак житуха?)))\n☺☻♥♦♣♠", &sets );
    sets . iFlags ^= D7GUITEXT_AA_LCD_INV;
    A7GuiDraw_TextWide ( pDst, 256, 128*3, L"Привет мир, дремучий!\nКак житуха?)))\n☺☻♥♦♣♠", &sets );
    sets . iFlags ^= D7GUITEXT_AA_LCD_INV;
    sets . iFlags ^= D7GUITEXT_AA_LCD;
    sets . iFlags ^= D7GUITEXT_AA_GRAYSCALE;
    A7GuiDraw_TextWide ( pDst, 512, 128*1, L"Привет мир, дремучий!\nКак житуха?)))\n☺☻♥♦♣♠", &sets );
    sets . iFlags ^= D7GUITEXT_AA_LCD;
    A7GuiDraw_TextWide ( pDst, 512, 128*2, L"Привет мир, дремучий!\nКак житуха?)))\n☺☻♥♦♣♠", &sets );
    sets . iFlags ^= D7GUITEXT_AA_LCD_INV;
    A7GuiDraw_TextWide ( pDst, 512, 128*3, L"Привет мир, дремучий!\nКак житуха?)))\n☺☻♥♦♣♠", &sets );

    sets . iFlags = D7GUITEXT_AA_LCD | D7GUITEXT_ALIGN_CENTER | D7GUITEXT_NO_KERNING;
    sets . nLineHeight = sets . nHeight / 3;
    LPCWSTR lpsz = L"КНОПКА";
    LPCWSTR a = lpsz;
    A7GuiDraw_ButtonTextWide ( pDst, 256, 32, lpsz, &sets, A7GetLineWidth_TextWide ( &a, &sets ) / 32, sets . nHeight / 32, 0xffffffff, 3.5f );

}



/* Отрисовать закруглённую форму с эффектом Ripple */
VOID A7GuiDraw_ShapeRoundWithRipple ( S7Tex *pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB, FLOAT fR, UINT nrX, UINT nrY, FLOAT frR, UINT32 irARGB ) {
    BYTE * CONST pbd = pDst -> pData;
    CONST UINT iR = ceilf ( fR );
    CONST UINT iX = nW - iR - 1;
    CONST UINT iY = nH - iR - 1;
    CONST UINT cA = ( ( iARGB >> 030 ) & 0xff );
    CONST UINT cR = ( ( iARGB >> 020 ) & 0xff );
    CONST UINT cG = ( ( iARGB >> 010 ) & 0xff );
    CONST UINT cB = ( ( iARGB >> 000 ) & 0xff );

    CONST UINT crA = ( ( irARGB >> 030 ) & 0xff );
    CONST UINT crR = ( ( irARGB >> 020 ) & 0xff );
    CONST UINT crG = ( ( irARGB >> 010 ) & 0xff );
    CONST UINT crB = ( ( irARGB >> 000 ) & 0xff );

    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = iy + nY;
        if ( idy & 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ix + nX;
            if ( idx & 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 3;
            CONST FLOAT fx = ix < iR ? ( FLOAT ) ( ix ) - fR :
                ix > iX ? ( FLOAT ) ( nW - ix - 1 ) - fR : 0.0f;
            CONST FLOAT fy = iy < iR ? ( FLOAT ) ( iy ) - fR :
                iy > iY ? ( FLOAT ) ( nH - iy - 1 ) - fR : 0.0f;
            CONST FLOAT r = fR - sqrtf ( fx * fx + fy * fy ) + 1.0f;
            CONST UINT  a = r > 1.0f ? cA : r > 0.0f ? r * (FLOAT) cA : 0x00;
            pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + cB * a ) / 0xff;
            pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + cG * a ) / 0xff;
            pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + cR * a ) / 0xff;

            CONST FLOAT frx = ( FLOAT ) ( INT ) ( ix - nrX );
            CONST FLOAT fry = ( FLOAT ) ( INT ) ( iy - nrY );
            CONST FLOAT rr = frR - sqrtf ( frx * frx + fry * fry ) + 1.0f;
            CONST UINT  ar = rr > 1.0f ? crA : rr > 0.0f ? rr * (FLOAT) crA : 0x00;
            CONST UINT  ra = r > 1.0f ? ar : r > 0.0f ? r * (FLOAT) ar : 0x00;
            pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - ra ) + crB * ra ) / 0xff;
            pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - ra ) + crG * ra ) / 0xff;
            pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - ra ) + crR * ra ) / 0xff;
        }
    }
}




/* Отрисовать закруглённую форму */
VOID A7GuiDraw_ShapeRound ( S7Tex *pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB, FLOAT fR ) {
    BYTE * CONST pbd = pDst -> pData;
    CONST UINT iR = ceilf ( fR );
    CONST UINT iX = nW - iR - 1;
    CONST UINT iY = nH - iR - 1;
    CONST UINT cA = ( ( iARGB >> 030 ) & 0xff );
    CONST UINT cR = ( ( iARGB >> 020 ) & 0xff );
    CONST UINT cG = ( ( iARGB >> 010 ) & 0xff );
    CONST UINT cB = ( ( iARGB >> 000 ) & 0xff );
    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = iy + nY;
        if ( idy & 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ix + nX;
            if ( idx & 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 3;
            CONST FLOAT fx = ix < iR ? ( FLOAT ) ( ix ) - fR :
                ix > iX ? ( FLOAT ) ( nW - ix - 1 ) - fR : 0.0f;
            CONST FLOAT fy = iy < iR ? ( FLOAT ) ( iy ) - fR :
                iy > iY ? ( FLOAT ) ( nH - iy - 1 ) - fR : 0.0f;
            CONST FLOAT r = fR - sqrtf ( fx * fx + fy * fy ) + 1.0f;
            CONST UINT  a = r > 1.0f ? cA : r > 0.0f ? r * (FLOAT) cA : 0x00;
            pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + cB * a ) / 0xff;
            pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + cG * a ) / 0xff;
            pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + cR * a ) / 0xff;
            // pbd [ id + 2 ] = fx + fy;
        }
    }
}


VOID A7GuiDraw_ShapeOutlinedRound ( S7Tex *pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB, FLOAT fR, FLOAT fr ) {
    BYTE * CONST pbd = pDst -> pData;
    CONST UINT iR = ceilf ( fR );
    CONST UINT iX = nW - iR - 1;
    CONST UINT iY = nH - iR - 1;
    CONST UINT cA = ( ( iARGB >> 030 ) & 0xff );
    CONST UINT cR = ( ( iARGB >> 020 ) & 0xff );
    CONST UINT cG = ( ( iARGB >> 010 ) & 0xff );
    CONST UINT cB = ( ( iARGB >> 000 ) & 0xff );
    for ( UINT iy = 0; iy < nH; ++iy ) {
        CONST UINT idy = iy + nY;
        if ( idy & 0x80000000U ) continue;
        if ( idy >= pDst -> nHeight ) break;
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT idx = ix + nX;
            if ( idx & 0x80000000U ) continue;
            if ( idx >= pDst -> nWidth ) break;
            CONST UINT id = idy * pDst -> nStride + idx * 3;
            CONST FLOAT fx = ix < iR ? ( FLOAT ) ( ix ) - fR :
                ix > iX ? ( FLOAT ) ( nW - ix - 1 ) - fR : 0.0f;
            CONST FLOAT fy = iy < iR ? ( FLOAT ) ( iy ) - fR :
                iy > iY ? ( FLOAT ) ( nH - iy - 1 ) - fR : 0.0f;
            CONST FLOAT q = sqrtf ( fx * fx + fy * fy );
            CONST FLOAT r = fR - q + 1.0f;
            CONST FLOAT R = q - fr + 1.0f;
            CONST UINT  a = R > 1.0f ? ( r > 1.0f ? cA : r > 0.0f ? r * (FLOAT) cA : 0x00 ) : R > 0.0f ? ( r > 1.0f ? R * (FLOAT) cA : r > 0.0f ? R * r * (FLOAT) cA : 0x00 ) : 0x00;
            pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + cB * a ) / 0xff;
            pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + cG * a ) / 0xff;
            pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + cR * a ) / 0xff;
            // pbd [ id + 2 ] = fx + fy;
        }
    }
}

FT_F26Dot6 A7GetWordWidth_TextWide ( LPCWSTR * pOIText, S7GuiTextSets *pSets ) {
    CONST FT_Face   ftFace      = pSets -> ftFace;
    CONST BOOL      bKerning    = ( pSets -> iFlags & D7GUITEXT_KERNING ) && ( FT_HAS_KERNING ( ftFace ) );
    FT_UInt         iGlyphIndexLast = 0;
    D7ERR_FREETYPE ( FT_Set_Char_Size, ftFace, 0, pSets -> nHeight * 72 / g7Tex_nDpiVert, g7Tex_nDpiHorz, g7Tex_nDpiVert );
    FT_F26Dot6 width = 0;
    LPCWSTR pText = *pOIText;
    while ( iswalnum ( *pText ) ) {
        CONST FT_UInt iGlyphIndex = FT_Get_Char_Index ( ftFace, *pText );
        if ( bKerning && ( iGlyphIndexLast != 0 ) && ( iGlyphIndex != 0 ) ) {
            FT_Vector delta;
            D7ERR_FREETYPE ( FT_Get_Kerning, ftFace, iGlyphIndexLast, iGlyphIndex, FT_KERNING_DEFAULT, &delta );
            width += delta . x;
        }
        D7ERR_FREETYPE ( FT_Load_Glyph, ftFace, iGlyphIndex, FT_LOAD_DEFAULT );
        CONST FT_GlyphSlot glyph = ftFace -> glyph;
        iGlyphIndexLast = iGlyphIndex;
        width += glyph -> advance.x + pSets -> nTracking;
        ++pText;
    }
    *pOIText = pText;
    return width;
}
FT_F26Dot6 A7GetLineWidth_TextWide ( LPCWSTR * pOIText, S7GuiTextSets *pSets ) {
    CONST FT_Face   ftFace      = pSets -> ftFace;
    CONST BOOL      bKerning    = ( pSets -> iFlags & D7GUITEXT_KERNING ) && ( FT_HAS_KERNING ( ftFace ) );
    FT_UInt         iGlyphIndexLast = 0;
    D7ERR_FREETYPE ( FT_Set_Char_Size, ftFace, 0, pSets -> nHeight * 72 / g7Tex_nDpiVert, g7Tex_nDpiHorz, g7Tex_nDpiVert );
    FT_F26Dot6 width = 0;
    LPCWSTR pText = *pOIText;
    while ( ( *pText != 0 ) && ( *pText != '\n' ) ) {
        CONST FT_UInt iGlyphIndex = FT_Get_Char_Index ( ftFace, *pText );
        if ( bKerning && ( iGlyphIndexLast != 0 ) && ( iGlyphIndex != 0 ) ) {
            FT_Vector delta;
            D7ERR_FREETYPE ( FT_Get_Kerning, ftFace, iGlyphIndexLast, iGlyphIndex, FT_KERNING_DEFAULT, &delta );
            width += delta . x;
        }
        D7ERR_FREETYPE ( FT_Load_Glyph, ftFace, iGlyphIndex, FT_LOAD_DEFAULT );
        CONST FT_GlyphSlot glyph = ftFace -> glyph;
        iGlyphIndexLast = iGlyphIndex;
        width += glyph -> advance.x + pSets -> nTracking;
        ++pText;
    }
    *pOIText = pText;
    return width;
}

/* Отрисовать текст */
VOID A7GuiDraw_TextWide ( S7Tex *pDst, UINT nX, UINT nY, LPCWSTR pText, S7GuiTextSets *pSets ) {
    BYTE * CONST pbd = pDst -> pData;
    CONST UINT cA = ( ( pSets -> iARGB >> 030 ) & 0xff );
    CONST UINT cR = ( ( pSets -> iARGB >> 020 ) & 0xff );
    CONST UINT cG = ( ( pSets -> iARGB >> 010 ) & 0xff );
    CONST UINT cB = ( ( pSets -> iARGB >> 000 ) & 0xff );
    CONST FT_Face   ftFace      = pSets -> ftFace;
    CONST BOOL      bKerning    = ( pSets -> iFlags & D7GUITEXT_KERNING ) && ( FT_HAS_KERNING ( ftFace ) );
    CONST BOOL      bOblique    = pSets -> nOblique != 0;
    CONST BOOL      bR2L        = ( pSets -> iFlags & D7GUITEXT_RIGHT2LEFT );
    CONST BOOL      bGS         = ( pSets -> iFlags & D7GUITEXT_AA_GRAYSCALE );
    CONST BOOL      bLCD        = ( pSets -> iFlags & D7GUITEXT_AA_LCD );
    CONST BOOL      bLCDINV     = ( pSets -> iFlags & D7GUITEXT_AA_LCD_INV );
    CONST BOOL      bAlignR     = ( pSets -> iFlags & D7GUITEXT_ALIGN_RIGHT );
    CONST BOOL      bAlignC     = ( pSets -> iFlags & D7GUITEXT_ALIGN_CENTER );
    FT_UInt         iGlyphIndexLast = 0;
    D7ERR_FREETYPE ( FT_Set_Char_Size, ftFace, 0, pSets -> nHeight * 72 / g7Tex_nDpiVert, g7Tex_nDpiHorz, g7Tex_nDpiVert );

    FT_Vector pen = { .x = pSets -> nOffsetX, .y = - pSets -> nOffsetY, };
    VOID _newLine ( UINT i ) {
        iGlyphIndexLast = 0;
        pen . x = pSets -> nOffsetX;
        pen . y -= pSets -> nLineHeight;
        if ( bAlignR || bAlignC ) {
            PCWSTR pszLineStart = pText + i;
            FT_F26Dot6 len = A7GetLineWidth_TextWide ( &pszLineStart, pSets );
            pen . x -= bAlignR ? len : len / 2;
        }
    }
    _newLine ( 0 );
    for ( UINT i = 0; pText [ i ] != 0; ++i ) {
        if ( pText [ i ] == '\n' ) {
            _newLine ( i + 1 );
            continue;
        }
        CONST FT_UInt iGlyphIndex = FT_Get_Char_Index ( ftFace, pText [ i ] );
        if ( bKerning && ( iGlyphIndexLast != 0 ) && ( iGlyphIndex != 0 ) ) {
            FT_Vector delta;
            D7ERR_FREETYPE ( FT_Get_Kerning, ftFace, iGlyphIndexLast, iGlyphIndex, FT_KERNING_DEFAULT, &delta );
            if ( bR2L ) {
                pen . x -= delta . x;
            } else {
                pen . x += delta . x;
            }
        }
        if ( bOblique ) {
            FT_Matrix mat = { .xx = 0x10000L, .xy = pSets -> nOblique, .yx = 0, .yy = 0x10000L, };
            FT_Set_Transform ( ftFace, &mat, &pen );
        } else {
            FT_Set_Transform ( ftFace, NULL, &pen );
        }
        D7ERR_FREETYPE ( FT_Load_Glyph, ftFace, iGlyphIndex, FT_LOAD_DEFAULT /*FT_LOAD_RENDER*/ );
        CONST FT_GlyphSlot glyph = ftFace -> glyph;
        D7ERR_FREETYPE ( FT_Render_Glyph, glyph, ( bLCD && bGS ) ? FT_RENDER_MODE_LCD_V : bLCD ? FT_RENDER_MODE_LCD : bGS ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO );
        iGlyphIndexLast = iGlyphIndex;
        if ( bR2L ) {
            pen . x -= glyph -> advance.x + pSets -> nTracking;
        } else {
            pen . x += glyph -> advance.x + pSets -> nTracking;
        }
        FT_Bitmap *bmp = & glyph -> bitmap;
        CONST UINT nW = ( bLCD && !bGS ) ? bmp -> width / 3 : bmp -> width;
        CONST UINT nH = ( bLCD && bGS ) ? bmp -> rows / 3 : bmp -> rows;
        BYTE * CONST pbs = bmp -> buffer;
        for ( UINT iy = 0; iy < nH; ++iy ) {
            CONST UINT idy = iy + nY - glyph -> bitmap_top;
            if ( idy & 0x80000000U ) continue;
            if ( idy >= pDst -> nHeight ) break;
            for ( UINT ix = 0; ix < nW; ++ix ) {
                CONST UINT idx = ix + nX + glyph -> bitmap_left;
                if ( idx & 0x80000000U ) continue;
                if ( idx >= pDst -> nWidth ) break;
                CONST UINT is = ( bLCD && bGS ? iy * 3 : iy ) * bmp -> pitch + ( bLCD && !bGS ? ix * 3 : bGS ? ix : ( ix >> 3 ) );
                CONST UINT id = idy * pDst -> nStride + idx * 3;
                if ( bLCD && bGS ) {
                    CONST UINT is = iy * 3 * bmp -> pitch + ix;
                    CONST UINT aR = pbs [ is + ( bLCDINV ? 2 * bmp -> pitch : 0 ) ] * cA / 0xff;
                    CONST UINT aG = pbs [ is + bmp -> pitch ] * cA / 0xff;
                    CONST UINT aB = pbs [ is + ( bLCDINV ? 0 : 2 * bmp -> pitch ) ] * cA / 0xff;
                    pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - aB ) + cB * aB ) / 0xff;
                    pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - aG ) + cG * aG ) / 0xff;
                    pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - aR ) + cR * aR ) / 0xff;
                } else
                if ( bLCD ) {
                    CONST UINT aR = pbs [ is + ( bLCDINV ? 2 : 0 ) ] * cA / 0xff;
                    CONST UINT aG = pbs [ is + 1 ] * cA / 0xff;
                    CONST UINT aB = pbs [ is + ( bLCDINV ? 0 : 2 ) ] * cA / 0xff;
                    pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - aB ) + cB * aB ) / 0xff;
                    pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - aG ) + cG * aG ) / 0xff;
                    pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - aR ) + cR * aR ) / 0xff;
                } else {
                    CONST UINT a = bGS ? pbs [ is ] * cA / 0xff : ( pbs [ is ] & ( 0x80 >> ( ix & 7 ) ) ) ? cA : 0;
                    pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + cB * a ) / 0xff;
                    pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + cG * a ) / 0xff;
                    pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + cR * a ) / 0xff;
                }
            }
        }
    }
}



/* Отрисовать обычную кнопку */
VOID A7GuiDraw_ButtonTextWide ( S7Tex *pDst, UINT nX, UINT nY, LPCWSTR pText, S7GuiTextSets *pSets, UINT nW, UINT nH, UINT32 iARGB, FLOAT fR ) {
    A7GuiDraw_ShapeRound ( pDst, nX, nY, nW, nH, iARGB, fR );
    A7GuiDraw_TextWide ( pDst, nX + nW / 2, nY + nH / 2, pText, pSets );
}




/* Находится ли точка внутри формы круга, iX и iY - расстояние до центра Ripple */
FLOAT A7GuiAlpha_PointInRipple ( FLOAT iX, FLOAT iY, FLOAT fR ) {
    CONST FLOAT r = fR - sqrtf ( iX * iX + iY * iY ) + 1.0f;
    return r > 1.0f ? 1.0f : r > 0.0f ? r : 0.0f;
}
/* Находится ли точка внутри формы округлой, iX и iY начало от TopLeft края формы */
FLOAT A7GuiAlpha_PointInShapeRound ( FLOAT iX, FLOAT iY, FLOAT fR ) {
    if ( ( iX > fR ) || ( iY > fR ) ) return 0.0f;
    CONST FLOAT fx = iX - fR;
    CONST FLOAT fy = iY - fR;
    CONST FLOAT r = fR - sqrtf ( fx * fx + fy * fy ) + 1.0f;
    return r > 1.0f ? 1.0f : r > 0.0f ? r : 0.0f;
}
/* iCorners: 1 - TopLeft, 2 - TopRight, 4 - BottomLeft, 8 - BottomRight */
FLOAT A7GuiAlpha_PointInShapeRound9 ( FLOAT iX, FLOAT iY, FLOAT fR, UINT nW, UINT nH, UINT iCorners ) {
    if ( ( iX >= 0.0f ) && ( iY >= 0.0f ) && ( iX <= ( FLOAT ) ( nW - 1 ) ) && ( iY <= ( FLOAT ) ( nH - 1 ) ) ) {
        if ( ( iCorners & 0x1 ) && ( iX < fR ) && ( iY < fR )  ) {
            return A7GuiAlpha_PointInShapeRound ( iX, iY, fR );
        }
        if ( ( iCorners & 0x2 ) && ( iX > ( FLOAT ) ( nW - 1 ) - fR ) && ( iY < fR )  ) {
            return A7GuiAlpha_PointInShapeRound ( ( FLOAT ) ( nW - 1 ) - iX, iY, fR );
        }
        if ( ( iCorners & 0x4 ) && ( iX < fR ) && ( iY > ( FLOAT ) ( nH - 1 ) - fR )  ) {
            return A7GuiAlpha_PointInShapeRound ( iX, ( FLOAT ) ( nH - 1 ) - iY, fR );
        }
        if ( ( iCorners & 0x8 ) && ( iX > ( FLOAT ) ( nW - 1 ) - fR ) && ( iY > ( FLOAT ) ( nH - 1 ) - fR )  ) {
            return A7GuiAlpha_PointInShapeRound ( ( FLOAT ) ( nW - 1 ) - iX, ( FLOAT ) ( nH - 1 ) - iY, fR );
        }
        return 1.0f;
    }
    return 0.0f;
}




VOID A7GuiMdDraw_TextField ( S7Tex *pDst, S7GuiTextField *pE ) {

}



/* Добавляет [pNewChild] как наследника к [pNode] */
S7Node *A7Node_AppendChild ( S7Node *pNode, S7Node *pNewChild ) {
    if ( pNode == NULL ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return NULL;
    }
    if ( ( pNode -> iType & 0xffff0000 ) != DT7_NODE ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return NULL;
    }
    if ( pNewChild == NULL ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return NULL;
    }
    if ( ( pNewChild -> iType & 0xffff0000 ) != DT7_NODE ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return NULL;
    }

    pNewChild -> pParentNode = pNode;
    if ( pNode -> pChildLast == NULL ) {
        pNode -> pChildLast = pNewChild;
        pNode -> pChildFirst = pNewChild;
    } else {
        pNode -> pChildLast -> pSiblingNext = pNewChild;
        pNewChild -> pSiblingPrev = pNode -> pChildLast;
        pNode -> pChildLast = pNewChild;
    }
    return pNewChild;
}
/* Создаёт новую Ноду. Если [pNode] == NULL, то выделяет под неё место */
S7Node *A7Node_CreateNew ( S7Node *pNode, UINT iFlags, LPCWSTR szName ) {

    BOOL bAlloc = FALSE;
    if ( pNode == NULL ) {
        pNode = ( S7Node* ) malloc ( sizeof ( S7Node ) );
        if ( pNode == NULL ) {
            SetLastError ( ERROR_NOT_ENOUGH_MEMORY );
            D7ERR_WINAPI ( malloc );
            return NULL;
        }
        bAlloc = TRUE;
    }
    ZeroMemory ( pNode, sizeof ( S7Node ) );
    pNode -> iType  = DT7_NODE;
    pNode -> iFlags = bAlloc ? iFlags | D7NODE_ALLOCATED : iFlags;
    pNode -> szName = szName;
    return pNode;
}
/* Удаляет Ноду */
VOID    A7Node_Release ( S7Node *pNode ) {
    if ( pNode == NULL ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return;
    }
    if ( ( pNode -> iType & 0xffff0000 ) != DT7_NODE ) {
        SetLastError ( ERROR_INVALID_DATA );
        D7ERR_WINAPI ( __FUNCTION__ );
        return;
    }
    while ( pNode -> pChildLast != NULL ) {
        A7Node_Release ( pNode -> pChildLast );
    }
    if ( pNode -> pParentNode != NULL ) {
        if ( pNode -> pParentNode -> pChildLast == pNode ) {
            pNode -> pParentNode -> pChildLast = pNode -> pSiblingPrev;
        }
        if ( pNode -> pParentNode -> pChildFirst == pNode ) {
            pNode -> pParentNode -> pChildFirst = pNode -> pSiblingNext;
        }
    }
    if ( pNode -> pSiblingNext != NULL ) {
        pNode -> pSiblingNext -> pSiblingPrev = pNode -> pSiblingPrev;
    }
    if ( pNode -> pSiblingPrev != NULL ) {
        pNode -> pSiblingPrev -> pSiblingNext = pNode -> pSiblingNext;
    }
    if ( pNode -> iFlags & D7NODE_ALLOCATED ) {
        free ( pNode );
    }
}


VOID _Node_Ripple_rOnMouseUp ( S7Node *pThis, INT iX, INT iY, UINT iButton ) {
    pThis -> iClockEnd      = clock ();
}

VOID _Node_Ripple_rOnPaint ( S7Node *pThis ) {
    FLOAT a = 1.0f;
    if ( pThis -> iClockEnd != 0 ) {
        a = 1.0f - ( FLOAT ) ( clock () - pThis -> iClockEnd ) * 0.001f;
        if ( a < 0.0f ) {
            A7Node_Release ( pThis );
            return;
        }
    }

    FLOAT t = ( FLOAT ) ( clock () - pThis -> iClockStart ) * 0.0001f;
    if ( t > 1.0f ) t = 1.0f;

    CONST UINT nH = pThis -> pParentNode -> nH;
    CONST UINT nW = pThis -> pParentNode -> nW;

    CONST FLOAT fw = ( FLOAT ) nW * 0.5f;
    CONST FLOAT fh = ( FLOAT ) nH * 0.5f;
    CONST FLOAT fx = fw * t + (FLOAT) pThis -> nX * ( 1.0f - t );
    CONST FLOAT fy = fh * t + (FLOAT) pThis -> nY * ( 1.0f - t );
    CONST FLOAT fr = sqrtf ( fw * fw + fh * fh ) * t;

    S7Tex * CONST pDst = pThis -> pTex;
    BYTE * CONST pbd = pDst -> pData;

    CONST UINT cA = ( ( pThis -> iARGB >> 030 ) & 0xff );
    CONST UINT cR = ( ( pThis -> iARGB >> 020 ) & 0xff );
    CONST UINT cG = ( ( pThis -> iARGB >> 010 ) & 0xff );
    CONST UINT cB = ( ( pThis -> iARGB >> 000 ) & 0xff );

    for ( UINT iy = 0; iy < nH; ++iy ) {
        for ( UINT ix = 0; ix < nW; ++ix ) {
            CONST UINT _id = iy * pDst -> nStride + ix * 3;
            CONST FLOAT _x = (FLOAT) ix - fx;
            CONST FLOAT _y = (FLOAT) iy - fy;
            CONST UINT  _a = ( FLOAT ) cA * a * A7GuiAlpha_PointInRipple ( _x, _y, fr );
            pbd [ _id + 0 ] = ( pbd [ _id + 0 ] * ( 0xff - _a ) + cB * _a ) / 0xff;
            pbd [ _id + 1 ] = ( pbd [ _id + 1 ] * ( 0xff - _a ) + cG * _a ) / 0xff;
            pbd [ _id + 2 ] = ( pbd [ _id + 2 ] * ( 0xff - _a ) + cR * _a ) / 0xff;
        }
    }
}

S7Node *A7Node_CreateNew_Ripple ( S7Node *pNode, UINT nX, UINT nY, S7Tex *pTex, UINT32 iARGB ) {
    pNode = A7Node_CreateNew ( pNode, 0, L"Ripple" );
    pNode -> iClockStart    = clock ();
    pNode -> nX             = nX;
    pNode -> nY             = nY;
    pNode -> pTex           = pTex;
    pNode -> iARGB          = iARGB;
    pNode -> rOnMouseUp     = _Node_Ripple_rOnMouseUp;
    pNode -> rOnPaint       = _Node_Ripple_rOnPaint;
    return pNode;
}
