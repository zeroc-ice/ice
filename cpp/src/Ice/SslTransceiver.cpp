// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Security.h>
#include <Ice/SslConnection.h>
#include <Ice/SslTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>
#include <Ice/SslException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using IceSecurity::SecurityException;
using IceSecurity::Ssl::InitException;
using IceSecurity::Ssl::ReInitException;
using IceSecurity::Ssl::ShutdownException;

int
IceInternal::SslTransceiver::fd()
{
    return _fd;
}

void
IceInternal::SslTransceiver::close()
{
    METHOD_INV("SslTransceiver::close()");

    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "closing ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    int fd = _fd;
    cleanUpSSL();
    _fd = INVALID_SOCKET;
    ::shutdown(fd, SHUT_RDWR); // helps to unblock threads in recv()
    closeSocket(fd);

    METHOD_RET("SslTransceiver::close()");
}

void
IceInternal::SslTransceiver::shutdown()
{
    METHOD_INV("SslTransceiver::shutdown()");

    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing

    METHOD_RET("SslTransceiver::shutdown()");
}

void
IceInternal::SslTransceiver::write(Buffer& buf, int timeout)
{
    METHOD_INV("SslTransceiver::write()")
    _sslConnection->write(buf, timeout);
    METHOD_RET("SslTransceiver::write()");
}

void
IceInternal::SslTransceiver::read(Buffer& buf, int timeout)
{
    METHOD_INV("SslTransceiver::read()");

    if (!_sslConnection->read(buf, timeout))
    {
        ConnectionLostException clEx(__FILE__, __LINE__);
        clEx.error = 0;
        throw clEx;
    }

    METHOD_RET("SslTransceiver::read()");
}

string
IceInternal::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

IceInternal::SslTransceiver::SslTransceiver(const InstancePtr& instance, int fd, Connection* sslConnection) :
    _instance(instance),
    _fd(fd),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _sslConnection(sslConnection)
{
    assert(sslConnection != 0);

    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::SslTransceiver::~SslTransceiver()
{
    assert(_fd == INVALID_SOCKET);

    cleanUpSSL();
}

void
IceInternal::SslTransceiver::cleanUpSSL()
{
    if (_sslConnection != 0)
    {
        _sslConnection->shutdown();
        delete _sslConnection;
        _sslConnection = 0;
    }
}
