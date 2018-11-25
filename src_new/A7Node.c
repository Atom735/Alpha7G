/* Создаёт новую Ноду. Если [p] == NULL, то выделяет под неё место */
S7Node *A7Node_New ( S7Node *p, UINT32 iType, UINT32 iFlags, S7NodeVtbl *pVtbl, LPCWSTR szName ) {
    p = ( S7Node* ) A7New ( ( S7Base* ) p, iType, iFlags, 0 );
    D7ERR_MALLOC ( A7New, p == NULL ) { return NULL; }
    p -> pVtbl  = pVtbl;
    p -> szName = szName;
    return p;
}
/* Удаляет Ноду */
VOID    A7Node_Release ( S7Node *p ) {
    if ( D7ERR_OTHER ( A7Node_Release, ( ( p == NULL ) || !DT7_VALID_NODE ( p -> _ . iType ) ), L"Некорректный аргумент функции" ) ) {
        return;
    }
    /* Удаляем всех детей */
    while ( p -> pChildLast != NULL ) {
        A7Release ( ( S7Base* ) p -> pChildLast );
    }
    /* Отвязываем элемент */
    if ( p -> pParentNode != NULL ) {
        if ( p -> pParentNode -> pChildLast == p ) {
            p -> pParentNode -> pChildLast = p -> pSiblingPrev;
        }
        if ( p -> pParentNode -> pChildFirst == p ) {
            p -> pParentNode -> pChildFirst = p -> pSiblingNext;
        }
    }
    if ( p -> pSiblingNext != NULL ) {
        p -> pSiblingNext -> pSiblingPrev = p -> pSiblingPrev;
    }
    if ( p -> pSiblingPrev != NULL ) {
        p -> pSiblingPrev -> pSiblingNext = p -> pSiblingNext;
    }
    switch ( p -> _ . iType ) {
        case DT7_NODE_ROOT:
            A7Release ( ( S7Base* ) ( & ( ( ( S7Node_Root* ) p ) -> gdi ) ) );
            return;
        case DT7_NODE_RIPPLE:
            return;
    }
    ( VOID ) D7ERR_OTHER ( A7Node_Release, TRUE, L"Неизвестный тип структуры S7Node" );
}
/* Добавляет [pNewChild] к [p] как последнего ребенка */
S7Node *A7Node_AppendChild ( S7Node *p, S7Node *pNewChild ) {
    if ( D7ERR_OTHER ( A7Node_AppendChild, ( ( p == NULL ) || !DT7_VALID_NODE ( p -> _ . iType ) || ( pNewChild == NULL ) || !DT7_VALID_NODE ( pNewChild -> _ . iType ) ), L"Некорректный аргумент функции" ) ) {
        return NULL;
    }
    pNewChild -> pParentNode = p;
    if ( p -> pChildLast == NULL ) {
        p -> pChildLast = pNewChild;
        p -> pChildFirst = pNewChild;
    } else {
        p -> pChildLast -> pSiblingNext = pNewChild;
        pNewChild -> pSiblingPrev = p -> pChildLast;
        p -> pChildLast = pNewChild;
    }
    return pNewChild;
}
/* Добавляет [pNewChild] к [p] как первого ребенка */
S7Node *A7Node_AddFirstChild ( S7Node *p, S7Node *pNewChild ) {
    if ( D7ERR_OTHER ( A7Node_AddFirstChild, ( ( p == NULL ) || !DT7_VALID_NODE ( p -> _ . iType ) || ( pNewChild == NULL ) || !DT7_VALID_NODE ( pNewChild -> _ . iType ) ), L"Некорректный аргумент функции" ) ) {
        return NULL;
    }
    pNewChild -> pParentNode = p;
    if ( p -> pChildFirst == NULL ) {
        p -> pChildFirst = pNewChild;
        p -> pChildLast = pNewChild;
    } else {
        p -> pChildFirst -> pSiblingPrev = pNewChild;
        pNewChild -> pSiblingNext = p -> pChildFirst;
        p -> pChildFirst = pNewChild;
    }
    return pNewChild;
}

UINT A7Node_Root_rOnFocus ( S7Node_Root *p ) {
    if ( D7ERR_OTHER ( A7Node_Root_rOnFocus, ( ( p == NULL ) || !DT7_VALID_NODE ( ( ( S7Base* ) p ) -> iType ) || ( ( ( S7Base* ) p ) -> iType != DT7_NODE_ROOT ) ), L"Некорректный аргумент функции" ) ) {
        return 1;
    }
    return 0;
}
UINT A7Node_Root_rOnBlur ( S7Node_Root *p ) {
    if ( D7ERR_OTHER ( A7Node_Root_rOnBlur, ( ( p == NULL ) || !DT7_VALID_NODE ( ( ( S7Base* ) p ) -> iType ) || ( ( ( S7Base* ) p ) -> iType != DT7_NODE_ROOT ) ), L"Некорректный аргумент функции" ) ) {
        return 1;
    }
    return 0;
}
UINT A7Node_Root_rOnResize ( S7Node_Root *p, UINT nW, UINT nH ) {
    if ( D7ERR_OTHER ( A7Node_Root_rOnResize, ( ( p == NULL ) || !DT7_VALID_NODE ( ( ( S7Base* ) p ) -> iType ) || ( ( ( S7Base* ) p ) -> iType != DT7_NODE_ROOT ) ), L"Некорректный аргумент функции" ) ) {
        return 1;
    }
    A7Tex_Resize_GDI ( & p -> gdi, nW, nH );
    return 0;
}
UINT A7Node_Root_rOnPaint ( S7Node_Root *p, HDC hDC ) {
    if ( D7ERR_OTHER ( A7Node_Root_rOnPaint, ( ( p == NULL ) || !DT7_VALID_NODE ( ( ( S7Base* ) p ) -> iType ) || ( ( ( S7Base* ) p ) -> iType != DT7_NODE_ROOT ) || ( hDC == NULL ) ), L"Некорректный аргумент функции" ) ) {
        return 1;
    }

    BOOL bAnim = 0;

    for ( S7Node *pC = ( ( S7Node* ) p ) -> pChildFirst; pC != NULL; pC = pC -> pSiblingNext ) {
        switch ( ( ( S7Base* ) pC ) -> iType ) {
            case DT7_NODE_RIPPLE:
                if ( I7Node_rOnPaint ( pC, & p -> gdi ) == 2 ) { ++bAnim; }
                break;
        }
    }

    BitBlt ( hDC, 0, 0, p -> gdi . _ . nWidth, p -> gdi . _ . nHeight, p -> gdi . hDC, 0, 0, SRCCOPY );
    return bAnim == 0 ? 0 : 2;
}

S7NodeVtbl g_A7NodeVtbl_Root = {
    .rOnFocus    = (R7NODE_ONFOCUS   ) A7Node_Root_rOnFocus,
    .rOnBlur     = (R7NODE_ONBLUR    ) A7Node_Root_rOnBlur,
    .rOnResize   = (R7NODE_ONRESIZE  ) A7Node_Root_rOnResize,
    .rOnPaint    = (R7NODE_ONPAINT   ) A7Node_Root_rOnPaint,
};


S7Node_Root *A7Node_New_Root ( S7Node_Root *p, HWND hWnd ) {
    p = ( S7Node_Root* ) A7Node_New ( ( S7Node* ) p, DT7_NODE_ROOT, 0, &g_A7NodeVtbl_Root, L"root" );
    D7ERR_MALLOC ( A7Node_New, p == NULL ) { return NULL; }
    HDC hDC = GetDC ( hWnd );
    D7ERR_WINAPI ( GetDC, hDC == NULL );
    A7Tex_New_GDI ( & p -> gdi, hDC, 1, 1 );
    ReleaseDC ( hWnd, hDC );
    return p;
}

S7Sets_Ripple g_A7Sets_Ripple = {
    .cARGB      = 0x7f000000,
    .fAnimBegin = 10000.0f,
    .fAnimEnd   = 10000.0f,
};


UINT A7Node_Ripple_rOnPaint ( S7Node_Ripple *p, S7Tex *gdi ) {
    if ( D7ERR_OTHER ( A7Node_Ripple_rOnPaint, ( ( p == NULL ) || !DT7_VALID_NODE ( ( ( S7Base* ) p ) -> iType ) || ( ( ( S7Base* ) p ) -> iType != DT7_NODE_RIPPLE ) || ( gdi == NULL ) || !DT7_VALID_TEX ( ( ( S7Base* ) gdi ) -> iType ) ), L"Некорректный аргумент функции" ) ) {
        return 1;
    }
    if ( ( ( S7Base* ) gdi ) -> iType == DT7_TEX_GDI_BGR24 ) {
        FLOAT a = 1.0f;
        if ( p -> iClockEnd != 0 ) {
            a = 1.0f - ( FLOAT ) ( clock () - p -> iClockEnd ) / p -> pSets -> fAnimEnd;
            if ( a < 0.0f ) {
                A7Release ( ( S7Base* ) p );
                return 0;
            }
        }
        FLOAT t = ( FLOAT ) ( clock () - p -> iClockBegin ) / p -> pSets -> fAnimBegin;
        if ( t > 1.0f ) t = 1.0f;
        UINT nW = 0, nH = 0;
        S7Base * pParent = ( S7Base* ) ( ( S7Node* ) p ) -> pParentNode;
        switch ( pParent -> iType ) {
            case DT7_NODE_ROOT:
                nW = ( ( S7Node_Root* ) pParent ) -> gdi . _ . nWidth;
                nH = ( ( S7Node_Root* ) pParent ) -> gdi . _ . nHeight;
                break;
        }
        CONST FLOAT fw = ( FLOAT ) nW * 0.5f, fh = ( FLOAT ) nH * 0.5f;
        CONST FLOAT fx = fw * t + (FLOAT) p -> nX * ( 1.0f - t );
        CONST FLOAT fy = fh * t + (FLOAT) p -> nY * ( 1.0f - t );
        CONST FLOAT fr = sqrtf ( fw * fw + fh * fh ) * t;
        CONST FLOAT fq = fr * fr;
        CONST FLOAT fQ = fq + 2.0f * fr + 1.0f;

        BYTE * CONST pbd = ( BYTE* ) ( ( S7Tex* ) gdi ) -> pData;
        CONST UINT cARGB = p -> pSets -> cARGB;
        CONST UINT cA = ( ( cARGB >> 030 ) & 0xff );
        CONST UINT cR = ( ( cARGB >> 020 ) & 0xff );
        CONST UINT cG = ( ( cARGB >> 010 ) & 0xff );
        CONST UINT cB = ( ( cARGB >> 000 ) & 0xff );
        CONST FLOAT A = ( FLOAT ) cA * a;

        for ( UINT iy = 0; iy < nH; ++iy ) {
            for ( UINT ix = 0; ix < nW; ++ix ) {
                CONST UINT _id = iy * ( ( S7Tex* ) gdi ) -> nStride + ix * 3;
                CONST FLOAT _x = (FLOAT) ix - fx;
                CONST FLOAT _y = (FLOAT) iy - fy;
                CONST FLOAT _q = _x * _x + _y * _y;
                CONST FLOAT _r = _q < fq ? 1.0f : _q > fQ ? 0.0f : fr - sqrtf ( _q ) + 1.0f;
                CONST UINT  _a = ( UINT ) ( A * ( _r > 1.0f ? 1.0f : _r > 0.0f ? _r : 0.0f ) );
                CONST UINT  _A = 0xff - _a;
                pbd [ _id + 0 ] = ( ( UINT ) pbd [ _id + 0 ] * _A + cB * _a ) / 0xff;
                pbd [ _id + 1 ] = ( ( UINT ) pbd [ _id + 1 ] * _A + cG * _a ) / 0xff;
                pbd [ _id + 2 ] = ( ( UINT ) pbd [ _id + 2 ] * _A + cR * _a ) / 0xff;
            }
        }

        return t == 1.0f ? 0 : 2;
    } else {
        ( VOID ) D7ERR_OTHER ( A7Node_Ripple_rOnPaint, ( TRUE ), L"Неподдерживаемая S7Tex" );
        return 1;
    }
    return 0;
}

S7NodeVtbl g_A7NodeVtbl_Ripple = {
    .rOnFocus    = (R7NODE_ONFOCUS   ) NULL,
    .rOnBlur     = (R7NODE_ONBLUR    ) NULL,
    .rOnResize   = (R7NODE_ONRESIZE  ) NULL,
    .rOnPaint    = (R7NODE_ONPAINT   ) A7Node_Ripple_rOnPaint,
};


S7Node_Ripple *A7Node_New_Ripple ( S7Node_Ripple *p, S7Sets_Ripple *pSets, UINT nX, UINT nY ) {
    p = ( S7Node_Ripple* ) A7Node_New ( ( S7Node* ) p, DT7_NODE_RIPPLE, 0, &g_A7NodeVtbl_Ripple, L"ripple" );
    D7ERR_MALLOC ( A7Node_New, p == NULL ) { return NULL; }
    p -> pSets          = pSets == NULL ? &g_A7Sets_Ripple : pSets;
    p -> nX             = nX;
    p -> nY             = nY;
    p -> iClockBegin    = clock ();
    p -> iClockEnd      = 0;
    return p;
}
