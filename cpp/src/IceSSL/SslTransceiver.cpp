// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/OpenSSL.h>
#include <IceSSL/SslConnection.h>
#include <IceSSL/SslTransceiver.h>
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
IceSSL::SslTransceiver::fd()
{
    return _fd;
}

void
IceSSL::SslTransceiver::close()
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
IceSSL::SslTransceiver::shutdown()
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
IceSSL::SslTransceiver::write(Buffer& buf, int timeout)
{
    _sslConnection->write(buf, timeout);
}

void
IceSSL::SslTransceiver::read(Buffer& buf, int timeout)
{
    if (!_sslConnection->read(buf, timeout))
    {
        if (_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
            _logger->trace(_traceLevels->securityCat, "WRN reading from ssl connection returns no bytes");
        }
    }
}

string
IceSSL::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

IceSSL::SslTransceiver::SslTransceiver(const InstancePtr& instance,
                                       SOCKET fd,
                                       const ConnectionPtr& sslConnection) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _fd(fd),
    _sslConnection(sslConnection)
{
    assert(sslConnection != 0);

    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceSSL::SslTransceiver::~SslTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}
