#ifndef _H_A7BMP_H_
#define _H_A7BMP_H_

#include <Windows.h>

#include <ft2build.h>
#include FT_FREETYPE_H

/* Структура текстур */
typedef struct _S7Bmp S7Bmp;
struct _S7Bmp {
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
};

enum {
    D7BMP_NULL = 0,
    /* Обычные A7 карты */
    D7BMP_A7_R, /* GEN_TAB */
    D7BMP_A7_RG,
    D7BMP_A7_RGB, /* JPEG */
    D7BMP_A7_RGBA,
    /* GDI карты */
    D7BMP_GDI_RGB, /* GDI RGB */
    D7BMP_GDI_RGBA, /* GDI RGB+Alpha */
    /* Glyph Alpha карта */
    D7BMP_GLYPH, /* FREETYPE */
};

/* Копирование части текстуры в другую текстуру */
VOID A7BmpCopy ( S7Bmp * pDst, UINT nDX, UINT nDY, S7Bmp * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH );
/* Копирование максимально большей части текстуры в другую текстуру */
VOID A7BmpCopyFull ( S7Bmp * pDst, UINT nDX, UINT nDY, S7Bmp * pSrc );



/* Создание новой текстуры */
S7Bmp *A7BmpCreate ( UINT iType, UINT nWidth, UINT nHeight, UINT nStride, VOID *pData );
/* Освобождение ресурсов текстуры */
VOID A7BmpFree ( S7Bmp * p );



/* Загрузка текстуры из Jpeg файла */
S7Bmp *A7BmpCreateByJpegFileA ( LPCSTR szFileName );
/* Создание текстуры в контексте GDI */
S7Bmp *A7BmpCreateByGDI ( HDC hDC, UINT nWidth, UINT nHeight, BOOL bAlpha );
/* Создание маски символа ( является временной ссылкой ) */
S7Bmp *A7BmpCreateByFreeType ( S7Bmp *o, FT_Face ftFace, UINT iUnicode, UINT nGlyphHeight, UINT nGlyphOffsetX );



/* Рисует прямоугольник с указаным цветом на цветовую карту */
VOID A7BmpDrawRect ( S7Bmp *pDst, UINT nX, UINT nY, UINT nW, UINT nH, BYTE cR, BYTE cG, BYTE cB );
VOID A7BmpDrawRectA ( S7Bmp *pDst, UINT nX, UINT nY, UINT nW, UINT nH, BYTE cR, BYTE cG, BYTE cB, BYTE cA );


/* Рисует маску с указаным цветом на цветовую карту */
VOID A7BmpDrawAlphaMap ( S7Bmp *pDst, S7Bmp *pSrc, UINT nX, UINT nY, BYTE cR, BYTE cG, BYTE cB, BOOL bMirrorX );
VOID A7BmpDrawAlphaMapA ( S7Bmp *pDst, S7Bmp *pSrc, UINT nX, UINT nY, BYTE cR, BYTE cG, BYTE cB, BYTE cA, BOOL bMirrorX );



/* Рисует часть GDI текстуры на контекст */
VOID A7BmpDraw_GDI ( HDC hDC, UINT nDX, UINT nDY, S7Bmp * pSrc, UINT nSX, UINT nSY, UINT nW, UINT nH );
/* Рисует максимально наибольшую часть GDI текстуры на контекст */
VOID A7BmpDrawFull_GDI ( HDC hDC, UINT nDX, UINT nDY, S7Bmp * pSrc );



/* Получает длину строки в текселях */
UINT A7BmpGetStringWidth ( FT_Face ftFace, CONST CHAR *pStr, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking );
/* Рисует строку на текстуру */
VOID A7BmpDrawText ( S7Bmp *pDst, FT_Face ftFace, CONST CHAR *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB );
VOID A7BmpDrawTextA ( S7Bmp *pDst, FT_Face ftFace, CONST CHAR *pStr, UINT nX, UINT nY, UINT nGlyphHeight, UINT nGlyphOffsetX, UINT nTracking, BYTE cR, BYTE cG, BYTE cB, BYTE cA );



/* Генерация маски вкладки */
S7Bmp *A7BmpGen_Tab ( CONST UINT nH, CONST FLOAT _an, CONST FLOAT _r0, CONST FLOAT _r1 );




#endif /* _H_A7BMP_H_ */
