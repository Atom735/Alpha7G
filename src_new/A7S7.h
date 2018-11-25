#ifndef _H_A7S7_H_
#define _H_A7S7_H_


typedef struct _S7Base S7Base;
typedef struct _S7Tex S7Tex;
typedef struct _S7TexGdi S7TexGdi;
typedef struct _S7NodeVtbl S7NodeVtbl;
typedef struct _S7Node S7Node;
typedef struct _S7Node_Root S7Node_Root;


/* Дефайны на проверку соотетсия типов */
#define DT7_VALID_S7(_i)        ( ( _i & 0xff000000 ) == DT7_BASE )
#define DT7_VALID_TEX(_i)       ( ( _i & 0xffff0000 ) == DT7_TEX )
#define DT7_VALID_NODE(_i)      ( ( _i & 0xffff0000 ) == DT7_NODE )
/* Дефайны на битовые поля iFlag */
#define DF7_ALLOCATED (1<<31)
/* Перечесление поля iType */
enum {
    DT7_BASE = 0xA7000000,

    DT7_TEX = 0xA70A0000,
    DT7_TEX_GDI_BGR24,

    DT7_NODE = 0xA70D0000,
    DT7_NODE_ROOT,
    DT7_NODE_RIPPLE,

};

/* Базовая структура S7 */
struct _S7Base {
    UINT32  iType, iFlags;
};

/* Получает размер типа структуры */
UINT    A7SizeOf ( UINT32 iType );
/* Создаёт новый объект S7Base. Если [pThis] == NULL, то выделяет под неё место и дополнительные [nExtraSize] */
S7Base *A7New ( S7Base *pThis, UINT32 iType, UINT32 iFlags, UINT nExtraSize );
/* Удаляет объект S7 */
VOID    A7Release ( S7Base *pThis );


/* Структуры Текстур */
struct _S7Tex {
    S7Base  _;
    UINT    nWidth, nHeight, nStride;
    VOID   *pData;
};

/* Создаёт новую Текстуру. Если [pThis] == NULL, то выделяет под неё место, и Если [pData] == NULL, то и под неё тоже */
S7Tex  *A7Tex_New ( S7Tex *pThis, UINT32 iType, UINT32 iFlags, UINT nWidth, UINT nHeight, UINT nStride, VOID *pData );
/* Удаляет Текстуру */
VOID    A7Tex_Release ( S7Tex *pThis );

struct _S7TexGdi {
    S7Tex   _;
    HDC     hDC;
    HBITMAP hBMP;
};
S7TexGdi* A7Tex_New_GDI ( S7TexGdi *pThis, HDC hDC, UINT nWidth, UINT nHeight );
S7TexGdi* A7Tex_Resize_GDI ( S7TexGdi *pThis, UINT nWidth, UINT nHeight );




/* Структуры Узлов */

#define I7Node_rOnFocus(p)          ( ( S7Node* ) ( p ) ) -> pVtbl -> rOnFocus ( ( S7Node* ) ( p ) )
#define I7Node_rOnBlur(p)           ( ( S7Node* ) ( p ) ) -> pVtbl -> rOnBlur ( ( S7Node* ) ( p ) )
#define I7Node_rOnResize(p,w,h)     ( ( S7Node* ) ( p ) ) -> pVtbl -> rOnResize ( ( S7Node* ) ( p ), w, h )
#define I7Node_rOnPaint(p,t)     ( ( S7Node* ) ( p ) ) -> pVtbl -> rOnPaint ( ( S7Node* ) ( p ), ( S7Tex* ) t )

typedef UINT (*R7NODE_ONFOCUS   )( S7Node *pThis );
typedef UINT (*R7NODE_ONBLUR    )( S7Node *pThis );
typedef UINT (*R7NODE_ONRESIZE  )( S7Node *pThis, UINT nW, UINT nH);
typedef UINT (*R7NODE_ONPAINT   )( S7Node *pThis, S7Tex *pTarget );


struct _S7NodeVtbl {
    R7NODE_ONFOCUS  rOnFocus;
    R7NODE_ONBLUR   rOnBlur;
    R7NODE_ONRESIZE rOnResize;
    R7NODE_ONPAINT  rOnPaint;
};
struct _S7Node {
    S7Base      _;
    S7NodeVtbl *pVtbl;
    LPCWSTR     szName;

    S7Node      *pParentNode,
                *pSiblingNext, *pSiblingPrev,
                *pChildFirst,  *pChildLast;
};


/* Создаёт новую Ноду. Если [pThis] == NULL, то выделяет под неё место */
S7Node *A7Node_New ( S7Node *pThis, UINT32 iType, UINT32 iFlags, S7NodeVtbl *pVtbl, LPCWSTR szName );
/* Удаляет Ноду */
VOID    A7Node_Release ( S7Node *pThis );
/* Добавляет [pNewChild] к [pThis] как последнего ребенка */
S7Node *A7Node_AppendChild ( S7Node *pThis, S7Node *pNewChild );
/* Добавляет [pNewChild] к [pThis] как первого ребенка */
S7Node *A7Node_AddFirstChild ( S7Node *pThis, S7Node *pNewChild );


struct _S7Node_Root {
    S7Node      _;
    S7TexGdi    gdi; /* GDI BGR24, которая рисуется в клиентскую область */
};
S7Node_Root *A7Node_New_Root ( S7Node_Root *pThis, HWND hWnd );

/* Структура настроек эффекта Ripple */
typedef struct _S7Sets_Ripple S7Sets_Ripple;
struct _S7Sets_Ripple {
    UINT32      cARGB; /* Цвет эффекта */
    FLOAT       fAnimBegin; /* Время анимции полного заполнения в мс */
    FLOAT       fAnimEnd; /* Время анимции полного исчезновения в мс */
};
/* Максимальные размеры эффекта и центр, беруться от родителя. */
typedef struct _S7Node_Ripple S7Node_Ripple;
struct _S7Node_Ripple {
    S7Node      _;
    S7Sets_Ripple *pSets;
    UINT        nX, nY; /* Позиция центра эффекта */
    UINT        iClockBegin; /* Время начала */
    UINT        iClockEnd; /* Время конца */
};
S7Node_Ripple *A7Node_New_Ripple ( S7Node_Ripple *pThis, S7Sets_Ripple *pSets, UINT nX, UINT nY );



#endif /* _H_A7S7_H_ */
