// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/SslConnection.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/PluginBaseI.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using IceSSL::ConnectionPtr;

SOCKET
IceSSL::SslTransceiver::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceSSL::SslTransceiver::close()
{
    if(_traceLevels->network >= 1)
    {
   	Trace out(_logger, _traceLevels->networkCat);
	out << "closing ssl connection\n" << toString();
    }

    try
    {
	int shutdown = 0;
	int numRetries = 100;
	int retries = -numRetries;
	do
	{
	    shutdown = _sslConnection->shutdown();
	    retries++;
	}
	while((shutdown == 0) && (retries < 0));
    }
    catch(...)
    {
 	assert(_fd != INVALID_SOCKET);
	closeSocket(_fd);
	_fd = INVALID_SOCKET;
	throw;
    }

    assert(_fd != INVALID_SOCKET);
    closeSocket(_fd);
    _fd = INVALID_SOCKET;
}

void
IceSSL::SslTransceiver::shutdown()
{
    if(_traceLevels->network >= 2)
    {
 	Trace out(_logger, _traceLevels->networkCat);
	out << "shutting down ssl connection\n" << toString();
    }

    int shutdown = 0;
    int numRetries = 100;
    int retries = -numRetries;
    do
    {
        shutdown = _sslConnection->shutdown();
        retries++;
    }
    while((shutdown == 0) && (retries < 0));

    assert(_fd != INVALID_SOCKET);
    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing
}

void
IceSSL::SslTransceiver::write(Buffer& buf, int timeout)
{
    assert(_fd != INVALID_SOCKET);
    _sslConnection->write(buf, timeout);
}

void
IceSSL::SslTransceiver::read(Buffer& buf, int timeout)
{
    assert(_fd != INVALID_SOCKET);
    if(!_sslConnection->read(buf, timeout))
    {
        if(_traceLevels->security >= IceSSL::SECURITY_WARNINGS)
        { 
	    Trace out(_logger, _traceLevels->securityCat);
            out << "WRN reading from ssl connection returns no bytes";
        }
    }
}

string
IceSSL::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

IceSSL::SslTransceiver::SslTransceiver(const PluginBaseIPtr& plugin,
                                       SOCKET fd,
                                       const ConnectionPtr& sslConnection) :
    _traceLevels(plugin->getTraceLevels()),
    _logger(plugin->getLogger()),
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
