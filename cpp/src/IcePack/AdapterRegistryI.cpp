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

#include <IcePack/AdapterRegistryI.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::AdapterRegistryI::AdapterRegistryI(const Freeze::DBPtr& db, const TraceLevelsPtr& traceLevels) :
    _dict(db),
    _traceLevels(traceLevels)
{
}

void
IcePack::AdapterRegistryI::add(const string& id, const AdapterPrx& adapter, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(id);
    if(p != _dict.end())
    {
	try
	{
	    p->second->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    p.set(adapter);

	    if(_traceLevels->adapterRegistry > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
		out << "added adapter `" << id << "'";
	    }

	    return;
	}
	catch(const Ice::LocalException&)
	{
	}
	throw AdapterExistsException();
    }
    _dict.insert(make_pair(id, adapter));

    if(_traceLevels->adapterRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
	out << "added adapter `" << id << "'";
    }
}

void
IcePack::AdapterRegistryI::remove(const string& id, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(id);
    if(p == _dict.end())
    {
	throw AdapterNotExistException();
    }
    
    _dict.erase(p);

    if(_traceLevels->adapterRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
	out << "removed adapter `" << id << "'";
    }
}

AdapterPrx
IcePack::AdapterRegistryI::findById(const string& id, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(id);
    if(p != _dict.end())
    {
	try
	{
	    return AdapterPrx::checkedCast(p->second->ice_collocationOptimization(false));
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    _dict.erase(p);
	}
	catch(const Ice::LocalException&)
	{
	    return AdapterPrx::uncheckedCast(p->second->ice_collocationOptimization(false));
	}
    }
    throw AdapterNotExistException();
}

Ice::StringSeq
IcePack::AdapterRegistryI::getAll(const Ice::Current&) const
{
    Ice::StringSeq ids;
    ids.reserve(_dict.size());

    for(StringObjectProxyDict::const_iterator p = _dict.begin(); p != _dict.end(); ++p)
    {
	ids.push_back(p->first);
    }

    return ids;
}
