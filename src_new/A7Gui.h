#ifndef _H_A7GUI_H_
#define _H_A7GUI_H_

/* Плотность дисплея */
extern FT_F26Dot6 g7Tex_nDpiHorz;
extern FT_F26Dot6 g7Tex_nDpiVert;

typedef struct _S7Tex           S7Tex;
typedef struct _S7TexGdi        S7TexGdi;
typedef struct _S7TexShape      S7TexShape;
typedef struct _S7TexGlyph      S7TexGlyph;

typedef struct _S7StaticText    S7StaticText;
typedef struct _S7Shape         S7Shape;
typedef struct _S7SetsButton    S7SetsButton;
typedef struct _S7Button        S7Button;

/* Типы структуры S7 */
enum {
    DT7_NULL = 0,

    DT7_TEX_NULL = 0x10,
    DT7_TEX_GLYPH_A8,
    DT7_TEX_SHAPE9_A8,
    DT7_TEX_GDI_BGR24,
    DT7_TEX_GDI_BGRA32,

    DT7_SETS_BUTTON,

    DT7_STATIC_TEXT,

    DT7_BUTTON,
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

struct _S7TexShape {
    S7Tex   _tex;
    UINT    nX, nY;
};

struct _S7TexGlyph {
    S7Tex       _tex;
    FT_F26Dot6  nAdvance;
    FT_F26Dot6  nOffsetX, nOffsetY;
    FT_UInt     iGlyphIndex;
    INT         nOffsetLeft, nOffsetTop;
};

struct _S7StaticText {
    UINT        iType;
    FT_Face     ftFace;
    BYTE        *pText;
    UINT32      iARGB;
    UINT32      iFlags;
    FT_F26Dot6  nHeight,
                nX, nY,
                nLineHeight, nLineWidth,
                nTracking;
    FT_Fixed    nOblique;
};



S7TexGdi* A7TexCreate_GDI ( S7TexGdi *pDst, HDC hDC, UINT nWidth, UINT nHeight, BOOL bAlpha );
S7TexGlyph* A7TexCreate_Glyph ( S7TexGlyph *pDst, FT_Face ftFace, FT_UInt iUnicode, FT_F26Dot6 nHeight );
VOID A7TexFree ( S7Tex *p );

VOID A7TexDraw_GDI ( HDC hDC, UINT nDX, UINT nDY, S7TexGdi * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH );
#define A7TexDraw_GDI_FULL(_hDC,_nDX,_nDY,_pSrc) A7TexDraw_GDI ( _hDC, _nDX, _nDY, _pSrc, 0, 0, ( _pSrc ) -> _tex . nWidth, ( _pSrc ) -> _tex . nHeight )

VOID A7TexFillRect ( S7Tex * pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB );
#define A7TexFillRect_FULL(_pDst,_iARGB) A7TexFillRect ( _pDst, 0, 0, ( _pDst ) -> nWidth, ( _pDst ) -> nHeight, _iARGB )

VOID A7TexDrawAlphaMap ( S7Tex *pDst, S7Tex *pSrc, UINT nX, UINT nY, UINT32 iARGB, UINT32 iFlags );
#define A7TexDrawAlphaMapGlyph(_pDst,_pTexGlyph,_ftFace,_iUnicode,_nHeight,_nX,_nY,_iARGB) \
    A7TexDrawAlphaMap ( _pDst, ( S7Tex* ) A7TexCreate_Glyph ( _pTexGlyph, _ftFace, _iUnicode, _nHeight ), _nX, _nY, _iARGB, 0 )



#define D7TEXDAM_MIRROR_HORI    (0x80000000>>0)
#define D7TEXDAM_MIRROR_VERT    (0x80000000>>1)

#define D7_ARGB(_a,_r,_g,_b) ( ( ( ( _a ) & 0xff ) << 030 ) | ( ( ( _r ) & 0xff ) << 020 ) | ( ( ( _g ) & 0xff ) << 010 ) | ( ( ( _b ) & 0xff ) ) )
#define D7_RGBA(_r,_g,_b,_a) D7_ARGB ( _a,_r,_g,_b )

VOID A7TexDraw_Button ( S7Tex *pDst );






#define D7GUITEXT_ALIGN_LEFT    (0)
#define D7GUITEXT_ALIGN_RIGHT   (0x100<<0)
#define D7GUITEXT_ALIGN_CENTER  (0x100<<1)
#define D7GUITEXT_LEFT2RIGHT    (0)
#define D7GUITEXT_RIGHT2LEFT    (0x100<<2)
#define D7GUITEXT_NO_KERNING    (0)
#define D7GUITEXT_KERNING       (0x100<<3)
#define D7GUITEXT_AA_MONO       (0)
#define D7GUITEXT_AA_GRAYSCALE  (0x100<<4)
#define D7GUITEXT_AA_LCD        (0x100<<5)
#define D7GUITEXT_AA_LCD_INV    (0x100<<6)

typedef struct _S7GuiTextSets S7GuiTextSets;
struct _S7GuiTextSets {
    UINT        iType;
    FT_Face     ftFace;
    UINT32      iARGB;
    UINT32      iFlags;
    FT_F26Dot6  nHeight, /* 1/64 px */
                nOffsetX, nOffsetY,
                nLineHeight, nLineWidth,
                nTracking;
    FT_Fixed    nOblique;  /* 0x10000L */
};

/* Отрисовать закруглённую форму */
VOID A7GuiDraw_ShapeRound ( S7Tex *pDst, UINT nX, UINT nY, UINT nW, UINT nH, UINT32 iARGB, FLOAT fR );
/* Отрисовать текст */
VOID A7GuiDraw_TextWide ( S7Tex *pDst, UINT nX, UINT nY, LPCWSTR pText, S7GuiTextSets *pSets );

#endif /* _H_A7GUI_H_ */
