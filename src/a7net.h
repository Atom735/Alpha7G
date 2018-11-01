#ifndef _H_A7NET_H_
#define _H_A7NET_H_

#include <WinSock2.h>
#include <Windows.h>

/* Количество активных соединений */
extern int g_A7NetConnectsCount;

/* Структура данных для создания окна подключения */
typedef struct _S7WDA7NETCRT S7WDA7NETCRT;

/* Структура данных приязанных к окну подключения */
typedef struct _S7WDA7NET S7WDA7NET;

/* Таблица Callback функций */
typedef struct _S7WDA7NETCB S7WDA7NETCB;


struct _S7WDA7NETCRT {
    BYTE    aIp[16];    /* IPv6 адресс,
        если IPv4,
        то первые 10 байт 0х00,
        потом 2 байта 0хff,
        потом 4 байта IPv4 адресс */
    UINT16  nPort;      /* Порт подключения */
    BOOL    bTls;       /* Использовать TLS? */
    S7WDA7NETCB *pCBS;  /* Укзатель на таблицу Callback функций */
    BOOL    bForceV6;   /* Принудительно использовать IPv6,
        даже если можно использовать IPv4 */
};


struct _S7WDA7NET {
    HWND    hWnd;   /* Хендл окна к которому привязаны данные */
    SOCKET  iSock;  /* Номер сокета подключения */
    BOOL    bTls;   /* Флаг установки защищенного соединения  */
    S7WDA7NETCB *pCBS;  /* Укзатель на таблицу Callback функций */

    CHAR   *szText; /* Имя окна подключения */
    FILE   *fLog;   /* Файл лога */
    ULONG   nClock; /* Количество тиков при создании окна подключения */

    BYTE    buf[4096];
    INT     send;
};

struct _S7WDA7NETCB {
    /* Если Callback возвращает не FALSE, то соединение будет закрыто,
        а окно уничтожено */

    /*  Callback вызывемый в случае успешной установки подключения */
    BOOL ( CALLBACK *OnConnect )( S7WDA7NET *p );
    /*  Callback вызывемый в случае закрытия соединения */
    BOOL ( CALLBACK *OnClose )( S7WDA7NET *p );
    /*  Callback вызывемый в случае пояления места для записи */
    BOOL ( CALLBACK *OnWrite )( S7WDA7NET *p );
    /*  Callback вызывемый в случае пояления данных для чтения */
    BOOL ( CALLBACK *OnRead )( S7WDA7NET *p );
    /*  Callback вызывемый в случае ошибки */
    BOOL ( CALLBACK *OnError )( S7WDA7NET *p, int etp, int err );
};

/*  Инициализация системы A7NET
    @hInstance  > Instance приложения
    @<          >   0   в случае успеха
                    -1  в случае ошибки
*/
extern int      A7NetInit           ( HINSTANCE hInstance );

/*  Высвобождение ресурсов системы A7NET */
extern void     A7NetRelease        ( void );

/*  Подключение к серверу по TCP/IPv6 или TCP/IPv4
    @pCRT       > Данные подключения
    @<          >   HWND окна, который обрабатывает подключения
                    NULL в случае ошибки
*/
extern HWND     A7NetNewConnect     ( S7WDA7NETCRT *pCRT );

/*  Получить данные привязанные к окну A7NET
    @hWnd       > HWND окна A7NET
    @<          >   Укзатель на данные окна
                    NULL в случае оишбки
*/
extern S7WDA7NET * A7NetGetWndData  ( HWND hWnd );


/*  Подключение к серверу по TCP/IPv4
    @pIp        > IPv4 сервера (4байта)
    @nPort      > Порт сервера (2байта)
    @bTls       > Флаг установки защищенного соединения
    @pCBS       > Указатель на таблицу Callback функций (может быть NULL)
    @<          >   HWND окна, который обрабатывает подключения
                    NULL в случае ошибки
*/
extern HWND     A7NetNewConnect4    ( BYTE *pIp, UINT16 nPort, BOOL bTls,
    S7WDA7NETCB *pCBS );

/*  Подключение к серверу по TCP/IPv6
    @pIp        > IPv6 сервера (16байт)
    @nPort      > Порт сервера (2байта)
    @bTls       > Флаг установки защищенного соединения
    @pCBS       > Указатель на таблицу Callback функций (может быть NULL)
    @<          >   HWND окна, который обрабатывает подключения
                    NULL в случае ошибки
*/
extern HWND     A7NetNewConnect6    ( UINT16 *pIp, UINT16 nPort, BOOL bTls,
    S7WDA7NETCB *pCBS );

/*  Подключение к DNS серверу по TCP/IPv4
    @pIp        > IPv4 сервера (4байта)
    @<          >   HWND окна, который обрабатывает подключения
                    NULL в случае ошибки
*/
extern HWND     A7NetNewConnectDns4 ( BYTE *pIp );

/*  Подключение к DNS серверу по TCP/IPv6
    @pIp        > IPv4 сервера (16байт)
    @<          >   HWND окна, который обрабатывает подключения
                    NULL в случае ошибки
*/
extern HWND     A7NetNewConnectDns6 ( UINT16 *pIp );


#endif
