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
#include <Ice/Logger.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

int
IceInternal::UdpTransceiver::fd()
{
    return _fd;
}

void
IceInternal::UdpTransceiver::close()
{
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	if (_sender)
	{
	    s << "stopping to send udp packets to " << toString();
	}
	else
	{
	    s << "stopping to receive udp packets at " << toString();
	}
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    int fd = _fd;
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
    assert(_sender);
    assert(buf.i == buf.b.begin());
    const int packetSize = 64 * 1024; // TODO: configurable
    assert(packetSize >= static_cast<int>(buf.b.size())); // TODO: exception

repeat:
    int ret = ::send(_fd, buf.b.begin(), buf.b.size(), 0);
    
    if (ret == SOCKET_ERROR)
    {
	if (interrupted())
	{
	    goto repeat;
	}

	throw SocketException(__FILE__, __LINE__);
    }

#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 3)
    {
	ostringstream s;
	s << "sent " << ret << " bytes via udp to " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	
    
    assert(ret == static_cast<int>(buf.b.size()));
    buf.i = buf.b.end();
}

void
IceInternal::UdpTransceiver::read(Buffer& buf, int)
{
    assert(!_sender);
    assert(buf.i == buf.b.begin());
    const int packetSize = 64 * 1024; // TODO: configurable
    assert(packetSize >= static_cast<int>(buf.b.size())); // TODO: exception
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:
    int ret = ::recv(_fd, buf.b.begin(), packetSize, 0);
    
    if (ret == SOCKET_ERROR)
    {
	if (interrupted())
	{
	    goto repeat;
	}
	
	throw SocketException(__FILE__, __LINE__);
    }
    
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 3)
    {
	ostringstream s;
	s << "received " << ret << " bytes via udp at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    buf.b.resize(ret);
    buf.i = buf.b.end();
}

string
IceInternal::UdpTransceiver::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::UdpTransceiver::equivalent(const string& host, int port) const
{
    if (_sender)
    {
	return false;
    }

    struct sockaddr_in addr;
    getAddress(host.c_str(), port, addr);
    if (addr.sin_addr.s_addr == htonl(INADDR_LOOPBACK))
    {
	return port == ntohs(_addr.sin_port);
    }

    struct sockaddr_in localAddr;
    getLocalAddress(ntohs(_addr.sin_port), localAddr);
    return memcmp(&addr, &localAddr, sizeof(struct sockaddr_in)) == 0;    
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _sender(true)
{
#ifndef ICE_NO_TRACE
    _traceLevels = _instance->traceLevels();
    _logger = _instance->logger();
#endif

    try
    {
	getAddress(host.c_str(), port, _addr);

	_fd = createSocket(true);
	doConnect(_fd, _addr, -1);
	
#ifndef ICE_NO_TRACE
	if (_traceLevels->network >= 1)
	{
	    ostringstream s;
	    s << "starting to send udp packets to " << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}
#endif	
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, int port) :
    _instance(instance),
    _sender(false)
{
#ifndef ICE_NO_TRACE
    _traceLevels = _instance->traceLevels();
    _logger = _instance->logger();
#endif

    try
    {
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(port);
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	_fd = createSocket(true);
	doBind(_fd, _addr);
	    
#ifndef ICE_NO_TRACE
	if (_traceLevels->network >= 1)
	{
	    ostringstream s;
	    s << "starting to receive udp packets at " << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}
#endif	
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

IceInternal::UdpTransceiver::~UdpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
