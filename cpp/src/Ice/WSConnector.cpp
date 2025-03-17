// Copyright (c) ZeroC, Inc.

#include "WSConnector.h"
#include "HttpParser.h"
#include "TargetCompare.h"
#include "WSEndpoint.h"
#include "WSTransceiver.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::WSConnector::connect()
{
    return make_shared<WSTransceiver>(_instance, _delegate->connect(), _host, _resource);
}

int16_t
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
    const auto* p = dynamic_cast<const WSConnector*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (_resource != p->_resource)
    {
        return false;
    }

    return Ice::targetEqualTo(_delegate, p->_delegate);
}

bool
IceInternal::WSConnector::operator<(const Connector& r) const
{
    const auto* p = dynamic_cast<const WSConnector*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (this == p)
    {
        return false;
    }

    if (_resource < p->_resource)
    {
        return true;
    }
    else if (p->_resource < _resource)
    {
        return false;
    }

    return Ice::targetLess(_delegate, p->_delegate);
}

IceInternal::WSConnector::WSConnector(ProtocolInstancePtr instance, ConnectorPtr del, string host, string resource)
    : _instance(std::move(instance)),
      _delegate(std::move(del)),
      _host(std::move(host)),
      _resource(std::move(resource))
{
}

IceInternal::WSConnector::~WSConnector() = default;
