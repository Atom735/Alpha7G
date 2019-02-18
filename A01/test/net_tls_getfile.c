#include <WinSock2.h>
#include <Windows.h>

#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>

#include <OpenSSL/SSL.h>
#include <OpenSSL/ERR.h>

void log_ssl()
{
    int err;
    while (err = ERR_get_error()) {
        char *str = ERR_error_string(err, 0);
        if (!str)
            return;
        printf(str);
        printf("\n");
        fflush(stdout);
    }
}

int main ( int argc, char const *argv[] ) {
    WSADATA wsd;
    if ( ( WSAStartup ( WINSOCK_VERSION, &wsd ) ) != 0 ) {
        printf( "can't init wsa\n" );
        return (-1);
    }
    SOCKET s = INVALID_SOCKET;
    if ( ( s = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
        printf( "can't create socket\n" );
        return (-1);
    }
    SOCKADDR_IN sa;
    memset ( &sa, 0, sizeof ( sa ) );
    sa.sin_family      = AF_INET;
    /*
    vk.com.             83  IN  A   87.240.190.67
    vk.com.             683 IN  A   87.240.129.71
    vk.com.             683 IN  A   87.240.180.136
    vk.com.             683 IN  A   87.240.129.133
    m.vk.com.           345 IN  A   87.240.129.182
    m.vk.com.           345 IN  A   87.240.190.68
    m.vk.com.           345 IN  A   87.240.129.76
    pp.userapi.com.     261 IN  A   93.186.238.48
    pp.userapi.com.     261 IN  A   87.240.182.228
    pp.userapi.com.     261 IN  A   87.240.137.143
    pp.userapi.com.     261 IN  A   93.186.238.32
    example.com.    75594   IN  A   93.184.216.34
    example.com.    85883   IN  AAAA    2606:2800:220:1:248:1893:25c8:1946
    example.com.    85883   IN  NS  b.iana-servers.net.
    example.com.    85883   IN  NS  a.iana-servers.net.
    */
    sa.sin_addr.s_addr = inet_addr("93.186.238.32"); // vk.com
    sa.sin_port        = htons(443); // https

    if( connect ( s, ( SOCKADDR* )( &sa ), sizeof ( sa ) ) != 0 ) {
        printf( "can't connect\n" );
        return (-1);
    }

    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);
    SSL *ssl = SSL_new (ctx);
    if (!ssl) {
        printf("Error creating SSL.\n");
        log_ssl();
        return -1;
    }


    SSL_set_fd ( ssl, s );

    SSL_connect( ssl );

    #define SIZEDD (4096)
    CHAR buf[SIZEDD] =
        "GET /c847016/v847016261/1222d3/03LAUefk5Ec.jpg HTTP/1.1\r\n"
        "Host: pp.userapi.com\r\n"
        "\r\n";
    /* pp.userapi.com/c847016/v847016261/1222d3/03LAUefk5Ec.jpg */
    INT Szs = strlen(buf);
    printf("len %d\n", Szs );

    SSL_write( ssl, buf, Szs );
    int sz = SSL_read ( ssl, buf, SIZEDD );
    int SZ = 0;
    int ff = 0;

    printf("Readed: %d\n", sz );

    FILE* pf = fopen( "txt.log", "wb" );
    FILE* kf = fopen( "jpg.log", "wb" );
    FILE* jf = fopen( "jpg.jpg", "wb" );
    fwrite( buf, 1, sz, pf );

    for( int i=0; i<sz; ++i ) {
        if(memcmp ( buf+i,"Content-Length:", 15 ) == 0) {
            SZ = 0;
            i+=15;
            while(buf[i]==' ') ++i;
            while((buf[i]>='0')&&(buf[i]<='9')) {
                SZ *= 10;
                SZ += buf[i]-'0';
                ++i;
            }
            ff = SZ;

            printf("Content-Length: %d\n", SZ );
        }
        if(memcmp ( buf+i,"\r\n\r\n", 4 ) == 0) {
            i+=4;
            sz -= i;
            fwrite( buf+i, 1, sz, jf );
            fwrite( buf+i, 1, sz, kf );
            printf("Header: %d\n", i );
            printf("Readed: %d/%d\n", sz, SZ );
            SZ-=sz;
        }
    }
    while(SZ>0) {
        sz = SSL_read ( ssl, buf, SIZEDD );
        printf("Readed: %d/%d\n", sz, SZ );
        fwrite( buf, 1, sz, jf );
        fwrite( buf, 1, sz, kf );
        // fwrite( buf, 1, sz, pf );
        SZ-=sz;
    }


    fclose(pf);
    fclose(kf);
    fclose(jf);


    SSL_shutdown ( ssl );


    SSL_free ( ssl );



    SSL_CTX_free ( ctx );




    shutdown ( s, SD_SEND );
    closesocket ( s );



    if ( WSACleanup() == SOCKET_ERROR ) {
        // TODO:ERROR
    }

    return 0;
}
