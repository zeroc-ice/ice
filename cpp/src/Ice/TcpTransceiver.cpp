// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

int
IceInternal::TcpTransceiver::fd()
{
    return _fd;
}

void
IceInternal::TcpTransceiver::close()
{
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "closing connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    int fd = _fd;
    _fd = INVALID_SOCKET;
    ::shutdown(fd, SHUT_RDWR); // helps to unblock threads in recv()
    closeSocket(fd);
}

void
IceInternal::TcpTransceiver::shutdown()
{
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing
}

void
IceInternal::TcpTransceiver::write(Buffer& buf, int timeout)
{
    int packetSize = buf.b.end() - buf.i;
    
#ifdef WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if (packetSize > 64 * 1024)
    {
	packetSize = 64 * 1024;
    }
#endif

    while (buf.i != buf.b.end())
    {
	int ret = ::send(_fd, buf.i, packetSize, 0);

	if (ret == 0)
	{
#ifdef WIN32
	    WSASetLastError(0);
#else
	    errno = 0;
#endif
	    throw ConnectionLostException(__FILE__, __LINE__);
	}

	if (ret == SOCKET_ERROR)
	{
	    if (interrupted())
	    {
		continue;
	    }

	    if (noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if (wouldBlock())
	    {
		int fd = _fd; // Copy fd, in case another thread calls close()
		if (fd != -1)
		{
		repeatSelect:
		    int ret;
		    FD_SET(fd, &wFdSet);
		    if (timeout >= 0)
		    {
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
			ret = ::select(fd + 1, 0, &wFdSet, 0, &tv);
		    }
		    else
		    {
			ret = ::select(fd + 1, 0, &wFdSet, 0, 0);
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
		}
		
		continue;
	    }
	    
	    if (connectionLost())
	    {
		throw ConnectionLostException(__FILE__, __LINE__);
	    }
	    else
	    {
		throw SocketException(__FILE__, __LINE__);
	    }
	}

#ifndef ICE_NO_TRACE
	if (_traceLevels->network >= 3)
	{
	    ostringstream s;
	    s << "sent " << ret << " of " << packetSize << " bytes via TCP\n" << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}
#endif	

	buf.i += ret;

	if (packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }
}

void
IceInternal::TcpTransceiver::read(Buffer& buf, int timeout)
{
    int packetSize = buf.b.end() - buf.i;
    
    while (buf.i != buf.b.end())
    {
	int ret = ::recv(_fd, buf.i, packetSize, 0);

	if (ret == 0)
	{
#ifdef WIN32
	    WSASetLastError(0);
#else
	    errno = 0;
#endif
	    throw ConnectionLostException(__FILE__, __LINE__);
	}

	if (ret == SOCKET_ERROR)
	{
	    if (interrupted())
	    {
		continue;
	    }
	    
	    if (noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if (wouldBlock())
	    {
		int fd = _fd; // Copy fd, in case another thread calls close()
		if (fd != -1)
		{
		repeatSelect:
		    int ret;
		    FD_SET(fd, &rFdSet);
		    if (timeout >= 0)
		    {
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
			ret = ::select(fd + 1, &rFdSet, 0, 0, &tv);
		    }
		    else
		    {
			ret = ::select(fd + 1, &rFdSet, 0, 0, 0);
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
		}
	    
		continue;
	    }
	    
	    if (connectionLost())
	    {
		throw ConnectionLostException(__FILE__, __LINE__);
	    }
	    else
	    {
		throw SocketException(__FILE__, __LINE__);
	    }
	}

#ifndef ICE_NO_TRACE
	if (_traceLevels->network >= 3)
	{
	    ostringstream s;
	    s << "received " << ret << " of " << packetSize << " bytes via TCP\n" << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}
#endif	

	buf.i += ret;

	if (packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }
}

string
IceInternal::TcpTransceiver::toString() const
{
    return fdToString(_fd);
}

IceInternal::TcpTransceiver::TcpTransceiver(const InstancePtr& instance, int fd) :
    _instance(instance),
    _fd(fd)
{
    FD_ZERO(&rFdSet);
    FD_ZERO(&wFdSet);

#ifndef ICE_NO_TRACE
    _traceLevels = _instance->traceLevels();
    _logger = _instance->logger();
#endif
}

IceInternal::TcpTransceiver::~TcpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
