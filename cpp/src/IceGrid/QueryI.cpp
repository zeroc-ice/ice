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
#include <IceGrid/Database.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

QueryI::QueryI(const CommunicatorPtr& communicator, const DatabasePtr& database) :
    _communicator(communicator),
    _database(database)
{
}

QueryI::~QueryI()
{
}

Ice::ObjectPrx
QueryI::findObjectById(const Ice::Identity& id, const Ice::Current&) const
{
    return _database->getObjectProxy(id);
}

Ice::ObjectPrx 
QueryI::findObjectByType(const string& type, const Ice::Current&) const
{
    return _database->getObjectByType(type);
}

Ice::ObjectPrx 
QueryI::findObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample, const Ice::Current&) const
{
    return _database->getObjectByTypeOnLeastLoadedNode(type, sample);
}

Ice::ObjectProxySeq 
QueryI::findAllObjectsWithType(const string& type, const Ice::Current&) const
{
    return _database->getObjectsWithType(type);
}


