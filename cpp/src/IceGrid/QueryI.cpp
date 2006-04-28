// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/Internal.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/ObjectCache.h>
#include <IceGrid/SessionI.h>

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

void
QueryI::findObjectById_async(const AMD_Query_findObjectByIdPtr& cb, const Ice::Identity& id, const Ice::Current&) const
{
    try
    {
	cb->ice_response(_database->getObjectProxy(id, 0));
    }
    catch(const ObjectNotRegisteredException&)
    {
	cb->ice_response(0);
    }
}

void
QueryI::findObjectByType_async(const AMD_Query_findObjectByTypePtr& cb, const string& type, const Ice::Current&) const
{
    try
    {
	cb->ice_response(_database->getObjectByType(type));
    }
    catch(const ObjectNotRegisteredException&)
    {
	cb->ice_response(0);
    }
}

void
QueryI::findObjectByTypeOnLeastLoadedNode_async(const AMD_Query_findObjectByTypeOnLeastLoadedNodePtr& cb, 
						const string& type,
						LoadSample sample, 
						const Ice::Current&) const
{
    try
    {
	cb->ice_response(_database->getObjectByTypeOnLeastLoadedNode(type, sample));
    }
    catch(const ObjectNotRegisteredException&)
    {
	cb->ice_response(0);
    }
}

void
QueryI::findAllObjectsByType_async(const AMD_Query_findAllObjectsByTypePtr& cb,
				   const string& type, 
				   const Ice::Current&) const
{
    try
    {
	cb->ice_response(_database->getObjectsByType(type));
    }
    catch(const ObjectNotRegisteredException&)
    {
	cb->ice_response(Ice::ObjectProxySeq());
    }
}


