// **********************************************************************
//
// Copyright (c) 2003
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

#include <Ice/UdpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceInternal::UdpTransceiver::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceInternal::UdpTransceiver::close()
{
    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "closing udp connection\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    closeSocket(_fd);
    _fd = INVALID_SOCKET;
}

void
IceInternal::UdpTransceiver::shutdown()
{
}

void
IceInternal::UdpTransceiver::write(Buffer& buf, int)
{
    assert(buf.i == buf.b.begin());
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
	Ice::DatagramLimitException ex(__FILE__, __LINE__);
	ex.maxSize = packetSize;
	throw ex;
    }

repeat:

    assert(_fd != INVALID_SOCKET);
    ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0);
    
    if(ret == SOCKET_ERROR)
    {
	if(interrupted())
	{
	    goto repeat;
	}

	if(wouldBlock())
	{
	repeatSelect:

	    assert(_fd != INVALID_SOCKET);
	    FD_SET(_fd, &_wFdSet);
	    int rs = ::select(_fd + 1, 0, &_wFdSet, 0, 0);
	    
	    if(rs == SOCKET_ERROR)
	    {
		if(interrupted())
		{
		    goto repeatSelect;
		}
		
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    
	    goto repeat;
	}
	
	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }

    if(_traceLevels->network >= 3)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "sent " << ret << " bytes via udp\n" << toString();
    }
    
    if(_stats)
    {
	_stats->bytesSent(_name, static_cast<Int>(ret));
    }

    assert(ret == static_cast<ssize_t>(buf.b.size()));
    buf.i = buf.b.end();
}

void
IceInternal::UdpTransceiver::read(Buffer& buf, int)
{
    assert(buf.i == buf.b.begin());

    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
	Ice::DatagramLimitException ex(__FILE__, __LINE__);
	ex.maxSize = packetSize;
	throw ex;
    }
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:

    ssize_t ret;
    if(_connect)
    {
	//
	// If we must connect, then we connect to the first peer that
	// sends us a packet.
	//
	struct sockaddr_in peerAddr;
	memset(&peerAddr, 0, sizeof(struct sockaddr_in));
	socklen_t len = static_cast<socklen_t>(sizeof(peerAddr));
	assert(_fd != INVALID_SOCKET);
	ret = recvfrom(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize,
		       0, reinterpret_cast<struct sockaddr*>(&peerAddr), &len);
	if(ret != SOCKET_ERROR)
	{
	    doConnect(_fd, peerAddr, -1);
	    _connect = false; // We are connected now.

	    if(_traceLevels->network >= 1)
	    {
		Trace out(_logger, _traceLevels->networkCat);
		out << "connected udp socket\n" << toString();
	    }
	}
    }
    else
    {
	assert(_fd != INVALID_SOCKET);
	ret = ::recv(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0);
    }
    
    if(ret == SOCKET_ERROR)
    {
	if(interrupted())
	{
	    goto repeat;
	}
	
	if(wouldBlock())
	{
	repeatSelect:
	    
	    assert(_fd != INVALID_SOCKET);
	    FD_SET(_fd, &_rFdSet);
	    int rs = ::select(_fd + 1, &_rFdSet, 0, 0, 0);
	    
	    if(rs == SOCKET_ERROR)
	    {
		if(interrupted())
		{
		    goto repeatSelect;
		}
		
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    
	    goto repeat;
	}

	if(recvTruncated())
	{
	    DatagramLimitException ex(__FILE__, __LINE__);
	    ex.maxSize = packetSize;
	    if(_warn)
	    {
		Warning out(_logger);
		out << "datagram exception:\n" << ex << '\n' << toString();
	    }
	    throw ex;

	}

	SocketException ex(__FILE__, __LINE__);
	ex.error = getSocketErrno();
	throw ex;
    }
    
    if(_traceLevels->network >= 3)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "received " << ret << " bytes via udp\n" << toString();
    }

    if(_stats)
    {
	_stats->bytesReceived(_name, static_cast<Int>(ret));
    }

    buf.b.resize(ret);
    buf.i = buf.b.end();
}

string
IceInternal::UdpTransceiver::toString() const
{
    return fdToString(_fd);
}

int
IceInternal::UdpTransceiver::maxRecvSize() const
{
    return _rcvSize;
}

int
IceInternal::UdpTransceiver::maxSendSize() const
{
    return _sndSize;
}

bool
IceInternal::UdpTransceiver::equivalent(const string& host, int port) const
{
    struct sockaddr_in addr;
    getAddress(host, port, addr);
    return compareAddress(addr, _addr);
}

int
IceInternal::UdpTransceiver::effectivePort() const
{
    return ntohs(_addr.sin_port);
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _incoming(false),
    _connect(true)
{
    try
    {
	_fd = createSocket(true);
	setBufSize(instance);
	setBlock(_fd, false);
	getAddress(host, port, _addr);
	doConnect(_fd, _addr, -1);
	_connect = false; // We're connected now
	
	if(_traceLevels->network >= 1)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "starting to send udp packets\n" << toString();
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

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port, bool connect) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _stats(instance->stats()),
    _name("udp"),
    _incoming(true),
    _connect(connect)
{
    try
    {
	_fd = createSocket(true);
	setBufSize(instance);
	setBlock(_fd, false);
	getAddress(host, port, _addr);
	if(_traceLevels->network >= 2)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "attempting to bind to udp socket " << addrToString(_addr);
	}
	doBind(_fd, _addr);
	    
	if(_traceLevels->network >= 1)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "starting to receive udp packets\n" << toString();
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

//
// Set UDP receive and send buffer sizes.
//

void
IceInternal::UdpTransceiver::setBufSize(const InstancePtr& instance)
{
    assert(_fd != INVALID_SOCKET);

    _warn = instance->properties()->getPropertyAsInt("Ice.Warn.Datagrams") > 0;

    for(int i = 0; i < 2; ++i)
    {
	string direction;
	string prop;
	int* addr;
	int dfltSize;
	if(i == 0)
	{
	    direction = "receive";
	    prop = "Ice.UDP.RcvSize";
	    addr = &_rcvSize;
	    dfltSize = getRecvBufferSize(_fd);
	}
	else
	{
	    direction = "send";
	    prop = "Ice.UDP.SndSize";
	    addr = &_sndSize;
	    dfltSize = getSendBufferSize(_fd);
	}

	//
	// Get property for buffer size and check for sanity.
	//
	Int sizeRequested = instance->properties()->getPropertyAsIntWithDefault(prop, dfltSize);
	if(sizeRequested < _udpOverhead)
	{
	    Warning out(_logger);
	    out << "Invalid " << prop << " value of " << sizeRequested << " adjusted to " << dfltSize;
	    sizeRequested = dfltSize;
	}

	//
	// Ice.MessageSizeMax overrides UDP buffer sizes if Ice.MessageSizeMax + _udpOverhead is less.
	//
	size_t messageSizeMax = instance->messageSizeMax();
	if(static_cast<size_t>(sizeRequested) > messageSizeMax + _udpOverhead)
	{
	    Warning out(_logger);
	    out << "UDP " << direction << " buffer size: requested size of " << sizeRequested << " adjusted to ";
	    sizeRequested = min(messageSizeMax + _udpOverhead, static_cast<size_t>(_maxPacketSize) + _udpOverhead);
	    out << sizeRequested << " (Ice.MessageSizeMax takes precendence)";
	}
	    
	if(sizeRequested != dfltSize)
	{
	    //
	    // Try to set the buffer size. The kernel will silently adjust
	    // the size to an acceptable value. Then read the size back to
	    // get the size that was actually set.
	    //
	    if(i == 0)
	    {
		setRecvBufferSize(_fd, sizeRequested);
		*addr = getRecvBufferSize(_fd);
	    }
	    else
	    {
		setSendBufferSize(_fd, sizeRequested);
		*addr = getSendBufferSize(_fd);
	    }

	    //
	    // Warn if the size that was set is less than the requested size.
	    //
	    if(*addr < sizeRequested)
	    {
		Warning out(_logger);
		out << "UDP " << direction << " buffer size: requested size of "
		    << sizeRequested << " adjusted to " << *addr;
	    }
	}
	else
	{
	    *addr = dfltSize;
	}
    }
}

//
// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
// to get the maximum payload.
//
const int IceInternal::UdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::UdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;
