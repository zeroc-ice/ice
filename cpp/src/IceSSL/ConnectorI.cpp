// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/ConnectorI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/TransceiverI.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect(int timeout)
{
    //
    // The plugin may not be initialized.
    //
    if(!_instance->context())
    {
	PluginInitializationException ex(__FILE__, __LINE__);
	ex.reason = "IceSSL: plugin is not initialized";
	throw ex;
    }

    if(_instance->networkTraceLevel() >= 2)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "trying to establish ssl connection to " << toString();
    }

    SOCKET fd = IceInternal::createSocket(false);
    IceInternal::setBlock(fd, false);
    IceInternal::doConnect(fd, _addr, timeout);

    // This static_cast is necessary due to 64bit windows. There SOCKET is a non-int type.
    BIO* bio = BIO_new_socket(static_cast<int>(fd), BIO_CLOSE);
    if(!bio)
    {
	IceInternal::closeSocketNoThrow(fd);
	SecurityException ex(__FILE__, __LINE__);
	ex.reason = "openssl failure";
	throw ex;
    }

    SSL* ssl = SSL_new(_instance->context());
    if(!ssl)
    {
	BIO_free(bio); // Also closes the socket.
	SecurityException ex(__FILE__, __LINE__);
	ex.reason = "openssl failure";
	throw ex;
    }
    SSL_set_bio(ssl, bio, bio);

    try
    {
	do
	{
	    int result = SSL_connect(ssl);
	    switch(SSL_get_error(ssl, result))
	    {
	    case SSL_ERROR_NONE:
		break;
	    case SSL_ERROR_ZERO_RETURN:
	    {
		ConnectionLostException ex(__FILE__, __LINE__);
		ex.error = IceInternal::getSocketErrno();
		throw ex;
	    }
	    case SSL_ERROR_WANT_READ:
		if(!selectRead(fd, timeout))
		{
		    throw ConnectTimeoutException(__FILE__, __LINE__);
		}
		break;
	    case SSL_ERROR_WANT_WRITE:
		if(!selectWrite(fd, timeout))
		{
		    throw ConnectTimeoutException(__FILE__, __LINE__);
		}
		break;
	    case SSL_ERROR_SYSCALL:
	    {
		if(result == -1)
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

			continue;
		    }

		    if(IceInternal::connectionLost())
		    {
			ConnectionLostException ex(__FILE__, __LINE__);
			ex.error = IceInternal::getSocketErrno();
			throw ex;
		    }
		}

		if(result == 0)
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
		ex.reason = "SSL error for new outgoing connection:\nremote address = " +
		    IceInternal::addrToString(_addr) + "\n" + _instance->sslErrors();
		throw ex;
	    }
	    }
	}
	while(!SSL_is_init_finished(ssl));

	_instance->verifyPeer(ssl, fd, _host, "", false);
    }
    catch(...)
    {
	SSL_free(ssl);
	throw;
    }

    if(_instance->networkTraceLevel() >= 1)
    {
	Trace out(_logger, _instance->networkTraceCategory());
	out << "ssl connection established\n" << IceInternal::fdToString(fd);
    }

    if(_instance->securityTraceLevel() >= 1)
    {
	_instance->traceConnection(ssl, false);
    }

    return new TransceiverI(_instance, ssl, fd, false);
}

string
IceSSL::ConnectorI::toString() const
{
    return IceInternal::addrToString(_addr);
}

IceSSL::ConnectorI::ConnectorI(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _host(host),
    _logger(instance->communicator()->getLogger())
{
    IceInternal::getAddress(host, port, _addr);
}

IceSSL::ConnectorI::~ConnectorI()
{
}
