#ifndef _H_A7UNICODE_
#define _H_A7UNICODE_

#include <Windows.h>

/* Дайют юникод символа в строке, и возвращает указатель на след байт */
CONST BYTE *A7UnicodeByUTF8 ( UINT *pUnicode, CONST BYTE *pStr );

#endif /* _H_A7UNICODE_ */


