// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/WSConnector.h>
#include <Ice/WSTransceiver.h>
#include <Ice/WSEndpoint.h>
#include <Ice/HttpParser.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::WSConnector::connect()
{
    return new WSTransceiver(_instance, _delegate->connect(), _host, _port, _resource);
}

Short
IceInternal::WSConnector::type() const
{
    return _delegate->type();
}

string
IceInternal::WSConnector::toString() const
{
    return _delegate->toString();
}

bool
IceInternal::WSConnector::operator==(const Connector& r) const
{
    const WSConnector* p = dynamic_cast<const WSConnector*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_delegate != p->_delegate)
    {
        return false;
    }

    if(_resource != p->_resource)
    {
        return false;
    }

    return true;
}

bool
IceInternal::WSConnector::operator!=(const Connector& r) const
{
    return !operator==(r);
}

bool
IceInternal::WSConnector::operator<(const Connector& r) const
{
    const WSConnector* p = dynamic_cast<const WSConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
        return false;
    }

    if(_delegate < p->_delegate)
    {
        return true;
    }
    else if(p->_delegate < _delegate)
    {
        return false;
    }

    if(_resource < p->_resource)
    {
        return true;
    }
    else if(p->_resource < _resource)
    {
        return false;
    }

    return false;
}

IceInternal::WSConnector::WSConnector(const ProtocolInstancePtr& instance, const ConnectorPtr& del, const string& host, 
                                        int port, const string& resource) :
    _instance(instance), _delegate(del), _host(host), _port(port), _resource(resource)
{
}

IceInternal::WSConnector::~WSConnector()
{
}
