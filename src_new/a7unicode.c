#include <Windows.h>

#include <Assert.h>

#include "a7unicode.h"

#define D7UTF8_ASCII            0x00
#define D7UTF8_ASCII_MASK       0x80
#define D7UTF8_NEXT             0x80
#define D7UTF8_NEXT_MASK        0xC0
#define D7UTF8_2                0xC0
#define D7UTF8_2_MASK           0xE0
#define D7UTF8_3                0xE0
#define D7UTF8_3_MASK           0xF0
#define D7UTF8_4                0xF0
#define D7UTF8_4_MASK           0xF8
#define D7UTF8_5                0xF8
#define D7UTF8_5_MASK           0xFC
#define D7UTF8_6                0xFC
#define D7UTF8_6_MASK           0xFE
#define D7UTF8_7                0xFE
#define D7UTF8_7_MASK           0xFF

/* Дайют юникод символа в строке, и возвращает указатель на след байт */
CONST BYTE *A7UnicodeByUTF8 ( UINT *pUnicode, CONST BYTE *pStr ) {
    if ( *pStr == 0 ) {
        /* End of string */
        *pUnicode = 0x00;
        return NULL;
    }
    if ( ( pStr [ 0 ] & D7UTF8_ASCII_MASK ) == D7UTF8_ASCII ) {
        /* ASCII */
        *pUnicode = pStr [ 0 ];
        return pStr + 1;
    }
    if ( ( pStr [ 0 ] & D7UTF8_NEXT_MASK ) == D7UTF8_NEXT ) {
        /* NEXT */
        *pUnicode = 0x1;
        assert ( 0 );
        return NULL;
    }
    if ( ( pStr [ 0 ] & D7UTF8_2_MASK ) == D7UTF8_2 ) {
        /* 2 OCTET */
        if ( ( pStr [ 1 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x2;
            assert ( 0 );
            return NULL;
        }
        *pUnicode = ( ( pStr [ 0 ] ^ D7UTF8_2 ) << 6 ) | ( ( pStr [ 1 ] ^ D7UTF8_NEXT ) << 0 );
        return pStr + 2;
    }
    if ( ( pStr [ 0 ] & D7UTF8_3_MASK ) == D7UTF8_3 ) {
        /* 3 OCTET */
        if ( ( pStr [ 1 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x3;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 2 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x3;
            assert ( 0 );
            return NULL;
        }
        *pUnicode = ( ( pStr [ 0 ] ^ D7UTF8_3 ) << 12 ) | ( ( pStr [ 1 ] ^ D7UTF8_NEXT ) << 6 ) | ( ( pStr [ 2 ] ^ D7UTF8_NEXT ) << 0 );
        return pStr + 3;
    }
    if ( ( pStr [ 0 ] & D7UTF8_4_MASK ) == D7UTF8_4 ) {
        /* 4 OCTET */
        if ( ( pStr [ 1 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x4;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 2 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x4;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 3 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x4;
            assert ( 0 );
            return NULL;
        }
        *pUnicode = ( ( pStr [ 0 ] ^ D7UTF8_4 ) << 18 ) | ( ( pStr [ 1 ] ^ D7UTF8_NEXT ) << 12 ) | ( ( pStr [ 2 ] ^ D7UTF8_NEXT ) << 6 ) | ( ( pStr [ 3 ] ^ D7UTF8_NEXT ) << 0 );
        return pStr + 4;
    }
    if ( ( pStr [ 0 ] & D7UTF8_5_MASK ) == D7UTF8_5 ) {
        /* 5 OCTET */
        if ( ( pStr [ 1 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x5;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 2 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x5;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 3 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x5;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 4 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x5;
            assert ( 0 );
            return NULL;
        }
        *pUnicode = ( ( pStr [ 0 ] ^ D7UTF8_5 ) << 24 ) | ( ( pStr [ 1 ] ^ D7UTF8_NEXT ) << 18 ) | ( ( pStr [ 2 ] ^ D7UTF8_NEXT ) << 12 ) | ( ( pStr [ 3 ] ^ D7UTF8_NEXT ) << 6 ) | ( ( pStr [ 4 ] ^ D7UTF8_NEXT ) << 0 );
        return pStr + 5;
    }
    if ( ( pStr [ 0 ] & D7UTF8_6_MASK ) == D7UTF8_6 ) {
        /* 6 OCTET */
        if ( ( pStr [ 1 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x6;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 2 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x6;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 3 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x6;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 4 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x6;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 5 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x6;
            assert ( 0 );
            return NULL;
        }
        *pUnicode = ( ( pStr [ 0 ] ^ D7UTF8_5 ) << 30 ) | ( ( pStr [ 1 ] ^ D7UTF8_NEXT ) << 24 ) | ( ( pStr [ 2 ] ^ D7UTF8_NEXT ) << 18 ) | ( ( pStr [ 3 ] ^ D7UTF8_NEXT ) << 12 ) | ( ( pStr [ 4 ] ^ D7UTF8_NEXT ) << 6 ) | ( ( pStr [ 5 ] ^ D7UTF8_NEXT ) << 0 );
        return pStr + 6;
    }
    if ( ( pStr [ 0 ] & D7UTF8_7_MASK ) == D7UTF8_7 ) {
        /* 7 OCTET */
        if ( ( pStr [ 1 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x7;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 2 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x7;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 3 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x7;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 4 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x7;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 5 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x7;
            assert ( 0 );
            return NULL;
        }
        if ( ( pStr [ 6 ] & D7UTF8_NEXT_MASK ) != D7UTF8_NEXT ) {
            /* NEXT */
            *pUnicode = 0x7;
            assert ( 0 );
            return NULL;
        }
        *pUnicode = ( ( pStr [ 1 ] ^ D7UTF8_NEXT ) << 30 ) | ( ( pStr [ 2 ] ^ D7UTF8_NEXT ) << 24 ) | ( ( pStr [ 3 ] ^ D7UTF8_NEXT ) << 18 ) | ( ( pStr [ 4 ] ^ D7UTF8_NEXT ) << 12 ) | ( ( pStr [ 5 ] ^ D7UTF8_NEXT ) << 6 ) | ( ( pStr [ 6 ] ^ D7UTF8_NEXT ) << 0 );
        return pStr + 7;
    }
    *pUnicode = 0x8;
    return NULL;
}
