// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI(const Ice::PropertiesPtr& properties) :
    _properties(properties)
{
}

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

std::string
TestI::getProperty(const std::string& name, const Ice::Current&)
{
    return _properties->getProperty(name);
}
