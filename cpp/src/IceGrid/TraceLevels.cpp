//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <IceGrid/TraceLevels.h>

using namespace std;
using namespace IceGrid;

TraceLevels::TraceLevels(const shared_ptr<Ice::Communicator>& communicator, const string& prefix) :
    admin(),
    adminCat("Admin"),
    application(),
    applicationCat("Application"),
    node(),
    nodeCat("Node"),
    replica(),
    replicaCat("Replica"),
    server(),
    serverCat("Server"),
    adapter(),
    adapterCat("Adapter"),
    object(),
    objectCat("Object"),
    activator(),
    activatorCat("Activator"),
    locator(),
    locatorCat("Locator"),
    session(),
    sessionCat("Session"),
    discovery(),
    discoveryCat("Discovery"),
    logger(communicator->getLogger())
{
    auto properties = communicator->getProperties();

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
