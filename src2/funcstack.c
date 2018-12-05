#include "header.h"

S7FuncStack *A7FS_New ( P7FuncStackProc rFunc, VOID *pvCtx ) {
    LOG_ENTER ( );
    LOG_ARG_HANDLE  ( rFunc );
    LOG_ARG_HANDLE  ( pvCtx );
    S7FuncStack *pThis = ( S7FuncStack* ) malloc ( sizeof ( S7FuncStack ) );
    F7ERR_MALLOC ( pThis, L"Под структуру S7FuncStack внутри функции A7FS_New" );
    pThis -> bAllocated = TRUE;
    pThis -> rFunc = rFunc;
    pThis -> pPrev = NULL;
    pThis -> pvCtx = pvCtx;
    LOG_RET_HANDLE ( pThis );
    LOG_LEAVE ( );
    return pThis;
}
S7FuncStack *A7FS_Push ( S7FuncStack *pThis, S7FuncStack *pPush ) {
    LOG_ENTER ( );
    LOG_ARG_HANDLE  ( pThis );
    LOG_ARG_HANDLE  ( pPush );
    if ( pPush -> rFunc ( K7FS_PUSH, pPush -> pvCtx ) ) {
        LOG_RET_HANDLE ( pThis );
        LOG_LEAVE ( );
        return pThis;
    }
    pPush -> pPrev = pThis;
    LOG_RET_HANDLE ( pPush );
    LOG_LEAVE ( );
    return pPush;
}
S7FuncStack *A7FS_PushNew ( S7FuncStack *pThis, P7FuncStackProc rFunc, VOID *pvCtx ) {
    LOG_ENTER ( );
    LOG_ARG_HANDLE  ( pThis );
    LOG_ARG_HANDLE  ( rFunc );
    LOG_ARG_HANDLE  ( pvCtx );
    S7FuncStack *pPush = ( S7FuncStack* ) malloc ( sizeof ( S7FuncStack ) );
    F7ERR_MALLOC ( pPush, L"Под структуру S7FuncStack внутри функции A7FS_PushNew" );
    pPush -> bAllocated = TRUE;
    pPush -> rFunc = rFunc;
    pPush -> pPrev = NULL;
    pPush -> pvCtx = pvCtx;
    if ( pPush -> rFunc ( K7FS_PUSH, pPush -> pvCtx ) ) {
        LOG_RET_HANDLE ( pThis );
        LOG_LEAVE ( );
        return pThis;
    }
    pPush -> pPrev = pThis;
    LOG_RET_HANDLE ( pPush );
    LOG_LEAVE ( );
    return pPush;
}
S7FuncStack *A7FS_Pop ( S7FuncStack *pThis ) {
    LOG_ENTER ( );
    LOG_ARG_HANDLE  ( pThis );
    if ( pThis -> rFunc ( K7FS_POP, pThis -> pvCtx ) ) {
        LOG_RET_HANDLE ( NULL );
        LOG_LEAVE ( );
        return NULL;
    }
    S7FuncStack *pPrev = pThis -> pPrev;
    if ( pThis -> bAllocated ) {
        free ( pThis );
    }
    LOG_RET_HANDLE ( pPrev );
    LOG_LEAVE ( );
    return pPrev;

}
