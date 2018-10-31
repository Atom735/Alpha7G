#ifndef _H_A7LOG_H_
#define _H_A7LOG_H_

#include <StdArg.h>
#include <StdIO.h>


extern int A7LogOpen ( char const * fileName );
extern void A7LogClose ( void );

extern int A7Log ( const char * fmt, ... );
extern int A7LogV ( const char * fmt, va_list args );

#define A7PREFIXERR     "ERROR in %s:%d => "
#define A7PREFIXWARN    "WARNING in %s:%d => "
#define A7PREFIXINFO    "INFO in %s:%d => "
#define A7SUFFIXC       "%s with 0x%x %d"
#define A7SUFFIXCS      "%s with 0x%x %d %s"
#define A7POSTFIX       "\n"

#define A7LOGREFIXERR() A7Log ( A7PREFIXERR  , __FILE__, __LINE__ )
#define A7LOGREFIXWARN() A7Log ( A7PREFIXWARN , __FILE__, __LINE__ )
#define A7LOGREFIXINFO() A7Log ( A7PREFIXINFO , __FILE__, __LINE__ )
#define A7LOGSUFFIXC( _fname, _err ) A7Log ( A7SUFFIXC , _fname, (unsigned)_err, (int)_err )
#define A7LOGSUFFIXCS( _fname, _err, _str ) A7Log ( A7SUFFIXCS , _fname,  (unsigned)_err, (int)_err, _str )
#define A7LOGPOSTFIX() A7Log ( A7POSTFIX )

#define A7LOGERR( ... ) ( A7LOGREFIXERR(), A7Log ( __VA_ARGS__ ), A7LOGPOSTFIX() )
#define A7LOGWARN( ... ) ( A7LOGREFIXWARN(), A7Log ( __VA_ARGS__ ), A7LOGPOSTFIX() )
#define A7LOGINFO( ... ) ( A7LOGREFIXINFO(), A7Log ( __VA_ARGS__ ), A7LOGPOSTFIX() )
#define A7LOGERRC( _fname, _err ) ( A7LOGREFIXERR(), A7LOGSUFFIXC ( _fname, _err ), A7LOGPOSTFIX() )
#define A7LOGWARNC( _fname, _err ) ( A7LOGREFIXWARN(), A7LOGSUFFIXC ( _fname, _err ), A7LOGPOSTFIX() )
#define A7LOGINFOC( _fname, _err ) ( A7LOGREFIXINFO(), A7LOGSUFFIXC ( _fname, _err ), A7LOGPOSTFIX() )
#define A7LOGERRCS( _fname, _err, _str ) ( A7LOGREFIXERR(), A7LOGSUFFIXCS ( _fname, _err, _str ), A7LOGPOSTFIX() )
#define A7LOGWARNCS( _fname, _err, _str ) ( A7LOGREFIXWARN(), A7LOGSUFFIXCS ( _fname, _err, _str ), A7LOGPOSTFIX() )
#define A7LOGINFOCS( _fname, _err, _str ) ( A7LOGREFIXINFO(), A7LOGSUFFIXCS ( _fname, _err, _str ), A7LOGPOSTFIX() )

#define A7LOGFREFIXERR( _pf ) fprintf ( _pf, A7PREFIXERR  , __FILE__, __LINE__ )
#define A7LOGFREFIXWARN( _pf ) fprintf ( _pf, A7PREFIXWARN , __FILE__, __LINE__ )
#define A7LOGFREFIXINFO( _pf ) fprintf ( _pf, A7PREFIXINFO , __FILE__, __LINE__ )
#define A7LOGFSUFFIXC( _pf, _fname, _err ) fprintf ( _pf, A7SUFFIXC , _fname,  (unsigned)_err, (int)_err )
#define A7LOGFSUFFIXCS( _pf, _fname, _err, _str ) fprintf ( _pf, A7SUFFIXCS , _fname,  (unsigned)_err, (int)_err, _str )
#define A7LOGFPOSTFIX( _pf ) fprintf ( _pf, A7POSTFIX )

#define A7LOGFERR( _pf, ... ) ( A7LOGFREFIXERR( _pf ), fprintf ( _pf, __VA_ARGS__ ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFWARN( _pf, ... ) ( A7LOGFREFIXWARN( _pf ), fprintf ( _pf, __VA_ARGS__ ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFINFO( _pf, ... ) ( A7LOGFREFIXINFO( _pf ), fprintf ( _pf, __VA_ARGS__ ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFERRC( _pf, _fname, _err ) ( A7LOGFREFIXERR( _pf ), A7LOGFSUFFIXC ( _pf, _fname, _err ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFWARNC( _pf, _fname, _err ) ( A7LOGFREFIXWARN( _pf ), A7LOGFSUFFIXC ( _pf, _fname, _err ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFINFOC( _pf, _fname, _err ) ( A7LOGFREFIXINFO( _pf ), A7LOGFSUFFIXC ( _pf, _fname, _err ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFERRCS( _pf, _fname, _err, _str ) ( A7LOGFREFIXERR( _pf ), A7LOGFSUFFIXCS ( _pf, _fname, _err, _str ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFWARNCS( _pf, _fname, _err, _str ) ( A7LOGFREFIXWARN( _pf ), A7LOGFSUFFIXCS ( _pf, _fname, _err, _str ), A7LOGFPOSTFIX( _pf ) )
#define A7LOGFINFOCS( _pf, _fname, _err, _str ) ( A7LOGFREFIXINFO( _pf ), A7LOGFSUFFIXCS ( _pf, _fname, _err, _str ), A7LOGFPOSTFIX( _pf ) )

#endif
