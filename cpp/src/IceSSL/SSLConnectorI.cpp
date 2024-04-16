//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLConnectorI.h"
#include "SSLInstance.h"

#include "../Ice/NetworkProxy.h"
#include "../Ice/StreamSocket.h"
#include "Ice/Communicator.h"
#include "Ice/LocalException.h"
#include "Ice/LoggerUtil.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLUtil.h"

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect()
{
    return _instance->engine()->createTransceiver(_instance, _delegate->connect(), _host, false);
}

int16_t
IceSSL::ConnectorI::type() const
{
    return _delegate->type();
}

string
IceSSL::ConnectorI::toString() const
{
    return _delegate->toString();
}

bool
IceSSL::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    return Ice::targetEqualTo(_delegate, p->_delegate);
}

bool
IceSSL::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (this == p)
    {
        return false;
    }

    return Ice::targetLess(_delegate, p->_delegate);
}

IceSSL::ConnectorI::ConnectorI(const InstancePtr& instance, const IceInternal::ConnectorPtr& del, const string& h)
    : _instance(instance),
      _delegate(del),
      _host(h)
{
}

IceSSL::ConnectorI::~ConnectorI() {}
