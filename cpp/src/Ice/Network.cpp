// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

bool
IceInternal::interrupted()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAEINTR)
    {
	return true;
    }
#else
    if (errno == EINTR ||
	errno == EPROTO)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::acceptInterrupted()
{
    if (interrupted())
	return true;

#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAECONNABORTED ||
	error == WSAECONNRESET ||
	error == WSAETIMEDOUT)
    {
	return true;
    }
#else
    if (errno == ECONNABORTED ||
	errno == ECONNRESET ||
	errno == ETIMEDOUT)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::noBuffers()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAENOBUFS ||
	error == WSAEFAULT)
    {
	return true;
    }
#else
    if (errno == ENOBUFS)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::wouldBlock()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK)
    {
	return true;
    }
#else
    if (errno == EAGAIN ||
	errno == EWOULDBLOCK)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::connectFailed()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAECONNREFUSED ||
	error == WSAETIMEDOUT ||
	error == WSAENETUNREACH ||
	error == WSAECONNRESET ||
	error == WSAESHUTDOWN ||
	error == WSAECONNABORTED)
    {
	return true;
    }
#else
    if (errno == ECONNREFUSED ||
	errno == ETIMEDOUT ||
	errno == ENETUNREACH ||
	errno == ECONNRESET ||
	errno == ESHUTDOWN ||
	errno == ECONNABORTED)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::connectInProgress()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK)
    {
	return true;
    }
#else
    if (errno == EINPROGRESS)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::connectionLost()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAECONNRESET ||
	error == WSAESHUTDOWN ||
	error == WSAECONNABORTED)
    {
	return true;
    }
#else
    if (errno == ECONNRESET ||
	errno == ESHUTDOWN ||
	errno == ECONNABORTED)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

bool
IceInternal::notConnected()
{
#ifdef WIN32
    int error = WSAGetLastError();
    if (error == WSAENOTCONN)
    {
	return true;
    }
#else
    if (errno == ENOTCONN)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

int
IceInternal::createSocket(bool udp)
{
    int fd;

    if (udp)
    {
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    }

    if (fd == INVALID_SOCKET)
    {
	throw SocketException(__FILE__, __LINE__);
    }

    setBlock(fd, false);

    if (!udp)
    {
	setTcpNoDelay(fd);
	setKeepAlive(fd);
    }

    return fd;
}

void
IceInternal::closeSocket(int fd)
{
#ifdef WIN32
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
IceInternal::setBlock(int fd, bool block)
{
    if (block)
    {
#ifdef WIN32
	unsigned long arg = 0;
	ioctlsocket(fd, FIONBIO, &arg);
#else
	int flags = fcntl(fd, F_GETFL);
	flags &= ~O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
#endif
    }
    else
    {
#ifdef WIN32
	unsigned long arg = 1;
	ioctlsocket(fd, FIONBIO, &arg);
#else
	int flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
#endif
    }
}

void
IceInternal::setTcpNoDelay(int fd)
{
    int flag = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }
}
    
void
IceInternal::setKeepAlive(int fd)
{
    int flag = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }
}

void
IceInternal::setSendBufferSize(int fd, int sz)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }
}

void
IceInternal::doBind(int fd, struct sockaddr_in& addr)
{
#ifndef WIN32
    int flag = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }
#endif

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }

    socklen_t len = sizeof(addr);
    int ret = getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len);
    assert(ret != SOCKET_ERROR);
}

void
IceInternal::doListen(int fd, int backlog)
{
repeatListen:
    if (::listen(fd, backlog) == SOCKET_ERROR)
    {
	if (interrupted())
	{
	    goto repeatListen;
	}
	
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }
}

void
IceInternal::doConnect(int fd, struct sockaddr_in& addr, int timeout)
{
#ifdef WIN32
    //
    // Set larger send buffer size to avoid performance problems on
    // WIN32
    //
    setSendBufferSize(fd, 64 * 1024);
#endif

repeatConnect:
    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
	if (interrupted())
	{
	    goto repeatConnect;
	}
	
	if (connectInProgress())
	{
	repeatSelect:
	    int ret;
	    fd_set wFdSet;
	    FD_ZERO(&wFdSet);
	    FD_SET(fd, &wFdSet);
#ifdef WIN32
	    //
	    // WIN32 notifies about connection failures
	    // through the exception filedescriptors
	    //
	    fd_set xFdSet;
	    FD_ZERO(&xFdSet);
	    FD_SET(fd, &xFdSet);
#endif
	    if (timeout >= 0)
	    {
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
#ifdef WIN32
		ret = ::select(fd + 1, 0, &wFdSet, &xFdSet, &tv);
#else
		ret = ::select(fd + 1, 0, &wFdSet, 0, &tv);
#endif
	    }
	    else
	    {
#ifdef WIN32
		ret = ::select(fd + 1, 0, &wFdSet, &xFdSet, 0);
#else
		ret = ::select(fd + 1, 0, &wFdSet, 0, 0);
#endif
	    }
	    
	    if (ret == 0)
	    {
		closeSocket(fd);
		throw ConnectTimeoutException(__FILE__, __LINE__);
	    }
	    else if (ret == SOCKET_ERROR)
	    {
		if (interrupted())
		{
		    goto repeatSelect;
		}
		
		throw SocketException(__FILE__, __LINE__);
	    }
	    
#ifdef WIN32
	    //
	    // Strange windows bug: The following call to Sleep() is
	    // necessary, otherwise no error is reported through
	    // getsockopt.
	    //
	    Sleep(0);
#endif
	    socklen_t len = sizeof(socklen_t);
	    int val;
	    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
	    {
		closeSocket(fd);
		throw SocketException(__FILE__, __LINE__);
	    }
	    
	    if (val > 0)
	    {
		closeSocket(fd);
#ifdef WIN32
		WSASetLastError(val);
#else
		errno = val;
#endif
		if (connectFailed())
		{
		    throw ConnectFailedException(__FILE__, __LINE__);
		}
		else
		{
		    throw SocketException(__FILE__, __LINE__);
		}
	    }
	    
	    return;
	}
    
	closeSocket(fd);
	if (connectFailed())
	{
	    throw ConnectFailedException(__FILE__, __LINE__);
	}
	else
	{
	    throw SocketException(__FILE__, __LINE__);
	}
    }
}

int
IceInternal::doAccept(int fd, int timeout)
{
    int ret;

repeatAccept:
    if ((ret = ::accept(fd, 0, 0)) == INVALID_SOCKET)
    {
	if (acceptInterrupted())
	{
	    goto repeatAccept;
	}

	if (wouldBlock())
	{
	repeatSelect:
	    int ret;
	    fd_set fdSet;
	    FD_ZERO(&fdSet);
	    FD_SET(fd, &fdSet);
	    if (timeout >= 0)
	    {
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
		ret = ::select(fd + 1, 0, &fdSet, 0, &tv);
	    }
	    else
	    {
		ret = ::select(fd + 1, 0, &fdSet, 0, 0);
	    }
	    
	    if (ret == SOCKET_ERROR)
	    {
		if (interrupted())
		{
		    goto repeatSelect;
		}
		
		throw SocketException(__FILE__, __LINE__);
	    }
	    
	    if (ret == 0)
	    {
		throw TimeoutException(__FILE__, __LINE__);
	    }
	    
	    goto repeatAccept;
	}
	
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }

    setTcpNoDelay(ret);
    setKeepAlive(ret);

#ifdef WIN32
    //
    // Set larger send buffer size to avoid performance problems on
    // WIN32
    //
    setSendBufferSize(ret, 64 * 1024);
#endif
    
    return ret;
}

static JTCMutex getHostByNameMutex;

void
IceInternal::getAddress(const char* host, int port, struct sockaddr_in& addr)
{
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
	JTCSyncT<JTCMutex> sync(getHostByNameMutex);

	struct hostent* entry;
	int retry = 5;

	do
	{
	    entry = gethostbyname(host);
	}
#ifdef WIN32
	while (!entry && WSAGetLastError() == WSATRY_AGAIN && --retry >= 0);
#else
	while (!entry && h_errno == TRY_AGAIN && --retry >= 0);
#endif

	if (!entry)
	{
	    throw DNSException(__FILE__, __LINE__);
	}

	memcpy(&addr.sin_addr, entry->h_addr, entry->h_length);
    }
}

void
IceInternal::getLocalAddress(int port, struct sockaddr_in& addr)
{
    char host[1024 + 1];
    if (gethostname(host, 1024) == -1)
    {
	throw SystemException(__FILE__, __LINE__);
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    {
	JTCSyncT<JTCMutex> sync(getHostByNameMutex);
	
	struct hostent* entry;
	int retry = 5;
	
	do
	{
	    entry = gethostbyname(host);
	}
#ifdef WIN32
	while (!entry && WSAGetLastError() == WSATRY_AGAIN && --retry >= 0);
#else
	while (!entry && h_errno == TRY_AGAIN && --retry >= 0);
#endif
	
	if (!entry)
	{
	    throw DNSException(__FILE__, __LINE__);
	}

	memcpy(&addr.sin_addr, entry->h_addr, entry->h_length);
    }
}

string
IceInternal::getLocalHost(bool numeric)
{
    char host[1024 + 1];
    if (gethostname(host, 1024) == -1)
    {
	throw SystemException(__FILE__, __LINE__);
    }

    {
	JTCSyncT<JTCMutex> sync(getHostByNameMutex);
	
	struct hostent* entry;
	int retry = 5;
	
	do
	{
	    entry = gethostbyname(host);
	}
#ifdef WIN32
	while (!entry && WSAGetLastError() == WSATRY_AGAIN && --retry >= 0);
#else
	while (!entry && h_errno == TRY_AGAIN && --retry >= 0);
#endif
	
	if (!entry)
	{
	    throw DNSException(__FILE__, __LINE__);
	}

	if (numeric)
	{
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(struct sockaddr_in));
	    memcpy(&addr.sin_addr, entry->h_addr, entry->h_length);
	    return string(inet_ntoa(addr.sin_addr));
	}
	else
	{
	    return string(entry->h_name);
	}
    }
}

void
IceInternal::createPipe(int fds[2])
{
#ifdef WIN32

    int fd = createSocket(false);
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    doBind(fd, addr);
    doListen(fd, 1);

    try
    {
	fds[0] = createSocket(false);
    }
    catch(...)
    {
	closeSocket(fd);
	throw;
    }

    try
    {
	doConnect(fds[0], addr, -1);
	fds[1] = doAccept(fd, -1);
    }
    catch(...)
    {
	closeSocket(fd);
	closeSocket(fds[0]);
	throw;
    }

    closeSocket(fd);

#else

    if (::pipe(fds) != 0)
    {
	throw SystemException(__FILE__, __LINE__);
    }

#endif
}

#ifdef WIN32

string
IceInternal::errorToString(int error)
{
    if (error < WSABASEERR)
    {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL,
		      error,
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		      (LPTSTR)&lpMsgBuf,
		      0,
		      NULL);
	string result = (LPCTSTR)lpMsgBuf;
	LocalFree( lpMsgBuf );
	return result;
    }

    switch (error)
    {
    case WSAEINTR:
	return "WSAEINTR";
	
    case WSAEBADF:
	return "WSAEBADF";
	
    case WSAEACCES:
	return "WSAEACCES";
	
    case WSAEFAULT:
	return "WSAEFAULT";
	
    case WSAEINVAL:
	return "WSAEINVAL";
	
    case WSAEMFILE:
	return "WSAEMFILE";
	
    case WSAEWOULDBLOCK:
	return "WSAEWOULDBLOCK";
	
    case WSAEINPROGRESS:
	return "WSAEINPROGRESS";
	
    case WSAEALREADY:
	return "WSAEALREADY";
	
    case WSAENOTSOCK:
	return "WSAENOTSOCK";
	
    case WSAEDESTADDRREQ:
	return "WSAEDESTADDRREQ";
	
    case WSAEMSGSIZE:
	return "WSAEMSGSIZE";
	
    case WSAEPROTOTYPE:
	return "WSAEPROTOTYPE";
	
    case WSAENOPROTOOPT:
	return "WSAENOPROTOOPT";
	
    case WSAEPROTONOSUPPORT:
	return "WSAEPROTONOSUPPORT";
	
    case WSAESOCKTNOSUPPORT:
	return "WSAESOCKTNOSUPPORT";
	
    case WSAEOPNOTSUPP:
	return "WSAEOPNOTSUPP";
	
    case WSAEPFNOSUPPORT:
	return "WSAEPFNOSUPPORT";
	
    case WSAEAFNOSUPPORT:
	return "WSAEAFNOSUPPORT";
	
    case WSAEADDRINUSE:
	return "WSAEADDRINUSE";
	
    case WSAEADDRNOTAVAIL:
	return "WSAEADDRNOTAVAIL";
	
    case WSAENETDOWN:
	return "WSAENETDOWN";
	
    case WSAENETUNREACH:
	return "WSAENETUNREACH";
	
    case WSAENETRESET:
	return "WSAENETRESET";
	
    case WSAECONNABORTED:
	return "WSAECONNABORTED";
	
    case WSAECONNRESET:
	return "WSAECONNRESET";
	
    case WSAENOBUFS:
	return "WSAENOBUFS";
	
    case WSAEISCONN:
	return "WSAEISCONN";
	
    case WSAENOTCONN:
	return "WSAENOTCONN";
	
    case WSAESHUTDOWN:
	return "WSAESHUTDOWN";
	
    case WSAETOOMANYREFS:
	return "WSAETOOMANYREFS";
	
    case WSAETIMEDOUT:
	return "WSAETIMEDOUT";
	
    case WSAECONNREFUSED:
	return "WSAECONNREFUSED";
	
    case WSAELOOP:
	return "WSAELOOP";
	
    case WSAENAMETOOLONG:
	return "WSAENAMETOOLONG";
	
    case WSAEHOSTDOWN:
	return "WSAEHOSTDOWN";
	
    case WSAEHOSTUNREACH:
	return "WSAEHOSTUNREACH";
	
    case WSAENOTEMPTY:
	return "WSAENOTEMPTY";
	
    case WSAEPROCLIM:
	return "WSAEPROCLIM";
	
    case WSAEUSERS:
	return "WSAEUSERS";
	
    case WSAEDQUOT:
	return "WSAEDQUOT";
	
    case WSAESTALE:
	return "WSAESTALE";
	
    case WSAEREMOTE:
	return "WSAEREMOTE";
	
    case WSAEDISCON:
	return "WSAEDISCON";
	
    case WSASYSNOTREADY:
	return "WSASYSNOTREADY";
	
    case WSAVERNOTSUPPORTED:
	return "WSAVERNOTSUPPORTED";
	
    case WSANOTINITIALISED:
	return "WSANOTINITIALISED";
	
    case WSAHOST_NOT_FOUND:
	return "WSAHOST_NOT_FOUND";
	
    case WSATRY_AGAIN:
	return "WSATRY_AGAIN";
	
    case WSANO_RECOVERY:
	return "WSANO_RECOVERY";
	
    case WSANO_DATA:
	return "WSANO_DATA";

    default:
	return "unknown socket error";
    }
}

string
IceInternal::errorToStringDNS(int error)
{
    return errorToString(error);
}

#else

string
IceInternal::errorToString(int error)
{
    return strerror(error);
}

string
IceInternal::errorToStringDNS(int error)
{
    switch (error)
    {
    case NETDB_SUCCESS:
	return "no problem";

    case NETDB_INTERNAL:
	return "internal problem";

    case HOST_NOT_FOUND:
	return "no such host is known";
	
    case TRY_AGAIN:
	return "temporary error, try again";
	
    case NO_RECOVERY:
	return "unexpected server failure";
	
    case NO_DATA:
	return "name has no IP address";

    default:
	return "unknown DNS error";
    }
}

#endif

string
IceInternal::lastErrorToString()
{
#ifdef WIN32
    return errorToString(WSAGetLastError());
#else
    return errorToString(errno);
#endif
}

string
IceInternal::lastErrorToStringDNS()
{
#ifdef WIN32
    return errorToStringDNS(WSAGetLastError());
#else
    return errorToStringDNS(h_errno);
#endif
}

static JTCMutex inetNtoaMutex;

std::string
IceInternal::fdToString(int fd)
{
    socklen_t localLen = sizeof(struct sockaddr_in);
    struct sockaddr_in localAddr;
    if (getsockname(fd, reinterpret_cast<struct sockaddr*>(&localAddr), &localLen) == SOCKET_ERROR)
    {
	closeSocket(fd);
	throw SocketException(__FILE__, __LINE__);
    }
    
    bool peerNotConnected = false;
    socklen_t remoteLen = sizeof(struct sockaddr_in);
    struct sockaddr_in remoteAddr;
    if (getpeername(fd, reinterpret_cast<struct sockaddr*>(&remoteAddr), &remoteLen) == SOCKET_ERROR)
    {
	if (notConnected())
	{
	    peerNotConnected = true;
	}
	else
	{
	    closeSocket(fd);
	    throw SocketException(__FILE__, __LINE__);
	}
    }

    ostringstream s;

    {
	JTCSyncT<JTCMutex> sync(inetNtoaMutex);

	s << "local address = " << inet_ntoa(localAddr.sin_addr) << ':' << ntohs(localAddr.sin_port);
	if (peerNotConnected)
	{
	    s << "\nremote address = <not connected>";
	}
	else
	{
	    s << "\nremote address = " << inet_ntoa(remoteAddr.sin_addr) << ':' << ntohs(remoteAddr.sin_port);
	}
    }

    return s.str();
}

std::string
IceInternal::addrToString(const struct sockaddr_in& addr)
{
    JTCSyncT<JTCMutex> sync(inetNtoaMutex);
    ostringstream s;
    s << inet_ntoa(addr.sin_addr) << ':' << ntohs(addr.sin_port);
    return s.str();
}
