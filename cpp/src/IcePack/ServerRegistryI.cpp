// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePack/ServerRegistryI.h>
#include <IcePack/TraceLevels.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace IcePack;

const string ServerRegistryI::_dbName = "serverregistry";
const string ServerRegistryI::_dbDescriptorName = "serverdescriptors";

ServerRegistryI::ServerRegistryI(const Ice::CommunicatorPtr& communicator, const string& envName, 
				 const TraceLevelsPtr& traceLevels) :
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _dictCache(_connectionCache, _dbName),
    _dictDescriptorCache(_connectionCache, _dbDescriptorName),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator)
{
}

void
ServerRegistryI::add(const string& name, 
			      const ServerPrx& server,
			      const ServerDescriptorPtr& descriptor,
			      const Ice::Current& current)
{
    while(true)
    {
	ServerPrx oldServer;
	try
	{
	    oldServer = findByName(name, current);
	    oldServer->ice_ping();
	    throw ServerExistsException();
	}
	catch(const ServerNotExistException&)
	{
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	    throw ServerExistsException();
	}
	
	Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
	StringObjectProxyDict dict(connection, _dbName); 
	StringServerDescriptorDict dictDescriptor(connection, _dbDescriptorName);
	
	StringObjectProxyDict::iterator p = dict.find(name);
	if(p != dict.end())
	{
	    if(oldServer && oldServer != p->second)
	    {
		continue;
	    }
	}

	dict.put(pair<const string, const Ice::ObjectPrx>(name, server));
	dictDescriptor.put(pair<const string, const ServerDescriptorPtr>(name, descriptor));
	
	if(_traceLevels->serverRegistry > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
	    out << "added server `" << name << "'";
	}

	break;
    }
}

ServerPrx
ServerRegistryI::remove(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 
    StringServerDescriptorDict dictDescriptor(connection, _dbDescriptorName);

    StringObjectProxyDict::iterator p = dict.find(name);
    if(p == dict.end())
    {
	throw ServerNotExistException();
    }

    ServerPrx server = ServerPrx::uncheckedCast(p->second);
    dict.erase(p);
    dictDescriptor.erase(name);

    if(_traceLevels->serverRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverRegistryCat);
	out << "removed server `" << name << "'";
    }

    return server;
}

ServerPrx
ServerRegistryI::findByName(const string& name, const Ice::Current&)
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

ServerDescriptorPtr
ServerRegistryI::getDescriptor(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringServerDescriptorDict dictDescriptor(connection, _dbDescriptorName); 

    StringServerDescriptorDict::iterator p = dictDescriptor.find(name);
    if(p == dictDescriptor.end())
    {
	throw ServerNotExistException();
    }
    return p->second;
}

Ice::StringSeq
ServerRegistryI::getAll(const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    Ice::StringSeq names;
    names.reserve(dict.size());

    for(StringObjectProxyDict::iterator p = dict.begin(); p != dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}

ServerDescriptorSeq
ServerRegistryI::getAllDescriptorsOnNode(const string& node, const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringServerDescriptorDict dict(connection, _dbDescriptorName); 

    ServerDescriptorSeq descriptors;

    for(StringServerDescriptorDict::iterator p = dict.begin(); p != dict.end(); ++p)
    {
	if(p->second->node == node)
	{
	    descriptors.push_back(p->second);
	}
    }

    return descriptors;
}
