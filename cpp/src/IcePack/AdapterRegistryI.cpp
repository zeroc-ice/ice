// **********************************************************************
//
// Copyright (c) 2001
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
IcePack::AdapterRegistryI::add(const string& name, const AdapterPrx& adapter, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(name);
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
		out << "added adapter `" << name << "'";
	    }

	    return;
	}
	catch(const Ice::LocalException&)
	{
	}
	throw AdapterExistsException();
    }
    _dict.insert(make_pair(name, adapter));

    if(_traceLevels->adapterRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
	out << "added adapter `" << name << "'";
    }
}

void
IcePack::AdapterRegistryI::remove(const string& name, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(name);
    if(p == _dict.end())
    {
	throw AdapterNotExistException();
    }
    
    _dict.erase(p);

    if(_traceLevels->adapterRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
	out << "removed adapter `" << name << "'";
    }
}

AdapterPrx
IcePack::AdapterRegistryI::findByName(const string& name, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(name);
    if(p != _dict.end())
    {
	try
	{
	    return AdapterPrx::checkedCast(p->second);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    _dict.erase(p);
	}
	catch(const Ice::LocalException&)
	{
	    return AdapterPrx::uncheckedCast(p->second);
	}
    }
    throw AdapterNotExistException();
}

Ice::StringSeq
IcePack::AdapterRegistryI::getAll(const Ice::Current&) const
{
    Ice::StringSeq names;
    names.reserve(_dict.size());

    for(StringObjectProxyDict::const_iterator p = _dict.begin(); p != _dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
