#include <WinSock2.h>
#include <Windows.h>

#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library  library;
FT_Face     face;      /* handle to face object */

int main ( int argc, char const *argv[] ) {

    int h=12;
    FT_Init_FreeType( &library );
    FT_New_Face( library, "C:\\Windows\\Fonts\\RobotoSlab-Regular.ttf", 0, &face );

    FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          0,      /* pixel_width           */
          h );   /* pixel_height          */

    FT_Set_Char_Size(
          face,    /* handle to face object           */
          0,       /* char_width in 1/64th of points  */
          h*64,   /* char_height in 1/64th of points */
          119,     /* horizontal device resolution    */
          114 );   /* vertical device resolution      */

    HWND hWnd = GetDesktopWindow();
    HDC hDC = GetWindowDC ( hWnd );

    HDC hBackBufferDC = CreateCompatibleDC ( hDC );

    BITMAPINFO bi;
    memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biWidth = 512;
    bi.bmiHeader.biHeight = 512;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biPlanes = 1;
    BYTE *buf;
    HBITMAP hBackBuffer = CreateDIBSection ( hBackBufferDC, &bi, DIB_RGB_COLORS, (void**)&buf, NULL, 0 );

    SelectObject ( hBackBufferDC, hBackBuffer );

    int pen_x = 5;
    int pen_y = 500;

    LPCSTR text = "Hello World! Atom735!";


    void my_draw_bitmap( FT_Bitmap *bmp, int _x, int _y ) {
        for (int iy = 0; iy < bmp->rows; ++iy)
        {
            if(_y+iy<0) iy=-_y;
            if(_y+iy>=bi.bmiHeader.biHeight) break;
            for (int ix = 0; ix < bmp->width; ++ix)
            {
                if(_x+ix<0) iy=-_y;
                if(_x+ix>=bi.bmiHeader.biWidth) break;
                int is = iy*bmp->pitch+ix;
                int id = (bi.bmiHeader.biHeight-iy-_y-1)*bi.bmiHeader.biWidth+(_x+ix);
                buf[id*3+0]=bmp->buffer[is];
                buf[id*3+1]=bmp->buffer[is]/2;
                buf[id*3+2]=0;
            }
        }
    }

    for ( int n = 0; n < strlen(text); n++ )
    {
        int error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
          if ( error )
            continue;  /* ignore errors */

          /* now, draw to our target surface */
          my_draw_bitmap( &face->glyph->bitmap,
                          pen_x + face->glyph->bitmap_left,
                          pen_y - face->glyph->bitmap_top );

          /* increment pen position */
          pen_x += face->glyph->advance.x >> 6;

    }


    pen_x = 5;
    pen_y = 200;

    text = "abcdefghijklmnopqrstuvwxyz";

    for ( int n = 0; n < strlen(text); n++ )
    {
        int error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
          if ( error )
            continue;  /* ignore errors */

          /* now, draw to our target surface */
          my_draw_bitmap( &face->glyph->bitmap,
                          pen_x + face->glyph->bitmap_left,
                          pen_y - face->glyph->bitmap_top );

          /* increment pen position */
          pen_x += face->glyph->advance.x >> 6;
    }


    pen_x = 5;
    pen_y -= h+1;

    FT_Vector pen = { .x = pen_x<<6, .y = pen_y<<6, };

    for ( int n = 0; n < strlen(text); n++ )
    {
        FT_Set_Transform( face, NULL, &pen );
        int error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
          if ( error )
            continue;  /* ignore errors */

          /* now, draw to our target surface */
          my_draw_bitmap( &face->glyph->bitmap,
                           face->glyph->bitmap_left,
                           face->glyph->bitmap_top );

          /* increment pen position */
          // pen_x += face->glyph->advance.x >> 6;
          pen.x += face->glyph->advance.x;
          pen.y += face->glyph->advance.y;
    }



    BitBlt ( hDC, 0, 0, bi.bmiHeader.biWidth, bi.bmiHeader.biHeight, hBackBufferDC, 0, 0, SRCCOPY);


    DeleteObject ( hBackBuffer );



    DeleteDC ( hBackBufferDC );


    ReleaseDC ( hWnd, hDC );
    return 0;
}
