// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

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
