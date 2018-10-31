#define __A7NLT(_err,_str) case _err: A7NetLog ( p, "    %s\n", _str ); break
#define __A7NLT2(_err,_str,_str2) case _err: A7NetLog ( p, "    %s\n    %s\n", _str,_str2 ); break
#define __A7NLT3(_err,_str,_str2,_str3) case _err: A7NetLog ( p, "    %s\n    %s\n    %s\n", _str, _str2 _str3 ); break
#define __A7NLE(_str) default: A7NetLog ( p, "    %s\n", _str )
#define __A7NLD() __A7NLE( "Unknown error." )
#define __A7NLS(_err) case _err: return #_err;
#define __A7NLF(_err,_str) case _err: A7NetLog ( p, "    %s => %s\n", #_err, _str ); break


static const char * A7Str_WSAGetLastError( int err ) {
    switch ( err ) {
        __A7NLS(WSAEINTR);
        __A7NLS(WSAEBADF);
        __A7NLS(WSAEACCES);
        __A7NLS(WSAEFAULT);
        __A7NLS(WSAEINVAL);
        __A7NLS(WSAEMFILE);
        __A7NLS(WSAEWOULDBLOCK);
        __A7NLS(WSAEINPROGRESS);
        __A7NLS(WSAEALREADY);
        __A7NLS(WSAENOTSOCK);
        __A7NLS(WSAEDESTADDRREQ);
        __A7NLS(WSAEMSGSIZE);
        __A7NLS(WSAEPROTOTYPE);
        __A7NLS(WSAENOPROTOOPT);
        __A7NLS(WSAEPROTONOSUPPORT);
        __A7NLS(WSAESOCKTNOSUPPORT);
        __A7NLS(WSAEOPNOTSUPP);
        __A7NLS(WSAEPFNOSUPPORT);
        __A7NLS(WSAEAFNOSUPPORT);
        __A7NLS(WSAEADDRINUSE);
        __A7NLS(WSAEADDRNOTAVAIL);
        __A7NLS(WSAENETDOWN);
        __A7NLS(WSAENETUNREACH);
        __A7NLS(WSAENETRESET);
        __A7NLS(WSAECONNABORTED);
        __A7NLS(WSAECONNRESET);
        __A7NLS(WSAENOBUFS);
        __A7NLS(WSAEISCONN);
        __A7NLS(WSAENOTCONN);
        __A7NLS(WSAESHUTDOWN);
        __A7NLS(WSAETOOMANYREFS);
        __A7NLS(WSAETIMEDOUT);
        __A7NLS(WSAECONNREFUSED);
        __A7NLS(WSAELOOP);
        __A7NLS(WSAENAMETOOLONG);
        __A7NLS(WSAEHOSTDOWN);
        __A7NLS(WSAEHOSTUNREACH);
        __A7NLS(WSAENOTEMPTY);
        __A7NLS(WSAEPROCLIM);
        __A7NLS(WSAEUSERS);
        __A7NLS(WSAEDQUOT);
        __A7NLS(WSAESTALE);
        __A7NLS(WSAEREMOTE);
        __A7NLS(WSASYSNOTREADY);
        __A7NLS(WSAVERNOTSUPPORTED);
        __A7NLS(WSANOTINITIALISED);
        __A7NLS(WSAEDISCON);
        __A7NLS(WSAENOMORE);
        __A7NLS(WSAECANCELLED);
        __A7NLS(WSAEINVALIDPROCTABLE);
        __A7NLS(WSAEINVALIDPROVIDER);
        __A7NLS(WSAEPROVIDERFAILEDINIT);
        __A7NLS(WSASYSCALLFAILURE);
        __A7NLS(WSASERVICE_NOT_FOUND);
        __A7NLS(WSATYPE_NOT_FOUND);
        __A7NLS(WSA_E_NO_MORE);
        __A7NLS(WSA_E_CANCELLED);
        __A7NLS(WSAEREFUSED);
        __A7NLS(WSAHOST_NOT_FOUND);
        __A7NLS(WSATRY_AGAIN);
        __A7NLS(WSANO_RECOVERY);
        __A7NLS(WSANO_DATA);
        __A7NLS(WSA_QOS_RECEIVERS);
        __A7NLS(WSA_QOS_SENDERS);
        __A7NLS(WSA_QOS_NO_SENDERS);
        __A7NLS(WSA_QOS_NO_RECEIVERS);
        __A7NLS(WSA_QOS_REQUEST_CONFIRMED);
        __A7NLS(WSA_QOS_ADMISSION_FAILURE);
        __A7NLS(WSA_QOS_POLICY_FAILURE);
        __A7NLS(WSA_QOS_BAD_STYLE);
        __A7NLS(WSA_QOS_BAD_OBJECT);
        __A7NLS(WSA_QOS_TRAFFIC_CTRL_ERROR);
        __A7NLS(WSA_QOS_GENERIC_ERROR);
        __A7NLS(WSA_QOS_ESERVICETYPE);
        __A7NLS(WSA_QOS_EFLOWSPEC);
        __A7NLS(WSA_QOS_EPROVSPECBUF);
        __A7NLS(WSA_QOS_EFILTERSTYLE);
        __A7NLS(WSA_QOS_EFILTERTYPE);
        __A7NLS(WSA_QOS_EFILTERCOUNT);
        __A7NLS(WSA_QOS_EOBJLENGTH);
        __A7NLS(WSA_QOS_EFLOWCOUNT);
        __A7NLS(WSA_QOS_EUNKNOWNPSOBJ);
        __A7NLS(WSA_QOS_EPOLICYOBJ);
        __A7NLS(WSA_QOS_EFLOWDESC);
        __A7NLS(WSA_QOS_EPSFLOWSPEC);
        __A7NLS(WSA_QOS_EPSFILTERSPEC);
        __A7NLS(WSA_QOS_ESDMODEOBJ);
        __A7NLS(WSA_QOS_ESHAPERATEOBJ);
        __A7NLS(WSA_QOS_RESERVED_PETYPE);
    }
    return "UNKNOWN";
}

static void A7NetLog_socket ( S7WDA7NET *p, int err, BOOL bv4 ) {
    if ( err == 0 ) {
        A7NetLog ( p, "@I % 8i %s\n", clock() - p -> nClock,
            ( bv4 ? "Socket TCP/IPv4 created." : "Socket TCP/IPv6 created." ) );
        A7NetLog ( p, "    SOCKET = %p\n", ( void* ) p -> iSock );
        return;
    }
    A7NetLog ( p, "@E % 8i %s (%d) %s\n", clock() - p -> nClock,
        ( bv4 ? "Can't create socket TCP/IPv4" : "Can't create socket TCP/IPv6" ), err, A7Str_WSAGetLastError ( err ) );
    switch ( err ) {
        __A7NLT( WSANOTINITIALISED, "A successful WSAStartup call must occur before using this function." );
        __A7NLT( WSAENETDOWN, "The network subsystem or the associated service provider has failed." );
        __A7NLT( WSAEAFNOSUPPORT, "The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer." );
        __A7NLT( WSAEINPROGRESS, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function." );
        __A7NLT( WSAEMFILE, "No more socket descriptors are available." );
        __A7NLT( WSAEINVAL, "An invalid argument was supplied. This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified." );
        __A7NLT( WSAEINVALIDPROVIDER, "The service provider returned a version other than 2.2." );
        __A7NLT( WSAEINVALIDPROCTABLE, "The service provider returned an invalid or incomplete procedure table to the WSPStartup." );
        __A7NLT( WSAENOBUFS, "No buffer space is available. The socket cannot be created." );
        __A7NLT( WSAEPROTONOSUPPORT, "The specified protocol is not supported." );
        __A7NLT( WSAEPROTOTYPE, "The specified protocol is the wrong type for this socket." );
        __A7NLT( WSAEPROVIDERFAILEDINIT, "The service provider failed to initialize. This error is returned if a layered service provider (LSP) or namespace provider was improperly installed or the provider fails to operate correctly." );
        __A7NLT( WSAESOCKTNOSUPPORT, "The specified socket type is not supported in this address family." );
        __A7NLD();
    }
}

static void A7NetLog_connect ( S7WDA7NET *p, int err ) {
    if ( err == 0 ) {
        A7NetLog ( p, "@I % 8i %s\n", clock() - p -> nClock,
            "Socket connected." );
        return;
    }
    A7NetLog ( p, "@%c % 8i %s (%d) %s\n", ( err != WSAEWOULDBLOCK ? 'E' : 'W' ),
        clock() - p -> nClock,
        "Can't connect socket", err, A7Str_WSAGetLastError ( err ) );
    switch ( err ) {
        __A7NLT( WSANOTINITIALISED, "A successful WSAStartup call must occur before using this function." );
        __A7NLT( WSAENETDOWN, "The network subsystem has failed." );
        __A7NLT( WSAEADDRINUSE, "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until the connect function if the bind was to a wildcard address (INADDR_ANY or in6addr_any) for the local IP address. A specific address needs to be implicitly bound by the connect function." );
        __A7NLT( WSAEINTR, "The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall." );
        __A7NLT( WSAEINPROGRESS, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function." );
        __A7NLT2( WSAEALREADY, "A nonblocking connect call is in progress on the specified socket.", "Note: In order to preserve backward compatibility, this error is reported as WSAEINVAL to Windows Sockets 1.1 applications that link to either Winsock.dll or Wsock32.dll." );
        __A7NLT( WSAEADDRNOTAVAIL, "The remote address is not a valid address (such as INADDR_ANY or in6addr_any) ." );
        __A7NLT( WSAEAFNOSUPPORT, "Addresses in the specified family cannot be used with this socket." );
        __A7NLT( WSAECONNREFUSED, "The attempt to connect was forcefully rejected." );
        __A7NLT( WSAEFAULT, "The sockaddr structure pointed to by the name contains incorrect address format for the associated address family or the namelen parameter is too small. This error is also returned if the sockaddr structure pointed to by the name parameter with a length specified in the namelen parameter is not in a valid part of the user address space." );
        __A7NLT( WSAEINVAL, "The parameter s is a listening socket." );
        __A7NLT( WSAEISCONN, "The socket is already connected (connection-oriented sockets only)." );
        __A7NLT( WSAENETUNREACH, "The network cannot be reached from this host at this time." );
        __A7NLT( WSAEHOSTUNREACH, "A socket operation was attempted to an unreachable host." );
        __A7NLT( WSAENOBUFS, "Note: No buffer space is available. The socket cannot be connected." );
        __A7NLT( WSAENOTSOCK, "The descriptor specified in the s parameter is not a socket." );
        __A7NLT( WSAETIMEDOUT, "An attempt to connect timed out without establishing a connection." );
        __A7NLT( WSAEWOULDBLOCK, "The socket is marked as nonblocking and the connection cannot be completed immediately." );
        __A7NLT( WSAEACCES, "An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled." );
        __A7NLD();
    }
}

static void A7NetLog_WSAAsyncSelect ( S7WDA7NET *p, int err ) {
    if ( err == 0 ) {
        A7NetLog ( p, "@I % 8i %s\n", clock() - p -> nClock,
                "Socket select set to async." );
        return;
    }
    A7NetLog ( p, "@E % 8i %s (%d) %s\n", clock() - p -> nClock,
        "Can't set socket select to async", err, A7Str_WSAGetLastError ( err ) );
    switch ( err ) {
        __A7NLT( WSANOTINITIALISED, "A successful WSAStartup call must occur before using this function." );
        __A7NLT( WSAENETDOWN, "The network subsystem failed." );
        __A7NLT( WSAEINVAL, "One of the specified parameters was invalid, such as the window handle not referring to an existing window, or the specified socket is in an invalid state." );
        __A7NLT( WSAEINPROGRESS, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function." );
        __A7NLT( WSAENOTSOCK, "The descriptor is not a socket." );
        __A7NLD();
    }
}

static void A7NetLog_WSAAsyncSelect_WM ( S7WDA7NET *p, int err, int etp ) {
    if ( err == 0 ) {
        A7NetLog ( p, "@I % 8i %s\n", clock() - p -> nClock,
                "New async event." );
        switch ( etp ) {
            __A7NLF( FD_READ, "Readiness for reading." );
            __A7NLF( FD_WRITE, "Readiness for writing." );
            __A7NLF( FD_OOB, "The arrival of OOB data." );
            __A7NLF( FD_ACCEPT, "Incoming connections." );
            __A7NLF( FD_CONNECT, "Completed connection or multipoint join operation." );
            __A7NLF( FD_CLOSE, "Socket closure." );
            __A7NLF( FD_QOS, "Socket Quality of Service (QoS) changes." );
            __A7NLF( FD_GROUP_QOS, "Socket group Quality of Service (QoS) changes (reserved for future use with socket groups). Reserved." );
            __A7NLF( FD_ROUTING_INTERFACE_CHANGE, "Routing interface changes for the specified destination(s)." );
            __A7NLF( FD_ADDRESS_LIST_CHANGE, "Local address list changes for the socket protocol family." );
            __A7NLE( "Unknown event." );
        }
        return;
    }
    A7NetLog ( p, "@E % 8i %s (%d) %s\n", clock() - p -> nClock,
        "Async event with error", err, A7Str_WSAGetLastError ( err ) );
    switch ( etp ) {
        __A7NLF( FD_READ, "Readiness for reading." );
        __A7NLF( FD_WRITE, "Readiness for writing." );
        __A7NLF( FD_OOB, "The arrival of OOB data." );
        __A7NLF( FD_ACCEPT, "Incoming connections." );
        __A7NLF( FD_CONNECT, "Completed connection or multipoint join operation." );
        __A7NLF( FD_CLOSE, "Socket closure." );
        __A7NLF( FD_QOS, "Socket Quality of Service (QoS) changes." );
        __A7NLF( FD_GROUP_QOS, "Socket group Quality of Service (QoS) changes (reserved for future use with socket groups). Reserved." );
        __A7NLF( FD_ROUTING_INTERFACE_CHANGE, "Routing interface changes for the specified destination(s)." );
        __A7NLF( FD_ADDRESS_LIST_CHANGE, "Local address list changes for the socket protocol family." );
        __A7NLE( "Unknown event." );
    }
    if ( etp == FD_CONNECT )
    switch ( err ) {
        __A7NLT( WSAEAFNOSUPPORT, "Addresses in the specified family cannot be used with this socket." );
        __A7NLT( WSAECONNREFUSED, "The attempt to connect was rejected." );
        __A7NLT( WSAENETUNREACH, "The network cannot be reached from this host at this time." );
        __A7NLT( WSAEFAULT, "The namelen parameter is invalid." );
        __A7NLT( WSAEINVAL, "The socket is already bound to an address." );
        __A7NLT( WSAEISCONN, "The socket is already connected." );
        __A7NLT( WSAEMFILE, "No more file descriptors are available." );
        __A7NLT( WSAENOBUFS, "No buffer space is available. The socket cannot be connected." );
        __A7NLT( WSAENOTCONN, "The socket is not connected." );
        __A7NLT( WSAETIMEDOUT, "Attempt to connect timed out without establishing a connection." );
        __A7NLD();
    }

    if ( etp == FD_CLOSE )
    switch ( err ) {
        __A7NLT( WSAENETDOWN, "The network subsystem failed." );
        __A7NLT( WSAECONNRESET, "The connection was reset by the remote side." );
        __A7NLT( WSAECONNABORTED, "The connection was terminated due to a time-out or other failure." );
        __A7NLD();
    }

    if ( etp == FD_ROUTING_INTERFACE_CHANGE )
    switch ( err ) {
        __A7NLT( WSAENETUNREACH, "The specified destination is no longer reachable." );
        __A7NLT( WSAENETDOWN, "The network subsystem failed." );
        __A7NLD();
    }
}
