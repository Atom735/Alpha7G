#ifndef _H_A7ERR_H_
#define _H_A7ERR_H_

#include "a7log.h"

#define A7LOGERR_WSA( _fn, _err ) A7LOGERRCS( _fn, _err, A7Err_WSAGetLastError( _err ) )
#define A7LOGWARN_WSA( _fn, _err ) A7LOGWARNCS( _fn, _err, A7Err_WSAGetLastError( _err ) )

#define A7LOGFERR_WSA( _pf, _fn, _err ) A7LOGFERRCS( _pf, _fn, _err, A7Err_WSAGetLastError( _err ) )
#define A7LOGFWARN_WSA( _pf, _fn, _err ) A7LOGFWARNCS( _pf, _fn, _err, A7Err_WSAGetLastError( _err ) )


extern const char * A7Err_WSAGetLastError ( int err );

#endif
