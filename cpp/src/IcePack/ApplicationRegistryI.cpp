// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePack/ApplicationRegistryI.h>
#include <IcePack/TraceLevels.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace IcePack;

const string ApplicationRegistryI::_dbName = "applicationregistry";

ApplicationRegistryI::ApplicationRegistryI(const Ice::CommunicatorPtr& communicator, 
					   const ServerRegistryPtr& serverRegistry,
					   const string& envName, 
					   const TraceLevelsPtr& traceLevels) :
    _serverRegistry(serverRegistry),
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _dictCache(_connectionCache, _dbName),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator)
{
}

void
ApplicationRegistryI::add(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringStringSeqDict dict(connection, _dbName); 

    StringStringSeqDict::iterator p = dict.find(name);
    if(p != dict.end())
    {
	throw ApplicationExistsException();
    }
    
    dict.put(pair<const string, const Ice::StringSeq>(name, Ice::StringSeq()));

    if(_traceLevels->applicationRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationRegistryCat);
	out << "added application `" << name << "'";
    }
}

void
ApplicationRegistryI::remove(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringStringSeqDict dict(connection, _dbName); 

    StringStringSeqDict::iterator p = dict.find(name);
    if(p == dict.end())
    {
	throw ApplicationNotExistException();
    }

    dict.erase(p);

    if(_traceLevels->applicationRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationRegistryCat);
	out << "removed application `" << name << "'";
    }
}

void
ApplicationRegistryI::registerServer(const string& application, const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringStringSeqDict dict(connection, _dbName); 

    StringStringSeqDict::iterator p = dict.find(application);
    if(p == dict.end())
    {
	throw ApplicationNotExistException();
    }

    Ice::StringSeq servers = p->second;
    servers.push_back(name);
    p.set(servers);
}

void
ApplicationRegistryI::unregisterServer(const string& application, const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringStringSeqDict dict(connection, _dbName); 

    StringStringSeqDict::iterator p = dict.find(application);
    if(p == dict.end())
    {
	throw ApplicationNotExistException();
    }

    Ice::StringSeq servers = p->second;
    for(Ice::StringSeq::iterator q = servers.begin(); q != servers.end(); ++q)
    {
	if(*q == name)
	{
	    servers.erase(q);
	    break;
	}
    }
    p.set(servers);
}

ApplicationDescriptorPtr
ApplicationRegistryI::getDescriptor(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringStringSeqDict dict(connection, _dbName); 

    StringStringSeqDict::iterator p = dict.find(name);
    if(p == dict.end())
    {
	throw ApplicationNotExistException();
    }

    ApplicationDescriptorPtr descriptor = new ApplicationDescriptor();
    descriptor->name = name;
    for(Ice::StringSeq::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
    {
	try
	{
	    descriptor->servers.push_back(_serverRegistry->getDescriptor(*q));
	}
	catch(ServerNotExistException&)
	{
	}
    }
    return descriptor;
}

Ice::StringSeq
ApplicationRegistryI::getAll(const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringStringSeqDict dict(connection, _dbName); 

    Ice::StringSeq names;
    names.reserve(dict.size());

    for(StringStringSeqDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
