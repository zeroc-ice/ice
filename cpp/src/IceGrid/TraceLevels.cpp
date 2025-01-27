// Copyright (c) ZeroC, Inc.

#include "TraceLevels.h"
#include "Ice/Communicator.h"
#include "Ice/Properties.h"

using namespace std;
using namespace IceGrid;

TraceLevels::TraceLevels(const shared_ptr<Ice::Communicator>& communicator, const string& prefix)
    : adminCat("Admin"),
      applicationCat("Application"),
      nodeCat("Node"),
      replicaCat("Replica"),
      serverCat("Server"),
      adapterCat("Adapter"),
      objectCat("Object"),
      activatorCat("Activator"),
      locatorCat("Locator"),
      sessionCat("Session"),
      discoveryCat("Discovery"),
      logger(communicator->getLogger())
{
    auto properties = communicator->getProperties();

    const string keyBase = prefix + ".Trace.";
    // We don't use getIcePropertyAsInt because not all trace properties exist for all prefixes.
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
