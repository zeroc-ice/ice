// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

const string AdapterRegistryI::_dbName = "adapterregistry";

IcePack::AdapterRegistryI::AdapterRegistryI(const Ice::CommunicatorPtr& communicator, const string& envName, 
					    const TraceLevelsPtr& traceLevels) :
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _dictCache(_connectionCache, _dbName),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator)
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
		out << "updated adapter `" << id << "'";
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
IcePack::AdapterRegistryI::remove(const string& id, const AdapterPrx& orig, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(id);
    if(p == dict.end())
    {
	throw AdapterNotExistException();
    }

    AdapterPrx adapter = AdapterPrx::uncheckedCast(p->second);
    if(orig != 0 && orig != adapter) // Only remove if the adapter is equal to the provided proxy (if not null)
    {
	return 0;
    }
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
	return AdapterPrx::uncheckedCast(p->second); 
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
