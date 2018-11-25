
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

typedef struct _S7WindowData S7WindowData;
struct _S7WindowData {
    S7TexGdi    texGdiLayer;
    S7Node_Root root;
};


/* Название процедуры главного окна */
LRESULT CALLBACK A7MainWinProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    S7WindowData * p;
    SetLastError ( 0 );
    if ( ( p = ( S7WindowData* ) GetWindowLongPtrW ( hWnd, GWLP_USERDATA ) ) == NULL ) {
        D7ERR_WINAPI ( SetWindowLongPtrW );
    }

    static INT      iMouseX = 0;
    static INT      iMouseY = 0;
    static BOOL     iMouseI = 0;

    static S7GuiTextSets Input_TSets = {
                .iType = 0,
                .ftFace = NULL,
                .iARGB = 0xff000000,
                .iFlags = D7GUITEXT_AA_LCD,
                .nHeight = 64*24,
                .nOffsetX = 0,
                .nOffsetY = 0,
                .nLineHeight = 64*32,
                .nLineWidth = 0,
                .nTracking = 0,
                .nOblique = 0,
            };


    static BOOL Input_bFocus0 = FALSE;
    static BOOL Input_bHover0 = FALSE;
    static UINT Input_nX0 = 16;
    static UINT Input_nY0 = 16;
    static UINT Input_nW0 = 256;
    static UINT Input_nH0 = 64;
    static LPCWSTR Input_sz0 = L"Телефон или email";

    static BOOL Input_bFocus1 = FALSE;
    static BOOL Input_bHover1 = FALSE;
    static UINT Input_nX1 = 16;
    static UINT Input_nY1 = 16+64+16;
    static UINT Input_nW1 = 64;
    static UINT Input_nH1 = 256;
    static LPCWSTR Input_sz1 = L"Пароль";



    switch ( uMsg ) {
        case WM_LBUTTONDOWN: {

            iMouseX = GET_X_LPARAM ( lParam );
            iMouseY = GET_Y_LPARAM ( lParam );
            BOOL bFocus = FALSE;
            bFocus = ( iMouseX >= Input_nX0 ) && ( iMouseY >= Input_nY0 ) && ( A7GuiAlpha_PointInShapeRound9 ( iMouseX - Input_nX0, iMouseY - Input_nY0, 10.0f, Input_nW0, Input_nH0, 0x3 ) > 0.5f );
            Input_bFocus0 = bFocus;

            bFocus = ( iMouseX >= Input_nX1 ) && ( iMouseY >= Input_nY1 ) && ( A7GuiAlpha_PointInShapeRound9 ( iMouseX - Input_nX1, iMouseY - Input_nY1, 10.0f, Input_nW1, Input_nH1, 0x3 ) > 0.5f );
            Input_bFocus1 = bFocus;


            A7Node_AppendChild ( & p -> nodeRoot, A7Node_CreateNew_Ripple ( NULL, iMouseX, iMouseY, & p -> texGdiLayer . _tex, 0x7f007fff ) );


            for ( S7Node *pNode = p -> nodeRoot . pChildFirst; pNode != NULL; pNode = pNode -> pSiblingNext ) {
                if ( pNode -> rOnMouseDown != NULL ) {
                    pNode -> rOnMouseDown ( pNode, iMouseX, iMouseY, 0 );
                }
            }

            InvalidateRect ( hWnd, NULL, FALSE );


            return 0;
        }

        case WM_LBUTTONUP: {

            iMouseX = GET_X_LPARAM ( lParam );
            iMouseY = GET_Y_LPARAM ( lParam );



            A7Node_AppendChild ( & p -> nodeRoot, A7Node_CreateNew_Ripple ( NULL, iMouseX, iMouseY, & p -> texGdiLayer . _tex, 0x7fff7f00 ) );


            for ( S7Node *pNode = p -> nodeRoot . pChildFirst; pNode != NULL; pNode = pNode -> pSiblingNext ) {
                if ( pNode -> rOnMouseUp != NULL ) {
                    pNode -> rOnMouseUp ( pNode, iMouseX, iMouseY, 0 );
                }
            }

            InvalidateRect ( hWnd, NULL, FALSE );

            return 0;
        }
        case WM_MOUSELEAVE: {
            iMouseI = 0;
            InvalidateRect ( hWnd, NULL, FALSE );
            BOOL bHover = FALSE;
            Input_bHover0 = bHover;
            Input_bHover1 = bHover;
            return 0;
        }
        case WM_MOUSEMOVE: {
            iMouseX = GET_X_LPARAM ( lParam );
            iMouseY = GET_Y_LPARAM ( lParam );
            if ( iMouseI == 0 ) {
                iMouseI = 1;
                TRACKMOUSEEVENT tme = {
                    .cbSize = sizeof ( TRACKMOUSEEVENT ),
                    .dwFlags = TME_LEAVE,
                    .hwndTrack = hWnd,
                    .dwHoverTime = 0,
                };
                if ( TrackMouseEvent ( &tme ) == FALSE ) {
                    D7ERR_WINAPI ( TrackMouseEvent );
                }
            }

            BOOL bHover = FALSE;


            bHover = ( iMouseX >= Input_nX0 ) && ( iMouseY >= Input_nY0 ) && ( iMouseX - Input_nX0 < Input_nW0  ) && ( iMouseY - Input_nY0 < Input_nH0 );

            bHover = ( iMouseX >= Input_nX0 ) && ( iMouseY >= Input_nY0 ) && ( A7GuiAlpha_PointInShapeRound9 ( iMouseX - Input_nX0, iMouseY - Input_nY0, 10.0f, Input_nW0, Input_nH0, 0x3 ) > 0.5f );
            Input_bHover0 = bHover;

            bHover = ( iMouseX >= Input_nX1 ) && ( iMouseY >= Input_nY1 ) && ( iMouseX - Input_nX1 < Input_nW1  ) && ( iMouseY - Input_nY1 < Input_nH1 );

            bHover = ( iMouseX >= Input_nX1 ) && ( iMouseY >= Input_nY1 ) && ( A7GuiAlpha_PointInShapeRound9 ( iMouseX - Input_nX1, iMouseY - Input_nY1, 10.0f, Input_nW1, Input_nH1, 0x3 ) > 0.5f );
            Input_bHover1 = bHover;

            InvalidateRect ( hWnd, NULL, FALSE );

            return 0;
        }
        case WM_KEYDOWN: {
            switch ( wParam ) {
                case 'D': {
                    // p -> stext . nOblique -= 0x10000/64;
                    // InvalidateRect ( hWnd, NULL, FALSE );
                    break;
                }
            }
            return 0;
        }
        case WM_CREATE: {
            p = malloc ( sizeof ( S7WindowData ) );
            if ( p == NULL ) {
                SetLastError ( ERROR_NOT_ENOUGH_MEMORY );
                D7ERR_WINAPI ( malloc );
                return -1;
            }
            ZeroMemory ( p, sizeof ( S7WindowData ) );
            SetLastError ( 0 );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( p ) ) == 0 ) {
                D7ERR_WINAPI ( SetWindowLongPtrW );
            }

            A7Node_New_Root ( & p -> root );


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

            p -> nodeRoot . nW = LOWORD ( lParam );
            p -> nodeRoot . nH = HIWORD ( lParam );

            return 0;
        }
        case WM_DESTROY: {
            A7Node_Release ( & p-> nodeRoot );

            A7TexFree ( ( S7Tex* ) &( p -> texGdiLayer ) );
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

            A7TexFillRect_FULL ( & p -> texGdiLayer . _tex, 0x00ff0000 );

            A7TexDraw_Button ( ( S7Tex* ) &( p -> texGdiLayer ) );


            A7TexFillRect_FULL ( & p -> texGdiLayer . _tex, 0x00afafaf );

            WCHAR str[1024];
            snwprintf ( str, 1024, L"X: %d\nY: %d\nC: %d", iMouseX, iMouseY, iMouseI );
            S7GuiTextSets sets = {
                .iType = 0,
                .ftFace = g_ftFace,
                .iARGB = 0xff000000,
                .iFlags = D7GUITEXT_AA_LCD,
                .nHeight = 64*24,
                .nOffsetX = 0,
                .nOffsetY = 0,
                .nLineHeight = 64*32,
                .nLineWidth = 0,
                .nTracking = 0,
                .nOblique = 0,
            };
            A7GuiDraw_TextWide ( & p -> texGdiLayer . _tex, 256, 256, str, &sets );

            S7Tex * pDst = & p -> texGdiLayer . _tex;

            Input_TSets . ftFace = g_ftFace;
            UINT cB, cG, cR, cA;
            cB = 0xff;
            cG = 0xff;
            cR = 0xff;

            BYTE * pbd = pDst -> pData;

            cA = Input_bFocus0 ? 0xff : Input_bHover0 ? 0x7f : 0x3f;
            for ( UINT iy = 0; iy < Input_nH0; ++iy ) {
                CONST UINT idy = iy + Input_nY0;
                if ( idy & 0x80000000U ) continue;
                if ( idy >= pDst -> nHeight ) break;
                for ( UINT ix = 0; ix < Input_nW0; ++ix ) {
                    CONST UINT idx = ix + Input_nX0;
                    if ( idx & 0x80000000U ) continue;
                    if ( idx >= pDst -> nWidth ) break;
                    CONST UINT id = idy * pDst -> nStride + idx * 3;
                    CONST UINT  a = ( UINT ) ( A7GuiAlpha_PointInShapeRound9 ( ix, iy, 10.0f, Input_nW0, Input_nH0, 0x3 ) * ( FLOAT ) cA );
                    pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + cB * a ) / 0xff;
                    pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + cG * a ) / 0xff;
                    pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + cR * a ) / 0xff;
                }
            }
            Input_TSets . iARGB = Input_bHover0 ? 0x7f000000 : 0x3f000000;
            A7GuiDraw_TextWide ( & p -> texGdiLayer . _tex, Input_nX0, Input_nY0, Input_sz0, &Input_TSets );


            cA = Input_bFocus1 ? 0xff : Input_bHover1 ? 0x7f : 0x3f;
            for ( UINT iy = 0; iy < Input_nH1; ++iy ) {
                CONST UINT idy = iy + Input_nY1;
                if ( idy & 0x80000000U ) continue;
                if ( idy >= pDst -> nHeight ) break;
                for ( UINT ix = 0; ix < Input_nW1; ++ix ) {
                    CONST UINT idx = ix + Input_nX1;
                    if ( idx & 0x80000000U ) continue;
                    if ( idx >= pDst -> nWidth ) break;
                    CONST UINT id = idy * pDst -> nStride + idx * 3;
                    CONST UINT  a = ( UINT ) ( A7GuiAlpha_PointInShapeRound9 ( ix, iy, 10.0f, Input_nW1, Input_nH1, 0x3 ) * ( FLOAT ) cA );
                    pbd [ id + 0 ] = ( pbd [ id + 0 ] * ( 0xff - a ) + cB * a ) / 0xff;
                    pbd [ id + 1 ] = ( pbd [ id + 1 ] * ( 0xff - a ) + cG * a ) / 0xff;
                    pbd [ id + 2 ] = ( pbd [ id + 2 ] * ( 0xff - a ) + cR * a ) / 0xff;
                }
            }
            Input_TSets . iARGB = Input_bHover1 ? 0x7f000000 : 0x3f000000;
            A7GuiDraw_TextWide ( & p -> texGdiLayer . _tex, Input_nX1, Input_nY1, Input_sz1, &Input_TSets );

            for ( S7Node *pNode = p -> nodeRoot . pChildFirst; pNode != NULL; pNode = pNode -> pSiblingNext ) {
                if ( pNode -> rOnPaint != NULL ) {
                    pNode -> rOnPaint ( pNode );
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
    D7ERR_FREETYPE ( FT_New_Face, g_ftLibrary, "data\\Roboto-Medium.ttf", 0, &g_ftFace );



    /* Регистрация класса главного окна */
    ;{
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
    if ( ( g_hWndMain = CreateWindowExW ( 0, ( ( LPCWSTR ) ( LONG_PTR ) g_iMainClassAtom ), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, NULL ) ) == NULL ) {
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

    D7ERR_FREETYPE ( FT_Done_Face, g_ftFace );
    D7ERR_FREETYPE ( FT_Done_FreeType, g_ftLibrary );

    return 0;
}













