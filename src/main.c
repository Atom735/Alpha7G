#include <WinSock2.h>
#include <Windows.h>

#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>

#include <OpenSSL/SSL.h>
#include <OpenSSL/ERR.h>

typedef struct _A7DNSHEAD {
    UINT16 id;      /* ID - произвольный */
    UINT16 flags;   /* Параметры запроса */
    UINT16 QDCount; /* Количество вопросов */
    UINT16 ANCount; /* Количество ответов */
    UINT16 NSCount; /* Количество записей об уполномоченных серверах */
    UINT16 ARCount; /* Количество дополнительных записей */
} A7DNSHEAD;

void A7DnsPackTranslateHead ( A7DNSHEAD *pDst, A7DNSHEAD *pSrc ) {
    if ( pSrc == NULL ) pSrc = pDst;
    pDst -> id      = htons ( pSrc -> id      );
    pDst -> flags   = htons ( pSrc -> flags   );
    pDst -> QDCount = htons ( pSrc -> QDCount );
    pDst -> ANCount = htons ( pSrc -> ANCount );
    pDst -> NSCount = htons ( pSrc -> NSCount );
    pDst -> ARCount = htons ( pSrc -> ARCount );
}

#define _A7DNSHEADFLAG_QR (1<<15)
#define _A7DNSHEADFLAG_AA (1<<10)
#define _A7DNSHEADFLAG_TC (1<<9)
#define _A7DNSHEADFLAG_RD (1<<8)
#define _A7DNSHEADFLAG_RA (1<<7)

void A7DnsPackPrintHead ( A7DNSHEAD *p ) {
    printf ( "DNS_HEAD    ID = %d\n", p -> id );
    printf ( "            QR = %s\n", ((p -> flags & (1<<15))>>15)?"TRUE":"FALSE" );
    printf ( "        OPCODE = %d\n", ((p -> flags & ((1<<14)|(1<<13)|(1<<12)|(1<<11)))>>11) );
    printf ( "            AA = %s\n", ((p -> flags & (1<<10))>>10)?"TRUE":"FALSE" );
    printf ( "            TC = %s\n", ((p -> flags & (1<<9))>>9)?"TRUE":"FALSE" );
    printf ( "            RD = %s\n", ((p -> flags & (1<<8))>>8)?"TRUE":"FALSE" );
    printf ( "            RA = %s\n", ((p -> flags & (1<<7))>>7)?"TRUE":"FALSE" );
    printf ( "             Z = %d\n", ((p -> flags & ((1<<6)|(1<<5)|(1<<4)))>>4) );
    printf ( "         RCODE = %d\n", ((p -> flags & ((1<<3)|(1<<2)|(1<<1)|(1<<0)))>>0) );
    printf ( "       QDCOUNT = %d\n", p -> QDCount );
    printf ( "       ANCOUNT = %d\n", p -> ANCount );
    printf ( "       NSCOUNT = %d\n", p -> NSCount );
    printf ( "       ARCOUNT = %d\n", p -> ARCount );
}

int A7DnsPackAddHead ( BYTE* buf, A7DNSHEAD * pHead ) {
    A7DNSHEAD h;
    A7DnsPackTranslateHead ( &h, pHead );
    memcpy ( buf, &h, sizeof ( A7DNSHEAD ) );
    return sizeof ( A7DNSHEAD );
}
int A7DnsPackAddQuestion ( BYTE* buf, const char ** QNames, UINT16 QType, UINT16 QClass ) {
    int j = 0;
    for ( int i = 0; QNames[i] != NULL; ++i ) {
        int sz = strlen ( QNames[i] );
        buf[j] = sz; ++j;
        memcpy ( buf+j, QNames[i], sz );
        j += sz;
    }
    buf[j] = 0x00; ++j;
    UINT16 p[2]; p[0] = htons ( QType ); p[1] = htons ( QClass );
    memcpy ( buf+j, p, 4 ); j +=4 ;
    return j;
}

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
        assert ( 0 );
        return (-1);
    }
    printf("LINE %d\n", __LINE__ );

    SOCKET s = INVALID_SOCKET;

    if ( ( s = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
        printf( "can't create socket\n" );
        assert ( 0 );
        return (-1);
    }
    printf("LINE %d\n", __LINE__ );

    SOCKADDR_IN sa;
    memset ( &sa, 0, sizeof ( sa ) );
    sa.sin_family      = AF_INET;
    printf("LINE %d\n", __LINE__ );
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
        assert ( 0 );
        return (-1);
    }
    printf("LINE %d\n", __LINE__ );


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
    printf("LINE %d\n", __LINE__ );

    SSL_set_fd ( ssl, s );
    printf("LINE %d\n", __LINE__ );
    SSL_connect( ssl );
    printf("LINE %d\n", __LINE__ );
    #define SIZEDD (4096)
    CHAR buf[SIZEDD] =
        "GET /c847016/v847016261/1222d3/03LAUefk5Ec.jpg HTTP/1.1\r\n"
        "Host: pp.userapi.com\r\n"
        "\r\n";
    INT Szs = strlen(buf);
    printf("len %d\n", Szs );



    SSL_write( ssl, buf, Szs );
    int sz = SSL_read ( ssl, buf, SIZEDD );
    int SZ = 0;

    printf("Readed: %d\n", sz );

    FILE* pf = fopen( "txt.txt", "w" );
    FILE* kf = fopen( "jpg.txt", "w" );
    FILE* jf = fopen( "jpg.jpg", "w" );
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

            printf("Content-Length: %d\n", SZ );
        }
        if(memcmp ( buf+i,"\r\n\r\n", 4 ) == 0) {
            i+=4;
            fwrite( buf+i, 1, sz-i, jf );
            fwrite( buf+i, 1, sz-i, kf );
            SZ-=sz-i;

            printf("Header: %d\n", i );
            printf("File: %d/%d\n", sz-i, SZ );
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
    printf("LINE %d\n", __LINE__ );

    SSL_free ( ssl );
    printf("LINE %d\n", __LINE__ );


    SSL_CTX_free ( ctx );
    printf("LINE %d\n", __LINE__ );



    shutdown ( s, SD_SEND );
    closesocket ( s );
    printf("LINE %d\n", __LINE__ );


    if ( WSACleanup() == SOCKET_ERROR ) {
        // TODO:ERROR
    }
    printf("LINE %d\n", __LINE__ );
    return 0;
}
