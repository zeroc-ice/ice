// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/ConnectorI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/TransceiverI.h>
#include <IceSSL/EndpointI.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect()
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

    try
    {
        SOCKET fd = IceInternal::createSocket(false, _addr.ss_family);
        IceInternal::setBlock(fd, false);
        IceInternal::setTcpBufSize(fd, _instance->communicator()->getProperties(), _logger);
        bool connected = IceInternal::doConnect(fd, _addr);

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

        //
        // SSL handshaking is performed in TransceiverI::initialize, since
        // connect must not block.
        //
        return new TransceiverI(_instance, ssl, fd, connected, false);
    }
    catch(const Ice::LocalException& ex)
    {
        if(_instance->networkTraceLevel() >= 2)
        {
            Trace out(_logger, _instance->networkTraceCategory());
            out << "failed to establish ssl connection to " << toString() << "\n" << ex;
        }
        throw;
    }
}

Short
IceSSL::ConnectorI::type() const
{
    return IceSSL::EndpointType;
}

string
IceSSL::ConnectorI::toString() const
{
    return IceInternal::addrToString(_addr);
}

bool
IceSSL::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return false;
    }

    if(IceInternal::compareAddress(_addr, p->_addr) != 0)
    {
        return false;
    }

    if(_timeout != p->_timeout)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceSSL::ConnectorI::operator!=(const IceInternal::Connector& r) const
{
    return !operator==(r);
}

bool
IceSSL::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_timeout < p->_timeout)
    {
        return true;
    }
    else if(p->_timeout < _timeout)
    {
        return false;
    }

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }

    return IceInternal::compareAddress(_addr, p->_addr) == -1;
}

IceSSL::ConnectorI::ConnectorI(const InstancePtr& instance, const struct sockaddr_storage& addr, Ice::Int timeout,
                               const string& connectionId) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _addr(addr),
    _timeout(timeout),
    _connectionId(connectionId)
{
}

IceSSL::ConnectorI::~ConnectorI()
{
}
