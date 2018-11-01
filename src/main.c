#include <WinSock2.h>
#include <Windows.h>

#include <Assert.h>
#include <StdLib.h>
#include <StdIO.h>
#include <String.h>

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


int main ( int argc, char const *argv[] ) {
    WSADATA wsd;
    if ( ( WSAStartup ( WINSOCK_VERSION, &wsd ) ) != 0 ) {
        printf( "can't init wsa\n" );
        assert ( 0 );
        return (-1);
    }

    SOCKET s = INVALID_SOCKET;

    if ( ( s = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == INVALID_SOCKET ) {
        printf( "can't create socket\n" );
        assert ( 0 );
        return (-1);
    }

    SOCKADDR_IN sa;
    memset ( &sa, 0, sizeof ( sa ) );
    sa.sin_family       = AF_INET;
    sa.sin_port         = htons ( 53 );
    sa.sin_addr.s_addr  = inet_addr ( "8.8.8.8" );

    if( connect ( s, ( SOCKADDR* )( &sa ), sizeof ( sa ) ) != 0 ) {
        printf( "can't connect\n" );
        assert ( 0 );
        return (-1);
    }

    BYTE buf[4096] = {
        0xAA, 0xAA, // - ID
        0x01, 0x00, // – Параметры запроса
        0x00, 0x01, // – Количество вопросов
        0x00, 0x00, // – Количество ответов
        0x00, 0x00, // – Количество записей об уполномоченных серверах
        0x00, 0x00, // – Количество дополнительных записей
        // 12 байт
        0x07, 0x65, // – у 'example' длина 7, e
        0x78, 0x61, // – x, a
        0x6D, 0x70, // – m, p
        0x6C, 0x65, // – l, e
        0x03, 0x63, // – у 'com' длина 3, c
        0x6F, 0x6D, // – o, m
        0x00,       // - нулевой байт для окончания поля QNAME
        0x00, 0x01, // – QTYPE
        0x00, 0x01, // – QCLASS
        // 12 + 17 = 29 байт
    };
    int i = 0;
    A7DNSHEAD head = {
        .id      = 1,
        .flags   = _A7DNSHEADFLAG_RD,
        .QDCount = 1,
        .ANCount = 0,
        .NSCount = 0,
        .ARCount = 0,
    };

    A7DnsPackPrintHead ( &head );
    i += A7DnsPackAddHead ( buf+i, &head );
    const char *strs[] = { "example", "com", NULL };
    i += A7DnsPackAddQuestion ( buf+i, strs, 1, 1 );

    int l = 0;

    if ( ( l = send ( s, buf, i, 0 ) ) == SOCKET_ERROR ) {
        printf( "can't send\n" );
        assert ( 0 );
        return (-1);
    }
    printf( "sended %d/%d bytes\n", l, 29 );

   if ( ( l = recv ( s, buf, 1024, 0 ) ) == SOCKET_ERROR ) {
        printf( "can't recv\n" );
        assert ( 0 );
        return (-1);
    }
    printf( "recived %d bytes\n", l );

    if ( l >= sizeof ( A7DNSHEAD ) ) {
        A7DnsPackTranslateHead ( &head, (A7DNSHEAD*) buf );
        i = sizeof ( A7DNSHEAD );
        A7DnsPackPrintHead ( &head );
    }

    for ( int i = 0; i < l; ++i ) {
        if( i % 4 == 0) printf("\n" );
        printf(" %02x ", buf[i] );
    }
    printf("\n" );

    shutdown ( s, SD_SEND );
    closesocket ( s );


    if ( WSACleanup() == SOCKET_ERROR ) {
        // TODO:ERROR
    }
    return 0;
}
