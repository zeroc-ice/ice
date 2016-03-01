// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EndpointFactory.h>
#include <Ice/Instance.h>
#include <Ice/EndpointFactoryManager.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(EndpointFactory* p) { return p; }

IceInternal::EndpointFactory::EndpointFactory()
{
}

IceInternal::EndpointFactory::~EndpointFactory()
{
}

IceInternal::EndpointFactoryPlugin::EndpointFactoryPlugin(const CommunicatorPtr& communicator,
                                                          const EndpointFactoryPtr& factory)
{
    assert(communicator);
    getInstance(communicator)->endpointFactoryManager()->add(factory);
}

void
IceInternal::EndpointFactoryPlugin::initialize()
{
}

void
IceInternal::EndpointFactoryPlugin::destroy()
{
}
