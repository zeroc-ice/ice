// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifdef SSL_EXTENSION
#include <Ice/SslConnection.h>
#endif

#include <Ice/SslTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

#ifdef SSL_EXTENSION
#include <Ice/Security.h>
#include <Ice/SslException.h>
#include <sstream>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifdef SSL_EXTENSION
using IceSecurity::SecurityException;
using IceSecurity::Ssl::InitException;
using IceSecurity::Ssl::ReInitException;
using IceSecurity::Ssl::ShutdownException;
#endif

int
IceInternal::SslTransceiver::fd()
{
    return _fd;
}

void
IceInternal::SslTransceiver::close()
{
#ifndef SSL_EXTENSION
    METHOD_INV("SslTransceiver::close()");
#endif

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

#ifndef SSL_EXTENSION
    METHOD_RET("SslTransceiver::close()");
#endif
}

void
IceInternal::SslTransceiver::shutdown()
{
#ifndef SSL_EXTENSION
    METHOD_INV("SslTransceiver::shutdown()");
#endif

    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "shutting down ssl connection\n" << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    ::shutdown(_fd, SHUT_WR); // Shutdown socket for writing

#ifndef SSL_EXTENSION
    METHOD_RET("SslTransceiver::shutdown()");
#endif
}

void
IceInternal::SslTransceiver::write(Buffer& buf, int timeout)
{
#ifndef SSL_EXTENSION
    METHOD_INV("SslTransceiver::write()")
    int packetSize = buf.b.end() - buf.i;
    
#ifdef WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if (packetSize > 64 * 1024)
    {
	packetSize = 64 * 1024;
    }
#endif

    while (buf.i != buf.b.end())
    {
	int ret = ::send(_fd, buf.i, packetSize, 0);

	if (ret == 0)
	{
	    ConnectionLostException ex(__FILE__, __LINE__);
	    ex.error = 0;
	    throw ex;
	}

	if (ret == SOCKET_ERROR)
	{
	    if (interrupted())
	    {
		continue;
	    }

	    if (noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if (wouldBlock())
	    {
		int fd = _fd; // Copy fd, in case another thread calls close()
		if (fd != -1)
		{
		repeatSelect:
		    int ret;
		    FD_SET(fd, &_wFdSet);
		    if (timeout >= 0)
		    {
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
			ret = ::select(fd + 1, 0, &_wFdSet, 0, &tv);
		    }
		    else
		    {
			ret = ::select(fd + 1, 0, &_wFdSet, 0, 0);
		    }
		    
		    if (ret == SOCKET_ERROR)
		    {
			if (interrupted())
			{
			    goto repeatSelect;
			}
			
			SocketException ex(__FILE__, __LINE__);
			ex.error = getSocketErrno();
			throw ex;
		    }
		    
		    if (ret == 0)
		    {
			throw TimeoutException(__FILE__, __LINE__);
		    }
		}
		
		continue;
	    }
	    
	    if (connectionLost())
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    else
	    {
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	}

	if (_traceLevels->network >= 3)
	{
	    ostringstream s;
	    s << "sent " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}

	buf.i += ret;

	if (packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }

    METHOD_INV("SslTransceiver::write()");
#else

    _sslConnection->write(buf, timeout);

#endif
}

void
IceInternal::SslTransceiver::read(Buffer& buf, int timeout)
{
#ifndef SSL_EXTENSION
    METHOD_INV("SslTransceiver::read()");

    int packetSize = buf.b.end() - buf.i;
    
    while (buf.i != buf.b.end())
    {
	int ret = ::recv(_fd, buf.i, packetSize, 0);

	if (ret == 0)
	{
	    ConnectionLostException ex(__FILE__, __LINE__);
	    ex.error = 0;
	    throw ex;
	}

	if (ret == SOCKET_ERROR)
	{
	    if (interrupted())
	    {
		continue;
	    }
	    
	    if (noBuffers() && packetSize > 1024)
	    {
		packetSize /= 2;
		continue;
	    }

	    if (wouldBlock())
	    {
		int fd = _fd; // Copy fd, in case another thread calls close()
		if (fd != -1)
		{
		repeatSelect:
		    int ret;
		    FD_SET(fd, &_rFdSet);
		    if (timeout >= 0)
		    {
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
			ret = ::select(fd + 1, &_rFdSet, 0, 0, &tv);
		    }
		    else
		    {
			ret = ::select(fd + 1, &_rFdSet, 0, 0, 0);
		    }
		    
		    if (ret == SOCKET_ERROR)
		    {
			if (interrupted())
			{
			    goto repeatSelect;
			}
			
			SocketException ex(__FILE__, __LINE__);
			ex.error = getSocketErrno();
			throw ex;
		    }
		    
		    if (ret == 0)
		    {
			throw TimeoutException(__FILE__, __LINE__);
		    }
		}
	    
		continue;
	    }
	    
	    if (connectionLost())
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	    else
	    {
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
	}

	if (_traceLevels->network >= 3)
	{
	    ostringstream s;
	    s << "received " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
	    _logger->trace(_traceLevels->networkCat, s.str());
	}

	buf.i += ret;

	if (packetSize > buf.b.end() - buf.i)
	{
	    packetSize = buf.b.end() - buf.i;
	}
    }

    METHOD_INV("SslTransceiver::read()");
#else

    if (!_sslConnection->read(buf, timeout))
    {
        ConnectionLostException clEx(__FILE__, __LINE__);
        clEx.error = 0;
        throw clEx;
    }

#endif
}

string
IceInternal::SslTransceiver::toString() const
{
    return fdToString(_fd);
}

#ifndef SSL_EXTENSION
IceInternal::SslTransceiver::SslTransceiver(const InstancePtr& instance, int fd) :
    _instance(instance),
    _fd(fd),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
#else
IceInternal::SslTransceiver::SslTransceiver(const InstancePtr& instance, int fd, Connection* sslConnection) :
    _instance(instance),
    _fd(fd),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _sslConnection(sslConnection)
#endif
{
#ifdef SSL_EXTENSION
    assert(sslConnection != 0);
#endif

    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
}

IceInternal::SslTransceiver::~SslTransceiver()
{
    assert(_fd == INVALID_SOCKET);

#ifdef SSL_EXTENSION
    cleanUpSSL();
#endif
}

void
IceInternal::SslTransceiver::cleanUpSSL()
{
#ifdef SSL_EXTENSION
    if (_sslConnection != 0)
    {
        _sslConnection->shutdown();
        delete _sslConnection;
        _sslConnection = 0;
    }
#endif
}
