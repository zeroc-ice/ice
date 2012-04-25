// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// The following is required on HP-UX in order to bring in
// the definition for the ip_mreq structure.
//
#if defined(__hpux)
#  undef _XOPEN_SOURCE_EXTENDED
#  define _XOPEN_SOURCE
#  include <netinet/in.h>
#endif

//
// The following is required for the Vista PSDK to bring in
// the definitions of the IN6_IS_ADDR_* macros.
//
#if defined(_WIN32) && !defined(_WIN32_WINNT)
#       define _WIN32_WINNT 0x0501
#endif

#include <IceUtil/StringUtil.h>
#include <IceUtil/Unicode.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h> // For setTcpBufSize
#include <Ice/LoggerUtil.h> // For setTcpBufSize

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <iphlpapi.h>
#  include <Mswsock.h>
#else
#  include <net/if.h>
#  include <sys/ioctl.h>
#endif

#if defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
#  include <ifaddrs.h>
#elif defined(__sun)
#  include <sys/sockio.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

#if defined(__sun) && !defined(__GNUC__)
#    define INADDR_NONE (in_addr_t)0xffffffff
#endif

namespace
{

vector<struct sockaddr_storage>
getLocalAddresses(ProtocolSupport protocol)
{
    vector<struct sockaddr_storage> result;

#if defined(_WIN32)
    try
    {
        for(int i = 0; i < 2; i++)
        {
            if((i == 0 && protocol == EnableIPv6) || (i == 1 && protocol == EnableIPv4))
            {
                continue;
            }

            SOCKET fd = createSocket(false, i == 0 ? AF_INET : AF_INET6);

            vector<unsigned char> buffer;
            buffer.resize(1024);
            unsigned long len = 0;
            int rs = WSAIoctl(fd, SIO_ADDRESS_LIST_QUERY, 0, 0,
                                &buffer[0], static_cast<DWORD>(buffer.size()),
                                &len, 0, 0);
            if(rs == SOCKET_ERROR)
            {
                //
                // If the buffer wasn't big enough, resize it to the
                // required length and try again.
                //
                if(getSocketErrno() == WSAEFAULT)
                {
                    buffer.resize(len);
                    rs = WSAIoctl(fd, SIO_ADDRESS_LIST_QUERY, 0, 0,
                                  &buffer[0], static_cast<DWORD>(buffer.size()),
                                  &len, 0, 0);
                }

                if(rs == SOCKET_ERROR)
                {
                    closeSocketNoThrow(fd);
                    SocketException ex(__FILE__, __LINE__);
                    ex.error = getSocketErrno();
                    throw ex;
                }
            }

            //
            // Add the local interface addresses.
            //
            SOCKET_ADDRESS_LIST* addrs = reinterpret_cast<SOCKET_ADDRESS_LIST*>(&buffer[0]);
            for (int i = 0; i < addrs->iAddressCount; ++i)
            {
                sockaddr_storage addr;
                memcpy(&addr, addrs->Address[i].lpSockaddr, addrs->Address[i].iSockaddrLength);
                if(addr.ss_family == AF_INET && protocol != EnableIPv6)
                {
                    if(reinterpret_cast<struct sockaddr_in*>(&addr)->sin_addr.s_addr != 0)
                    {
                        result.push_back(addr);
                    }
                }
                else if(addr.ss_family == AF_INET6 && protocol != EnableIPv4)
                {
                    struct in6_addr* inaddr6 = &reinterpret_cast<struct sockaddr_in6*>(&addr)->sin6_addr;
                    if(!IN6_IS_ADDR_UNSPECIFIED(inaddr6) && !IN6_IS_ADDR_LOOPBACK(inaddr6))
                    {
                        result.push_back(addr);
                    }
                }
            }

            closeSocket(fd);
        }
    }
    catch(const Ice::LocalException&)
    {
        //
        // TODO: Warning?
        //
    }
#elif defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
    struct ifaddrs* ifap;
    if(::getifaddrs(&ifap) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    struct ifaddrs* curr = ifap;
    while(curr != 0)
    {
        if(curr->ifa_addr && !(curr->ifa_flags & IFF_LOOPBACK))  // Don't include loopback interface addresses
        {
            if(curr->ifa_addr->sa_family == AF_INET && protocol != EnableIPv6)
            {
                sockaddr_storage addr;
                memcpy(&addr, curr->ifa_addr, sizeof(sockaddr_in));
                if(reinterpret_cast<struct sockaddr_in*>(&addr)->sin_addr.s_addr != 0)
                {
                    result.push_back(addr);
                }
            }
            else if(curr->ifa_addr->sa_family == AF_INET6 && protocol != EnableIPv4)
            {
                sockaddr_storage addr;
                memcpy(&addr, curr->ifa_addr, sizeof(sockaddr_in6));
                if(!IN6_IS_ADDR_UNSPECIFIED(&reinterpret_cast<struct sockaddr_in6*>(&addr)->sin6_addr))
                {
                    result.push_back(*reinterpret_cast<struct sockaddr_storage*>(curr->ifa_addr));
                }
            }
        }

        curr = curr->ifa_next;
    }

    ::freeifaddrs(ifap);
#else
    for(int i = 0; i < 2; i++)
    {
        if((i == 0 && protocol == EnableIPv6) || (i == 1 && protocol == EnableIPv4))
        {
            continue;
        }
        SOCKET fd = createSocket(false, i == 0 ? AF_INET : AF_INET6);

#ifdef _AIX
        int cmd = CSIOCGIFCONF;
#else
        int cmd = SIOCGIFCONF;
#endif
        struct ifconf ifc;
        int numaddrs = 10;
        int old_ifc_len = 0;

        //
        // Need to call ioctl multiple times since we do not know up front
        // how many addresses there will be, and thus how large a buffer we need.
        // We keep increasing the buffer size until subsequent calls return
        // the same length, meaning we have all the addresses.
        //
        while(true)
        {
            int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
            ifc.ifc_len = bufsize;
            ifc.ifc_buf = (char*)malloc(bufsize);

            int rs = ioctl(fd, cmd, &ifc);
            if(rs == SOCKET_ERROR)
            {
                free(ifc.ifc_buf);
                closeSocketNoThrow(fd);
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else if(ifc.ifc_len == old_ifc_len)
            {
                //
                // Returned same length twice in a row, finished.
                //
                break;
            }
            else
            {
                old_ifc_len = ifc.ifc_len;
            }

            numaddrs += 10;
            free(ifc.ifc_buf);
        }
        closeSocket(fd);

        numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
        struct ifreq* ifr = ifc.ifc_req;
        for(int i = 0; i < numaddrs; ++i)
        {
            if(!(ifr[i].ifr_flags & IFF_LOOPBACK)) // Don't include loopback interface addresses
            {
		//
		// On Solaris the above Loopback check does not always work so we double 
		// check the address below. Solaris also returns duplicate entries that need
		// to be filtered out.
		//
                if(ifr[i].ifr_addr.sa_family == AF_INET && protocol != EnableIPv6)
                {
                    sockaddr_storage addr;
                    memcpy(&addr, &ifr[i].ifr_addr, sizeof(sockaddr_in));
                    struct in_addr* inaddr = &reinterpret_cast<struct sockaddr_in*>(&addr)->sin_addr;
                    if(inaddr->s_addr != 0 && inaddr->s_addr != htonl(INADDR_LOOPBACK))
                    {
			unsigned int j;
			for(j = 0; j < result.size(); ++j)
			{
			   if(compareAddress(addr, result[j]) == 0)
			   {
			       break;
			   }
			}
			if(j == result.size())
			{
                            result.push_back(addr);
			}
                    }
                }
                else if(ifr[i].ifr_addr.sa_family == AF_INET6 && protocol != EnableIPv4)
                {
                    sockaddr_storage addr;
                    memcpy(&addr, &ifr[i].ifr_addr, sizeof(sockaddr_in6));
                    struct in6_addr* inaddr6 = &reinterpret_cast<struct sockaddr_in6*>(&addr)->sin6_addr;
                    if(!IN6_IS_ADDR_UNSPECIFIED(inaddr6) && !IN6_IS_ADDR_LOOPBACK(inaddr6))
                    {
			unsigned int j;
			for(j = 0; j < result.size(); ++j)
			{
			   if(compareAddress(addr, result[j]) == 0)
			   {
			       break;
			   }
			}
			if(j == result.size())
			{
                            result.push_back(addr);
			}
                    }
                }
            }
        }
        free(ifc.ifc_buf);
    }
#endif

    return result;
}

struct sockaddr_storage
getAddressImpl(const string& host, int port, ProtocolSupport protocol, bool server)
{
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(struct sockaddr_storage));

    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if(host.empty())
    {
        if(protocol == EnableIPv6)
        {
            sockaddr_in6* addrin6 = reinterpret_cast<sockaddr_in6*>(&addr);
            addrin6->sin6_family = AF_INET6;
            addrin6->sin6_port = htons(port);
            addrin6->sin6_addr = server ? in6addr_any : in6addr_loopback;
        }
        else
        {
            sockaddr_in* addrin = reinterpret_cast<sockaddr_in*>(&addr);
            addrin->sin_family = AF_INET;
            addrin->sin_port = htons(port);
            addrin->sin_addr.s_addr = server ? htonl(INADDR_ANY) : htonl(INADDR_LOOPBACK);
        }
        return addr;
    }

    struct addrinfo* info = 0;
    int retry = 5;

    struct addrinfo hints = { 0 };

    if(server)
    {
        //
        // If host is empty, getaddrinfo will return the wildcard
        // address instead of the loopack address.
        //
        hints.ai_flags |= AI_PASSIVE;
    }

    if(protocol == EnableIPv4)
    {
        hints.ai_family = PF_INET;
    }
    else if(protocol == EnableIPv6)
    {
        hints.ai_family = PF_INET6;
    }
    else
    {
        hints.ai_family = PF_UNSPEC;
    }

    int rs = 0;
    do
    {
        rs = getaddrinfo(host.c_str(), 0, &hints, &info);
    }
    while(info == 0 && rs == EAI_AGAIN && --retry >= 0);

    if(rs != 0)
    {
        DNSException ex(__FILE__, __LINE__);
        ex.error = rs;
        ex.host = host;
        throw ex;
    }

    memcpy(&addr, info->ai_addr, info->ai_addrlen);
    if(info->ai_family == PF_INET)
    {
        reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(port);
    }
    else if(info->ai_family == PF_INET6)
    {
        reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port = htons(port);
    }
    else // Unknown address family.
    {
        freeaddrinfo(info);
        DNSException ex(__FILE__, __LINE__);
        ex.host = host;
        throw ex;
    }
    freeaddrinfo(info);
    return addr;
}

bool
isWildcard(const string& host, ProtocolSupport protocol)
{
    try
    {
        sockaddr_storage addr = getAddressImpl(host, 0, protocol, false);
        if(addr.ss_family == AF_INET)
        {
            struct sockaddr_in* addrin = reinterpret_cast<sockaddr_in*>(&addr);
            if(addrin->sin_addr.s_addr == INADDR_ANY)
            {
                return true;
            }
        }
        else if(addr.ss_family)
        {
            struct sockaddr_in6* addrin6 = reinterpret_cast<sockaddr_in6*>(&addr);
            if(IN6_IS_ADDR_UNSPECIFIED(&addrin6->sin6_addr))
            {
                return true;
            }
        }
    }
    catch(const DNSException&)
    {
    }
    return false;
}

int
getInterfaceIndex(const string& name)
{
    int index = 0;
#ifdef _WIN32
    IP_ADAPTER_ADDRESSES addrs;
    ULONG buflen = 0;
    if(::GetAdaptersAddresses(AF_INET6, 0, 0, &addrs, &buflen) == ERROR_BUFFER_OVERFLOW)
    {
        PIP_ADAPTER_ADDRESSES paddrs;
        char* buf = new char[buflen];
        paddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buf);
        if(::GetAdaptersAddresses(AF_INET6, 0, 0, paddrs, &buflen) == NO_ERROR)
        {
            while(paddrs)
            {
                if(IceUtil::wstringToString(paddrs->FriendlyName) == name)
                {
                    index = paddrs->Ipv6IfIndex;
                    break;
                }
                paddrs = paddrs->Next;
            }
        }
        delete[] buf;
    }
#elif !defined(__hpux)
    index = if_nametoindex(name.c_str());
#endif
    return index;
}

struct in_addr
getInterfaceAddress(const string& name)
{
    struct in_addr addr;
    addr.s_addr = INADDR_ANY;
#ifdef _WIN32
    IP_ADAPTER_ADDRESSES addrs;
    ULONG buflen = 0;
    if(::GetAdaptersAddresses(AF_INET, 0, 0, &addrs, &buflen) == ERROR_BUFFER_OVERFLOW)
    {
        PIP_ADAPTER_ADDRESSES paddrs;
        char* buf = new char[buflen];
        paddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buf);
        if(::GetAdaptersAddresses(AF_INET, 0, 0, paddrs, &buflen) == NO_ERROR)
        {
            while(paddrs)
            {
                if(IceUtil::wstringToString(paddrs->FriendlyName) == name)
                {
                    struct sockaddr_in addrin;
                    memcpy(&addrin, paddrs->FirstUnicastAddress->Address.lpSockaddr,
                           paddrs->FirstUnicastAddress->Address.iSockaddrLength);
                    addr =  addrin.sin_addr;
                    break;
                }
                paddrs = paddrs->Next;
            }
        }
        delete[] buf;
    }
#else
    ifreq if_address;
    strcpy(if_address.ifr_name, name.c_str());

    SOCKET fd = createSocket(false, AF_INET);
    int rc = ioctl(fd, SIOCGIFADDR, &if_address);
    closeSocketNoThrow(fd);

    if(rc != SOCKET_ERROR)
    {
        addr = reinterpret_cast<struct sockaddr_in*>(&if_address.ifr_addr)->sin_addr;
    }
#endif

    return addr;
}

}

#ifdef ICE_USE_IOCP
IceInternal::AsyncInfo::AsyncInfo(SocketOperation s)
{
    ZeroMemory(this, sizeof(AsyncInfo));
    status = s;
}
#endif

int
IceInternal::getSocketErrno()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool
IceInternal::interrupted()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEINTR;
#else
#   ifdef EPROTO
    return errno == EINTR || errno == EPROTO;
#   else
    return errno == EINTR;
#   endif
#endif
}

bool
IceInternal::acceptInterrupted()
{
    if(interrupted())
    {
        return true;
    }

#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNABORTED ||
           error == WSAECONNRESET ||
           error == WSAETIMEDOUT;
#else
    return errno == ECONNABORTED ||
           errno == ECONNRESET ||
           errno == ETIMEDOUT;
#endif
}

bool
IceInternal::noBuffers()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAENOBUFS ||
           error == WSAEFAULT;
#else
    return errno == ENOBUFS;
#endif
}

bool
IceInternal::wouldBlock()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

bool
IceInternal::connectFailed()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED ||
           error == WSAETIMEDOUT ||
           error == WSAENETUNREACH ||
           error == WSAEHOSTUNREACH ||
           error == WSAECONNRESET ||
           error == WSAESHUTDOWN ||
           error == WSAECONNABORTED;
#else
    return errno == ECONNREFUSED ||
           errno == ETIMEDOUT ||
           errno == ENETUNREACH ||
           errno == EHOSTUNREACH ||
           errno == ECONNRESET ||
           errno == ESHUTDOWN ||
           errno == ECONNABORTED;
#endif
}

bool
IceInternal::connectionRefused()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED || error == ERROR_CONNECTION_REFUSED;
#else
    return errno == ECONNREFUSED;
#endif
}

bool
IceInternal::connectInProgress()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
#else
    return errno == EINPROGRESS;
#endif
}

bool
IceInternal::connectionLost()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNRESET ||
           error == WSAESHUTDOWN ||
           error == WSAENOTCONN ||
#ifdef ICE_USE_IOCP
           error == ERROR_NETNAME_DELETED ||
#endif
           error == WSAECONNABORTED;
#else
    return errno == ECONNRESET ||
           errno == ENOTCONN ||
           errno == ESHUTDOWN ||
           errno == ECONNABORTED ||
           errno == EPIPE;
#endif
}

bool
IceInternal::notConnected()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAENOTCONN;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    return errno == ENOTCONN || errno == EINVAL;
#else
    return errno == ENOTCONN;
#endif
}

bool
IceInternal::recvTruncated()
{
#ifdef _WIN32
    int err = WSAGetLastError();
    return  err == WSAEMSGSIZE || err == ERROR_MORE_DATA;
#else
    // We don't get an error under Linux if a datagram is truncated.
    return false;
#endif
}

bool
IceInternal::noMoreFds(int error)
{
#ifdef _WIN32
    return error == WSAEMFILE;
#else
    return error == EMFILE || error == ENFILE;
#endif
}

SOCKET
IceInternal::createSocket(bool udp, int family)
{
    SOCKET fd;

    if(udp)
    {
        fd = socket(family, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
    }

    if(fd == INVALID_SOCKET)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    if(!udp)
    {
        setTcpNoDelay(fd);
        setKeepAlive(fd);
    }

    return fd;
}

void
IceInternal::closeSocket(SOCKET fd)
{
#ifdef _WIN32
    int error = WSAGetLastError();
    if(closesocket(fd) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    WSASetLastError(error);
#else
    int error = errno;
    if(close(fd) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    errno = error;
#endif
}

void
IceInternal::closeSocketNoThrow(SOCKET fd)
{
#ifdef _WIN32
    int error = WSAGetLastError();
    closesocket(fd);
    WSASetLastError(error);
#else
    int error = errno;
    close(fd);
    errno = error;
#endif
}

void
IceInternal::shutdownSocketWrite(SOCKET fd)
{
    if(shutdown(fd, SHUT_WR) == SOCKET_ERROR)
    {
        //
        // Ignore errors indicating that we are shutdown already.
        //
#if defined(_WIN32)
        int error = WSAGetLastError();
        //
        // Under Vista its possible to get a WSAECONNRESET. See
        // http://bugzilla.zeroc.com/bugzilla/show_bug.cgi?id=1739 for
        // some details.
        //
        if(error == WSAENOTCONN || error == WSAECONNRESET)
        {
            return;
        }
#elif defined(__APPLE__) || defined(__FreeBSD__)
        if(errno == ENOTCONN || errno == EINVAL)
        {
            return;
        }
#else
        if(errno == ENOTCONN)
        {
            return;
        }
#endif
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::shutdownSocketReadWrite(SOCKET fd)
{
    if(shutdown(fd, SHUT_RDWR) == SOCKET_ERROR)
    {
        //
        // Ignore errors indicating that we are shutdown already.
        //
#if defined(_WIN32)
        int error = WSAGetLastError();
        //
        // Under Vista its possible to get a WSAECONNRESET. See
        // http://bugzilla.zeroc.com/bugzilla/show_bug.cgi?id=1739 for
        // some details.
        //
        if(error == WSAENOTCONN || error == WSAECONNRESET)
        {
            return;
        }
#elif defined(__APPLE__) || defined(__FreeBSD__)
        if(errno == ENOTCONN || errno == EINVAL)
        {
            return;
        }
#else
        if(errno == ENOTCONN)
        {
            return;
        }
#endif

        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setBlock(SOCKET fd, bool block)
{
    if(block)
    {
#ifdef _WIN32
        unsigned long arg = 0;
        if(ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = WSAGetLastError();
            throw ex;
        }
#else
        int flags = fcntl(fd, F_GETFL);
        flags &= ~O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = errno;
            throw ex;
        }
#endif
    }
    else
    {
#ifdef _WIN32
        unsigned long arg = 1;
        if(ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = WSAGetLastError();
            throw ex;
        }
#else
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = errno;
            throw ex;
        }
#endif
    }
}

void
IceInternal::setTcpNoDelay(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setKeepAlive(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setSendBufferSize(SOCKET fd, int sz)
{
    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

int
IceInternal::getSendBufferSize(SOCKET fd)
{
    int sz;
    socklen_t len = sizeof(sz);
    if(getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, &len) == SOCKET_ERROR || 
       static_cast<unsigned int>(len) != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    return sz;
}

void
IceInternal::setRecvBufferSize(SOCKET fd, int sz)
{
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

int
IceInternal::getRecvBufferSize(SOCKET fd)
{
    int sz;
    socklen_t len = sizeof(sz);
    if(getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, &len) == SOCKET_ERROR || 
       static_cast<unsigned int>(len) != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    return sz;
}

void
IceInternal::setMcastGroup(SOCKET fd, const struct sockaddr_storage& group, const string& interface)
{
    int rc;
    if(group.ss_family == AF_INET)
    {
        struct ip_mreq mreq;
        mreq.imr_multiaddr = reinterpret_cast<const struct sockaddr_in*>(&group)->sin_addr;
        mreq.imr_interface.s_addr = INADDR_ANY;
        if(interface.size() > 0)
        {
            //
            // First see if it is the interface name. If not check if IP Address.
            //
            mreq.imr_interface = getInterfaceAddress(interface);
            if(mreq.imr_interface.s_addr == INADDR_ANY)
            {
                struct sockaddr_storage addr = getAddressForServer(interface, 0, EnableIPv4);
                mreq.imr_interface = reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_addr;
            }
        }
        rc = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, int(sizeof(mreq)));
    }
    else
    {
        struct ipv6_mreq mreq;
        mreq.ipv6mr_multiaddr = reinterpret_cast<const struct sockaddr_in6*>(&group)->sin6_addr;
        mreq.ipv6mr_interface = 0;
        if(interface.size() != 0)
        {
            //
            // First check if it is the interface name. If not check if index.
            //
            mreq.ipv6mr_interface = getInterfaceIndex(interface);
            if(mreq.ipv6mr_interface == 0)
            {
                istringstream p(interface);
                if(!(p >> mreq.ipv6mr_interface) || !p.eof())
                {
                    closeSocketNoThrow(fd);
                    SocketException ex(__FILE__, __LINE__);
                    ex.error = 0;
                    throw ex;
                }
            }
        }
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq, int(sizeof(mreq)));
    }
    if(rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setMcastInterface(SOCKET fd, const string& interface, bool IPv4)
{
    int rc;
    if(IPv4)
    {
        //
        // First see if it is the interface name. If not check if IP Address.
        //
        struct in_addr iface = getInterfaceAddress(interface);
        if(iface.s_addr == INADDR_ANY)
        {
            struct sockaddr_storage addr = getAddressForServer(interface, 0, EnableIPv4);
            iface = reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_addr;
        }
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&iface, int(sizeof(iface)));
    }
    else
    {
        //
        // First check if it is the interface name. If not check if index.
        //
        int interfaceNum = getInterfaceIndex(interface);
        if(interfaceNum == 0)
        {
            istringstream p(interface);
            if(!(p >> interfaceNum) || !p.eof())
            {
                closeSocketNoThrow(fd);
                SocketException ex(__FILE__, __LINE__);
                ex.error = 0;
            }
        }
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char*)&interfaceNum, int(sizeof(int)));
    }
    if(rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setMcastTtl(SOCKET fd, int ttl, bool IPv4)
{
    int rc;
    if(IPv4)
    {
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, int(sizeof(int)));
    }
    else
    {
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char*)&ttl, int(sizeof(int)));
    }
    if(rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setReuseAddress(SOCKET fd, bool reuse)
{
    int flag = reuse ? 1 : 0;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

struct sockaddr_storage
IceInternal::doBind(SOCKET fd, const struct sockaddr_storage& addr)
{
    int size;
    if(addr.ss_family == AF_INET)
    {
        size = sizeof(sockaddr_in);
    }
    else if(addr.ss_family == AF_INET6)
    {
        size = sizeof(sockaddr_in6);
    }
    else
    {
        assert(false);
        size = 0; // Keep the compiler happy.
    }

    if(bind(fd, reinterpret_cast<const struct sockaddr*>(&addr), size) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    struct sockaddr_storage local;
    socklen_t len = static_cast<socklen_t>(sizeof(local));
#ifdef NDEBUG
    getsockname(fd, reinterpret_cast<struct sockaddr*>(&local), &len);
#else
    int ret = getsockname(fd, reinterpret_cast<struct sockaddr*>(&local), &len);
    assert(ret != SOCKET_ERROR);
#endif
    return local;
}

void
IceInternal::doListen(SOCKET fd, int backlog)
{
repeatListen:
    if(::listen(fd, backlog) == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeatListen;
        }

        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

bool
IceInternal::doConnect(SOCKET fd, const struct sockaddr_storage& addr)
{
repeatConnect:
    int size;
    if(addr.ss_family == AF_INET)
    {
        size = sizeof(sockaddr_in);
    }
    else if(addr.ss_family == AF_INET6)
    {
        size = sizeof(sockaddr_in6);
    }
    else
    {
        assert(false);
        size = 0; // Keep the compiler happy.
    }

    if(::connect(fd, reinterpret_cast<const struct sockaddr*>(&addr), size) == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeatConnect;
        }

        if(connectInProgress())
        {
            return false;
        }

        closeSocketNoThrow(fd);
        if(connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

#if defined(__linux)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    struct sockaddr_storage localAddr;
    fdToLocalAddress(fd, localAddr);
    if(compareAddress(addr, localAddr) == 0)
    {
        ConnectionRefusedException ex(__FILE__, __LINE__);
        ex.error = 0; // No appropriate errno
        throw ex;
    }
#endif
    return true;
}

void
IceInternal::doFinishConnect(SOCKET fd)
{
    //
    // Note: we don't close the socket if there's an exception. It's the responsability
    // of the caller to do so.
    //

    //
    // Strange windows bug: The following call to Sleep() is
    // necessary, otherwise no error is reported through
    // getsockopt.
    //
#ifdef _WIN32
    Sleep(0);
#endif

    int val;
    socklen_t len = static_cast<socklen_t>(sizeof(int));
    if(getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    if(val > 0)
    {
#ifdef _WIN32
        WSASetLastError(val);
#else
        errno = val;
#endif
        if(connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

#if defined(__linux)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    struct sockaddr_storage localAddr;
    fdToLocalAddress(fd, localAddr);
    struct sockaddr_storage remoteAddr;
    if(fdToRemoteAddress(fd, remoteAddr) && compareAddress(remoteAddr, localAddr) == 0)
    {
        ConnectionRefusedException ex(__FILE__, __LINE__);
        ex.error = 0; // No appropriate errno
        throw ex;
    }
#endif
}

#ifdef ICE_USE_IOCP
void
IceInternal::doConnectAsync(SOCKET fd, const struct sockaddr_storage& addr, AsyncInfo& info)
{
    //
    // NOTE: It's the caller's responsability to close the socket upon
    // failure to connect. The socket isn't closed by this method.
    //

    struct sockaddr_storage bindAddr;
    memset(&bindAddr, 0, sizeof(bindAddr));

    int size;
    if(addr.ss_family == AF_INET)
    {
        size = sizeof(sockaddr_in);

        struct sockaddr_in* addrin = reinterpret_cast<struct sockaddr_in*>(&bindAddr);
        addrin->sin_family = AF_INET;
        addrin->sin_port = htons(0);
        addrin->sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if(addr.ss_family == AF_INET6)
    {
        size = sizeof(sockaddr_in6);

        struct sockaddr_in6* addrin = reinterpret_cast<struct sockaddr_in6*>(&bindAddr);
        addrin->sin6_family = AF_INET6;
        addrin->sin6_port = htons(0);
        addrin->sin6_addr = in6addr_any;
    }
    else
    {
        assert(false);
        size = 0; // Keep the compiler happy.
    }

    if(bind(fd, reinterpret_cast<const struct sockaddr*>(&bindAddr), size) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    LPFN_CONNECTEX ConnectEx = NULL; // a pointer to the 'ConnectEx()' function
    GUID GuidConnectEx = WSAID_CONNECTEX; // The Guid
    DWORD dwBytes;
    if(WSAIoctl(fd, 
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidConnectEx,
                sizeof(GuidConnectEx),
                &ConnectEx,
                sizeof(ConnectEx),
                &dwBytes,
                NULL, 
                NULL) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }        

    if(!ConnectEx(fd, reinterpret_cast<const struct sockaddr*>(&addr), size, 0, 0, 0, 
#if defined(_MSC_VER) && (_MSC_VER < 1300) // COMPILER FIX: VC60
                  reinterpret_cast<LPOVERLAPPED>(&info)
#else
                  &info
#endif
                  ))
    {
        if(!connectInProgress())
        {
            if(connectionRefused())
            {
                ConnectionRefusedException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else if(connectFailed())
            {
                ConnectFailedException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }
    }

}

void
IceInternal::doFinishConnectAsync(SOCKET fd, AsyncInfo& info)
{
    //
    // NOTE: It's the caller's responsability to close the socket upon
    // failure to connect. The socket isn't closed by this method.
    //

    if(info.count == SOCKET_ERROR)
    {
        WSASetLastError(info.error);
        if(connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

    if(setsockopt(fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}
#endif

SOCKET
IceInternal::doAccept(SOCKET fd)
{
#ifdef _WIN32
    SOCKET ret;
#else
    int ret;
#endif

repeatAccept:
    if((ret = ::accept(fd, 0, 0)) == INVALID_SOCKET)
    {
        if(acceptInterrupted())
        {
            goto repeatAccept;
        }

        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    setTcpNoDelay(ret);
    setKeepAlive(ret);
    return ret;
}

struct sockaddr_storage
IceInternal::getAddressForServer(const string& host, int port, ProtocolSupport protocol)
{
    return getAddressImpl(host, port, protocol, true);
}

struct sockaddr_storage
IceInternal::getAddress(const string& host, int port, ProtocolSupport protocol)
{
    return getAddressImpl(host, port, protocol, false);
}

vector<struct sockaddr_storage>
IceInternal::getAddresses(const string& host, int port, ProtocolSupport protocol, bool blocking)
{
    vector<struct sockaddr_storage> result;
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(struct sockaddr_storage));

    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if(host.empty())
    {
        if(protocol != EnableIPv4)
        {
            sockaddr_in6* addrin6 = reinterpret_cast<sockaddr_in6*>(&addr);
            addrin6->sin6_family = AF_INET6;
            addrin6->sin6_port = htons(port);
            addrin6->sin6_addr = in6addr_loopback;
            result.push_back(addr);
        }
        if(protocol != EnableIPv6)
        {
            sockaddr_in* addrin = reinterpret_cast<sockaddr_in*>(&addr);
            addrin->sin_family = AF_INET;
            addrin->sin_port = htons(port);
            addrin->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            result.push_back(addr);
        }
        return result;
    }

    struct addrinfo* info = 0;
    int retry = 5;

    struct addrinfo hints = { 0 };
    if(protocol == EnableIPv4)
    {
        hints.ai_family = PF_INET;
    }
    else if(protocol == EnableIPv6)
    {
        hints.ai_family = PF_INET6;
    }
    else
    {
        hints.ai_family = PF_UNSPEC;
    }

    if(!blocking)
    {
        hints.ai_flags = AI_NUMERICHOST;
    }

    int rs = 0;
    do
    {
        rs = getaddrinfo(host.c_str(), 0, &hints, &info);
    }
    while(info == 0 && rs == EAI_AGAIN && --retry >= 0);

    // In theory, getaddrinfo should only return EAI_NONAME if AI_NUMERICHOST is specified and the host name
    // is not a IP address. However on some platforms (e.g. Mac OS X 10.4.x) EAI_NODATA is also returned so 
    // we also check for it.
#ifdef EAI_NODATA
    if(!blocking && (rs == EAI_NONAME || rs == EAI_NODATA))
#else
    if(!blocking && rs == EAI_NONAME)
#endif
    {
        return result; // Empty result indicates that a blocking lookup is necessary.
    }
    else if(rs != 0)
    {
        DNSException ex(__FILE__, __LINE__);
        ex.error = rs;
        ex.host = host;
        throw ex;
    }

    struct addrinfo* p;
    for(p = info; p != NULL; p = p->ai_next)
    {
        memcpy(&addr, p->ai_addr, p->ai_addrlen);
        if(p->ai_family == PF_INET)
        {
            struct sockaddr_in* addrin = reinterpret_cast<sockaddr_in*>(&addr);
            addrin->sin_port = htons(port);
        }
        else if(p->ai_family == PF_INET6)
        {
            struct sockaddr_in6* addrin6 = reinterpret_cast<sockaddr_in6*>(&addr);
            addrin6->sin6_port = htons(port);
        }

        bool found = false;
        for(unsigned int i = 0; i < result.size(); ++i)
        {
            if(compareAddress(result[i], addr) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            result.push_back(addr);
        }
    }

    freeaddrinfo(info);

    if(result.size() == 0)
    {
        DNSException ex(__FILE__, __LINE__);
        ex.host = host;
        throw ex;
    }

    return result;
}

int
IceInternal::compareAddress(const struct sockaddr_storage& addr1, const struct sockaddr_storage& addr2)
{
    if(addr1.ss_family < addr2.ss_family)
    {
        return -1;
    }
    else if(addr2.ss_family < addr1.ss_family)
    {
        return 1;
    }

    if(addr1.ss_family == AF_INET)
    {
        const struct sockaddr_in* addr1in = reinterpret_cast<const sockaddr_in*>(&addr1);
        const struct sockaddr_in* addr2in = reinterpret_cast<const sockaddr_in*>(&addr2);

        if(addr1in->sin_port < addr2in->sin_port)
        {
            return -1;
        }
        else if(addr2in->sin_port < addr1in->sin_port)
        {
            return 1;
        }

        if(addr1in->sin_addr.s_addr < addr2in->sin_addr.s_addr)
        {
            return -1;
        }
        else if(addr2in->sin_addr.s_addr < addr1in->sin_addr.s_addr)
        {
            return 1;
        }
    }
    else
    {
        const struct sockaddr_in6* addr1in = reinterpret_cast<const sockaddr_in6*>(&addr1);
        const struct sockaddr_in6* addr2in = reinterpret_cast<const sockaddr_in6*>(&addr2);

        if(addr1in->sin6_port < addr2in->sin6_port)
        {
            return -1;
        }
        else if(addr2in->sin6_port < addr1in->sin6_port)
        {
            return 1;
        }

        int res = memcmp(&addr1in->sin6_addr, &addr2in->sin6_addr, sizeof(struct in6_addr));
        if(res < 0)
        {
            return -1;
        }
        else if(res > 0)
        {
            return 1;
        }
    }

    return 0;
}

void
IceInternal::createPipe(SOCKET fds[2])
{
#ifdef _WIN32

    SOCKET fd = createSocket(false, AF_INET);
    setBlock(fd, true);

    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));

    struct sockaddr_in* addrin = reinterpret_cast<struct sockaddr_in*>(&addr);
    addrin->sin_family = AF_INET;
    addrin->sin_port = htons(0);
    addrin->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    addr = doBind(fd, addr);
    doListen(fd, 1);

    try
    {
        fds[0] = createSocket(false, AF_INET);
    }
    catch(...)
    {
        ::closesocket(fd);
        throw;
    }

    try
    {
        setBlock(fds[0], true);
#ifndef NDEBUG
        bool connected = doConnect(fds[0], addr);
        assert(connected);
#else
        doConnect(fds[0], addr);
#endif
    }
    catch(...)
    {
        // fds[0] is closed by doConnect
        ::closesocket(fd);
        throw;
    }

    try
    {
        fds[1] = doAccept(fd);
    }
    catch(...)
    {
        ::closesocket(fds[0]);
        ::closesocket(fd);
        throw;
    }

    ::closesocket(fd);

    try
    {
        setBlock(fds[1], true);
    }
    catch(...)
    {
        ::closesocket(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#else

    if(::pipe(fds) != 0)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    try
    {
        setBlock(fds[0], true);
    }
    catch(...)
    {
        // fds[0] is closed by setBlock
        closeSocketNoThrow(fds[1]);
        throw;
    }

    try
    {
        setBlock(fds[1], true);
    }
    catch(...)
    {
        closeSocketNoThrow(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#endif
}

#ifdef _WIN32

string
IceInternal::errorToStringDNS(int error)
{
    return IceUtilInternal::errorToString(error);
}

#else

string
IceInternal::errorToStringDNS(int error)
{
    return gai_strerror(error);
}

#endif

std::string
IceInternal::fdToString(SOCKET fd)
{
    if(fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    struct sockaddr_storage localAddr;
    fdToLocalAddress(fd, localAddr);

    struct sockaddr_storage remoteAddr;
    bool peerConnected = fdToRemoteAddress(fd, remoteAddr);

    return addressesToString(localAddr, remoteAddr, peerConnected);
};

void
IceInternal::fdToAddressAndPort(SOCKET fd, string& localAddress, int& localPort, string& remoteAddress, int& remotePort)
{
    if(fd == INVALID_SOCKET)
    {
        localAddress.clear();
        remoteAddress.clear();
        localPort = -1;
        remotePort = -1;
        return;
    }

    struct sockaddr_storage localAddr;
    fdToLocalAddress(fd, localAddr);
    addrToAddressAndPort(localAddr, localAddress, localPort);

    struct sockaddr_storage remoteAddr;
    if(fdToRemoteAddress(fd, remoteAddr))
    {
        addrToAddressAndPort(remoteAddr, remoteAddress, remotePort);
    }
    else
    {
        remoteAddress.clear();
        remotePort = -1;
    }
}

void
IceInternal::addrToAddressAndPort(const struct sockaddr_storage& addr, string& address, int& port)
{
    address = inetAddrToString(addr);
    port = getPort(addr);
}

std::string
IceInternal::addressesToString(const struct sockaddr_storage& localAddr, const struct sockaddr_storage& remoteAddr,
                               bool peerConnected)
{
    ostringstream s;
    s << "local address = " << addrToString(localAddr);
    if(peerConnected)
    {
        s << "\nremote address = " << addrToString(remoteAddr);
    }
    else
    {
        s << "\nremote address = <not connected>";
    }
    return s.str();
}

void
IceInternal::fdToLocalAddress(SOCKET fd, struct sockaddr_storage& addr)
{
    socklen_t len = static_cast<socklen_t>(sizeof(struct sockaddr_storage));
    if(getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

bool
IceInternal::fdToRemoteAddress(SOCKET fd, struct sockaddr_storage& addr)
{
    socklen_t len = static_cast<socklen_t>(sizeof(struct sockaddr_storage));
    if(getpeername(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == SOCKET_ERROR)
    {
        if(notConnected())
        {
            return false;
        }
        else
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

    return true;
}

string
IceInternal::inetAddrToString(const struct sockaddr_storage& ss)
{
    int size = 0;
    if(ss.ss_family == AF_INET)
    {
        size = sizeof(sockaddr_in);
    }
    else if(ss.ss_family == AF_INET6)
    {
        size = sizeof(sockaddr_in6);
    }
    else
    {
        return "";
    }

    char namebuf[1024];
    namebuf[0] = '\0';
    getnameinfo(reinterpret_cast<const struct sockaddr *>(&ss), size, namebuf, sizeof(namebuf), 0, 0, NI_NUMERICHOST);
    return string(namebuf);
}

string
IceInternal::addrToString(const struct sockaddr_storage& addr)
{
    ostringstream s;
    string port;
    s << inetAddrToString(addr) << ':' << getPort(addr);
    return s.str();
}

bool
IceInternal::isMulticast(const struct sockaddr_storage& addr)
{
    if(addr.ss_family == AF_INET)
    {
        return IN_MULTICAST(ntohl(reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_addr.s_addr));
    }
    else if(addr.ss_family == AF_INET6)
    {
        return IN6_IS_ADDR_MULTICAST(&reinterpret_cast<const struct sockaddr_in6*>(&addr)->sin6_addr);
    }
    else
    {
        return false;
    }
}

int
IceInternal::getPort(const struct sockaddr_storage& addr)
{
    if(addr.ss_family == AF_INET)
    {
        return ntohs(reinterpret_cast<const sockaddr_in*>(&addr)->sin_port);
    }
    else if(addr.ss_family == AF_INET6)
    {
        return ntohs(reinterpret_cast<const sockaddr_in6*>(&addr)->sin6_port);
    }
    else
    {
        return -1;
    }
}

void
IceInternal::setPort(struct sockaddr_storage& addr, int port)
{
    if(addr.ss_family == AF_INET)
    {
        reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(port);
    }
    else
    {
        assert(addr.ss_family == AF_INET6);
        reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port = htons(port);
    }
}

vector<string>
IceInternal::getHostsForEndpointExpand(const string& host, ProtocolSupport protocolSupport, bool includeLoopback)
{
    vector<string> hosts;
    if(host.empty() || isWildcard(host, protocolSupport))
    {
        vector<struct sockaddr_storage> addrs = getLocalAddresses(protocolSupport);
        for(vector<struct sockaddr_storage>::const_iterator p = addrs.begin(); p != addrs.end(); ++p)
        {
            //
            // NOTE: We don't publish link-local IPv6 addresses as these addresses can only
            // be accessed in general with a scope-id.
            //
            if(p->ss_family != AF_INET6 ||
               !IN6_IS_ADDR_LINKLOCAL(&reinterpret_cast<const struct sockaddr_in6*>(&(*p))->sin6_addr))
            {
                hosts.push_back(inetAddrToString(*p));
            }
        }

        if(hosts.empty() || includeLoopback)
        {
            if(protocolSupport != EnableIPv6)
            {
                hosts.push_back("127.0.0.1");
            }
            if(protocolSupport != EnableIPv4)
            {
                hosts.push_back("0:0:0:0:0:0:0:1");
            }
        }
    }
    return hosts; // An empty host list indicates to just use the given host.
}

void
IceInternal::setTcpBufSize(SOCKET fd, const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& logger)
{
    assert(fd != INVALID_SOCKET);

    //
    // By default, on Windows we use a 128KB buffer size. On Unix
    // platforms, we use the system defaults.
    //
#ifdef _WIN32
    const int dfltBufSize = 128 * 1024;
#else
    const int dfltBufSize = 0;
#endif
    Int sizeRequested;

    sizeRequested = properties->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
    if(sizeRequested > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setRecvBufferSize(fd, sizeRequested);
        int size = getRecvBufferSize(fd);
        if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
        {
            Ice::Warning out(logger);
            out << "TCP receive buffer size: requested size of " << sizeRequested << " adjusted to " << size;
        }
    }

    sizeRequested = properties->getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
    if(sizeRequested > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setSendBufferSize(fd, sizeRequested);
        int size = getSendBufferSize(fd);
        if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
        {
            Ice::Warning out(logger);
            out << "TCP send buffer size: requested size of " << sizeRequested << " adjusted to " << size;
        }
    }
}
