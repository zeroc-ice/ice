// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceInternal::TcpTransceiver::fd()
{
    assert(_fd != INVALID_SOCKET);
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

    assert(_fd != INVALID_SOCKET);
    closeSocket(_fd);
    _fd = INVALID_SOCKET;
}

void
IceInternal::TcpTransceiver::shutdown()
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down tcp connection\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
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
	assert(_fd != INVALID_SOCKET);
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
	    repeatSelect:

		int ret;
		assert(_fd != INVALID_SOCKET);
		FD_SET(_fd, &_wFdSet);

		if(timeout >= 0)
		{
		    struct timeval tv;
		    tv.tv_sec = timeout / 1000;
		    tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
		    ret = ::select(_fd + 1, 0, &_wFdSet, 0, &tv);
		}
		else
		{
		    ret = ::select(_fd + 1, 0, &_wFdSet, 0, 0);
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

	if(_stats)
	{
	    _stats->bytesSent(_name, ret);
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
	assert(_fd != INVALID_SOCKET);
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
	    repeatSelect:

		int ret;
		assert(_fd != INVALID_SOCKET);
		FD_SET(_fd, &_rFdSet);

		if(timeout >= 0)
		{
		    struct timeval tv;
		    tv.tv_sec = timeout / 1000;
		    tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
		    ret = ::select(_fd + 1, &_rFdSet, 0, 0, &tv);
		}
		else
		{
		    ret = ::select(_fd + 1, &_rFdSet, 0, 0, 0);
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

	if(_stats)
	{
	    _stats->bytesReceived(_name, ret);
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
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _stats(instance->stats()),
    _name("tcp"),
    _fd(fd)
{
    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::TcpTransceiver::~TcpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
