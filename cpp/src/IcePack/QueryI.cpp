// **********************************************************************
//
// Copyright (c) 2003
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


