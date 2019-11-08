//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

TestI::TestI(shared_ptr<Ice::Properties>&& properties) :
    _properties(move(properties))
{
}

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

string
TestI::getProperty(string name, const Ice::Current&)
{
    return _properties->getProperty(name);
}
