/* Создаёт новую Текстуру. Если [p] == NULL, то выделяет под неё место, и Если [pData] == NULL, то и под неё тоже */
S7Tex  *A7Tex_New ( S7Tex *p, UINT32 iType, UINT32 iFlags, UINT nWidth, UINT nHeight, UINT nStride, VOID *pData ) {
    p = ( S7Tex* ) A7New ( ( S7Base* ) p, iType, iFlags, pData == NULL ? ( nStride == 0 ? nWidth : nStride ) * nHeight : 0 );
    D7ERR_MALLOC ( A7New, p == NULL ) { return NULL; }
    p -> nWidth  = nWidth;
    p -> nHeight = nHeight;
    p -> nStride = ( nStride == 0 ? nWidth : nStride );
    p -> pData   = pData == NULL ? ( BYTE* ) ( p ) + A7SizeOf ( iType ) : pData;
    return p;
}
/* Удаляет Текстуру */
VOID    A7Tex_Release ( S7Tex *p ) {
    if ( D7ERR_OTHER ( A7Tex_Release, ( ( p == NULL ) || !DT7_VALID_TEX ( p -> _ . iType ) ), L"Некорректный аргумент функции" ) ) {
        return;
    }
    switch ( p -> _ . iType ) {
        case DT7_TEX_GDI_BGR24:
            if ( ( ( S7TexGdi* ) p ) -> hBMP != NULL ) {
                DeleteObject ( ( ( S7TexGdi* ) p ) -> hBMP );
                ( ( S7TexGdi* ) p ) -> hBMP = NULL;
            }
            if ( ( ( S7TexGdi* ) p ) -> hDC != NULL ) {
                DeleteDC ( ( ( S7TexGdi* ) p ) -> hDC );
                ( ( S7TexGdi* ) p ) -> hDC = NULL;
            }
            return;
    }
    ( VOID ) D7ERR_OTHER ( A7Tex_Release, TRUE, L"Неизвестный тип структуры S7Tex" );
}

S7TexGdi* A7Tex_New_GDI ( S7TexGdi *p, HDC hDC, UINT nWidth, UINT nHeight ) {
    CONST UINT nW3 = nWidth * 3;
    p = ( S7TexGdi* ) A7Tex_New ( ( S7Tex* ) p, DT7_TEX_GDI_BGR24, 0, nWidth, nHeight, nW3 & 3 ? ( nW3 + 4 ) ^ ( nW3 & 3 ) : nW3, ( VOID* ) 0x100 );
    D7ERR_MALLOC ( A7Tex_New, p == NULL ) { return NULL; }
    BITMAPINFO bmi;
    ZeroMemory ( &bmi, sizeof ( BITMAPINFO ) );
    bmi.bmiHeader.biSize        = sizeof ( BITMAPINFOHEADER ) ;
    bmi.bmiHeader.biWidth       = nWidth;
    bmi.bmiHeader.biHeight      = -nHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = 0;
    p -> hDC  = CreateCompatibleDC ( hDC );
    p -> hBMP = CreateDIBSection ( p -> hDC, &bmi, DIB_RGB_COLORS, ( VOID** ) &( p -> _ . pData ), NULL, 0x0 );
    SelectObject ( p -> hDC, p -> hBMP );
    return p;
}

S7TexGdi* A7Tex_Resize_GDI ( S7TexGdi *p, UINT nWidth, UINT nHeight ) {
    if ( D7ERR_OTHER ( A7Tex_Resize_GDI, ( ( p == NULL ) || !DT7_VALID_TEX ( p -> _ . _ . iType ) ), L"Некорректный аргумент функции" ) ) {
        return NULL;
    }
    BITMAPINFO bmi;
    ZeroMemory ( &bmi, sizeof ( BITMAPINFO ) );
    bmi.bmiHeader.biSize        = sizeof ( BITMAPINFOHEADER ) ;
    bmi.bmiHeader.biWidth       = nWidth;
    bmi.bmiHeader.biHeight      = -nHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = 0;

    switch ( p -> _ . _ . iType ) {
        case DT7_TEX_GDI_BGR24:
            if ( p -> hBMP != NULL ) {
                DeleteObject ( p -> hBMP );
            }
            p -> _ . nWidth     = nWidth;
            p -> _ . nHeight    = nHeight;
            CONST UINT nW3 = nWidth * 3;
            p -> _ . nStride    = nW3 & 3 ? ( nW3 + 4 ) ^ ( nW3 & 3 ) : nW3;
            p -> hBMP = CreateDIBSection ( p -> hDC, &bmi, DIB_RGB_COLORS, ( VOID** ) & ( p -> _ . pData ), NULL, 0x0 );
            SelectObject ( p -> hDC, p -> hBMP );

            return p;
    }
    ( VOID ) D7ERR_OTHER ( A7Tex_Resize_GDI, TRUE, L"Неизвестный тип структуры S7TexGdi" );
    return NULL;
}
