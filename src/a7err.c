#include "a7err.h"

const char * A7Err_WSAGetLastError ( int err ) {
    switch ( err ) {
        #include "_a7err_wsa.h"
    }
    return "UNKNOWN";
}
