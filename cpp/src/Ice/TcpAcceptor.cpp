// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/TcpAcceptor.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

int
IceInternal::TcpAcceptor::fd()
{
    return _fd;
}

void
IceInternal::TcpAcceptor::close()
{
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "stopping to accept connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    int fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

void
IceInternal::TcpAcceptor::shutdown()
{
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down accepting connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    ::shutdown(_fd, SHUT_RD); // Shutdown socket for reading
}

void
IceInternal::TcpAcceptor::listen()
{
    try
    {
	doListen(_fd, _backlog);
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }

#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "accepting connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif
}

TransceiverPtr
IceInternal::TcpAcceptor::accept(int timeout)
{
    int fd = doAccept(_fd, timeout);
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "accepted connection\n" << fdToString(fd);
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	
    return new TcpTransceiver(_instance, fd);
}

string
IceInternal::TcpAcceptor::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::TcpAcceptor::equivalent(const string& host, int port) const
{
    struct sockaddr_in addr;
    getAddress(host.c_str(), port, addr);
    if (addr.sin_addr.s_addr == htonl(INADDR_LOOPBACK))
	return port == ntohs(_addr.sin_port);

    struct sockaddr_in localAddr;
    getLocalAddress(ntohs(_addr.sin_port), localAddr);
    return memcmp(&addr, &localAddr, sizeof(struct sockaddr_in)) == 0;    
}

IceInternal::TcpAcceptor::TcpAcceptor(const InstancePtr& instance, int port) :
    _instance(instance),
    _backlog(0)
{
#ifndef ICE_NO_TRACE
    _traceLevels = _instance->traceLevels();
    _logger = _instance->logger();
#endif

    if (_backlog <= 0)
        _backlog = 5;

    try
    {
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(port);
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
	_fd = createSocket(false);
	doBind(_fd, _addr);
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

IceInternal::TcpAcceptor::~TcpAcceptor()
{
    assert(_fd == INVALID_SOCKET);
}
