// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef WIN32
#   pragma warning(disable:4786)
#endif

#include <Ice/SslSystemInternal.h>
#include <Ice/SslAcceptor.h>
#include <Ice/SslTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Network.h>
#include <Ice/Properties.h>
#include <Ice/Exception.h>
#include <Ice/SecurityException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

using std::string;
using std::ostringstream;
using IceSSL::Connection;
using IceSSL::SystemInternalPtr;

SOCKET
IceInternal::SslAcceptor::fd()
{
    return _fd;
}

void
IceInternal::SslAcceptor::close()
{
    if (_traceLevels->network >= 1)
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
IceInternal::SslAcceptor::shutdown()
{
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down accepting ssl connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    ::shutdown(_fd, SHUT_RD); // Shutdown socket for reading
}

void
IceInternal::SslAcceptor::listen()
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

    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "accepting ssl connections at " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
}

TransceiverPtr
IceInternal::SslAcceptor::accept(int timeout)
{
    SOCKET fd = doAccept(_fd, timeout);

    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "accepted ssl connection\n" << fdToString(fd);
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    // Get an instance of the SslSystem
    SystemInternalPtr sslSystem = _instance->getSslSystem();
    assert(sslSystem != 0);

    IceSSL::ConnectionPtr connection = sslSystem->createConnection(IceSSL::Server, fd);
    TransceiverPtr transPtr = new SslTransceiver(_instance, fd, connection);

    return transPtr;
}

string
IceInternal::SslAcceptor::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::SslAcceptor::equivalent(const string& host, int port) const
{
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

int
IceInternal::SslAcceptor::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceInternal::SslAcceptor::SslAcceptor(const InstancePtr& instance, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _backlog(0)
{
    if (_backlog <= 0)
    {
        _backlog = 5;
    }

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

IceInternal::SslAcceptor::~SslAcceptor()
{
    assert(_fd == INVALID_SOCKET);
}
