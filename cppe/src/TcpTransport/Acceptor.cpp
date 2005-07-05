// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Acceptor.h>
#include <IceE/Transceiver.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Network.h>
#include <IceE/Exception.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(Acceptor* p) { p->__incRef(); }
void IceEInternal::decRef(Acceptor* p) { p->__decRef(); }

SOCKET
IceEInternal::Acceptor::fd()
{
    return _fd;
}

void
IceEInternal::Acceptor::close()
{
    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "stopping to accept tcp connections at " << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

void
IceEInternal::Acceptor::listen()
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

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "accepting tcp connections at " << toString();
    }
}

TransceiverPtr
IceEInternal::Acceptor::accept(int timeout)
{
    SOCKET fd = doAccept(_fd, timeout);
    setBlock(fd, false);

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "accepted tcp connection\n" << fdToString(fd);
    }

    return new Transceiver(_instance, fd);
}

void
IceEInternal::Acceptor::connectToSelf()
{
    SOCKET fd = createSocket();
    setBlock(fd, false);
    doConnect(fd, _addr, -1);
    closeSocket(fd);
}

string
IceEInternal::Acceptor::toString() const
{
    return addrToString(_addr);
}

bool
IceEInternal::Acceptor::equivalent(const string& host, int port) const
{
    struct sockaddr_in addr;
    getAddress(host, port, addr);
    return compareAddress(addr, _addr);
}

int
IceEInternal::Acceptor::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceEInternal::Acceptor::Acceptor(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _backlog(0)
{
    if(_backlog <= 0)
    {
        _backlog = 5;
    }

    try
    {
	_fd = createSocket();
	setBlock(_fd, false);
	getAddress(host, port, _addr);
	if(_traceLevels->network >= 2)
	{
	    Trace out(_logger, _traceLevels->networkCat);
	    out << "attempting to bind to tcp socket " << toString();
	}
	doBind(_fd, _addr);
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

IceEInternal::Acceptor::~Acceptor()
{
    assert(_fd == INVALID_SOCKET);
}
