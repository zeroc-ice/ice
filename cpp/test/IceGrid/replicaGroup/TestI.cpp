// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI(const Ice::PropertiesPtr& properties) :
    _properties(properties)
{
}

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
