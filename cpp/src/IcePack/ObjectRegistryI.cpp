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

#include <IcePack/ObjectRegistryI.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::ObjectRegistryI::ObjectRegistryI(const Freeze::DBPtr& objDb, const Freeze::DBPtr& typeDb, 
					  const TraceLevelsPtr& traceLevels) :
    _objects(objDb),
    _types(typeDb),
    _traceLevels(traceLevels)
{
}

void
IcePack::ObjectRegistryI::add(const ObjectDescription& obj, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    Ice::Identity id = obj.proxy->ice_getIdentity();

    IdentityObjectDescDict::iterator p = _objects.find(id);
    if(p != _objects.end())
    {
	throw ObjectExistsException();
    }

    //
    // Add the object to the object dictionary.
    //
    _objects.insert(pair<const Ice::Identity, const ObjectDescription>(id, obj));

    //
    // Add the object to the interface dictionary.
    //
    if(!obj.type.empty())
    {
	Ice::ObjectProxySeq seq;
	
	StringObjectProxySeqDict::iterator q = _types.find(obj.type);
	if(q != _types.end())
	{
	    seq = q->second;
	}
	
	seq.push_back(obj.proxy);
	
	if(q == _types.end())
	{
	    _types.insert(pair<const string, const Ice::ObjectProxySeq>(obj.type, seq));
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
IcePack::ObjectRegistryI::remove(const Ice::ObjectPrx& object, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    
    Ice::Identity id = object->ice_getIdentity();

    IdentityObjectDescDict::iterator p = _objects.find(id);
    if(p == _objects.end())
    {
	throw ObjectNotExistException();
    }

    ObjectDescription obj = p->second;
    
    if(!obj.type.empty())
    {
	//
	// Remove the object from the interface dictionary.
	//
	StringObjectProxySeqDict::iterator q = _types.find(obj.type);
	assert(q != _types.end());
	
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
	    _types.erase(q);
	}
	else
	{
	    q.set(seq);
	}
    }
    
    //
    // Remove the object from the object dictionary.
    //
    _objects.erase(p);    

    if(_traceLevels->objectRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectRegistryCat);
	out << "removed object `" << id << "'";
    }
}

ObjectDescription
IcePack::ObjectRegistryI::getObjectDescription(const Ice::Identity& id, const Ice::Current&) const
{
    IdentityObjectDescDict::const_iterator p = _objects.find(id);
    if(p == _objects.end())
    {
	throw ObjectNotExistException();
    }

    return p->second;
}

Ice::ObjectPrx
IcePack::ObjectRegistryI::findById(const Ice::Identity& id, const Ice::Current&) const
{
    IdentityObjectDescDict::const_iterator p = _objects.find(id);
    if(p == _objects.end())
    {
	throw ObjectNotExistException();
    }

    return p->second.proxy;
}

Ice::ObjectPrx
IcePack::ObjectRegistryI::findByType(const string& type, const Ice::Current&) const
{
    StringObjectProxySeqDict::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
	throw ObjectNotExistException();
    }

    int r = rand() % int(p->second.size());
    return p->second[r];
}

Ice::ObjectProxySeq
IcePack::ObjectRegistryI::findAllWithType(const string& type, const Ice::Current&) const
{
    StringObjectProxySeqDict::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
	throw ObjectNotExistException();
    }

    return p->second;
}
