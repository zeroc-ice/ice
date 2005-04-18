// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname
#include <IcePatch2/Util.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Activator.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/TraceLevels.h>

using namespace std;
using namespace IcePatch2;
using namespace IceGrid;

NodeI::NodeI(const Ice::CommunicatorPtr& communicator,
	     const ActivatorPtr& activator, 
	     const WaitQueuePtr& waitQueue,
	     const TraceLevelsPtr& traceLevels,
	     const string& name) :
    _communicator(communicator),
    _activator(activator),
    _waitQueue(waitQueue),
    _traceLevels(traceLevels),
    _name(name),
    _hostname(IceInternal::getProtocolPluginFacade(communicator)->getDefaultHost())
{
    const string dataDir = _communicator->getProperties()->getProperty("IceGrid.Node.Data");
    _serversDir = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "servers";
    _tmpDir = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "tmp";
}

ServerPrx
NodeI::loadServer(const ServerDescriptorPtr& desc, StringAdapterPrxDict& adapters, const Ice::Current& current)
{
    Ice::Identity id;
    id.category = "IceGridServer";
    id.name = desc->name;

    ServerI* server;
    {
	Lock sync(*this);
	Ice::ObjectPtr servant = current.adapter->find(id);
	if(!servant)
	{
	    server = new ServerI(this, _serversDir, desc->name);
	    current.adapter->add(server, id);
	}
	else
	{
	    server = dynamic_cast<ServerI*>(servant.get());
	}
    }
    ServerPrx proxy = ServerPrx::uncheckedCast(current.adapter->createProxy(id));
    server->load(desc, adapters, proxy, current);
    return proxy;
}

void
NodeI::destroyServer(const ServerDescriptorPtr& desc, const Ice::Current& current)
{
    StringAdapterPrxDict dict;
    ServerPrx server = loadServer(desc, dict, current);
    try
    {
	server->destroy();
    }
    catch(const Ice::LocalException&)
    {
    }
}

std::string
NodeI::getName(const Ice::Current&) const
{
    return _name;
}

std::string
NodeI::getHostname(const Ice::Current&) const
{
    return _hostname;
}

void
NodeI::shutdown(const Ice::Current&) const
{
    _activator->shutdown();
}

Ice::CommunicatorPtr
NodeI::getCommunicator() const
{
    return _communicator;
}

ActivatorPtr
NodeI::getActivator() const
{
    return _activator;
}

WaitQueuePtr
NodeI::getWaitQueue() const
{
    return _waitQueue;
}

TraceLevelsPtr
NodeI::getTraceLevels() const
{
    return _traceLevels;
}

void
NodeI::checkConsistency(const Ice::StringSeq& servers)
{
    //
    // Check if the servers directory doesn't contain more servers
    // than the registry really knows.
    //
    Ice::StringSeq contents = readDirectory(_serversDir);
    vector<string> remove;
    set_difference(contents.begin(), contents.end(), servers.begin(), servers.end(), back_inserter(remove));
    
    //
    // Remove the extra servers if possible.
    //
    vector<string>::iterator p = remove.begin();
    while(p != remove.end())
    {
	if(canRemoveServerDirectory(*p))
	{
	    removeRecursive(_serversDir + "/" + *p);
	    remove.erase(p++);
	}
	else
	{
	    ++p;
	}
    }

    if(remove.empty())
    {
	return;
    }

    //
    // If there's server that couldn't be removed we move them to the
    // temporary backup directory. First, we rotate the temporary
    // backup directories.
    //
    contents = readDirectory(_tmpDir);
    if(contents.size() < 10)
    {
	ostringstream os;
	os << "servers-" << contents.size();
	contents.push_back(os.str());
    }
    sort(contents.begin(), contents.end(), greater<string>());
    if(contents.size() == 10)
    {
	try
	{
	    removeRecursive(_tmpDir + "/" + *contents.begin());
	}
	catch(const string&)
	{
	}
    }
    for(Ice::StringSeq::const_iterator p = contents.begin(); p != --contents.end(); ++p)
    {
	rename(_tmpDir + "/" + *(p + 1), _tmpDir + "/" + *p);
    }
    createDirectoryRecursive(_tmpDir + "/servers-0");

    for(vector<string>::const_iterator p = remove.begin(); p != remove.end(); ++p)
    {
	rename(_serversDir + "/" + *p, _tmpDir + "/servers-0/" + *p);
    }
}

void 
NodeI::removeServerDirectory(const string& backupDir, const string& name)
{    
}

bool
NodeI::canRemoveServerDirectory(const string& name)
{
    //
    // Check if there's files which we didn't create.
    //
    Ice::StringSeq contents = readDirectory(_serversDir + "/" + name);
    remove(contents.begin(), contents.end(), "dbs");
    remove(contents.begin(), contents.end(), "config");
    if(!contents.empty())
    {
	return false;
    }

    contents = readDirectory(_serversDir + "/" + name + "/config");

    for(Ice::StringSeq::const_iterator p = contents.begin() ; p != contents.end(); ++p)
    {
	if(p->find("config") != 0)
	{
	    return false;
	}
    }

    contents = readDirectory(_serversDir + "/" + name + "/dbs");
    for(Ice::StringSeq::const_iterator p = contents.begin() ; p != contents.end(); ++p)
    {
	Ice::StringSeq files = readDirectory(_serversDir + "/" + name + "/dbs/" + *p);
	remove(files.begin(), files.end(), "DB_CONFIG");
	if(!files.empty())
	{
	    return false;
	}
    }

    return true;
}
