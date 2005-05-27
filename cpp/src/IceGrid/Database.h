// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_DATABASE_H
#define ICE_GRID_DATABASE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Cache.h>
#include <Freeze/ConnectionF.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Internal.h>
#include <IceGrid/StringApplicationDescriptorDict.h>
#include <IceGrid/IdentityObjectDescDict.h>
#include <IceGrid/StringObjectProxyDict.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Database : public IceUtil::Shared, public IceUtil::Mutex
{
public:
    
    Database(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const std::string&, const TraceLevelsPtr&);
    virtual ~Database();

    void addApplicationDescriptor(const ApplicationDescriptorPtr&);
    void updateApplicationDescriptor(const ApplicationDescriptorPtr&);
    void removeApplicationDescriptor(const std::string&);
    ApplicationDescriptorPtr getApplicationDescriptor(const std::string&);
    Ice::StringSeq getAllApplications(const std::string& = std::string());

    void addNode(const std::string&, const NodePrx&);
    NodePrx getNode(const std::string&) const;
    void removeNode(const std::string&);
    Ice::StringSeq getAllNodes(const std::string& = std::string());
    
    InstanceDescriptor getServerDescriptor(const std::string&);
    ServerPrx getServer(const std::string&);
    Ice::StringSeq getAllServers(const std::string& = std::string());
    Ice::StringSeq getAllNodeServers(const std::string&);

    void setAdapterDirectProxy(const std::string&, const Ice::ObjectPrx&);
    Ice::ObjectPrx getAdapterDirectProxy(const std::string&);
    AdapterPrx getAdapter(const std::string&);
    Ice::StringSeq getAllAdapters(const std::string& = std::string());

    void addObjectDescriptor(const ObjectDescriptor&);
    void removeObjectDescriptor(const Ice::Identity&);
    void updateObjectDescriptor(const Ice::ObjectPrx&);
    ObjectDescriptor getObjectDescriptor(const Ice::Identity&);
    Ice::ObjectPrx getObjectByType(const std::string&);
    Ice::ObjectProxySeq getObjectsWithType(const std::string&);
    ObjectDescriptorSeq getAllObjectDescriptors(const std::string& = std::string());

private:

    class ServerEntry : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
    {
    public:

	ServerEntry(Database&, const ServerDescriptorPtr&);

	void sync();
	void update(const ServerDescriptorPtr&);
	ServerDescriptorPtr getDescriptor();
	ServerPrx getProxy();
	AdapterPrx getAdapter(const std::string&);
	bool canRemove();

    private:

	ServerPrx sync(StringAdapterPrxDict& adapters);

	Database& _database;
	ServerDescriptorPtr _loaded;
	ServerDescriptorPtr _load;
	ServerDescriptorPtr _destroy;
	ServerPrx _proxy;
	std::map<std::string, AdapterPrx> _adapters;
	bool _synchronizing;
    };
    friend class ServerEntry;
    friend struct AddComponent;

    typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
    typedef std::vector<ServerEntryPtr> ServerEntrySeq;

    void addServers(const InstanceDescriptorSeq&, const std::set<std::string>&, ServerEntrySeq&);
    void updateServers(const ApplicationDescriptorPtr&, const ApplicationDescriptorPtr&,
		       const std::set<std::string>&, ServerEntrySeq&);
    void removeServers(const InstanceDescriptorSeq&, const std::set<std::string>&, ServerEntrySeq&);
    ServerEntryPtr addServer(const InstanceDescriptor&);
    ServerEntryPtr updateServer(const InstanceDescriptor&);
    ServerEntryPtr removeServer(const InstanceDescriptor&);
    void clearServer(const std::string&);
    void addComponent(const ServerEntryPtr&, const ComponentDescriptorPtr&);
    void removeComponent(const ComponentDescriptorPtr&);

    void checkServerForAddition(const std::string&);
    void checkAdapterForAddition(const std::string&);
    void checkObjectForAddition(const Ice::Identity&);

    static const std::string _descriptorDbName;
    static const std::string _nodeDbName;
    static const std::string _objectDbName;
    static const std::string _adapterDbName;

    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _internalAdapter;
    const std::string _envName;
    const TraceLevelsPtr _traceLevels;

    std::map<std::string, ServerEntryPtr> _servers;
    std::map<std::string, ServerEntryPtr> _serversByAdapterId;
    std::map<std::string, std::set<std::string> > _serversByNode;

    Freeze::ConnectionPtr _connection;
    StringApplicationDescriptorDict _descriptors;
    StringObjectProxyDict _nodes;
    IdentityObjectDescDict _objects;
    StringObjectProxyDict _adapters;
};
typedef IceUtil::Handle<Database> DatabasePtr;

};

#endif
