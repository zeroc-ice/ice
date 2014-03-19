// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ProtocolInstance.h>
#include <Ice/Instance.h>
#include <Ice/Initialize.h>
#include <Ice/IPEndpointI.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ProtocolInstance* p) { return p; }

IceInternal::ProtocolInstance::ProtocolInstance(const CommunicatorPtr& com, Short type, const string& protocol) :
    _instance(getInstance(com)),
    _traceLevel(_instance->traceLevels()->network),
    _traceCategory(_instance->traceLevels()->networkCat),
    _logger(_instance->initializationData().logger),
    _stats(_instance->initializationData().stats),
    _properties(_instance->initializationData().properties),
    _protocol(protocol),
    _type(type)
{
}

IceInternal::ProtocolInstance::ProtocolInstance(const InstancePtr& instance, Short type, const string& protocol) :
    _instance(instance),
    _traceLevel(_instance->traceLevels()->network),
    _traceCategory(_instance->traceLevels()->networkCat),
    _logger(_instance->initializationData().logger),
    _stats(_instance->initializationData().stats),
    _properties(_instance->initializationData().properties),
    _protocol(protocol),
    _type(type)
{
}

bool
IceInternal::ProtocolInstance::preferIPv6() const
{
    return _instance->preferIPv6();
}

ProtocolSupport
IceInternal::ProtocolInstance::protocolSupport() const
{
    return _instance->protocolSupport();
}
 
const string& 
IceInternal::ProtocolInstance::defaultHost() const
{
    return _instance->defaultsAndOverrides()->defaultHost;
}
    
const EncodingVersion&
IceInternal::ProtocolInstance::defaultEncoding() const
{
    return _instance->defaultsAndOverrides()->defaultEncoding;
}

NetworkProxyPtr
IceInternal::ProtocolInstance::networkProxy() const
{
    return _instance->networkProxy();
}

size_t
IceInternal::ProtocolInstance::messageSizeMax() const
{
    return _instance->messageSizeMax();
}

vector<ConnectorPtr> 
IceInternal::ProtocolInstance::resolve(const string& host, int port, EndpointSelectionType type,
                                       const IPEndpointIPtr& endpt) const
{
    return _instance->endpointHostResolver()->resolve(host, port, type, endpt);
}

void 
IceInternal::ProtocolInstance::resolve(const string& host, int port, EndpointSelectionType type, 
                                       const IPEndpointIPtr& endpt, const EndpointI_connectorsPtr& cb) const
{
    _instance->endpointHostResolver()->resolve(host, port, type, endpt, cb);
}

