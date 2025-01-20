// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

TestI::TestI(Ice::ObjectAdapterPtr adapter, Ice::PropertiesPtr properties)
    : _adapter(std::move(adapter)),
      _properties(std::move(properties))
{
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

std::string
TestI::getProperty(std::string name, const Ice::Current&)
{
    return _properties->getProperty(name);
}
