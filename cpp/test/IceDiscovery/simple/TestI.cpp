// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

string
TestIntfI::getAdapterId(const Ice::Current& current)
{
    return current.adapter->getCommunicator()->getProperties()->getProperty(current.adapter->getName() + ".AdapterId");
}

void
ControllerI::activateObjectAdapter(ICE_IN(string) name,
                                   ICE_IN(string) adapterId,
                                   ICE_IN(string) replicaGroupId,
                                   const Ice::Current& current)
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
ControllerI::deactivateObjectAdapter(ICE_IN(string) name, const Ice::Current& current)
{
    _adapters[name]->destroy();
    _adapters.erase(name);
}

void
ControllerI::addObject(ICE_IN(string) oaName, ICE_IN(string) id, const Ice::Current&)
{
    assert(_adapters[oaName]);
    Ice::Identity identity;
    identity.name = id;
    _adapters[oaName]->add(ICE_MAKE_SHARED(TestIntfI), identity);
}

void
ControllerI::removeObject(ICE_IN(string) oaName, ICE_IN(string) id, const Ice::Current&)
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
