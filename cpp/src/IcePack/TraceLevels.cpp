// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Properties.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& theLogger) :
    server(0),
    serverCat("Server"),
    adapter(0),
    adapterCat("Adapter"),
    activator(0),
    activatorCat("Activator"),
    applicationRegistry(0),
    applicationRegistryCat("ApplicationRegistry"),
    serverRegistry(0),
    serverRegistryCat("ServerRegistry"),
    adapterRegistry(0),
    adapterRegistryCat("AdapterRegistry"),
    objectRegistry(0),
    objectRegistryCat("ObjectRegistry"),
    nodeRegistry(0),
    nodeRegistryCat("NodeRegistry"),
    logger(theLogger)
{
    const string nodeKeyBase = "IcePack.Node.Trace.";
    const_cast<int&>(server) = properties->getPropertyAsInt(nodeKeyBase + serverCat);
    const_cast<int&>(adapter) = properties->getPropertyAsInt(nodeKeyBase + adapterCat);
    const_cast<int&>(activator) = properties->getPropertyAsInt(nodeKeyBase + activatorCat);

    const string registryKeyBase = "IcePack.Registry.Trace.";
    const_cast<int&>(applicationRegistry) = properties->getPropertyAsInt(registryKeyBase + applicationRegistryCat);
    const_cast<int&>(serverRegistry) = properties->getPropertyAsInt(registryKeyBase + serverRegistryCat);
    const_cast<int&>(adapterRegistry) = properties->getPropertyAsInt(registryKeyBase + adapterRegistryCat);
    const_cast<int&>(objectRegistry) = properties->getPropertyAsInt(registryKeyBase + objectRegistryCat);
    const_cast<int&>(nodeRegistry) = properties->getPropertyAsInt(registryKeyBase + nodeRegistryCat);
}

TraceLevels::~TraceLevels()
{
}
