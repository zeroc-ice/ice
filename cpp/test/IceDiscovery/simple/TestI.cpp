// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;

string
TestIntfI::getAdapterId(const Ice::Current& current)
{
    return current.adapter->getCommunicator()->getProperties()->getProperty(current.adapter->getName() + ".AdapterId");
}

void
ControllerI::activateObjectAdapter(string name, string adapterId, string replicaGroupId, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty(name + ".AdapterId", adapterId);
    properties->setProperty(name + ".ReplicaGroupId", replicaGroupId);
    properties->setProperty(name + ".Endpoints", "default");
    _adapters[name] = communicator->createObjectAdapter(name);
    _adapters[name]->activate();
}

void
ControllerI::deactivateObjectAdapter(string name, const Ice::Current&)
{
    _adapters[name]->destroy();
    _adapters.erase(name);
}

void
ControllerI::addObject(string oaName, string id, const Ice::Current&)
{
    assert(_adapters[oaName]);
    Ice::Identity identity;
    identity.name = id;
    _adapters[oaName]->add(make_shared<TestIntfI>(), identity);
}

void
ControllerI::removeObject(string oaName, string id, const Ice::Current&)
{
    assert(_adapters[oaName]);
    Ice::Identity identity;
    identity.name = id;
    _adapters[oaName]->remove(identity);
}

void
ControllerI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
