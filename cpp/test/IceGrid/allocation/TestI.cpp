// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

TestI::TestI(Ice::PropertiesPtr properties) : _properties(std::move(properties)) {}

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

std::string
TestI::getProperty(std::string name, const Ice::Current&)
{
    return _properties->getProperty(name);
}
