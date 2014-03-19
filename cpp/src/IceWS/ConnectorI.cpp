// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceWS/ConnectorI.h>
#include <IceWS/Instance.h>
#include <IceWS/TransceiverI.h>
#include <IceWS/EndpointI.h>
#include <IceWS/Util.h>

using namespace std;
using namespace Ice;
using namespace IceWS;

IceInternal::TransceiverPtr
IceWS::ConnectorI::connect()
{
    return new TransceiverI(_instance, _delegate->connect(), _host, _port, _resource);
}

Short
IceWS::ConnectorI::type() const
{
    return _delegate->type();
}

string
IceWS::ConnectorI::toString() const
{
    return _delegate->toString();
}

bool
IceWS::ConnectorI::operator==(const Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
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
IceWS::ConnectorI::operator!=(const Connector& r) const
{
    return !operator==(r);
}

bool
IceWS::ConnectorI::operator<(const Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
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

IceWS::ConnectorI::ConnectorI(const InstancePtr& instance, const IceInternal::ConnectorPtr& del,
                              const string& host, int port, const string& resource) :
    _instance(instance), _delegate(del), _host(host), _port(port), _resource(resource)
{
}

IceWS::ConnectorI::~ConnectorI()
{
}
