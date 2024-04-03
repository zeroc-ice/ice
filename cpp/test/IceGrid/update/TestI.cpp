//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"

TestI::TestI(const Ice::PropertiesPtr& properties) : _properties(properties) {}

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
