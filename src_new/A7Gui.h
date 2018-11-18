#ifndef _H_A7GUI_H_
#define _H_A7GUI_H_

/* Плотность дисплея */
extern FT_F26Dot6 g7Tex_nDpiHorz;
extern FT_F26Dot6 g7Tex_nDpiVert;

typedef struct _S7Tex           S7Tex;
typedef struct _S7TexGdi        S7TexGdi;
typedef struct _S7TexGlyph      S7TexGlyph;

typedef struct _S7StaticText    S7StaticText;
typedef struct _S7StaticIcon    S7StaticIcon;

/* Типы структуры S7 */
enum {
    DT7_NULL = 0,

    DT7_TEX_NULL = 0x10,
    DT7_TEX_GDI_BGR24,
    DT7_TEX_GDI_BGRA32,
    DT7_TEX_GLYPH_A8,

    DT7_STATIC_ICON,
    DT7_STATIC_TEXT,
};

struct _S7Tex {
    UINT    iType;
    BOOL    bAllocated;
    UINT    nWidth, nHeight,
            nStride;
    BYTE   *pData;
};

struct _S7TexGdi {
    S7Tex   _tex;
    HDC     hDC;
    HBITMAP hBMP;
};

struct _S7TexGlyph {
    S7Tex       _tex;
    FT_F26Dot6  nAdvance;
    FT_UInt     iGlyphIndex;
    INT         nOffsetLeft;
    INT         nOffsetTop;
};

struct _S7StaticText {
    UINT        iType;
    FT_Face     ftFace;
    BYTE        *pText;
    UINT32      iARGB;
    FT_F26Dot6  nHeight,
                nOffsetX, nOffsetY,
                nLineHeight, nLineWidth,
                nTracking;
};

struct _S7StaticIcon {
    UINT        iType;
    FT_Face     ftFace;
    FT_UInt     iSymbol;
    UINT32      iARGB;
    FT_F26Dot6  nHeight,
                nOffsetX, nOffsetY;
};


S7TexGdi* A7TexCreate_GDI ( S7TexGdi *pDst, HDC hDC, UINT nWidth, UINT nHeight, BOOL bAlpha );
S7TexGlyph* A7TexCreate_Glyph ( S7TexGlyph *pDst, FT_Face ftFace, FT_UInt iUnicode, FT_F26Dot6 nHeight, FT_F26Dot6 nOffsetX, FT_F26Dot6 nOffsetY );
VOID A7TexFree ( S7Tex *p );

VOID A7TexDraw_GDI ( HDC hDC, UINT nDX, UINT nDY, S7TexGdi * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH );
#define A7TexDraw_GDI_FULL(_hDC,_nDX,_nDY,_pSrc) A7TexDraw_GDI ( _hDC, _nDX, _nDY, _pSrc, 0, 0, ( _pSrc ) -> _tex . nWidth, ( _pSrc ) -> _tex . nHeight )

VOID A7TexFillRect ( S7Tex * pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB );
#define A7TexFillRect_FULL(_pDst,_iARGB) A7TexFillRect ( _pDst, 0, 0, ( _pDst ) -> nWidth, ( _pDst ) -> nHeight, _iARGB )

#define D7_ARGB(_a,_r,_g,_b) ( ( ( ( _a ) & 0xff ) << 030 ) | ( ( ( _r ) & 0xff ) << 020 ) | ( ( ( _g ) & 0xff ) << 010 ) | ( ( ( _b ) & 0xff ) ) )
#define D7_RGBA(_r,_g,_b,_a) D7_ARGB ( _a,_r,_g,_b )

#endif /* _H_A7GUI_H_ */
