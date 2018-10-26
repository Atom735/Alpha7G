
extern FILE *g_logfile;
#define     LOG(...) (fprintf( g_logfile, __VA_ARGS__ ), LOG_FLUSH())
#define     LOG_OPEN() ( g_logfile = fopen("log.log", "w") )
#define     LOG_CLOSE() fclose( g_logfile )
#define     LOG_ERR(_fname, _err) LOG_ERRS(_fname, _err, "UNKNOWN")
#define     LOG_ERRS(_fname, _err, _errs) LOG( "%s:%d => %s failed with error 0x%x (%d)[%s]\n", __FILE__, __LINE__, _fname, (unsigned)_err, (int)_err, _errs )
#define     LOG_FLUSH() fflush( g_logfile )
#define     ASSERT_ALLOC(_p) if( _p == NULL ) {LOG_ERR("bad_alloc", 0);}



