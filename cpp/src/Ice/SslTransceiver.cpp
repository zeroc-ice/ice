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
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceInternal::SslTransceiver::fd()
{
    return _fd;
}

void
IceInternal::SslTransceiver::close()
{
    ICE_METHOD_INV("SslTransceiver::close()");

    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "closing ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    _sslConnection->shutdown();
    ::shutdown(fd, SHUT_RDWR); // helps to unblock threads in recv()
    closeSocket(fd);

    ICE_METHOD_RET("SslTransceiver::close()");
}

void
IceInternal::SslTransceiver::shutdown()
{
    ICE_METHOD_INV("SslTransceiver::shutdown()");

    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    _sslConnection->shutdown();
    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing

    ICE_METHOD_RET("SslTransceiver::shutdown()");
}

void
IceInternal::SslTransceiver::write(Buffer& buf, int timeout)
{
    ICE_METHOD_INV("SslTransceiver::write()")
    _sslConnection->write(buf, timeout);
    ICE_METHOD_RET("SslTransceiver::write()");
}

void
IceInternal::SslTransceiver::read(Buffer& buf, int timeout)
{
    ICE_METHOD_INV("SslTransceiver::read()");

    if (!_sslConnection->read(buf, timeout))
    {
        ICE_WARNING("Connection::read() returning no bytes read.");

        // TODO: Perhaps this should be a NoApplicationDataException ???
        // ICE_WARNING("Throwing ConnectionLostException.");
        // ConnectionLostException clEx(__FILE__, __LINE__);
        // clEx.error = 0;
        // throw clEx;
    }

    ICE_METHOD_RET("SslTransceiver::read()");
}

string
IceInternal::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

IceInternal::SslTransceiver::SslTransceiver(const InstancePtr& instance, SOCKET fd, Connection* sslConnection) :
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

    if (_sslConnection != 0)
    {
        delete _sslConnection;
        _sslConnection = 0;
    }
}
