// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePack/Internal.h>
#include <IcePack/QueryI.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::QueryI::QueryI(const CommunicatorPtr& communicator, const ObjectRegistryPtr& objectRegistry) :
    _communicator(communicator),
    _objectRegistry(objectRegistry)
{
}

IcePack::QueryI::~QueryI()
{
}

Ice::ObjectPrx
IcePack::QueryI::findObjectById(const Ice::Identity& id, const Ice::Current&) const
{
    return _objectRegistry->findById(id);
}

Ice::ObjectPrx 
IcePack::QueryI::findObjectByType(const string& type, const Ice::Current&) const
{
    return _objectRegistry->findByType(type);
}

Ice::ObjectProxySeq 
IcePack::QueryI::findAllObjectsWithType(const string& type, const Ice::Current&) const
{
    return _objectRegistry->findAllWithType(type);
}


