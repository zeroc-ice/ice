// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/OpenSSL.h>
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
using IceSSL::ConnectionPtr;

SOCKET
IceInternal::SslTransceiver::fd()
{
    return _fd;
}

void
IceInternal::SslTransceiver::close()
{
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
}

void
IceInternal::SslTransceiver::shutdown()
{
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    _sslConnection->shutdown();
    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing
}

void
IceInternal::SslTransceiver::write(Buffer& buf, int timeout)
{
    _sslConnection->write(buf, timeout);
}

void
IceInternal::SslTransceiver::read(Buffer& buf, int timeout)
{
    if (!_sslConnection->read(buf, timeout))
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN Connection::read() returning no bytes read.");
        }
    }
}

string
IceInternal::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

IceInternal::SslTransceiver::SslTransceiver(const InstancePtr& instance,
                                            SOCKET fd,
                                            const ConnectionPtr& sslConnection) :
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
}
