// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/Internal.h>
#include <IceGrid/QueryI.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

QueryI::QueryI(const CommunicatorPtr& communicator, const ObjectRegistryPtr& objectRegistry) :
    _communicator(communicator),
    _objectRegistry(objectRegistry)
{
}

QueryI::~QueryI()
{
}

Ice::ObjectPrx
QueryI::findObjectById(const Ice::Identity& id, const Ice::Current&) const
{
    return _objectRegistry->findById(id);
}

Ice::ObjectPrx 
QueryI::findObjectByType(const string& type, const Ice::Current&) const
{
    return _objectRegistry->findByType(type);
}

Ice::ObjectProxySeq 
QueryI::findAllObjectsWithType(const string& type, const Ice::Current&) const
{
    return _objectRegistry->findAllWithType(type);
}


