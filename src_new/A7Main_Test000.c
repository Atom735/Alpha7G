
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

/* Название процедуры главного окна */
LRESULT CALLBACK A7MainWinProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    S7Node_Root * p = ( S7Node_Root* ) GetWindowLongPtrW ( hWnd, GWLP_USERDATA );
    SetLastError ( 0 );
    D7ERR_WINAPI ( GetWindowLongPtrW, p == NULL );
    switch ( uMsg ) {
        case WM_CREATE: {
            p = A7Node_New_Root ( NULL, hWnd );
            D7ERR_MALLOC ( A7Node_New_Root, p == NULL ) { return -1; }
            SetLastError ( 0 );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( p ) ) == 0 ) {
                D7ERR_WINAPI ( SetWindowLongPtrW, TRUE );
            }
            return 0;
        }
        case WM_SIZE: {
            HDC hDC = GetDC ( hWnd );
            D7ERR_WINAPI ( GetDC, hDC == NULL );
            ReleaseDC ( hWnd, hDC );
            I7Node_rOnResize ( p, LOWORD ( lParam ), HIWORD ( lParam ) );
            return 0;
        }
        case WM_DESTROY: {
            A7Node_Release ( & p -> _ );
            SetLastError ( 0 );
            if ( SetWindowLongPtrW ( hWnd, GWLP_USERDATA, ( LONG_PTR ) ( NULL ) ) == 0 ) {
                D7ERR_WINAPI ( SetWindowLongPtrW, TRUE );
            }
            PostQuitMessage ( 0 );
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint ( hWnd, &ps );
            D7ERR_WINAPI ( BeginPaint, hDC == NULL );
            BYTE * CONST pv = ( BYTE* ) p -> gdi . _ . pData;
            CONST UINT nW = p -> gdi . _ . nWidth;
            CONST UINT nH = p -> gdi . _ . nHeight;
            CONST UINT nS = p -> gdi . _ . nStride;
            for ( UINT y = 0; y < nH; ++y ) {
                for ( UINT x = 0; x < nW; ++x ) {
                    CONST UINT i = y * nS + x * 3;
                    pv [ i + 0 ] = x+y;
                    pv [ i + 1 ] = (x+y)/2;
                    pv [ i + 2 ] = x*255/nW;
                }
            }
            I7Node_rOnPaint ( p, hDC );
            EndPaint ( hWnd, &ps );
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
    g_iMainClassAtom = RegisterClassExW ( &wc );
    D7ERR_WINAPI ( RegisterClassExW, g_iMainClassAtom == 0 );

    /* Создание главного окна */
    g_hWndMain = CreateWindowExW ( 0, ( ( LPCWSTR ) ( LONG_PTR ) g_iMainClassAtom ), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, NULL );
    D7ERR_WINAPI ( CreateWindowExW, g_hWndMain == NULL );
    // ShowWindow ( g_hWndMain, nCmdShow );

    /* Входим в цикл обработки сообщений */
    MSG msg = { };
    while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
        TranslateMessage ( &msg );
        DispatchMessage ( &msg );
    }

    /* Освобождение класса главного окна */
    if ( ! UnregisterClassW ( kg_szMainClassName, g_hInstance ) ) {
        D7ERR_WINAPI ( UnregisterClassW, TRUE );
    }

    D7ERR_FREETYPE ( FT_Done_Face, g_ftFace );
    D7ERR_FREETYPE ( FT_Done_FreeType, g_ftLibrary );

    return 0;
}













