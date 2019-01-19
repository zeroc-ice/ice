//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI(const Ice::ObjectAdapterPtr& adapter, const Ice::PropertiesPtr& properties) :
    _adapter(adapter),
    _properties(properties)
{
}

void
TestI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

std::string
TestI::getProperty(const std::string& name, const Ice::Current&)
{
    return _properties->getProperty(name);
}
