// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    serverRegistry(0),
    serverRegistryCat("ServerRegistry"),
    adapterRegistry(0),
    adapterRegistryCat("AdapterRegistry"),
    nodeRegistry(0),
    nodeRegistryCat("NodeRegistry"),
    logger(theLogger)
{
    const string nodeKeyBase = "IcePack.Node.Trace.";
    const_cast<int&>(server) = properties->getPropertyAsInt(nodeKeyBase + serverCat);
    const_cast<int&>(adapter) = properties->getPropertyAsInt(nodeKeyBase + adapterCat);
    const_cast<int&>(activator) = properties->getPropertyAsInt(nodeKeyBase + activatorCat);

    const string registryKeyBase = "IcePack.Registry.Trace.";
    const_cast<int&>(serverRegistry) = properties->getPropertyAsInt(registryKeyBase + serverRegistryCat);
    const_cast<int&>(adapterRegistry) = properties->getPropertyAsInt(registryKeyBase + adapterRegistryCat);
    const_cast<int&>(nodeRegistry) = properties->getPropertyAsInt(registryKeyBase + nodeRegistryCat);
}

TraceLevels::~TraceLevels()
{
}
