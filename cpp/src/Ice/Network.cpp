// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Mutex.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

bool
IceInternal::interrupted()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAEINTR)
    {
	return true;
    }
#else
    if(errno == EINTR ||
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
    if(interrupted())
    {
	return true;
    }

#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAECONNABORTED ||
	error == WSAECONNRESET ||
	error == WSAETIMEDOUT)
    {
	return true;
    }
#else
    if(errno == ECONNABORTED ||
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
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAENOBUFS ||
	error == WSAEFAULT)
    {
	return true;
    }
#else
    if(errno == ENOBUFS)
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
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAEWOULDBLOCK)
    {
	return true;
    }
#else
    if(errno == EAGAIN ||
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
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAECONNREFUSED ||
	error == WSAETIMEDOUT ||
	error == WSAENETUNREACH ||
	error == WSAECONNRESET ||
	error == WSAESHUTDOWN ||
	error == WSAECONNABORTED)
    {
	return true;
    }
#else
    if(errno == ECONNREFUSED ||
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
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAEWOULDBLOCK)
    {
	return true;
    }
#else
    if(errno == EINPROGRESS)
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
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAECONNRESET ||
	error == WSAESHUTDOWN ||
	error == WSAECONNABORTED)
    {
	return true;
    }
#else
    if(errno == ECONNRESET ||
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
#ifdef _WIN32
    int error = WSAGetLastError();
    if(error == WSAENOTCONN)
    {
	return true;
    }
#else
    if(errno == ENOTCONN)
    {
	return true;
    }
#endif
    else
    {
	return false;
    }
}

SOCKET
IceInternal::createSocket(bool udp)
{
    SOCKET fd;

    if(udp)
    {
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    closesocket(fd);
    WSASetLastError(error);
#else
    int error = errno;
    close(fd);
    errno = error;
#endif
}
    
void
IceInternal::setBlock(SOCKET fd, bool block)
{
    if(block)
    {
#ifdef _WIN32
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
#ifdef _WIN32
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
IceInternal::setTcpNoDelay(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
}
    
void
IceInternal::setKeepAlive(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
}

void
IceInternal::setSendBufferSize(SOCKET fd, int sz)
{
    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
}

void
IceInternal::doBind(SOCKET fd, struct sockaddr_in& addr)
{
#ifndef _WIN32
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
#endif

    if(bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    socklen_t len = sizeof(addr);
#ifdef NDEBUG
    getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len);
#else
    int ret = getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len);
    assert(ret != SOCKET_ERROR);
#endif
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
	
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
}

void
IceInternal::doConnect(SOCKET fd, struct sockaddr_in& addr, int timeout)
{
#ifdef _WIN32
    //
    // Set larger send buffer size to avoid performance problems on
    // WIN32
    //
    setSendBufferSize(fd, 64 * 1024);
#endif

repeatConnect:
    if(::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
	if(interrupted())
	{
	    goto repeatConnect;
	}
	
	if(connectInProgress())
	{
	repeatSelect:
	    int ret;
	    fd_set wFdSet;
	    FD_ZERO(&wFdSet);
	    FD_SET(fd, &wFdSet);
#ifdef _WIN32
	    //
	    // WIN32 notifies about connection failures
	    // through the exception filedescriptors
	    //
	    fd_set xFdSet;
	    FD_ZERO(&xFdSet);
	    FD_SET(fd, &xFdSet);
#endif
	    if(timeout >= 0)
	    {
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
#ifdef _WIN32
		ret = ::select(fd + 1, 0, &wFdSet, &xFdSet, &tv);
#else
		ret = ::select(fd + 1, 0, &wFdSet, 0, &tv);
#endif
	    }
	    else
	    {
#ifdef _WIN32
		ret = ::select(fd + 1, 0, &wFdSet, &xFdSet, 0);
#else
		ret = ::select(fd + 1, 0, &wFdSet, 0, 0);
#endif
	    }
	    
	    if(ret == 0)
	    {
		closeSocket(fd);
		throw ConnectTimeoutException(__FILE__, __LINE__);
	    }
	    else if(ret == SOCKET_ERROR)
	    {
		if(interrupted())
		{
		    goto repeatSelect;
		}
		
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    
#ifdef _WIN32
	    //
	    // Strange windows bug: The following call to Sleep() is
	    // necessary, otherwise no error is reported through
	    // getsockopt.
	    //
	    Sleep(0);
#endif
	    socklen_t len = sizeof(socklen_t);
	    int val;
	    if(getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
	    {
		closeSocket(fd);
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    
	    if(val > 0)
	    {
		closeSocket(fd);
#ifdef _WIN32
		WSASetLastError(val);
#else
		errno = val;
#endif
		if(connectFailed())
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
	    
	    return;
	}
    
	closeSocket(fd);
	if(connectFailed())
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

SOCKET
IceInternal::doAccept(SOCKET fd, int timeout)
{
    int ret;

repeatAccept:
    if((ret = ::accept(fd, 0, 0)) == INVALID_SOCKET)
    {
	if(acceptInterrupted())
	{
	    goto repeatAccept;
	}

	if(wouldBlock())
	{
	repeatSelect:
	    int ret;
	    fd_set fdSet;
	    FD_ZERO(&fdSet);
	    FD_SET(fd, &fdSet);
	    if(timeout >= 0)
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
	    
	    if(ret == SOCKET_ERROR)
	    {
		if(interrupted())
		{
		    goto repeatSelect;
		}
		
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    
	    if(ret == 0)
	    {
		throw TimeoutException(__FILE__, __LINE__);
	    }
	    
	    goto repeatAccept;
	}
	
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    setTcpNoDelay(ret);
    setKeepAlive(ret);

#ifdef _WIN32
    //
    // Set larger send buffer size to avoid performance problems on
    // WIN32
    //
    setSendBufferSize(ret, 64 * 1024);
#endif
    
    return ret;
}

static IceUtil::Mutex getHostByNameMutex;

void
IceInternal::getAddress(const string& host, int port, struct sockaddr_in& addr)
{
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str());

    if(addr.sin_addr.s_addr == INADDR_NONE)
    {
	IceUtil::Mutex::Lock sync(getHostByNameMutex);

	struct hostent* entry;
	int retry = 5;

	do
	{
	    entry = gethostbyname(host.c_str());
	}
#ifdef _WIN32
	while(!entry && WSAGetLastError() == WSATRY_AGAIN && --retry >= 0);
#else
	while(!entry && h_errno == TRY_AGAIN && --retry >= 0);
#endif

	if(!entry)
	{
	    DNSException ex(__FILE__, __LINE__);
	    ex.error = getDNSErrno();
	    throw ex;
	}

	memcpy(&addr.sin_addr, entry->h_addr, entry->h_length);
    }
}

string
IceInternal::getLocalHost(bool numeric)
{
    char host[1024 + 1];
    if(gethostname(host, 1024) == SOCKET_ERROR)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
    
    {
	IceUtil::Mutex::Lock sync(getHostByNameMutex);
	
	struct hostent* entry;
	int retry = 5;
	
	do
	{
	    entry = gethostbyname(host);
	}
#ifdef _WIN32
	while(!entry && WSAGetLastError() == WSATRY_AGAIN && --retry >= 0);
#else
	while(!entry && h_errno == TRY_AGAIN && --retry >= 0);
#endif
	
	if(!entry)
	{
	    DNSException ex(__FILE__, __LINE__);
	    ex.error = getDNSErrno();
	    throw ex;
	}

	if(numeric)
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

bool
IceInternal::compareAddress(const struct sockaddr_in& addr1, const struct sockaddr_in& addr2)
{
    return (addr1.sin_family == addr2.sin_family) &&
           (addr1.sin_port == addr2.sin_port) &&
           (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr);
}

void
IceInternal::createPipe(SOCKET fds[2])
{
#ifdef _WIN32

    SOCKET fd = createSocket(false);
    setBlock(fd, true);
    
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
	setBlock(fds[0], true);
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
	setBlock(fds[1], true);
    }
    catch(...)
    {
	closeSocket(fd);
	closeSocket(fds[0]);
	throw;
    }

    closeSocket(fd);

#else

    if(::pipe(fds) != 0)
    {
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }

    setBlock(fds[0], true);
    setBlock(fds[1], true);

#endif
}

#ifdef _WIN32

string
IceInternal::errorToString(int error)
{
    if(error < WSABASEERR)
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

    switch(error)
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
    switch(error)
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
#ifdef _WIN32
    return errorToString(WSAGetLastError());
#else
    return errorToString(errno);
#endif
}

string
IceInternal::lastErrorToStringDNS()
{
#ifdef _WIN32
    return errorToStringDNS(WSAGetLastError());
#else
    return errorToStringDNS(h_errno);
#endif
}

static IceUtil::Mutex inetNtoaMutex;

std::string
IceInternal::fdToString(SOCKET fd)
{
    if(fd == INVALID_SOCKET)
    {
	return "<closed>";
    }

    socklen_t localLen = sizeof(struct sockaddr_in);
    struct sockaddr_in localAddr;
    if(getsockname(fd, reinterpret_cast<struct sockaddr*>(&localAddr), &localLen) == SOCKET_ERROR)
    {
	closeSocket(fd);
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
    
    bool peerNotConnected = false;
    socklen_t remoteLen = sizeof(struct sockaddr_in);
    struct sockaddr_in remoteAddr;
    if(getpeername(fd, reinterpret_cast<struct sockaddr*>(&remoteAddr), &remoteLen) == SOCKET_ERROR)
    {
	if(notConnected())
	{
	    peerNotConnected = true;
	}
	else
	{
	    closeSocket(fd);
	    SocketException ex(__FILE__, __LINE__);
	    ex.error = getSocketErrno();
	    throw ex;
	}
    }

    ostringstream s;

    {
	IceUtil::Mutex::Lock sync(inetNtoaMutex);

	s << "local address = " << inet_ntoa(localAddr.sin_addr) << ':' << ntohs(localAddr.sin_port);
	if(peerNotConnected)
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
    IceUtil::Mutex::Lock sync(inetNtoaMutex);
    ostringstream s;
    s << inet_ntoa(addr.sin_addr) << ':' << ntohs(addr.sin_port);
    return s.str();
}
