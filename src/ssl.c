

void main(int argc, char const *argv[])
{

    WSADATA     _wsadata = {0};
    WSAStartup( WINSOCK_VERSION, &_wsadata );
    int s;
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!s) {
        printf("Error creating socket.\n");
        return -1;
    }
    struct sockaddr_in sa;
    memset (&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = inet_addr("93.184.216.34");
    sa.sin_port        = htons(443);
    socklen_t socklen = sizeof(sa);
    if (connect(s, (struct sockaddr *)&sa, socklen)) {
        printf("Error connecting to server.\n");
        return -1;
    }
    SSL_library_init();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new ( A7_TLSv1_2_client_method() );
    ssl = SSL_new (ctx);
    if (!ssl) {
        printf("Error creating SSL.\n");
        log_ssl();
        return -1;
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0) {
        printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        return -1;
    }
    printf ("SSL connection using %s\n", SSL_get_cipher (ssl));

    char *request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    int RecvPacket()
    {
        int len=100;
        char buf[1000000];
        do {
            len=SSL_read(ssl, buf, 100);
            buf[len]=0;
            printf(buf);
        } while (len > 0);
        if (len < 0) {
            int err = SSL_get_error(ssl, len);
            if (err == SSL_ERROR_WANT_READ)
                return 0;
            if (err == SSL_ERROR_WANT_WRITE)
                return 0;
            if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL)
                return -1;
        }
    }

    int SendPacket(const char *buf)
    {
        int len = SSL_write(ssl, buf, strlen(buf));
        if (len < 0) {
            int err = SSL_get_error(ssl, len);
            switch (err) {
            case SSL_ERROR_WANT_WRITE:
                return 0;
            case SSL_ERROR_WANT_READ:
                return 0;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
            default:
                return -1;
            }
        }
    }
    return 0;
    return 0;
}
