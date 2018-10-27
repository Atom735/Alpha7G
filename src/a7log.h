#ifndef _H_A7LOG_H_
#define _H_A7LOG_H_

#include <StdArg.h>
#include <StdIO.h>

extern int A7LogOpen ( char const * fileName );
extern void A7LogClose ( void );

extern int A7Log ( const char * fmt, ... );
extern int A7LogV ( const char * fmt, va_list args );

#define A7LOGERRC( _fname, _err ) A7Log ( "ERROR in %s:%d => %s with code {0x%x}(%d)\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err )
#define A7LOGERRCS( _fname, _err, _errs ) A7Log ( "ERROR in %s:%d => %s with code {0x%x}(%d)[%s]\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err, _errs )

#define A7LOGWARNC( _fname, _err ) A7Log ( "WARNING in %s:%d => %s with code {0x%x}(%d)\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err )
#define A7LOGWARNCS( _fname, _err, _errs ) A7Log ( "WARNING in %s:%d => %s with code {0x%x}(%d)[%s]\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err, _errs )


#define A7LOGFERRC( _pf, _fname, _err ) fprintf ( _pf, "ERROR in %s:%d => %s with code {0x%x}(%d)\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err )
#define A7LOGFERRCS( _pf, _fname, _err, _errs ) fprintf ( _pf, "ERROR in %s:%d => %s with code {0x%x}(%d)[%s]\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err, _errs )

#define A7LOGFWARNC( _pf, _fname, _err ) fprintf ( _pf, "WARNING in %s:%d => %s with code {0x%x}(%d)\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err )
#define A7LOGFWARNCS( _pf, _fname, _err, _errs ) fprintf ( _pf, "WARNING in %s:%d => %s with code {0x%x}(%d)[%s]\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err, _errs )

#define A7LOGFINFO( _pf, ... ) ( fprintf ( _pf, "INFO in %s:%d => ", __FILE__, __LINE__ ), fprintf ( _pf, __VA_ARGS__ ), fprintf ( _pf, "\n" ) )


#endif
