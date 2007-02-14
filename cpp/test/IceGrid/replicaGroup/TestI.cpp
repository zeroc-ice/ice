// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI(const Ice::ObjectAdapterPtr& adapter, const Ice::PropertiesPtr& properties) :
    _adapter(adapter),
    _properties(properties)
{
}

std::string
TestI::getReplicaId(const Ice::Current&)
{
    return _properties->getProperty(_adapter->getName() + ".AdapterId");
}

std::string
TestI::getReplicaIdAndShutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
    return _properties->getProperty(_adapter->getName() + ".AdapterId");
}
