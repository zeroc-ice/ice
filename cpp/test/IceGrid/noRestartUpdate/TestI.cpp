//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI(std::shared_ptr<Ice::ObjectAdapter> adapter, std::shared_ptr<Ice::Properties> properties)
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
