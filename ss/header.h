#ifndef _H7_HEADER_H_
#define _H7_HEADER_H_

#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT _WIN32_WINNT_WIN7
#endif /*_WIN32_WINNT*/
#ifndef UNICODE
    #define UNICODE
#endif /*UNICODE*/

#include <winsock2.h>
#include <windows.h>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <time.h>

#include <openssl/ssl.h>
#include <jpeglib.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define WM_SOCK_DNS ( WM_USER + 1 )

#endif /*_H7_HEADER_H_*/
