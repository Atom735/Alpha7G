#include <WinSock2.h>
#include <WS2TCPIP.h>
// #include <IPHlpAPI.h>
#include <Windows.h>

#include <OpenSSL/BIO.h>
#include <OpenSSL/SSL.h>
#include <OpenSSL/ERR.h>

#include <StdLib.h>
#include <StdIO.h>
#include <String.h>
#include <TChar.h>
#include <Time.h>


#include <WinSock2.h>
#include <WS2TCPIP.h>
// #include <IPHlpAPI.h>
#include <Windows.h>

SSL *ssl;
BIO *bio1, *bio2;


int sock;

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


int main(int argc, char *argv[])
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
    sa.sin_addr.s_addr = inet_addr("87.240.182.224"); // vk.com
    sa.sin_port        = htons(443); // https
    socklen_t socklen = sizeof(sa);
    if (connect(s, (struct sockaddr *)&sa, socklen)) {
        printf("Error connecting to server.\n");
        return -1;
    }
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);
    ssl = SSL_new (ctx);
    if (!ssl) {
        printf("Error creating SSL.\n");
        log_ssl();
        return -1;
    }
    // sock = SSL_get_fd(ssl);
    int err;
    BIO_new_bio_pair(&bio1, 0, &bio2, 0);
    if (bio1 == NULL || bio2 == NULL) {
        printf("Error creating BIO err=%x\n", err);
        log_ssl();
        fflush(stdout);
        return -1;
    }
    // BIO_set_fd(bio, fd, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio1, bio1);

    BIO *internal_bio, *network_bio;

    while((err = SSL_connect(ssl)) == -1) {
        err = SSL_get_error(ssl,err);
        switch(err) {
            case SSL_ERROR_NONE: printf("ERROR = SSL_ERROR_NONE\n"); break;
            case SSL_ERROR_SSL: printf("ERROR = SSL_ERROR_SSL\n"); break;
            case SSL_ERROR_WANT_READ: printf("ERROR = SSL_ERROR_WANT_READ\n"); break;
            case SSL_ERROR_WANT_WRITE: printf("ERROR = SSL_ERROR_WANT_WRITE\n"); break;
            case SSL_ERROR_WANT_X509_LOOKUP: printf("ERROR = SSL_ERROR_WANT_X509_LOOKUP\n"); break;
            case SSL_ERROR_SYSCALL: printf("ERROR = SSL_ERROR_SYSCALL\n"); break;
            case SSL_ERROR_ZERO_RETURN: printf("ERROR = SSL_ERROR_ZERO_RETURN\n"); break;
            case SSL_ERROR_WANT_CONNECT: printf("ERROR = SSL_ERROR_WANT_CONNECT\n"); break;
            case SSL_ERROR_WANT_ACCEPT: printf("ERROR = SSL_ERROR_WANT_ACCEPT\n"); break;
        }

        log_ssl();
        char data[4096];
        int data_l = 0;
        int l;
        while((l = BIO_read( bio2, data+data_l, 4095-data_l ))>0)
        {
            printf("BIO2 Readed %d bytes\n", l);
            data_l += l;
        }
        printf("BIO2 Full Readed %d bytes\n", data_l);
        l = send( s,data,l,0 );
        printf("NET Sended %d bytes\n", l);
        l = recv( s,data,4095,0 );
        printf("NET Recived %d bytes\n", l);
        l = BIO_write( bio1, data, l );
        printf("BIO1 Writed %d bytes\n", l);
    }
    SSL_free(ssl);      /* implicitly frees internal_bio */
    BIO_free(network_bio);

    if (err <= 0) {
        printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        return -1;
    }
    printf ("SSL connection using %s\n", SSL_get_cipher (ssl));

    char *request = "GET / HTTP/1.1\r\nHost: vk.com\r\n\r\n";
    SendPacket(request);
    RecvPacket();
    return 0;
}

#if 0
#define S3I(s) (s->s3->internal)

int
ssl3_connect(SSL *s)
{
    void (*cb)(const SSL *ssl, int type, int val) = NULL;
    int ret = -1;
    int new_state, state, skip = 0;

    ERR_clear_error();
    errno = 0;

    if (s->internal->info_callback != NULL)
        cb = s->internal->info_callback;
    else if (s->ctx->internal->info_callback != NULL)
        cb = s->ctx->internal->info_callback;

    s->internal->in_handshake++;

    // Если не инициализирован
    // S3I(s)->hs.state = SSL_ST_BEFORE|SSL_ST_CONNECT;
    if (!SSL_in_init(s) || SSL_in_before(s))
        SSL_clear(s);


    for (;;) {
        state = S3I(s)->hs.state;

        switch (S3I(s)->hs.state) {
        case SSL_ST_RENEGOTIATE:
            s->internal->renegotiate = 1;
            S3I(s)->hs.state = SSL_ST_CONNECT;
            s->ctx->internal->stats.sess_connect_renegotiate++;
            /* break */
        case SSL_ST_BEFORE:
        case SSL_ST_CONNECT:
        case SSL_ST_BEFORE|SSL_ST_CONNECT:
        case SSL_ST_OK|SSL_ST_CONNECT:

            s->server = 0;
            if (cb != NULL)
                cb(s, SSL_CB_HANDSHAKE_START, 1);

            if ((s->version & 0xff00) != 0x0300) {
                SSLerror(s, ERR_R_INTERNAL_ERROR);
                ret = -1;
                goto end;
            }

            /* s->version=SSL3_VERSION; */
            s->internal->type = SSL_ST_CONNECT;

            if (!ssl3_setup_init_buffer(s)) {
                ret = -1;
                goto end;
            }
            if (!ssl3_setup_buffers(s)) {
                ret = -1;
                goto end;
            }
            if (!ssl_init_wbio_buffer(s, 0)) {
                ret = -1;
                goto end;
            }

            /* don't push the buffering BIO quite yet */

            if (!tls1_init_finished_mac(s)) {
                ret = -1;
                goto end;
            }

            S3I(s)->hs.state = SSL3_ST_CW_CLNT_HELLO_A;
            s->ctx->internal->stats.sess_connect++;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CW_CLNT_HELLO_A:
        case SSL3_ST_CW_CLNT_HELLO_B:
            s->internal->shutdown = 0;

            ret = ssl3_send_client_hello(s);
            if (ret <= 0)
                goto end;

            if (SSL_IS_DTLS(s) && D1I(s)->send_cookie) {
                S3I(s)->hs.state = SSL3_ST_CW_FLUSH;
                S3I(s)->hs.next_state = SSL3_ST_CR_SRVR_HELLO_A;
            } else
                S3I(s)->hs.state = SSL3_ST_CR_SRVR_HELLO_A;

            s->internal->init_num = 0;

            /* turn on buffering for the next lot of output */
            if (s->bbio != s->wbio)
                s->wbio = BIO_push(s->bbio, s->wbio);

            break;

        case SSL3_ST_CR_SRVR_HELLO_A:
        case SSL3_ST_CR_SRVR_HELLO_B:
            ret = ssl3_get_server_hello(s);
            if (ret <= 0)
                goto end;

            if (s->internal->hit) {
                S3I(s)->hs.state = SSL3_ST_CR_FINISHED_A;
                if (!SSL_IS_DTLS(s)) {
                    if (s->internal->tlsext_ticket_expected) {
                        /* receive renewed session ticket */
                        S3I(s)->hs.state = SSL3_ST_CR_SESSION_TICKET_A;
                    }
                }
            } else if (SSL_IS_DTLS(s)) {
                S3I(s)->hs.state = DTLS1_ST_CR_HELLO_VERIFY_REQUEST_A;
            } else {
                S3I(s)->hs.state = SSL3_ST_CR_CERT_A;
            }
            s->internal->init_num = 0;
            break;

        case DTLS1_ST_CR_HELLO_VERIFY_REQUEST_A:
        case DTLS1_ST_CR_HELLO_VERIFY_REQUEST_B:
            ret = dtls1_get_hello_verify(s);
            if (ret <= 0)
                goto end;
            dtls1_stop_timer(s);
            if (D1I(s)->send_cookie) /* start again, with a cookie */
                S3I(s)->hs.state = SSL3_ST_CW_CLNT_HELLO_A;
            else
                S3I(s)->hs.state = SSL3_ST_CR_CERT_A;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CR_CERT_A:
        case SSL3_ST_CR_CERT_B:
            ret = ssl3_check_finished(s);
            if (ret <= 0)
                goto end;
            if (ret == 2) {
                s->internal->hit = 1;
                if (s->internal->tlsext_ticket_expected)
                    S3I(s)->hs.state = SSL3_ST_CR_SESSION_TICKET_A;
                else
                    S3I(s)->hs.state = SSL3_ST_CR_FINISHED_A;
                s->internal->init_num = 0;
                break;
            }
            /* Check if it is anon DH/ECDH. */
            if (!(S3I(s)->hs.new_cipher->algorithm_auth &
                SSL_aNULL)) {
                ret = ssl3_get_server_certificate(s);
                if (ret <= 0)
                    goto end;
                if (s->internal->tlsext_status_expected)
                    S3I(s)->hs.state = SSL3_ST_CR_CERT_STATUS_A;
                else
                    S3I(s)->hs.state = SSL3_ST_CR_KEY_EXCH_A;
            } else {
                skip = 1;
                S3I(s)->hs.state = SSL3_ST_CR_KEY_EXCH_A;
            }
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CR_KEY_EXCH_A:
        case SSL3_ST_CR_KEY_EXCH_B:
            ret = ssl3_get_server_key_exchange(s);
            if (ret <= 0)
                goto end;
            S3I(s)->hs.state = SSL3_ST_CR_CERT_REQ_A;
            s->internal->init_num = 0;

            /*
             * At this point we check that we have the
             * required stuff from the server.
             */
            if (!ssl3_check_cert_and_algorithm(s)) {
                ret = -1;
                goto end;
            }
            break;

        case SSL3_ST_CR_CERT_REQ_A:
        case SSL3_ST_CR_CERT_REQ_B:
            ret = ssl3_get_certificate_request(s);
            if (ret <= 0)
                goto end;
            S3I(s)->hs.state = SSL3_ST_CR_SRVR_DONE_A;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CR_SRVR_DONE_A:
        case SSL3_ST_CR_SRVR_DONE_B:
            ret = ssl3_get_server_done(s);
            if (ret <= 0)
                goto end;
            if (SSL_IS_DTLS(s))
                dtls1_stop_timer(s);
            if (S3I(s)->tmp.cert_req)
                S3I(s)->hs.state = SSL3_ST_CW_CERT_A;
            else
                S3I(s)->hs.state = SSL3_ST_CW_KEY_EXCH_A;
            s->internal->init_num = 0;

            break;

        case SSL3_ST_CW_CERT_A:
        case SSL3_ST_CW_CERT_B:
        case SSL3_ST_CW_CERT_C:
        case SSL3_ST_CW_CERT_D:
            if (SSL_IS_DTLS(s))
                dtls1_start_timer(s);
            ret = ssl3_send_client_certificate(s);
            if (ret <= 0)
                goto end;
            S3I(s)->hs.state = SSL3_ST_CW_KEY_EXCH_A;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CW_KEY_EXCH_A:
        case SSL3_ST_CW_KEY_EXCH_B:
            if (SSL_IS_DTLS(s))
                dtls1_start_timer(s);
            ret = ssl3_send_client_key_exchange(s);
            if (ret <= 0)
                goto end;
            /*
             * EAY EAY EAY need to check for DH fix cert
             * sent back
             */
            /*
             * For TLS, cert_req is set to 2, so a cert chain
             * of nothing is sent, but no verify packet is sent
             */
            /*
             * XXX: For now, we do not support client
             * authentication in ECDH cipher suites with
             * ECDH (rather than ECDSA) certificates.
             * We need to skip the certificate verify
             * message when client's ECDH public key is sent
             * inside the client certificate.
             */
            if (S3I(s)->tmp.cert_req == 1) {
                S3I(s)->hs.state = SSL3_ST_CW_CERT_VRFY_A;
            } else {
                S3I(s)->hs.state = SSL3_ST_CW_CHANGE_A;
                S3I(s)->change_cipher_spec = 0;
            }
            if (!SSL_IS_DTLS(s)) {
                if (s->s3->flags & TLS1_FLAGS_SKIP_CERT_VERIFY) {
                    S3I(s)->hs.state = SSL3_ST_CW_CHANGE_A;
                    S3I(s)->change_cipher_spec = 0;
                }
            }

            s->internal->init_num = 0;
            break;

        case SSL3_ST_CW_CERT_VRFY_A:
        case SSL3_ST_CW_CERT_VRFY_B:
            if (SSL_IS_DTLS(s))
                dtls1_start_timer(s);
            ret = ssl3_send_client_verify(s);
            if (ret <= 0)
                goto end;
            S3I(s)->hs.state = SSL3_ST_CW_CHANGE_A;
            s->internal->init_num = 0;
            S3I(s)->change_cipher_spec = 0;
            break;

        case SSL3_ST_CW_CHANGE_A:
        case SSL3_ST_CW_CHANGE_B:
            if (SSL_IS_DTLS(s) && !s->internal->hit)
                dtls1_start_timer(s);
            ret = ssl3_send_change_cipher_spec(s,
                SSL3_ST_CW_CHANGE_A, SSL3_ST_CW_CHANGE_B);
            if (ret <= 0)
                goto end;

            S3I(s)->hs.state = SSL3_ST_CW_FINISHED_A;
            s->internal->init_num = 0;

            s->session->cipher = S3I(s)->hs.new_cipher;
            if (!tls1_setup_key_block(s)) {
                ret = -1;
                goto end;
            }

            if (!tls1_change_cipher_state(s,
                SSL3_CHANGE_CIPHER_CLIENT_WRITE)) {
                ret = -1;
                goto end;
            }

            if (SSL_IS_DTLS(s))
                dtls1_reset_seq_numbers(s, SSL3_CC_WRITE);

            break;

        case SSL3_ST_CW_FINISHED_A:
        case SSL3_ST_CW_FINISHED_B:
            if (SSL_IS_DTLS(s) && !s->internal->hit)
                dtls1_start_timer(s);
            ret = ssl3_send_finished(s, SSL3_ST_CW_FINISHED_A,
                SSL3_ST_CW_FINISHED_B, TLS_MD_CLIENT_FINISH_CONST,
                TLS_MD_CLIENT_FINISH_CONST_SIZE);
            if (ret <= 0)
                goto end;
            if (!SSL_IS_DTLS(s))
                s->s3->flags |= SSL3_FLAGS_CCS_OK;
            S3I(s)->hs.state = SSL3_ST_CW_FLUSH;

            /* clear flags */
            if (s->internal->hit) {
                S3I(s)->hs.next_state = SSL_ST_OK;
            } else {
                /* Allow NewSessionTicket if ticket expected */
                if (s->internal->tlsext_ticket_expected)
                    S3I(s)->hs.next_state =
                        SSL3_ST_CR_SESSION_TICKET_A;
                else
                    S3I(s)->hs.next_state =
                        SSL3_ST_CR_FINISHED_A;
            }
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CR_SESSION_TICKET_A:
        case SSL3_ST_CR_SESSION_TICKET_B:
            ret = ssl3_get_new_session_ticket(s);
            if (ret <= 0)
                goto end;
            S3I(s)->hs.state = SSL3_ST_CR_FINISHED_A;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CR_CERT_STATUS_A:
        case SSL3_ST_CR_CERT_STATUS_B:
            ret = ssl3_get_cert_status(s);
            if (ret <= 0)
                goto end;
            S3I(s)->hs.state = SSL3_ST_CR_KEY_EXCH_A;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CR_FINISHED_A:
        case SSL3_ST_CR_FINISHED_B:
            if (SSL_IS_DTLS(s))
                D1I(s)->change_cipher_spec_ok = 1;
            else
                s->s3->flags |= SSL3_FLAGS_CCS_OK;
            ret = ssl3_get_finished(s, SSL3_ST_CR_FINISHED_A,
                SSL3_ST_CR_FINISHED_B);
            if (ret <= 0)
                goto end;
            if (SSL_IS_DTLS(s))
                dtls1_stop_timer(s);

            if (s->internal->hit)
                S3I(s)->hs.state = SSL3_ST_CW_CHANGE_A;
            else
                S3I(s)->hs.state = SSL_ST_OK;
            s->internal->init_num = 0;
            break;

        case SSL3_ST_CW_FLUSH:
            s->internal->rwstate = SSL_WRITING;
            if (BIO_flush(s->wbio) <= 0) {
                if (SSL_IS_DTLS(s)) {
                    /* If the write error was fatal, stop trying */
                    if (!BIO_should_retry(s->wbio)) {
                        s->internal->rwstate = SSL_NOTHING;
                        S3I(s)->hs.state = S3I(s)->hs.next_state;
                    }
                }
                ret = -1;
                goto end;
            }
            s->internal->rwstate = SSL_NOTHING;
            S3I(s)->hs.state = S3I(s)->hs.next_state;
            break;

        case SSL_ST_OK:
            /* clean a few things up */
            tls1_cleanup_key_block(s);

            if (!SSL_IS_DTLS(s)) {
                BUF_MEM_free(s->internal->init_buf);
                s->internal->init_buf = NULL;
            }

            ssl_free_wbio_buffer(s);

            s->internal->init_num = 0;
            s->internal->renegotiate = 0;
            s->internal->new_session = 0;

            ssl_update_cache(s, SSL_SESS_CACHE_CLIENT);
            if (s->internal->hit)
                s->ctx->internal->stats.sess_hit++;

            ret = 1;
            /* s->server=0; */
            s->internal->handshake_func = ssl3_connect;
            s->ctx->internal->stats.sess_connect_good++;

            if (cb != NULL)
                cb(s, SSL_CB_HANDSHAKE_DONE, 1);

            if (SSL_IS_DTLS(s)) {
                /* done with handshaking */
                D1I(s)->handshake_read_seq = 0;
                D1I(s)->next_handshake_write_seq = 0;
            }

            goto end;
            /* break; */

        default:
            SSLerror(s, SSL_R_UNKNOWN_STATE);
            ret = -1;
            goto end;
            /* break; */
        }

        /* did we do anything */
        if (!S3I(s)->tmp.reuse_message && !skip) {
            if (s->internal->debug) {
                if ((ret = BIO_flush(s->wbio)) <= 0)
                    goto end;
            }

            if ((cb != NULL) && (S3I(s)->hs.state != state)) {
                new_state = S3I(s)->hs.state;
                S3I(s)->hs.state = state;
                cb(s, SSL_CB_CONNECT_LOOP, 1);
                S3I(s)->hs.state = new_state;
            }
        }
        skip = 0;
    }

end:
    s->internal->in_handshake--;
    if (cb != NULL)
        cb(s, SSL_CB_CONNECT_EXIT, ret);

    return (ret);
}

#endif
