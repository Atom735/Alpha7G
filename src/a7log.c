
#include <StdIO.h>
#include <Time.h>
#include <StdArg.h>
#include <Assert.h>

#include "A7Log.h"

static FILE *g_A7LogFile = NULL;
int A7LogOpen ( char const * fileName ) {
    assert ( g_A7LogFile == NULL );
    if( fileName == NULL ) fileName = "log.log";
    g_A7LogFile = fopen ( fileName, "w" );
    if ( g_A7LogFile == NULL ) return -1;
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    A7Log ( " ====== %s", asctime ( timeinfo ) );
    return 0;
}

void A7LogClose ( void ) {
    assert ( g_A7LogFile != NULL );
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    A7Log ( " ====== %s", asctime ( timeinfo ) );
    fclose ( g_A7LogFile );
}


int A7Log ( const char * fmt, ...) {
    if ( g_A7LogFile == NULL ) return -1;
    va_list args;
    va_start ( args, fmt );
    int n = A7LogV ( fmt, args );
    va_end ( args );
    fflush ( g_A7LogFile );
    return n;
}
int A7LogV ( const char * fmt, va_list args ) {
    if ( g_A7LogFile == NULL ) return -1;
    int n = vfprintf ( g_A7LogFile, fmt, args );
    fflush ( g_A7LogFile );
    return n;
}
