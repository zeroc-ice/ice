// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslTransceiver.h>
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
IceInternal::SslTransceiver::fd()
{
    return _fd;
}

void
IceInternal::SslTransceiver::close()
{
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "closing ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    int fd = _fd;
    _fd = INVALID_SOCKET;
    ::shutdown(fd, SHUT_RDWR); // helps to unblock threads in recv()
    closeSocket(fd);
}

void
IceInternal::SslTransceiver::shutdown()
{
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing
}

void
IceInternal::SslTransceiver::write(Buffer& buf, int timeout)
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
		    FD_SET(fd, &_wFdSet);
		    if (timeout >= 0)
		    {
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
			ret = ::select(fd + 1, 0, &_wFdSet, 0, &tv);
		    }
		    else
		    {
			ret = ::select(fd + 1, 0, &_wFdSet, 0, 0);
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

	if (_traceLevels->network >= 3)
	{
	    ostringstream s;
	    s << "sent " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}

	buf.i += ret;

	if (packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }
}

void
IceInternal::SslTransceiver::read(Buffer& buf, int timeout)
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
		    FD_SET(fd, &_rFdSet);
		    if (timeout >= 0)
		    {
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
			ret = ::select(fd + 1, &_rFdSet, 0, 0, &tv);
		    }
		    else
		    {
			ret = ::select(fd + 1, &_rFdSet, 0, 0, 0);
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

	if (_traceLevels->network >= 3)
	{
	    ostringstream s;
	    s << "received " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}

	buf.i += ret;

	if (packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }
}

string
IceInternal::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

IceInternal::SslTransceiver::SslTransceiver(const InstancePtr& instance, int fd) :
    _instance(instance),
    _fd(fd),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
{
    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::SslTransceiver::~SslTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
