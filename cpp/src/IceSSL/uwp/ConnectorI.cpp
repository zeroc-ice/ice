// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/uwp/ConnectorI.h>
#include <IceSSL/uwp/TransceiverI.h>
#include <IceSSL/uwp/EndpointI.h>

#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect()
{
    IceInternal::TransceiverPtr transceiver = new TransceiverI(_instance, createSocket(false, _addr), false);
    dynamic_cast<TransceiverI*>(transceiver.get())->connect(_addr);
    return transceiver;
}

Short
IceSSL::ConnectorI::type() const
{
    return _instance->type();
}

string
IceSSL::ConnectorI::toString() const
{
    return addrToString(_addr);
}

bool
IceSSL::ConnectorI::operator==(const Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return false;
    }

    if(type() != p->type())
    {
        return false;
    }

    if(compareAddress(_addr, p->_addr) != 0)
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
IceSSL::ConnectorI::operator<(const Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(type() < p->type())
    {
        return true;
    }
    else if(p->type() < type())
    {
        return false;
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
    return compareAddress(_addr, p->_addr) < 0;
}

IceSSL::ConnectorI::ConnectorI(const IceInternal::ProtocolInstancePtr& instance, const IceInternal::Address& addr,
                               Ice::Int timeout, const string& connectionId) :
    _instance(instance),
    _addr(addr),
    _timeout(timeout),
    _connectionId(connectionId)
{
}
