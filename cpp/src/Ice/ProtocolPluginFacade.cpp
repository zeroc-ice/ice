// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

void IceInternal::incRef(ProtocolPluginFacade* p) { p->__incRef(); }
void IceInternal::decRef(ProtocolPluginFacade* p) { p->__decRef(); }

ProtocolPluginFacadePtr
IceInternal::getProtocolPluginFacade(const CommunicatorPtr& communicator)
{
    return new ProtocolPluginFacade(getInstance(communicator));
}

IceInternal::ProtocolPluginFacade::ProtocolPluginFacade(const InstancePtr& instance)
    : _instance(instance)
{
}

CommunicatorPtr
IceInternal::ProtocolPluginFacade::getCommunicator() const
{
    return _instance->communicator();
}

string
IceInternal::ProtocolPluginFacade::getDefaultHost() const
{
    return _instance->defaultsAndOverrides()->defaultHost;
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

void
IceInternal::ProtocolPluginFacade::addEndpointFactory(const EndpointFactoryPtr& factory) const
{
    _instance->endpointFactoryManager()->add(factory);
}
