// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/AcceptorI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/TransceiverI.h>
#include <IceSSL/Util.h>

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
    // The plugin may not be initialized.
    //
    if(!_instance->context())
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: plugin is not initialized";
        throw ex;
    }

    SOCKET fd = IceInternal::doAccept(_fd, timeout);
    IceInternal::setBlock(fd, false);
    IceInternal::setTcpBufSize(fd, _instance->communicator()->getProperties(), _logger);

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

    if(_instance->networkTraceLevel() >= 1)
    {
        Trace out(_logger, _instance->networkTraceCategory());
        out << "attempting to accept ssl connection\n" << IceInternal::fdToString(fd);
    }

    //
    // SSL handshaking is performed in TransceiverI::initialize, since
    // accept must not block.
    //
    return new TransceiverI(_instance, ssl, fd, true, _adapterName);
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
    return IceInternal::compareAddress(addr, _addr) == 0;
}

int
IceSSL::AcceptorI::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceSSL::AcceptorI::AcceptorI(const InstancePtr& instance, const string& adapterName, const string& host, int port) :
    _instance(instance),
    _adapterName(adapterName),
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
        IceInternal::setTcpBufSize(_fd, _instance->communicator()->getProperties(), _logger);
#ifndef _WIN32
        //
        // Enable SO_REUSEADDR on Unix platforms to allow re-using the
        // socket even if it's in the TIME_WAIT state. On Windows,
        // this doesn't appear to be necessary and enabling
        // SO_REUSEADDR would actually not be a good thing since it
        // allows a second process to bind to an address even it's
        // already bound by another process.
        //
        // TODO: using SO_EXCLUSIVEADDRUSE on Windows would probably
        // be better but it's only supported by recent Windows
        // versions (XP SP2, Windows Server 2003).
        //
        IceInternal::setReuseAddress(_fd, true);
#endif
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
