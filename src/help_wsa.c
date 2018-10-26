#define LOG_ERR_WSA(_fn, _err) LOG_ERRS(_fn, _err, encode_WSAGetLastError( _err ) );

#define WSAGETASYNCBUFLEN(lParam) LOWORD(lParam)
#define WSAGETASYNCERROR(lParam)  HIWORD(lParam)

#define WM_WSAASYNCGETHOSTBYNAME (WM_USER + 1)

#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
#define WM_WSAASYNCSELECT (WM_USER + 2)


#define A7_WSAStartup(_wsadata) {\
        int err = WSAStartup( WINSOCK_VERSION, _wsadata );\
        if( FAILED(err) ) {\
            LOG_ERR_WSA( "WSAStartup()", err );\
            return (-1);\
        }\
    }
#define A7_WSACleanup() {\
        if ( WSACleanup() == SOCKET_ERROR ) {\
            int err = WSAGetLastError();\
            LOG_ERR_WSA( "WSACleanup()", err );\
        }\
    }



const char* encode_WSAGetLastError(int err) {
    static char buf[32];
    switch(err) {
        case WSAEINTR:               return "WSAEINTR";
        case WSAEBADF:               return "WSAEBADF";
        case WSAEACCES:              return "WSAEACCES";
        case WSAEFAULT:              return "WSAEFAULT";
        case WSAEINVAL:              return "WSAEINVAL";
        case WSAEMFILE:              return "WSAEMFILE";
        case WSAEWOULDBLOCK:         return "WSAEWOULDBLOCK";
        case WSAEINPROGRESS:         return "WSAEINPROGRESS";
        case WSAEALREADY:            return "WSAEALREADY";
        case WSAENOTSOCK:            return "WSAENOTSOCK";
        case WSAEDESTADDRREQ:        return "WSAEDESTADDRREQ";
        case WSAEMSGSIZE:            return "WSAEMSGSIZE";
        case WSAEPROTOTYPE:          return "WSAEPROTOTYPE";
        case WSAENOPROTOOPT:         return "WSAENOPROTOOPT";
        case WSAEPROTONOSUPPORT:     return "WSAEPROTONOSUPPORT";
        case WSAESOCKTNOSUPPORT:     return "WSAESOCKTNOSUPPORT";
        case WSAEOPNOTSUPP:          return "WSAEOPNOTSUPP";
        case WSAEPFNOSUPPORT:        return "WSAEPFNOSUPPORT";
        case WSAEAFNOSUPPORT:        return "WSAEAFNOSUPPORT";
        case WSAEADDRINUSE:          return "WSAEADDRINUSE";
        case WSAEADDRNOTAVAIL:       return "WSAEADDRNOTAVAIL";
        case WSAENETDOWN:            return "WSAENETDOWN";
        case WSAENETUNREACH:         return "WSAENETUNREACH";
        case WSAENETRESET:           return "WSAENETRESET";
        case WSAECONNABORTED:        return "WSAECONNABORTED";
        case WSAECONNRESET:          return "WSAECONNRESET";
        case WSAENOBUFS:             return "WSAENOBUFS";
        case WSAEISCONN:             return "WSAEISCONN";
        case WSAENOTCONN:            return "WSAENOTCONN";
        case WSAESHUTDOWN:           return "WSAESHUTDOWN";
        case WSAETOOMANYREFS:        return "WSAETOOMANYREFS";
        case WSAETIMEDOUT:           return "WSAETIMEDOUT";
        case WSAECONNREFUSED:        return "WSAECONNREFUSED";
        case WSAELOOP:               return "WSAELOOP";
        case WSAENAMETOOLONG:        return "WSAENAMETOOLONG";
        case WSAEHOSTDOWN:           return "WSAEHOSTDOWN";
        case WSAEHOSTUNREACH:        return "WSAEHOSTUNREACH";
        case WSAENOTEMPTY:           return "WSAENOTEMPTY";
        case WSAEPROCLIM:            return "WSAEPROCLIM";
        case WSAEUSERS:              return "WSAEUSERS";
        case WSAEDQUOT:              return "WSAEDQUOT";
        case WSAESTALE:              return "WSAESTALE";
        case WSAEREMOTE:             return "WSAEREMOTE";
        case WSASYSNOTREADY:         return "WSASYSNOTREADY";
        case WSAVERNOTSUPPORTED:     return "WSAVERNOTSUPPORTED";
        case WSANOTINITIALISED:      return "WSANOTINITIALISED";
        case WSAEDISCON:             return "WSAEDISCON";
        case WSAENOMORE:             return "WSAENOMORE";
        case WSAECANCELLED:          return "WSAECANCELLED";
        case WSAEINVALIDPROCTABLE:   return "WSAEINVALIDPROCTABLE";
        case WSAEINVALIDPROVIDER:    return "WSAEINVALIDPROVIDER";
        case WSAEPROVIDERFAILEDINIT: return "WSAEPROVIDERFAILEDINIT";
        case WSASYSCALLFAILURE:      return "WSASYSCALLFAILURE";
        case WSASERVICE_NOT_FOUND:   return "WSASERVICE_NOT_FOUND";
        case WSATYPE_NOT_FOUND:      return "WSATYPE_NOT_FOUND";
        case WSA_E_NO_MORE:          return "WSA_E_NO_MORE";
        case WSA_E_CANCELLED:        return "WSA_E_CANCELLED";
        case WSAEREFUSED:            return "WSAEREFUSED";
        #ifdef WSAHOST_NOT_FOUND
        case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
        #endif
        #ifdef WSATRY_AGAIN
        case WSATRY_AGAIN: return "WSATRY_AGAIN";
        #endif
        #ifdef WSANO_RECOVERY
        case WSANO_RECOVERY: return "WSANO_RECOVERY";
        #endif
        #ifdef WSANO_DATA
        case WSANO_DATA: return "WSANO_DATA";
        #endif
        #ifdef WSA_QOS_RECEIVERS
        case WSA_QOS_RECEIVERS: return "WSA_QOS_RECEIVERS";
        #endif
        #ifdef WSA_QOS_SENDERS
        case WSA_QOS_SENDERS: return "WSA_QOS_SENDERS";
        #endif
        #ifdef WSA_QOS_NO_SENDERS
        case WSA_QOS_NO_SENDERS: return "WSA_QOS_NO_SENDERS";
        #endif
        #ifdef WSA_QOS_NO_RECEIVERS
        case WSA_QOS_NO_RECEIVERS: return "WSA_QOS_NO_RECEIVERS";
        #endif
        #ifdef WSA_QOS_REQUEST_CONFIRMED
        case WSA_QOS_REQUEST_CONFIRMED: return "WSA_QOS_REQUEST_CONFIRMED";
        #endif
        #ifdef WSA_QOS_ADMISSION_FAILURE
        case WSA_QOS_ADMISSION_FAILURE: return "WSA_QOS_ADMISSION_FAILURE";
        #endif
        #ifdef WSA_QOS_POLICY_FAILURE
        case WSA_QOS_POLICY_FAILURE: return "WSA_QOS_POLICY_FAILURE";
        #endif
        #ifdef WSA_QOS_BAD_STYLE
        case WSA_QOS_BAD_STYLE: return "WSA_QOS_BAD_STYLE";
        #endif
        #ifdef WSA_QOS_BAD_OBJECT
        case WSA_QOS_BAD_OBJECT: return "WSA_QOS_BAD_OBJECT";
        #endif
        #ifdef WSA_QOS_TRAFFIC_CTRL_ERROR
        case WSA_QOS_TRAFFIC_CTRL_ERROR: return "WSA_QOS_TRAFFIC_CTRL_ERROR";
        #endif
        #ifdef WSA_QOS_GENERIC_ERROR
        case WSA_QOS_GENERIC_ERROR: return "WSA_QOS_GENERIC_ERROR";
        #endif
        #ifdef WSA_QOS_ESERVICETYPE
        case WSA_QOS_ESERVICETYPE: return "WSA_QOS_ESERVICETYPE";
        #endif
        #ifdef WSA_QOS_EFLOWSPEC
        case WSA_QOS_EFLOWSPEC: return "WSA_QOS_EFLOWSPEC";
        #endif
        #ifdef WSA_QOS_EPROVSPECBUF
        case WSA_QOS_EPROVSPECBUF: return "WSA_QOS_EPROVSPECBUF";
        #endif
        #ifdef WSA_QOS_EFILTERSTYLE
        case WSA_QOS_EFILTERSTYLE: return "WSA_QOS_EFILTERSTYLE";
        #endif
        #ifdef WSA_QOS_EFILTERTYPE
        case WSA_QOS_EFILTERTYPE: return "WSA_QOS_EFILTERTYPE";
        #endif
        #ifdef WSA_QOS_EFILTERCOUNT
        case WSA_QOS_EFILTERCOUNT: return "WSA_QOS_EFILTERCOUNT";
        #endif
        #ifdef WSA_QOS_EOBJLENGTH
        case WSA_QOS_EOBJLENGTH: return "WSA_QOS_EOBJLENGTH";
        #endif
        #ifdef WSA_QOS_EFLOWCOUNT
        case WSA_QOS_EFLOWCOUNT: return "WSA_QOS_EFLOWCOUNT";
        #endif
        #ifdef WSA_QOS_EUNKNOWNPSOBJ
        case WSA_QOS_EUNKNOWNPSOBJ: return "WSA_QOS_EUNKNOWNPSOBJ";
        #endif
        #ifdef WSA_QOS_EPOLICYOBJ
        case WSA_QOS_EPOLICYOBJ: return "WSA_QOS_EPOLICYOBJ";
        #endif
        #ifdef WSA_QOS_EFLOWDESC
        case WSA_QOS_EFLOWDESC: return "WSA_QOS_EFLOWDESC";
        #endif
        #ifdef WSA_QOS_EPSFLOWSPEC
        case WSA_QOS_EPSFLOWSPEC: return "WSA_QOS_EPSFLOWSPEC";
        #endif
        #ifdef WSA_QOS_EPSFILTERSPEC
        case WSA_QOS_EPSFILTERSPEC: return "WSA_QOS_EPSFILTERSPEC";
        #endif
        #ifdef WSA_QOS_ESDMODEOBJ
        case WSA_QOS_ESDMODEOBJ: return "WSA_QOS_ESDMODEOBJ";
        #endif
        #ifdef WSA_QOS_ESHAPERATEOBJ
        case WSA_QOS_ESHAPERATEOBJ: return "WSA_QOS_ESHAPERATEOBJ";
        #endif
        #ifdef WSA_QOS_RESERVED_PETYPE
        case WSA_QOS_RESERVED_PETYPE: return "WSA_QOS_RESERVED_PETYPE";
        #endif
    }
    snprintf(buf, 32, "UNKNOWN %x (%d)", err, err );
    return buf;
}
