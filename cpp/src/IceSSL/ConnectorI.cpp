// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect()
{
    //
    // The plug-in may not be initialized.
    //
    if(!_instance->context())
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: plug-in is not initialized";
        throw ex;
    }

    if(_instance->networkTraceLevel() >= 2)
    {
        Trace out(_logger, _instance->networkTraceCategory());
        out << "trying to establish ssl connection to " << toString();
    }

    try
    {
        return new TransceiverI(_instance, IceInternal::createSocket(false, _addr), _proxy, _host, _addr);
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
    return IceInternal::addrToString(!_proxy ? _addr : _proxy->getAddress());
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

IceSSL::ConnectorI::ConnectorI(const InstancePtr& instance, const string& host, const IceInternal::Address& addr,
                               const IceInternal::NetworkProxyPtr& proxy, Ice::Int timeout,
                               const string& connectionId) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _host(host),
    _addr(addr),
    _proxy(proxy),
    _timeout(timeout),
    _connectionId(connectionId)
{
}

IceSSL::ConnectorI::~ConnectorI()
{
}
