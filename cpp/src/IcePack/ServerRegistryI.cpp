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

#include <IcePack/ServerRegistryI.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::ServerRegistryI::ServerRegistryI(const Freeze::DBPtr& db, const TraceLevelsPtr& traceLevels) :
    _dict(db),
    _traceLevels(traceLevels)
{
}

void
IcePack::ServerRegistryI::add(const string& name, const ServerPrx& server, const Ice::Current&)
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
	    p.set(server);

	    if(_traceLevels->serverRegistry > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
		out << "added server `" << name << "'";
	    }

	    return;
	}
	catch(const Ice::LocalException&)
	{
	}
	throw ServerExistsException();
    }
    
    _dict.put(pair<const string, const Ice::ObjectPrx>(name, server));

    if(_traceLevels->serverRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
	out << "added server `" << name << "'";
    }
}

void
IcePack::ServerRegistryI::remove(const string& name, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(name);
    if(p == _dict.end())
    {
	throw ServerNotExistException();
    }
    
    _dict.erase(p);

    if(_traceLevels->serverRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
	out << "removed server `" << name << "'";
    }
}

ServerPrx
IcePack::ServerRegistryI::findByName(const string& name, const Ice::Current&)
{
    StringObjectProxyDict::iterator p = _dict.find(name);
    if(p != _dict.end())
    {
	try
	{
	    return ServerPrx::checkedCast(p->second);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    _dict.erase(p);
	}
	catch(const Ice::LocalException&)
	{
	    return ServerPrx::uncheckedCast(p->second);
	}
    }
    throw ServerNotExistException();
}

Ice::StringSeq
IcePack::ServerRegistryI::getAll(const Ice::Current&) const
{
    Ice::StringSeq names;
    names.reserve(_dict.size());

    for(StringObjectProxyDict::const_iterator p = _dict.begin(); p != _dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
