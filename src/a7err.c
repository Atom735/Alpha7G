#include <WinError.h>
#include <OpenSSL/SSL.h>

#include "a7err.h"

const char * A7Err_WSAGetLastError ( int err ) {
    switch ( err ) {
        #include "_a7err_wsa.h"
    }
    return "UNKNOWN";
}

const char * A7Err_SSL_get_error ( int err ) {
    switch ( err ) {
        #include "_a7err_ssl.h"
    }
    return "UNKNOWN";
}
