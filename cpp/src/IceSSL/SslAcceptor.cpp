// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>
#include <Ice/Network.h>

#include <IceSSL/PluginBaseI.h>
#include <IceSSL/SslAcceptor.h>
#include <IceSSL/SslTransceiver.h>
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
	ostringstream s;
	s << "stopping to accept ssl connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
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
	ostringstream s;
	s << "accepting ssl connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
}

TransceiverPtr
IceSSL::SslAcceptor::accept(int timeout)
{
    SOCKET fd = doAccept(_fd, timeout);
    setBlock(fd, false);

    if(_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "accepted ssl connection\n" << fdToString(fd);
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    IceSSL::ConnectionPtr connection = _plugin->createConnection(IceSSL::Server, fd);
    TransceiverPtr transPtr = new SslTransceiver(_plugin, fd, connection);

    return transPtr;
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

IceSSL::SslAcceptor::SslAcceptor(const PluginBaseIPtr& plugin, const string& host, int port) :
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
