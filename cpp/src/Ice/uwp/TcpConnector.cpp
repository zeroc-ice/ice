// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/uwp/TcpConnector.h>
#include <Ice/uwp/TcpTransceiver.h>
#include <Ice/uwp/TcpEndpointI.h>

#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::TcpConnector::connect()
{
    TransceiverPtr transceiver = new TcpTransceiver(_instance, createSocket(false, _addr), false);
    dynamic_cast<TcpTransceiver*>(transceiver.get())->connect(_addr);
    return transceiver;
}

Short
IceInternal::TcpConnector::type() const
{
    return _instance->type();
}

string
IceInternal::TcpConnector::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::TcpConnector::operator==(const Connector& r) const
{
    const TcpConnector* p = dynamic_cast<const TcpConnector*>(&r);
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
IceInternal::TcpConnector::operator!=(const Connector& r) const
{
    return !operator==(r);
}

bool
IceInternal::TcpConnector::operator<(const Connector& r) const
{
    const TcpConnector* p = dynamic_cast<const TcpConnector*>(&r);
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

IceInternal::TcpConnector::TcpConnector(const ProtocolInstancePtr& instance, const Address& addr,
                                        Ice::Int timeout, const string& connectionId) :
    _instance(instance),
    _addr(addr),
    _timeout(timeout),
    _connectionId(connectionId)
{
}
