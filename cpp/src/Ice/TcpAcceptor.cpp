// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpAcceptor.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceInternal::TcpAcceptor::fd()
{
    return _fd;
}

void
IceInternal::TcpAcceptor::close()
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

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "accepting tcp connections at " << toString();
    }
}

TransceiverPtr
IceInternal::TcpAcceptor::accept(int timeout)
{
    SOCKET fd = doAccept(_fd, timeout);
    setBlock(fd, false);

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "accepted tcp connection\n" << fdToString(fd);
    }

    return new TcpTransceiver(_instance, fd);
}

void
IceInternal::TcpAcceptor::connectToSelf()
{
    SOCKET fd = createSocket(false);
    setBlock(fd, false);
    doConnect(fd, _addr, -1);
    closeSocket(fd);
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
    getAddress(host, port, addr);
    return compareAddress(addr, _addr);
}

int
IceInternal::TcpAcceptor::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceInternal::TcpAcceptor::TcpAcceptor(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _backlog(0)
{
    if(_backlog <= 0)
    {
        _backlog = 5;
    }

    try
    {
	_fd = createSocket(false);
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

IceInternal::TcpAcceptor::~TcpAcceptor()
{
    assert(_fd == INVALID_SOCKET);
}
