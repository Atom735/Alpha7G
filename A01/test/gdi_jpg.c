#include <WinSock2.h>
#include <Windows.h>

#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>
#include <SetJmp.h>

#include <JpegLib.h>

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;    /* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;

void my_error_exit ( j_common_ptr cinfo ) {
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = ( my_error_ptr ) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

int main ( int argc, char const *argv[] ) {

    HWND hWnd = GetDesktopWindow();
    HDC hDC = GetWindowDC ( hWnd );

    HDC hBackBufferDC = CreateCompatibleDC ( hDC );


    FILE * infile = fopen ( "03LAUefk5Ec.jpg", "rb" );

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    const char* jcs(J_COLOR_SPACE j) {
        return
        (j==JCS_UNKNOWN)?"JCS_UNKNOWN":
        (j==JCS_GRAYSCALE)?"JCS_GRAYSCALE":
        (j==JCS_RGB)?"JCS_RGB":
        (j==JCS_YCbCr)?"JCS_YCbCr":
        (j==JCS_CMYK)?"JCS_CMYK":
        (j==JCS_YCCK)?"JCS_YCCK":
        (j==JCS_BG_RGB)?"JCS_BG_RGB":
        (j==JCS_BG_YCC)?"JCS_BG_YCC":
        "NULL";
    }
    const char* jdm(J_DCT_METHOD j) {
        return
        (j==JDCT_ISLOW)?"JDCT_ISLOW":
        (j==JDCT_IFAST)?"JDCT_IFAST":
        (j==JDCT_FLOAT)?"JDCT_FLOAT":
        "NULL";
    }
    const char* jdm2(J_DITHER_MODE j) {
        return
        (j==JDITHER_NONE)?"JDITHER_NONE":
        (j==JDITHER_ORDERED)?"JDITHER_ORDERED":
        (j==JDITHER_FS)?"JDITHER_FS":
        "NULL";
    }


    printf ( "image_width               %d\n", cinfo.image_width );
    printf ( "image_height              %d\n", cinfo.image_height );
    printf ( "num_components            %d\n", cinfo.num_components );
    printf ( "jpeg_color_space          %s\n", jcs(cinfo.jpeg_color_space) );
    printf ( "\n" );
    printf ( "out_color_space           %s\n", jcs(cinfo.out_color_space) );
    printf ( "scale_num                 %d\n", cinfo.scale_num );
    printf ( "scale_denom               %d\n", cinfo.scale_denom );
    printf ( "output_gamma              %lf\n", cinfo.output_gamma );
    printf ( "buffered_image            %s\n", cinfo.buffered_image?"TRUE":"FALSE" );
    printf ( "raw_data_out              %s\n", cinfo.raw_data_out?"TRUE":"FALSE" );
    printf ( "buffered_image            %s\n", cinfo.buffered_image?"TRUE":"FALSE" );
    printf ( "dct_method                %s\n", jdm(cinfo.out_color_space) );
    printf ( "do_fancy_upsampling       %s\n", cinfo.do_fancy_upsampling?"TRUE":"FALSE" );
    printf ( "do_block_smoothing        %s\n", cinfo.do_block_smoothing?"TRUE":"FALSE" );
    printf ( "quantize_colors           %s\n", cinfo.quantize_colors?"TRUE":"FALSE" );
    printf ( "dither_mode               %s\n", jdm2(cinfo.dither_mode) );
    printf ( "two_pass_quantize         %s\n", cinfo.two_pass_quantize?"TRUE":"FALSE" );
    printf ( "desired_number_of_colors  %d\n", cinfo.desired_number_of_colors );
    printf ( "enable_1pass_quant        %s\n", cinfo.enable_1pass_quant?"TRUE":"FALSE" );
    printf ( "enable_external_quant     %s\n", cinfo.enable_external_quant?"TRUE":"FALSE" );
    printf ( "enable_2pass_quant        %s\n", cinfo.enable_2pass_quant?"TRUE":"FALSE" );

    cinfo.scale_num   = 1;
    cinfo.scale_denom = 2;
    cinfo.out_color_space = JCS_YCbCr;

    jpeg_start_decompress(&cinfo);
    printf ( "\n" );
    printf ( "out_color_space           %s\n", jcs(cinfo.out_color_space) );
    printf ( "scale_num                 %d\n", cinfo.scale_num );
    printf ( "scale_denom               %d\n", cinfo.scale_denom );
    printf ( "output_gamma              %lf\n", cinfo.output_gamma );
    printf ( "buffered_image            %s\n", cinfo.buffered_image?"TRUE":"FALSE" );
    printf ( "raw_data_out              %s\n", cinfo.raw_data_out?"TRUE":"FALSE" );
    printf ( "buffered_image            %s\n", cinfo.buffered_image?"TRUE":"FALSE" );
    printf ( "dct_method                %s\n", jdm(cinfo.out_color_space) );
    printf ( "do_fancy_upsampling       %s\n", cinfo.do_fancy_upsampling?"TRUE":"FALSE" );
    printf ( "do_block_smoothing        %s\n", cinfo.do_block_smoothing?"TRUE":"FALSE" );
    printf ( "quantize_colors           %s\n", cinfo.quantize_colors?"TRUE":"FALSE" );
    printf ( "dither_mode               %s\n", jdm2(cinfo.dither_mode) );
    printf ( "two_pass_quantize         %s\n", cinfo.two_pass_quantize?"TRUE":"FALSE" );
    printf ( "desired_number_of_colors  %d\n", cinfo.desired_number_of_colors );
    printf ( "enable_1pass_quant        %s\n", cinfo.enable_1pass_quant?"TRUE":"FALSE" );
    printf ( "enable_external_quant     %s\n", cinfo.enable_external_quant?"TRUE":"FALSE" );
    printf ( "enable_2pass_quant        %s\n", cinfo.enable_2pass_quant?"TRUE":"FALSE" );
    printf ( "\n" );
    printf ( "output_width              %d\n", cinfo.output_width );
    printf ( "output_height             %d\n", cinfo.output_height );
    printf ( "out_color_components      %d\n", cinfo.out_color_components );
    printf ( "output_components         %d\n", cinfo.output_components );
    printf ( "rec_outbuf_height         %d\n", cinfo.rec_outbuf_height );

    BITMAPINFO bi;
    memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biWidth = cinfo.output_width;
    bi.bmiHeader.biHeight = cinfo.output_height;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biPlanes = 1;
    BYTE *buf;
    HBITMAP hBackBuffer = CreateDIBSection ( hBackBufferDC, &bi, DIB_RGB_COLORS, (void**)&buf, NULL, 0 );

    // for (int i = 0; i < cinfo.output_width*cinfo.output_height; ++i) {
    //     buf[i*3] = i/cinfo.output_width;
    // }

    BYTE *pb = buf+cinfo.output_scanline;
    CONST INT w = cinfo.output_width;
    while (cinfo.output_scanline < cinfo.output_height) {
        pb = buf+((cinfo.output_height-cinfo.output_scanline-1)*3*w);
        jpeg_read_scanlines(&cinfo, (void**)&pb, 1);
        for (int i = 0; i < w; ++i) {
            pb[i*3+1]=pb[i*3+0];
            pb[i*3+2]=pb[i*3+0];
        }
    }

    jpeg_finish_decompress(&cinfo);
    fclose ( infile );

    SelectObject ( hBackBufferDC, hBackBuffer );



    BitBlt ( hDC, 0, 0, cinfo.output_width, cinfo.output_height, hBackBufferDC, 0, 0, SRCCOPY);


    DeleteObject ( hBackBuffer );



    DeleteDC ( hBackBufferDC );


    ReleaseDC ( hWnd, hDC );
    return 0;
}
