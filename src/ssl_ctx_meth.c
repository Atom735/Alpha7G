
#include <stdio.h>

#include "F:\\sdk\\libressl-2.8.2\\ssl\\ssl_locl.h"
#include "F:\\sdk\\libressl-2.8.2\\ssl\\bytestring.h"
#include "F:\\sdk\\libressl-2.8.2\\ssl\\ssl_tlsext.h"

#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/objects.h>

int A7_ssl3_connect(SSL *s)
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

            if (SSL_IS_DTLS(s)) {
                if ((s->version & 0xff00) != (DTLS1_VERSION & 0xff00)) {
                    SSLerror(s, ERR_R_INTERNAL_ERROR);
                    ret = -1;
                    goto end;
                }
            } else {
                if ((s->version & 0xff00) != 0x0300) {
                    SSLerror(s, ERR_R_INTERNAL_ERROR);
                    ret = -1;
                    goto end;
                }
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

            if (!SSL_IS_DTLS(s)) {
                if (!tls1_init_finished_mac(s)) {
                    ret = -1;
                    goto end;
                }
            }

            S3I(s)->hs.state = SSL3_ST_CW_CLNT_HELLO_A;
            s->ctx->internal->stats.sess_connect++;
            s->internal->init_num = 0;

            if (SSL_IS_DTLS(s)) {
                /* mark client_random uninitialized */
                memset(s->s3->client_random, 0,
                    sizeof(s->s3->client_random));
                D1I(s)->send_cookie = 0;
                s->internal->hit = 0;
            }
            break;

        case SSL3_ST_CW_CLNT_HELLO_A:
        case SSL3_ST_CW_CLNT_HELLO_B:
            s->internal->shutdown = 0;

            if (SSL_IS_DTLS(s)) {
                /* every DTLS ClientHello resets Finished MAC */
                if (!tls1_init_finished_mac(s)) {
                    ret = -1;
                    goto end;
                }

                dtls1_start_timer(s);
            }

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

static const SSL_METHOD_INTERNAL A7_TLSv1_2_client_method_internal_data = {
    .version = TLS1_2_VERSION,
    .min_version = TLS1_2_VERSION,
    .max_version = TLS1_2_VERSION,
    .ssl_new = tls1_new,
    .ssl_clear = tls1_clear,
    .ssl_free = tls1_free,
    .ssl_accept = ssl_undefined_function,
    .ssl_connect = A7_ssl3_connect,
    .get_ssl_method = tls1_get_client_method,
    .get_timeout = tls1_default_timeout,
    .ssl_version = ssl_undefined_void_function,
    .ssl_renegotiate = ssl3_renegotiate,
    .ssl_renegotiate_check = ssl3_renegotiate_check,
    .ssl_get_message = ssl3_get_message,
    .ssl_read_bytes = ssl3_read_bytes,
    .ssl_write_bytes = ssl3_write_bytes,
    .ssl3_enc = &TLSv1_2_enc_data,
};

static const SSL_METHOD A7_TLSv1_2_client_method_data = {
    .ssl_dispatch_alert = ssl3_dispatch_alert,
    .num_ciphers = ssl3_num_ciphers,
    .get_cipher = ssl3_get_cipher,
    .get_cipher_by_char = ssl3_get_cipher_by_char,
    .put_cipher_by_char = ssl3_put_cipher_by_char,
    .internal = &A7_TLSv1_2_client_method_internal_data,
};

const SSL_METHOD * A7_TLSv1_2_client_method(void) {
    return (&A7_TLSv1_2_client_method_data);
}
