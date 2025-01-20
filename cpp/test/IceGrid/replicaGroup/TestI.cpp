// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

TestI::TestI(Ice::PropertiesPtr properties) : _properties(std::move(properties)) {}

std::string
TestI::getReplicaId(const Ice::Current& current)
{
    return _properties->getProperty(current.adapter->getName() + ".AdapterId");
}

std::string
TestI::getReplicaIdAndShutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    return _properties->getProperty(current.adapter->getName() + ".AdapterId");
}
