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


UINT A7Node_Root_rOnFocus ( S7Node_Root *p ) {
    return 0;
}
UINT A7Node_Root_rOnBlur ( S7Node_Root *p ) {
    return 0;
}
UINT A7Node_Root_rOnResize ( S7Node_Root *p, UINT nW, UINT nH ) {
    A7Tex_Resize_GDI ( & p -> gdi, nW, nH );
    return 0;
}
UINT A7Node_Root_rOnPaint ( S7Node_Root *p, HDC hDC ) {
    BitBlt ( hDC, 0, 0, p -> gdi . _ . nWidth, p -> gdi . _ . nHeight, p -> gdi . hDC, 0, 0, SRCCOPY );
    return 0;
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
