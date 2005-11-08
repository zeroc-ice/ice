// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>

#include <IceSSL/OpenSSLPluginI.h>
#include <IceSSL/SslAcceptor.h>
#include <IceSSL/SslServerTransceiver.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceSSL::SslAcceptor::fd()
{
    return _fd;
}

void
IceSSL::SslAcceptor::close()
{
    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "stopping to accept ssl connections at " << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

void
IceSSL::SslAcceptor::listen()
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
	out << "accepting ssl connections at " << toString();
    }
}

TransceiverPtr
IceSSL::SslAcceptor::accept(int timeout)
{
    SOCKET fd = doAccept(_fd, timeout);
    setBlock(fd, false);

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "accepted ssl connection\n" << fdToString(fd);
    }

    return _plugin->createServerTransceiver(fd, timeout);
}

void
IceSSL::SslAcceptor::connectToSelf()
{
    SOCKET fd = createSocket(false);
    setBlock(fd, false);
    doConnect(fd, _addr, -1);
    closeSocket(fd);
}

string
IceSSL::SslAcceptor::toString() const
{
    return addrToString(_addr);
}

bool
IceSSL::SslAcceptor::equivalent(const string& host, int port) const
{
    struct sockaddr_in addr;
    getAddress(host, port, addr);
    return compareAddress(addr, _addr);
}

int
IceSSL::SslAcceptor::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceSSL::SslAcceptor::SslAcceptor(const OpenSSLPluginIPtr& plugin, const string& host, int port) :
    _plugin(plugin),
    _traceLevels(plugin->getTraceLevels()),
    _logger(plugin->getLogger()),
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
	    out << "attempting to bind to ssl socket " << toString();
	}
	doBind(_fd, _addr);
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

IceSSL::SslAcceptor::~SslAcceptor()
{
    assert(_fd == INVALID_SOCKET);
}
