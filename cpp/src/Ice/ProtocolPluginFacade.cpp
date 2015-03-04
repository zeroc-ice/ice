// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/Instance.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/TraceLevels.h>
#include <Ice/Initialize.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ProtocolPluginFacade* p) { return p; }

ProtocolPluginFacadePtr
IceInternal::getProtocolPluginFacade(const CommunicatorPtr& communicator)
{
    return new ProtocolPluginFacade(communicator);
}

CommunicatorPtr
IceInternal::ProtocolPluginFacade::getCommunicator() const
{
    return _communicator;
}

string
IceInternal::ProtocolPluginFacade::getDefaultHost() const
{
    return _instance->defaultsAndOverrides()->defaultHost;
}

Ice::EncodingVersion
IceInternal::ProtocolPluginFacade::getDefaultEncoding() const
{
    return _instance->defaultsAndOverrides()->defaultEncoding;
}

int
IceInternal::ProtocolPluginFacade::getNetworkTraceLevel() const
{
    return _instance->traceLevels()->network;
}

const char*
IceInternal::ProtocolPluginFacade::getNetworkTraceCategory() const
{
    return _instance->traceLevels()->networkCat;
}

EndpointHostResolverPtr
IceInternal::ProtocolPluginFacade::getEndpointHostResolver() const
{
    return _instance->endpointHostResolver();
}

ProtocolSupport
IceInternal::ProtocolPluginFacade::getProtocolSupport() const
{
    return _instance->protocolSupport();
}

bool
IceInternal::ProtocolPluginFacade::preferIPv6() const
{
    return _instance->preferIPv6();
}

NetworkProxyPtr
IceInternal::ProtocolPluginFacade::getNetworkProxy() const
{
    return _instance->networkProxy();
}

void
IceInternal::ProtocolPluginFacade::addEndpointFactory(const EndpointFactoryPtr& factory) const
{
    _instance->endpointFactoryManager()->add(factory);
}

EndpointFactoryPtr
IceInternal::ProtocolPluginFacade::getEndpointFactory(Ice::Short type) const
{
    return _instance->endpointFactoryManager()->get(type);
}

IceInternal::ProtocolPluginFacade::ProtocolPluginFacade(const CommunicatorPtr& communicator) :
    _instance(getInstance(communicator)),
    _communicator(communicator)
{
}
