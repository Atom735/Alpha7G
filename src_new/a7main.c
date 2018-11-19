
/* Хендл приложения */
HINSTANCE       g_hInstance;
/* Имя класса главного окна */
CONST LPCWSTR   kg_szMainClassName = L"CWN-A7Main";
/* ATOM класса главного окна */
ATOM            g_iMainClassAtom;
/* Handle главного окна */
HWND            g_hWndMain;

FT_Library      g_ftLibrary;
FT_Face         g_ftFace;
FT_Face         g_ftFaceIcon;
FT_Face         g_ftFaceIcon2;

typedef struct _S7WindowData S7WindowData;
struct _S7WindowData {
    S7TexGdi    texGdiLayer;
    S7TexGlyph  texGlyph;
    S7StaticIcon iconLoading;
};


/* Название процедуры главного окна */
LRESULT CALLBACK A7MainWinProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    S7WindowData * p;
    SetLastError ( 0 );
    if ( ( p = ( S7WindowData* ) GetWindowLongPtrW ( hWnd, GWLP_USERDATA ) ) == NULL ) {
        D7ERR_WINAPI ( SetWindowLongPtrW );
    }
    switch ( uMsg ) {
        case WM_CREATE: {
            p = malloc ( sizeof ( S7WindowData ) );
            if ( p == NULL ) {
                SetLastError ( ERROR_OUTOFMEMORY );
                D7ERR_WINAPI ( malloc );
                return -1;
            }
            ZeroMemory ( p, sizeof ( S7WindowData ) );
            SetLastError ( 0 );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( p ) ) == 0 ) {
                D7ERR_WINAPI ( SetWindowLongPtrW );
            }

            p -> iconLoading . ftFace = g_ftFaceIcon;
            p -> iconLoading . iSymbol = 0xf276;
            p -> iconLoading . iARGB = 0xff117fff;
            p -> iconLoading . nHeight = 256 * 64;
            p -> iconLoading . nX = 256;
            p -> iconLoading . nY = 512;

            return 0;
        }
        case WM_SIZE: {
            HDC hDC = GetDC ( hWnd );
            if ( hDC == NULL ) {
                D7ERR_WINAPI ( GetDC );
                return 0;
            }
            A7TexCreate_GDI ( &( p -> texGdiLayer ), hDC, LOWORD ( lParam ), HIWORD ( lParam ), FALSE );


            ReleaseDC ( hWnd, hDC );
            return 0;
        }
        case WM_DESTROY: {
            A7TexFree ( ( S7Tex* ) &( p -> texGdiLayer ) );
            A7TexFree ( ( S7Tex* ) &( p -> texGlyph ) );
            free ( p );
            SetLastError ( 0 );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( NULL ) ) == 0 ) {
                D7ERR_WINAPI ( SetWindowLongPtrW );
            }
            PostQuitMessage ( 0 );
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );
            if ( hDC == NULL ) {
                D7ERR_WINAPI ( BeginPaint );
                return 0;
            }
            UINT nCWidth, nCHeight;
            {
                RECT rt;
                GetClientRect ( hWnd, &rt );
                nCWidth = rt.right - rt.left;
                nCHeight = rt.bottom - rt.top;
            }

            A7TexFillRect_FULL ( ( S7Tex* ) &( p -> texGdiLayer ), 0x00ff0000 );
            A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), 16, 16, 64, 64, 0x0000ff00 );
            A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), 32, 32, 32, 32, 0x000000ff );
            A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), 128, 128, 1024, 1024, 0xff000000 );

            p -> texGlyph . nAdvance = 0;
            for ( int i = 0; i < 16; ++i ) {
                p -> texGlyph . iGlyphIndex = 0;
                A7TexCreate_Glyph ( &( p -> texGlyph ), g_ftFace, L'Г', 72 * 64 );
                A7TexDrawAlphaMap ( ( S7Tex* ) &( p -> texGdiLayer ), ( S7Tex* ) &( p -> texGlyph ), 256, 256, 0x7f7f7fff, 0 );
                p -> texGlyph . iGlyphIndex = 0;
                A7TexCreate_Glyph ( &( p -> texGlyph ), g_ftFace, '.', 72 * 64 );
                A7TexDrawAlphaMap ( ( S7Tex* ) &( p -> texGdiLayer ), ( S7Tex* ) &( p -> texGlyph ), 256, 256, 0x7f7f7fff, 0 );
            }
            p -> texGlyph . nAdvance = 0;
            p -> texGlyph . iGlyphIndex = 0;
            for ( int i = 0; i < 16; ++i ) {
                A7TexCreate_Glyph ( &( p -> texGlyph ), g_ftFace, L'Г', 72 * 64 );
                A7TexDrawAlphaMap ( ( S7Tex* ) &( p -> texGdiLayer ), ( S7Tex* ) &( p -> texGlyph ), 256, 256, 0x3f00ff00, 0 );
                A7TexCreate_Glyph ( &( p -> texGlyph ), g_ftFace, '.', 72 * 64 );
                A7TexDrawAlphaMap ( ( S7Tex* ) &( p -> texGdiLayer ), ( S7Tex* ) &( p -> texGlyph ), 256, 256, 0x3f00ff00, 0 );
            }

            p -> iconLoading . fAngle = clock () * 0.0001f;


            A7TexDraw_StaticIcon ( ( S7Tex* ) &( p -> texGdiLayer ), & p -> iconLoading );

            CONST UINT _nHeight = 128;
            CONST UINT _nLineHeight = 156;
            CONST UINT _nX = 140;
            UINT _nY = 256;


            A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), _nX, _nY, 1024, 1, 0xff7f7f7f );

            for ( UINT j = 0; j < 3; ++j ) {
                FT_Face ftFace = j == 2 ?
                    g_ftFaceIcon2 : j == 1 ?
                    g_ftFaceIcon :
                    g_ftFace;
                LPCWSTR _szText = j == 2 ?
                    L"\n" : j == 1 ?
                    L"\n" :
                    L"Привет мир!\nWAWAWAWAWAWAWA\n0x12345678\n";
                FT_UInt iGlyphIndexLast = 0;

                D7ERR_FREETYPE ( FT_Set_Char_Size, ftFace, 0, _nHeight * 64, g7Tex_nDpiHorz, g7Tex_nDpiVert );

                FT_Vector pen = { .x = 0, .y = 0, };
                FT_Vector delta = { .x = 0, .y = 0, };

                for ( UINT i = 0; _szText[i] != 0; ++i ) {
                    FT_UInt iUnicode = _szText[i];
                    if ( iUnicode == '\n' ) {
                        _nY += _nLineHeight;
                        pen.x = 0;
                        iGlyphIndexLast = 0;
                        A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), _nX, _nY, 1024, 1, 0xff7f7f7f );
                        continue;
                    }
                    FT_UInt iGlyphIndex = FT_Get_Char_Index ( ftFace, iUnicode );
                    if ( ( FT_HAS_KERNING ( ftFace ) ) && ( iGlyphIndexLast != 0 ) && ( iGlyphIndex != 0 ) ) {
                        D7ERR_FREETYPE ( FT_Get_Kerning, ftFace, iGlyphIndexLast, iGlyphIndex, FT_KERNING_DEFAULT, &delta );
                    }
                    pen.x += delta.x;
                    FT_Set_Transform ( ftFace, NULL, &pen );
                    D7ERR_FREETYPE ( FT_Load_Glyph, ftFace, iGlyphIndex, FT_LOAD_RENDER );


                    FT_GlyphSlot glyph = ftFace -> glyph;
                    FT_Glyph_Metrics metrics = glyph -> metrics;

                    FT_Vector c = { .x = pen.x + metrics . horiBearingX + metrics . width / 2, .y = - metrics . horiBearingY + metrics . height / 2, };

                    CONST FLOAT f = clock ( ) * 0.01f;
                    CONST FLOAT w = (FLOAT) (c.x - pen.x);
                    CONST FLOAT h = (FLOAT) (c.y);
                    FT_Vector pen2 = {
                        .x = pen.x,
                        .y = 0,
                    };
                    FT_Matrix mat = {
                        .xx = 0x10000L,
                        .xy = 0x10000L,
                        .yx = 0,
                        .yy = 0x10000L,
                    };
                    iGlyphIndexLast = iGlyphIndex;
                    pen.x += glyph -> advance.x;

                    FT_Set_Transform ( ftFace, &mat, &pen2 );
                    D7ERR_FREETYPE ( FT_Load_Glyph, ftFace, iGlyphIndex, FT_LOAD_RENDER );

                    FT_Bitmap *bmp = & glyph -> bitmap;
                    p -> texGlyph . _tex . iType    = DT7_TEX_GLYPH_A8;
                    p -> texGlyph . _tex . nWidth   = bmp -> width;
                    p -> texGlyph . _tex . nHeight  = bmp -> rows;
                    p -> texGlyph . _tex . nStride  = bmp -> pitch;
                    p -> texGlyph . _tex . pData    = bmp -> buffer;
                    p -> texGlyph . nAdvance    = glyph -> advance.x;
                    p -> texGlyph . nOffsetLeft = glyph -> bitmap_left;
                    p -> texGlyph . nOffsetTop  =-glyph -> bitmap_top;

                    A7TexDrawAlphaMap ( ( S7Tex* ) &( p -> texGdiLayer ), ( S7Tex* ) &( p -> texGlyph ), _nX, _nY, 0xffffffff, 0 );

                    VOID _Fill ( INT x, INT y, INT w, INT h, UINT iARGB ) {
                        A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), _nX + x, _nY + y, w, h, iARGB );
                    }

                    _Fill ( glyph -> bitmap_left, -glyph -> bitmap_top, bmp -> width, 1, 0xffff00ff );
                    _Fill ( glyph -> bitmap_left, -glyph -> bitmap_top + bmp -> rows - 1, bmp -> width, 1, 0xffff00ff );


                    _Fill ( p -> texGlyph . nOffsetLeft, p -> texGlyph . nOffsetTop, 1, bmp -> rows, 0xffff00ff );
                    _Fill ( p -> texGlyph . nOffsetLeft + bmp -> width - 1, p -> texGlyph . nOffsetTop, 1, bmp -> rows, 0xffff00ff );

                    _Fill ( ( pen.x ) / 64 - 1, - 1, 3, 3, 0xffff0000 );

                    _Fill ( ( pen.x + glyph -> advance.x - delta.x )  / 64 - 2, - 2, 5, 5, 0xff00ff00 );




                    _Fill ( c.x / 64 - 2, c.y / 64- 2, 5, 5, 0xff0000ff );

                }
            }




            A7TexDraw_GDI_FULL ( hDC, 0, 0, &( p -> texGdiLayer ) );

            EndPaint ( hWnd, &ps );

            InvalidateRect ( hWnd, NULL, FALSE );
            return 0;
        }
    }
    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}



/* Точка входа в приложение */
INT APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd ) {
    g_hInstance = hInstance;

    /* Инициализация библиотеки FreeType */
    D7ERR_FREETYPE ( FT_Init_FreeType,  &g_ftLibrary );
    D7ERR_FREETYPE ( FT_New_Face, g_ftLibrary, "C:\\Windows\\Fonts\\times.ttf", 0, &g_ftFace );
    D7ERR_FREETYPE ( FT_New_Face, g_ftLibrary, "data\\Material-Design-Iconic-Font.ttf", 0, &g_ftFaceIcon );
    D7ERR_FREETYPE ( FT_New_Face, g_ftLibrary, "F:\\fonts\\MaterialIcons-Regular.ttf", 0, &g_ftFaceIcon2 );;



    /* Регистрация класса главного окна */
    {
        WNDCLASSEXW wc = {
            .cbSize        = sizeof ( WNDCLASSEXW ),
            .style         = CS_VREDRAW | CS_HREDRAW,
            .lpfnWndProc   = A7MainWinProc,
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = g_hInstance,
            .hIcon         = NULL,
            .hCursor       = NULL,
            .hbrBackground = NULL,
            .lpszMenuName  = NULL,
            .lpszClassName = kg_szMainClassName,
            .hIconSm       = NULL,
        };
        if ( ( g_iMainClassAtom = RegisterClassExW ( &wc ) ) == 0 ) {
            D7ERR_WINAPI ( RegisterClassExW );
        }
    }

    /* Создание главного окна */
    if ( ( g_hWndMain = CreateWindowExW ( 0, ( ( LPCWSTR ) ( g_iMainClassAtom ) ), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, NULL ) ) == NULL ) {
        D7ERR_WINAPI ( CreateWindowExW );
    }

    // ShowWindow ( g_hWndMain, nCmdShow );

    /* Входим в цикл обработки сообщений */
    {
        MSG msg = { };
        while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }
    }

    /* Освобождение класса главного окна */
    if ( ! UnregisterClassW ( kg_szMainClassName, g_hInstance ) ) {
        D7ERR_WINAPI ( UnregisterClassW );
    }

    D7ERR_FREETYPE ( FT_Done_Face, g_ftFaceIcon );
    D7ERR_FREETYPE ( FT_Done_Face, g_ftFace );
    D7ERR_FREETYPE ( FT_Done_FreeType, g_ftLibrary );

    return 0;
}













