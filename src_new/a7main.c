
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

typedef struct _S7WindowData S7WindowData;
struct _S7WindowData {
    S7TexGdi    texGdiLayer;
    S7TexGlyph  texGlyph;
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
            A7TexFillRect ( ( S7Tex* ) &( p -> texGdiLayer ), 128, 128, 32, 32, 0xff000000 );
            A7TexDraw_GDI_FULL ( hDC, 0, 0, &( p -> texGdiLayer ) );

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
    D7ERR_FREETYPE ( FT_New_Face, g_ftLibrary, "C:\\Windows\\Fonts\\RobotoSlab-Regular.ttf", 0, &g_ftFace );
    D7ERR_FREETYPE ( FT_New_Face, g_ftLibrary, "data\\Material-Design-Iconic-Font.ttf", 0, &g_ftFaceIcon );;


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













