// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePack/AdapterRegistryI.h>
#include <IcePack/TraceLevels.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace IcePack;

IcePack::AdapterRegistryI::AdapterRegistryI(const Ice::CommunicatorPtr& communicator,
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
IcePack::AdapterRegistryI::add(const string& id, const AdapterPrx& adapter, const Ice::Current& current)
{
    while(true)
    {
	AdapterPrx oldAdapter;
	try
	{
	    oldAdapter = findById(id, current);
	    oldAdapter->ice_ping();
	    throw AdapterExistsException();
	}
	catch(const AdapterNotExistException&)
	{
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	    throw AdapterExistsException();
	}
	
	Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
	StringObjectProxyDict dict(connection, _dbName); 

	StringObjectProxyDict::iterator p = dict.find(id);
	if(p != dict.end())
	{
	    if(oldAdapter && oldAdapter != p->second)
	    {
		continue;
	    }

	    p.set(adapter);

	    if(_traceLevels->adapterRegistry > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
		out << "added adapter `" << id << "'";
	    }
	}
	else
	{
	    dict.put(pair<const string, const Ice::ObjectPrx>(id, adapter));
	    
	    if(_traceLevels->adapterRegistry > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
		out << "added adapter `" << id << "'";
	    }
	}

	break;
    }
}

AdapterPrx
IcePack::AdapterRegistryI::remove(const string& id, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(id);
    if(p == dict.end())
    {
	throw AdapterNotExistException();
    }

    AdapterPrx adapter = AdapterPrx::uncheckedCast(p->second);
    dict.erase(p);

    if(_traceLevels->adapterRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterRegistryCat);
	out << "removed adapter `" << id << "'";
    }

    return adapter;
}

AdapterPrx
IcePack::AdapterRegistryI::findById(const string& id, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(id);
    if(p != dict.end())
    {
	try
	{
	    p->second->ice_ping();
	    return AdapterPrx::uncheckedCast(p->second->ice_collocationOptimization(false));
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    dict.erase(p);
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
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    Ice::StringSeq ids;
    ids.reserve(dict.size());

    for(StringObjectProxyDict::iterator p = dict.begin(); p != dict.end(); ++p)
    {
	ids.push_back(p->first);
    }

    return ids;
}
