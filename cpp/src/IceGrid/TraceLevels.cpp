//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <IceGrid/TraceLevels.h>

using namespace std;
using namespace IceGrid;

TraceLevels::TraceLevels(const Ice::CommunicatorPtr& communicator, const string& prefix) :
    admin(0),
    adminCat("Admin"),
    application(0),
    applicationCat("Application"),
    node(0),
    nodeCat("Node"),
    replica(0),
    replicaCat("Replica"),
    server(0),
    serverCat("Server"),
    adapter(0),
    adapterCat("Adapter"),
    object(0),
    objectCat("Object"),
    activator(0),
    activatorCat("Activator"),
    locator(0),
    locatorCat("Locator"),
    session(0),
    sessionCat("Session"),
    discovery(0),
    discoveryCat("Discovery"),
    logger(communicator->getLogger())
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    string keyBase = prefix + ".Trace.";
    const_cast<int&>(admin) = properties->getPropertyAsInt(keyBase + adminCat);
    const_cast<int&>(application) = properties->getPropertyAsInt(keyBase + applicationCat);
    const_cast<int&>(node) = properties->getPropertyAsInt(keyBase + nodeCat);
    const_cast<int&>(replica) = properties->getPropertyAsInt(keyBase + replicaCat);
    const_cast<int&>(server) = properties->getPropertyAsInt(keyBase + serverCat);
    const_cast<int&>(adapter) = properties->getPropertyAsInt(keyBase + adapterCat);
    const_cast<int&>(object) = properties->getPropertyAsInt(keyBase + objectCat);
    const_cast<int&>(activator) = properties->getPropertyAsInt(keyBase + activatorCat);
    const_cast<int&>(locator) = properties->getPropertyAsInt(keyBase + locatorCat);
    const_cast<int&>(session) = properties->getPropertyAsInt(keyBase + sessionCat);
    const_cast<int&>(discovery) = properties->getPropertyAsInt(keyBase + discoveryCat);
}

TraceLevels::~TraceLevels()
{
}
