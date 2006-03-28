// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TransceiverI.h>
#include <Instance.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

#include <openssl/err.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

SOCKET
IceSSL::TransceiverI::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceSSL::TransceiverI::close()
{
    if(_instance->networkTraceLevel() >= 1)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "closing ssl connection\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    SSL_free(_ssl);
    _ssl = 0;
    _fd = INVALID_SOCKET;
}

void
IceSSL::TransceiverI::shutdownWrite()
{
    if(_instance->networkTraceLevel() >= 2)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "shutting down ssl connection for writing\n" << toString();
    }

    int err = SSL_shutdown(_ssl);
    if(err < 0)
    {
	Warning out(_logger);
	out << "IceSSL: failure while performing SSL shutdown:\n" << _instance->sslErrors();
    }
    ERR_clear_error();

    assert(_fd != INVALID_SOCKET);
    IceInternal::shutdownSocketWrite(_fd);
}

void
IceSSL::TransceiverI::shutdownReadWrite()
{
    if(_instance->networkTraceLevel() >= 2)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "shutting down ssl connection for reading and writing\n" << toString();
    }

    int err = SSL_shutdown(_ssl);
    if(err < 0)
    {
	Warning out(_logger);
	out << "IceSSL: failure while performing SSL shutdown:\n" << _instance->sslErrors();
    }
    ERR_clear_error();

    assert(_fd != INVALID_SOCKET);
    IceInternal::shutdownSocketReadWrite(_fd);
}

void
IceSSL::TransceiverI::write(IceInternal::Buffer& buf, int timeout)
{
    IceInternal::Buffer::Container::difference_type packetSize = 
        static_cast<IceInternal::Buffer::Container::difference_type>(buf.b.end() - buf.i);
    
#ifdef _WIN32
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(_isPeerLocal && packetSize > 64 * 1024)
    { 
   	packetSize = 64 * 1024;
    }
#endif

    while(buf.i != buf.b.end())
    {
	assert(_fd != INVALID_SOCKET);
	int ret = SSL_write(_ssl, reinterpret_cast<const void*>(&*buf.i), packetSize);

	if(ret <= 0)
	{
	    switch(SSL_get_error(_ssl, ret))
	    {
	    case SSL_ERROR_NONE:
		assert(false);
		break;
	    case SSL_ERROR_ZERO_RETURN:
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = IceInternal::getSocketErrno();
		throw ex;
	    }
	    case SSL_ERROR_WANT_READ:
	    {
		if(!selectRead(_fd, timeout))
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
		continue;
	    }
	    case SSL_ERROR_WANT_WRITE:
	    {
		if(!selectWrite(_fd, timeout))
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
		continue;
	    }
	    case SSL_ERROR_SYSCALL:
	    {
		if(ret == -1)
		{
		    if(IceInternal::interrupted())
		    {
			continue;
		    }

		    if(IceInternal::noBuffers() && packetSize > 1024)
		    {
			packetSize /= 2;
			continue;
		    }

		    if(IceInternal::wouldBlock())
		    {
			if(SSL_want_read(_ssl))
			{
			    if(!selectRead(_fd, timeout))
			    {
				throw TimeoutException(__FILE__, __LINE__);
			    }
			}
			else if(SSL_want_write(_ssl))
			{
			    if(!selectWrite(_fd, timeout))
			    {
				throw TimeoutException(__FILE__, __LINE__);
			    }
			}

			continue;
		    }

		    if(IceInternal::connectionLost())
		    {
			ConnectionLostException ex(__FILE__, __LINE__);
			ex.error = IceInternal::getSocketErrno();
			throw ex;
		    }
		}

		if(ret == 0)
		{
		    ConnectionLostException ex(__FILE__, __LINE__);
		    ex.error = 0;
		    throw ex;
		}

		SocketException ex(__FILE__, __LINE__);
		ex.error = IceInternal::getSocketErrno();
		throw ex;
	    }
	    case SSL_ERROR_SSL:
	    {
		ProtocolException ex(__FILE__, __LINE__);
		ex.reason = "SSL protocol error during write:\n" + _instance->sslErrors();
		throw ex;
	    }
	    }
	}

	if(_instance->networkTraceLevel() >= 3)
	{
	    Trace out(_logger, _instance->networkTraceCategory());
	    out << "sent " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
	}

	if(_stats)
	{
	    _stats->bytesSent(type(), static_cast<Int>(ret));
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = static_cast<IceInternal::Buffer::Container::difference_type>(buf.b.end() - buf.i);
	}
    }
}

void
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, int timeout)
{
    IceInternal::Buffer::Container::difference_type packetSize = 
        static_cast<IceInternal::Buffer::Container::difference_type>(buf.b.end() - buf.i);
    
    while(buf.i != buf.b.end())
    {
	assert(_fd != INVALID_SOCKET);
	int ret = SSL_read(_ssl, reinterpret_cast<void*>(&*buf.i), packetSize);

	if(ret <= 0)
	{
	    switch(SSL_get_error(_ssl, ret))
	    {
	    case SSL_ERROR_NONE:
		assert(false);
		break;
	    case SSL_ERROR_ZERO_RETURN:
	    {
		//
		// If the connection is lost when reading data, we shut
		// down the write end of the socket. This helps to unblock
		// threads that are stuck in send() or select() while
		// sending data. Note: I don't really understand why
		// send() or select() sometimes don't detect a connection
		// loss. Therefore this helper to make them detect it.
		//
		//assert(_fd != INVALID_SOCKET);
		//shutdownSocketReadWrite(_fd);

		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = 0;
		throw ex;
	    }
	    case SSL_ERROR_WANT_READ:
	    {
		if(!selectRead(_fd, timeout))
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
		continue;
	    }
	    case SSL_ERROR_WANT_WRITE:
	    {
		if(!selectWrite(_fd, timeout))
		{
		    throw TimeoutException(__FILE__, __LINE__);
		}
		continue;
	    }
	    case SSL_ERROR_SYSCALL:
	    {
		if(ret == -1)
		{
		    if(IceInternal::interrupted())
		    {
			continue;
		    }

		    if(IceInternal::noBuffers() && packetSize > 1024)
		    {
			packetSize /= 2;
			continue;
		    }

		    if(IceInternal::wouldBlock())
		    {
			if(SSL_want_read(_ssl))
			{
			    if(!selectRead(_fd, timeout))
			    {
				throw TimeoutException(__FILE__, __LINE__);
			    }
			}
			else if(SSL_want_write(_ssl))
			{
			    if(!selectWrite(_fd, timeout))
			    {
				throw TimeoutException(__FILE__, __LINE__);
			    }
			}

			continue;
		    }

		    if(IceInternal::connectionLost())
		    {
			//
			// See the commment above about shutting down the
			// socket if the connection is lost while reading
			// data.
			//
			//assert(_fd != INVALID_SOCKET);
			//shutdownSocketReadWrite(_fd);

			ConnectionLostException ex(__FILE__, __LINE__);
			ex.error = IceInternal::getSocketErrno();
			throw ex;
		    }
		}

		if(ret == 0)
		{
		    ConnectionLostException ex(__FILE__, __LINE__);
		    ex.error = 0;
		    throw ex;
		}

		SocketException ex(__FILE__, __LINE__);
		ex.error = IceInternal::getSocketErrno();
		throw ex;
	    }
	    case SSL_ERROR_SSL:
	    {
		ProtocolException ex(__FILE__, __LINE__);
		ex.reason = "SSL protocol error during read:\n" + _instance->sslErrors();
		throw ex;
	    }
	    }
	}

	if(_instance->networkTraceLevel() >= 3)
	{
	    Trace out(_logger, _instance->networkTraceCategory());
	    out << "received " << ret << " of " << packetSize << " bytes via ssl\n" << toString();
	}

	if(_stats)
	{
	    _stats->bytesReceived(type(), static_cast<Int>(ret));
	}

	buf.i += ret;

	if(packetSize > buf.b.end() - buf.i)
	{
	    packetSize = static_cast<IceInternal::Buffer::Container::difference_type>(buf.b.end() - buf.i);
	}
    }
}

string
IceSSL::TransceiverI::type() const
{
    return "ssl";
}

string
IceSSL::TransceiverI::toString() const
{
    return _desc;
}

void
IceSSL::TransceiverI::initialize(int)
{
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance, SSL* ssl, SOCKET fd) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _stats(instance->communicator()->getStats()),
    _ssl(ssl),
    _fd(fd),
    _desc(IceInternal::fdToString(fd))
#ifdef _WIN32
    , _isPeerLocal(IceInternal::isPeerLocal(fd))
#endif
{
}

IceSSL::TransceiverI::~TransceiverI()
{
    assert(_fd == INVALID_SOCKET);
}
