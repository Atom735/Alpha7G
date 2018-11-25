/* Получает размер типа структуры */
UINT    A7SizeOf ( UINT32 iType ) {
    switch ( iType ) {
        case DT7_BASE:          return sizeof ( S7Base );

        case DT7_TEX:           return sizeof ( S7Tex );
        case DT7_TEX_GDI_BGR24: return sizeof ( S7TexGdi );

        case DT7_NODE:          return sizeof ( S7Node );
        case DT7_NODE_ROOT:     return sizeof ( S7Node_Root );
    }
    return 0;
}
/* Создаёт новый объект S7Base. Если [p] == NULL, то выделяет под неё место и дополнительные [nExtraSize] */
S7Base *A7New ( S7Base *p, UINT32 iType, UINT32 iFlags, UINT nExtraSize ) {
    BOOL bAlloc = ( p == NULL );
    if ( bAlloc ) {
        p = ( S7Base* ) malloc ( A7SizeOf ( iType ) + nExtraSize );
        D7ERR_MALLOC ( malloc, p == NULL ) { return NULL; }
    }
    ZeroMemory ( p, A7SizeOf ( iType ) + nExtraSize );
    p -> iType  = iType;
    p -> iFlags = bAlloc ? iFlags | DF7_ALLOCATED : iFlags;
    return p;
}
/* Удаляет объект S7 */
VOID    A7Release ( S7Base *p ) {
    if ( D7ERR_OTHER ( A7Release, ( ( p == NULL ) || !DT7_VALID_S7 ( p -> iType ) ), L"Некорректный аргумент функции" ) ) {
        return;
    }
    if ( DT7_VALID_NODE ( p -> iType ) ) { A7Node_Release ( ( S7Node* ) p ); }
    if ( DT7_VALID_TEX ( p -> iType ) ) { A7Tex_Release ( ( S7Node* ) p ); }
    if ( p -> iFlags & DF7_ALLOCATED ) { free ( p ); }
}
