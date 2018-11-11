#ifndef _H_A7ABNF_H_
#define _H_A7ABNF_H_

#define IS_C_IN_INTERVAL(c,a,b) (((c)>=(a))&&((c)<=(b)))

#define IS_C_ALPHA(a)   (IS_C_IN_INTERVAL(a,0x41,0x5A)||IS_C_IN_INTERVAL(a,0x61,0x7A))
#define IS_C_BIT(a)     (((a)=='0')||((a)=='1'))
#define IS_C_CHAR(a)    IS_C_IN_INTERVAL(a,0x01,0x7F)
#define IS_C_DIGIT(a)   IS_C_IN_INTERVAL(a,0x30,0x39)
#define IS_C_WSP(a)     (((a)==C_SP)||((a)==C_HTAB))
#define IS_C_OCTET(a)   IS_C_IN_INTERVAL(a,0x00,0xFF)
#define IS_C_VCHAR(a)   IS_C_IN_INTERVAL(a,0x21,0x7E)

#define IS_S_CRLF(a)    ((a[0]==C_CR)&&(a[1]==C_LF))

#define C_CR        0x0D
#define C_LF        0x0A
#define C_SP        0x20
#define C_DQUOTE    0x22
#define C_HTAB      0x09




#endif
