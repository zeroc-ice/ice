// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceInternal::TcpTransceiver::fd()
{
    return _fd;
}

void
IceInternal::TcpTransceiver::close()
{
    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "closing tcp connection\n" << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    ::shutdown(fd, SHUT_RDWR); // helps to unblock threads in recv()
    closeSocket(fd);
}

void
IceInternal::TcpTransceiver::shutdown()
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down tcp connection\n" << toString();
    }

    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing
}

void
IceInternal::TcpTransceiver::write(Buffer& buf, int timeout)
{
    int packetSize = buf.b.end() - buf.i;
    
#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(packetSize > 64 * 1024)
    {
	packetSize = 64 * 1024;
    }
#endif

    while(buf.i != buf.b.end())
    {
	int ret = ::send(_fd, &*buf.i, packetSize, 0);

	if(ret == 0)
	{
	    ConnectionLostException ex(__FILE__, __LINE__);
	    ex.error = 0;
	    throw ex;
	}

	if(ret == SOCKET_ERROR)
	{
	    if(interrupted())
	    {
		continue;
	    }

	    if(noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if(wouldBlock())
	    {
		SOCKET fd = _fd; // Copy fd, in case another thread calls close()
		if(fd != INVALID_SOCKET)
		{
		repeatSelect:
		    int ret;
		    FD_SET(fd, &_wFdSet);
		    if(timeout >= 0)
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
		}
		
		continue;
	    }
	    
	    if(connectionLost())
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
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

	if(_traceLevels->network >= 3)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "sent " << ret << " of " << packetSize << " bytes via tcp\n" << toString();
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }
}

void
IceInternal::TcpTransceiver::read(Buffer& buf, int timeout)
{
    int packetSize = buf.b.end() - buf.i;
    
    while(buf.i != buf.b.end())
    {
	int ret = ::recv(_fd, &*buf.i, packetSize, 0);

	if(ret == 0)
	{
	    ConnectionLostException ex(__FILE__, __LINE__);
	    ex.error = 0;
	    throw ex;
	}

	if(ret == SOCKET_ERROR)
	{
	    if(interrupted())
	    {
		continue;
	    }
	    
	    if(noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if(wouldBlock())
	    {
		SOCKET fd = _fd; // Copy fd, in case another thread calls close()
		if(fd != INVALID_SOCKET)
		{
		repeatSelect:
		    int ret;
		    FD_SET(fd, &_rFdSet);
		    if(timeout >= 0)
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
		}
	    
		continue;
	    }
	    
	    if(connectionLost())
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
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

	if(_traceLevels->network >= 3)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "received " << ret << " of " << packetSize << " bytes via tcp\n" << toString();
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
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

IceInternal::TcpTransceiver::TcpTransceiver(const InstancePtr& instance, SOCKET fd) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _fd(fd)
{
    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::TcpTransceiver::~TcpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
