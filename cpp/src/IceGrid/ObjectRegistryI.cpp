// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <Ice/IdentityUtil.h>
#include <IceGrid/ObjectRegistryI.h>
#include <IceGrid/TraceLevels.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace IceGrid;

const string ObjectRegistryI::_objectsDbName = "objectregistry";
const string ObjectRegistryI::_typesDbName = "objectregistry-types";

ObjectRegistryI::ObjectRegistryI(const Ice::CommunicatorPtr& communicator,
				 const string& envName, 
				 const TraceLevelsPtr& traceLevels) :
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _objectsCache(_connectionCache, _objectsDbName, true),
    _typesCache(_connectionCache, _typesDbName, true),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator)
{
}

void
ObjectRegistryI::add(const ObjectDescriptor& obj, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectsDbName);
    StringObjectProxySeqDict types(connection, _typesDbName);

    Ice::Identity id = obj.proxy->ice_getIdentity();

    IdentityObjectDescDict::iterator p = objects.find(id);
    if(p != objects.end())
    {
	throw ObjectExistsException();
    }

    //
    // Add the object to the object dictionary.
    //
    objects.put(pair<const Ice::Identity, const ObjectDescriptor>(id, obj));

    //
    // Add the object to the interface dictionary.
    //
    if(!obj.type.empty())
    {
	Ice::ObjectProxySeq seq;
	
	StringObjectProxySeqDict::iterator q = types.find(obj.type);
	if(q != types.end())
	{
	    seq = q->second;
	}
	
	seq.push_back(obj.proxy);
	
	if(q == types.end())
	{
	    types.put(pair<const string, const Ice::ObjectProxySeq>(obj.type, seq));
	}
	else
	{
	    q.set(seq);
	}
    }

    if(_traceLevels->objectRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectRegistryCat);
	out << "added object `" << Ice::identityToString(id) << "'";
    }
}

void
ObjectRegistryI::remove(const Ice::Identity& id, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectsDbName);
    StringObjectProxySeqDict types(connection, _typesDbName);

    IdentityObjectDescDict::iterator p = objects.find(id);
    if(p == objects.end())
    {
	throw ObjectNotExistException();
    }

    ObjectDescriptor obj = p->second;
    
    if(!obj.type.empty())
    {
	//
	// Remove the object from the interface dictionary.
	//
	StringObjectProxySeqDict::iterator q = types.find(obj.type);
	assert(q != types.end());
	
	Ice::ObjectProxySeq seq = q->second;
	
	Ice::ObjectProxySeq::iterator r;
	for(r = seq.begin(); r != seq.end(); ++r)
	{
	    if((*r)->ice_getIdentity() == id)
	    {
		break;
	    }
	}
	
	assert(r != seq.end());
	seq.erase(r);
	
	if(seq.size() == 0)
	{
	    types.erase(q);
	}
	else
	{
	    q.set(seq);
	}
    }
    
    //
    // Remove the object from the object dictionary.
    //
    objects.erase(p);    

    if(_traceLevels->objectRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectRegistryCat);
	out << "removed object `" << id << "'";
    }
}

ObjectDescriptor
ObjectRegistryI::getObjectDescriptor(const Ice::Identity& id, const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectsDbName);


    IdentityObjectDescDict::iterator p = objects.find(id);
    if(p == objects.end())
    {
	throw ObjectNotExistException();
    }

    return p->second;
}

Ice::ObjectPrx
ObjectRegistryI::findById(const Ice::Identity& id, const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectsDbName);

    IdentityObjectDescDict::iterator p = objects.find(id);
    if(p == objects.end())
    {
	throw ObjectNotExistException();
    }

    return p->second.proxy;
}

Ice::ObjectPrx
ObjectRegistryI::findByType(const string& type, const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxySeqDict types(connection, _typesDbName);

    StringObjectProxySeqDict::iterator p = types.find(type);
    if(p == types.end())
    {
	throw ObjectNotExistException();
    }

    int r = rand() % int(p->second.size());
    return p->second[r];
}

Ice::ObjectProxySeq
ObjectRegistryI::findAllWithType(const string& type, const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxySeqDict types(connection, _typesDbName);

    StringObjectProxySeqDict::iterator p = types.find(type);
    if(p == types.end())
    {
	throw ObjectNotExistException();
    }

    return p->second;
}

ObjectDescriptorSeq
ObjectRegistryI::findAll(const string& expression, const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectsDbName);

    ObjectDescriptorSeq result;
    for(IdentityObjectDescDict::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
	if(expression.empty() ||
	   IceUtil::match(Ice::identityToString(p->second.proxy->ice_getIdentity()), expression, true))
	{
	    result.push_back(p->second);
	}
    }
    return result;
}

