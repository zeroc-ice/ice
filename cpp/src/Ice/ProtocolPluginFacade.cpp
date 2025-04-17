// Copyright (c) ZeroC, Inc.

#include "ProtocolPluginFacade.h"
#include "DefaultsAndOverrides.h"
#include "EndpointFactoryManager.h"
#include "Ice/Initialize.h"
#include "Instance.h"
#include "TraceLevels.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::ProtocolPluginFacade::addEndpointFactory(const EndpointFactoryPtr& factory) const
{
    _instance->endpointFactoryManager()->add(factory);
}

EndpointFactoryPtr
IceInternal::ProtocolPluginFacade::getEndpointFactory(int16_t type) const
{
    return _instance->endpointFactoryManager()->get(type);
}

IceInternal::ProtocolPluginFacade::ProtocolPluginFacade(const CommunicatorPtr& communicator)
    : _instance(getInstance(communicator))
{
}
