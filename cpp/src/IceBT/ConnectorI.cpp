// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/ConnectorI.h>
#include <IceBT/Instance.h>
#include <IceBT/TransceiverI.h>

#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceInternal::TransceiverPtr
IceBT::ConnectorI::connect()
{
    //
    // The plug-in may not be initialized.
    //
    if(!_instance->initialized())
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceBT: plug-in is not initialized";
        throw ex;
    }

    //
    // The transceiver handles all connection activity.
    //
    return new TransceiverI(_instance, _addr, _uuid);
}

Short
IceBT::ConnectorI::type() const
{
    return _instance->type();
}

string
IceBT::ConnectorI::toString() const
{
    return _addr;
}

bool
IceBT::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return false;
    }

    if(_addr != p->_addr)
    {
        return false;
    }

    if(_uuid != p->_uuid)
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
IceBT::ConnectorI::operator!=(const IceInternal::Connector& r) const
{
    return !operator==(r);
}

bool
IceBT::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_addr < p->_addr)
    {
        return true;
    }

    if(_uuid < p->_uuid)
    {
        return true;
    }
    else if(p->_uuid < _uuid)
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

    return _connectionId < p->_connectionId;
}

IceBT::ConnectorI::ConnectorI(const InstancePtr& instance, const string& addr, const string& uuid, Int timeout,
                              const string& connectionId) :
    _instance(instance),
    _addr(addr),
    _uuid(uuid),
    _timeout(timeout),
    _connectionId(connectionId)
{
}
