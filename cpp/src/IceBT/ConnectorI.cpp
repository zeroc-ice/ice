//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ConnectorI.h"
#include "Ice/LocalExceptions.h"
#include "Instance.h"
#include "TransceiverI.h"

using namespace std;
using namespace Ice;
using namespace IceBT;

IceInternal::TransceiverPtr
IceBT::ConnectorI::connect()
{
    //
    // The plug-in may not be initialized.
    //
    if (!_instance->initialized())
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceBT: plug-in is not initialized");
    }

    //
    // The transceiver handles all connection activity.
    //
    return make_shared<TransceiverI>(_instance, _addr, _uuid);
}

int16_t
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
    if (!p)
    {
        return false;
    }

    if (_addr != p->_addr)
    {
        return false;
    }

    if (_uuid != p->_uuid)
    {
        return false;
    }

    if (_timeout != p->_timeout)
    {
        return false;
    }

    if (_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceBT::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (_addr < p->_addr)
    {
        return true;
    }

    if (_uuid < p->_uuid)
    {
        return true;
    }
    else if (p->_uuid < _uuid)
    {
        return false;
    }

    if (_timeout < p->_timeout)
    {
        return true;
    }
    else if (p->_timeout < _timeout)
    {
        return false;
    }

    return _connectionId < p->_connectionId;
}

IceBT::ConnectorI::ConnectorI(
    const InstancePtr& instance,
    const string& addr,
    const string& uuid,
    int32_t timeout,
    const string& connectionId)
    : _instance(instance),
      _addr(addr),
      _uuid(uuid),
      _timeout(timeout),
      _connectionId(connectionId)
{
}
