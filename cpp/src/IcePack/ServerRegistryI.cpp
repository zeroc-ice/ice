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
#include <Freeze/Initialize.h>

using namespace std;
using namespace IcePack;

IcePack::ServerRegistryI::ServerRegistryI(const Ice::CommunicatorPtr& communicator,
					  const string& envName, const string& dbName,
					  const TraceLevelsPtr& traceLevels) :
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _dictCache(_connectionCache, dbName),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator),
    _dbName(dbName)
{
}

void
IcePack::ServerRegistryI::add(const string& name, const ServerPrx& server, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(name);
    if(p != dict.end())
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
    
    dict.put(pair<const string, const Ice::ObjectPrx>(name, server));

    if(_traceLevels->serverRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
	out << "added server `" << name << "'";
    }
}

void
IcePack::ServerRegistryI::remove(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(name);
    if(p == dict.end())
    {
	throw ServerNotExistException();
    }
    
    dict.erase(p);

    if(_traceLevels->serverRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
	out << "removed server `" << name << "'";
    }
}

ServerPrx
IcePack::ServerRegistryI::findByName(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(name);
    if(p != dict.end())
    {
	try
	{
	    return ServerPrx::checkedCast(p->second);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    dict.erase(p);
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
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    Ice::StringSeq names;
    names.reserve(dict.size());

    for(StringObjectProxyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
