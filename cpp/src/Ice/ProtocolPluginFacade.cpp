// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

void IceInternal::incRef(ProtocolPluginFacade* p) { p->__incRef(); }
void IceInternal::decRef(ProtocolPluginFacade* p) { p->__decRef(); }

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

IceInternal::ProtocolPluginFacade::ProtocolPluginFacade(const CommunicatorPtr& communicator) :
    _instance(getInstance(communicator)),
    _communicator(communicator)
{
}
