// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/UdpTransceiver.h>
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
IceInternal::UdpTransceiver::fd()
{
    return _fd;
}

void
IceInternal::UdpTransceiver::close()
{
    if (_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "closing " << _protocolName << " connection\n" << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

void
IceInternal::UdpTransceiver::shutdown()
{
}

void
IceInternal::UdpTransceiver::write(Buffer& buf, int)
{
    assert(buf.i == buf.b.begin());
#ifndef NDEBUG
    const int packetSize = 64 * 1024; // TODO: configurable
    assert(packetSize >= static_cast<int>(buf.b.size())); // TODO: exception
#endif

repeat:
    int ret = ::send(_fd, &buf.b[0], buf.b.size(), 0);
    
    if (ret == SOCKET_ERROR)
    {
	if (interrupted())
	{
	    goto repeat;
	}

	if (wouldBlock())
	{
	    SOCKET fd = _fd; // Copy fd, in case another thread calls close()
	    if (fd != INVALID_SOCKET)
	    {
	    repeatSelect:

		FD_SET(fd, &_wFdSet);
		int ret = ::select(fd + 1, 0, &_wFdSet, 0, 0);
		
		if (ret == SOCKET_ERROR)
		{
		    if (interrupted())
		    {
			goto repeatSelect;
		    }
		    
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = getSocketErrno();
		    throw ex;
		}
	    }
	    
	    goto repeat;
	}

	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    if (_traceLevels->network >= 3)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "sent " << ret << " bytes via " << _protocolName << "\n" << toString();
    }
    
    assert(ret == static_cast<int>(buf.b.size()));
    buf.i = buf.b.end();
}

void
IceInternal::UdpTransceiver::read(Buffer& buf, int)
{
    assert(buf.i == buf.b.begin());
    const int packetSize = 64 * 1024; // TODO: configurable
    assert(packetSize >= static_cast<int>(buf.b.size())); // TODO: exception
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:
    int ret;
    if (_connect)
    {
	//
	// If we must connect, then we connect to the first peer that
	// sends us a packet.
	//
	struct sockaddr_in peerAddr;
	memset(&peerAddr, 0, sizeof(struct sockaddr_in));
	socklen_t len = sizeof(peerAddr);
	ret = recvfrom(_fd, &buf.b[0], packetSize, 0, reinterpret_cast<struct sockaddr*>(&peerAddr), &len);
	if (ret != SOCKET_ERROR)
	{
	    doConnect(_fd, peerAddr, -1);
	    _connect = false; // We're connected now

	    if (_traceLevels->network >= 1)
	    {
		Trace out(_logger, _traceLevels->networkCat);
		out << "connected " << _protocolName << " socket\n" << toString();
	    }
	}
    }
    else
    {
	ret = ::recv(_fd, &buf.b[0], packetSize, 0);
    }
    
    if (ret == SOCKET_ERROR)
    {
	if (interrupted())
	{
	    goto repeat;
	}
	
	if (wouldBlock())
	{
	    SOCKET fd = _fd; // Copy fd, in case another thread calls close()
	    if (fd != INVALID_SOCKET)
	    {
	    repeatSelect:

		FD_SET(fd, &_rFdSet);
		int ret = ::select(fd + 1, &_rFdSet, 0, 0, 0);
		
		if (ret == SOCKET_ERROR)
		{
		    if (interrupted())
		    {
			goto repeatSelect;
		    }
		    
		    SocketException ex(__FILE__, __LINE__);
		    ex.error = getSocketErrno();
		    throw ex;
		}
	    }
	    
	    goto repeat;
	}

	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
    
    if (_traceLevels->network >= 3)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "received " << ret << " bytes via " << _protocolName << "\n" << toString();
    }

    buf.b.resize(ret);
    buf.i = buf.b.end();
}

string
IceInternal::UdpTransceiver::toString() const
{
    return fdToString(_fd);
}

bool
IceInternal::UdpTransceiver::equivalent(const string& host, int port) const
{
    struct sockaddr_in addr;
    getAddress(host, port, addr);
    return compareAddress(addr, _addr);
}

int
IceInternal::UdpTransceiver::effectivePort()
{
    return ntohs(_addr.sin_port);
}

void
IceInternal::UdpTransceiver::setProtocolName(const string& protocolName)
{
    _protocolName = protocolName;
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port,
                                            const string& protocolName) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _incoming(false),
    _connect(true),
    _protocolName(protocolName)
{
    try
    {
	_fd = createSocket(true);
	setBlock(_fd, false);
	getAddress(host, port, _addr);
	doConnect(_fd, _addr, -1);
	_connect = false; // We're connected now
	
	if (_traceLevels->network >= 1)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "starting to send " << _protocolName << " packets\n" << toString();
	}
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }

    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port,
					    bool connect, const string& protocolName) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _incoming(true),
    _connect(connect),
    _protocolName(protocolName)
{
    try
    {
	_fd = createSocket(true);
	setBlock(_fd, false);
	getAddress(host, port, _addr);
	doBind(_fd, _addr);
	    
	if (_traceLevels->network >= 1)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "starting to receive " << _protocolName << " packets\n" << toString();
	}
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }

    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::UdpTransceiver::~UdpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
