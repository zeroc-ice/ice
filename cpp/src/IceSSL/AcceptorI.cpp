// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <AcceptorI.h>
#include <Instance.h>
#include <TransceiverI.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/Exception.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

SOCKET
IceSSL::AcceptorI::fd()
{
    return _fd;
}

void
IceSSL::AcceptorI::close()
{
    if(_instance->networkTraceLevel() >= 1)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "stopping to accept ssl connections at " << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    IceInternal::closeSocket(fd);
}

void
IceSSL::AcceptorI::listen()
{
    try
    {
	IceInternal::doListen(_fd, _backlog);
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }

    if(_instance->networkTraceLevel() >= 1)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "accepting ssl connections at " << toString();
    }
}

IceInternal::TransceiverPtr
IceSSL::AcceptorI::accept(int timeout)
{
    //
    // The plugin may not be fully initialized.
    //
    ContextPtr ctx = _instance->context();

    SOCKET fd = IceInternal::doAccept(_fd, timeout);
    IceInternal::setBlock(fd, false);

    //
    // Get a description of the remote address in case we need it later.
    //
    struct sockaddr_in remoteAddr;
    string desc;
    if(IceInternal::fdToRemoteAddress(fd, remoteAddr))
    {
	desc = IceInternal::addrToString(remoteAddr);
    }

    BIO* bio = BIO_new_socket(fd, BIO_CLOSE);
    if(!bio)
    {
	IceInternal::closeSocketNoThrow(fd);
	SecurityException ex(__FILE__, __LINE__);
	ex.reason = "openssl failure";
	throw ex;
    }

    SSL* ssl = SSL_new(ctx->ctx());
    if(!ssl)
    {
	BIO_free(bio); // Also closes the socket.
	SecurityException ex(__FILE__, __LINE__);
	ex.reason = "openssl failure";
	throw ex;
    }
    SSL_set_bio(ssl, bio, bio);

    if(_instance->networkTraceLevel() >= 2)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "trying to validate incoming ssl connection\n" << IceInternal::fdToString(fd);
    }

    // TODO: The timeout is 0 when called by the thread pool.
    // Make this configurable?
    if(timeout == 0)
    {
	timeout = -1;
    }

    try
    {
	do
	{
	    int ret = SSL_accept(ssl);
	    switch(SSL_get_error(ssl, ret))
	    {
	    case SSL_ERROR_NONE:
		assert(SSL_is_init_finished(ssl));
		break;
	    case SSL_ERROR_ZERO_RETURN:
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = IceInternal::getSocketErrno();
		throw ex;
	    }
	    case SSL_ERROR_WANT_READ:
	    {
		if(!selectRead(fd, timeout))
		{
		    throw ConnectTimeoutException(__FILE__, __LINE__);
		}
		break;
	    }
	    case SSL_ERROR_WANT_WRITE:
	    {
		if(!selectWrite(fd, timeout))
		{
		    throw ConnectTimeoutException(__FILE__, __LINE__);
		}
		break;
	    }
	    case SSL_ERROR_SYSCALL:
	    {
		if(ret == -1)
		{
		    if(IceInternal::interrupted())
		    {
			break;
		    }

		    if(IceInternal::wouldBlock())
		    {
			if(SSL_want_read(ssl))
			{
			    if(!selectRead(fd, timeout))
			    {
				throw ConnectTimeoutException(__FILE__, __LINE__);
			    }
			}
			else if(SSL_want_write(ssl))
			{
			    if(!selectWrite(fd, timeout))
			    {
				throw ConnectTimeoutException(__FILE__, __LINE__);
			    }
			}

			break;
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
		ex.reason = "SSL error occurred for new incoming connection:\nremote address = " + desc + "\n" +
		    _instance->sslErrors();
		throw ex;
	    }
	    }
	}
	while(!SSL_is_init_finished(ssl));

	_instance->context()->verifyPeer(ssl, "", true);
    }
    catch(...)
    {
	SSL_free(ssl);
	throw;
    }

    if(_instance->networkTraceLevel() >= 1)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "accepted ssl connection\n" << IceInternal::fdToString(fd);
    }

    if(_instance->securityTraceLevel() >= 1)
    {
	_instance->context()->traceConnection(ssl, true);
    }

    return new TransceiverI(_instance, ssl, fd);
}

void
IceSSL::AcceptorI::connectToSelf()
{
    SOCKET fd = IceInternal::createSocket(false);
    IceInternal::setBlock(fd, false);
    IceInternal::doConnect(fd, _addr, -1);
    IceInternal::closeSocket(fd);
}

string
IceSSL::AcceptorI::toString() const
{
    return IceInternal::addrToString(_addr);
}

bool
IceSSL::AcceptorI::equivalent(const string& host, int port) const
{
    struct sockaddr_in addr;
    IceInternal::getAddress(host, port, addr);
    return IceInternal::compareAddress(addr, _addr);
}

int
IceSSL::AcceptorI::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceSSL::AcceptorI::AcceptorI(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _backlog(0)
{
    if(_backlog <= 0)
    {
        _backlog = 5;
    }

    try
    {
	_fd = IceInternal::createSocket(false);
	IceInternal::setBlock(_fd, false);
	IceInternal::getAddress(host, port, _addr);
	if(_instance->networkTraceLevel() >= 2)
	{
	    Trace out(_logger, _instance->networkTraceCategory());
	    out << "attempting to bind to ssl socket " << toString();
	}
	IceInternal::doBind(_fd, _addr);
    }
    catch(...)
    {
	_fd = INVALID_SOCKET;
	throw;
    }
}

IceSSL::AcceptorI::~AcceptorI()
{
    assert(_fd == INVALID_SOCKET);
}
