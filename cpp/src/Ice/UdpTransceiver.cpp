// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
IceInternal::UdpTransceiver::shutdownWrite()
{
}

void
IceInternal::UdpTransceiver::shutdownReadWrite()
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down udp connection for reading and writing\n" << toString();
    }

    //
    // Set a flag and then shutdown the socket in order to wake a thread that is
    // blocked in read().
    //
    IceUtil::Mutex::Lock sync(_shutdownReadWriteMutex);
    _shutdownReadWrite = true;

    assert(_fd != INVALID_SOCKET);
    shutdownSocketReadWrite(_fd);

#if defined(_WIN32) || defined(__sun) || defined(__hppa) || defined(_AIX)
    //
    // This is required to unblock the select call when using thread per connection.
    //
    SOCKET fd = createSocket(true);
    setBlock(fd, false);
    doConnect(fd, _addr, -1);
    ::send(fd, "", 1, 0);
    closeSocket(fd);
#endif
}

void
IceInternal::UdpTransceiver::write(Buffer& buf, int)
{
    assert(buf.i == buf.b.begin());
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
	//
	// We don't log a warning here because the client gets an exception anyway.
	//
	throw Ice::DatagramLimitException(__FILE__, __LINE__);
    }

repeat:

    assert(_fd != INVALID_SOCKET);
#ifdef _WIN32
    ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), 
			 static_cast<int>(buf.b.size()), 0);
#else
    ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), 
			 buf.b.size(), 0);
#endif    

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
	    int rs = ::select(static_cast<int>(_fd + 1), 0, &_wFdSet, 0, 0);
	    
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
	_stats->bytesSent(type(), static_cast<Int>(ret));
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
	//
	// We log a warning here because this is the server side -- without the
	// the warning, there would only be silence.
	//
	if(_warn)
	{
	    Warning out(_logger);
	    out << "DatagramLimitException: maximum size of " << packetSize << " exceeded";
	}
	throw Ice::DatagramLimitException(__FILE__, __LINE__);
    }
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:

    //
    // Check the shutdown flag.
    //
    {
	IceUtil::Mutex::Lock sync(_shutdownReadWriteMutex);
	if(_shutdownReadWrite)
	{
	    throw ConnectionLostException(__FILE__, __LINE__);
	}
    }

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
	    int rs = ::select(static_cast<int>(_fd + 1), &_rFdSet, 0, 0, 0);

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
	    if(_warn)
	    {
		Warning out(_logger);
		out << "DatagramLimitException: maximum size of " << packetSize << " exceeded";
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
	_stats->bytesReceived(type(), static_cast<Int>(ret));
    }

    buf.b.resize(ret);
    buf.i = buf.b.end();
}

string
IceInternal::UdpTransceiver::type() const
{
    return "udp";
}

string
IceInternal::UdpTransceiver::toString() const
{
    return fdToString(_fd);
}

void
IceInternal::UdpTransceiver::initialize(int)
{
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
    _stats(instance->stats()),
    _incoming(false),
    _connect(true),
    _warn(instance->properties()->getPropertyAsInt("Ice.Warn.Datagrams") > 0),
    _shutdownReadWrite(false)
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
    _incoming(true),
    _connect(connect),
    _warn(instance->properties()->getPropertyAsInt("Ice.Warn.Datagrams") > 0),
    _shutdownReadWrite(false)
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
	    _rcvSize = dfltSize;
	}
	else
	{
	    direction = "send";
	    prop = "Ice.UDP.SndSize";
	    addr = &_sndSize;
	    dfltSize = getSendBufferSize(_fd);
	    _sndSize = dfltSize;
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
	    sizeRequested = min(static_cast<int>(messageSizeMax), _maxPacketSize) + _udpOverhead;
	    out << sizeRequested << " (Ice.MessageSizeMax takes precedence)";
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
    }
}

//
// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
// to get the maximum payload.
//
const int IceInternal::UdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::UdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;
