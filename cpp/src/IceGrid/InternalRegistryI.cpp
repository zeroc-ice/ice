// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceGrid/RegistryI.h>
#include <IceGrid/InternalRegistryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Topics.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/FileCache.h>

using namespace std;
using namespace IceGrid;

InternalRegistryI::InternalRegistryI(const RegistryIPtr& registry,
				     const DatabasePtr& database, 
				     const ReapThreadPtr& reaper,
				     const WellKnownObjectsManagerPtr& wellKnownObjects,
				     ReplicaSessionManager& session) : 
    _registry(registry),
    _database(database),
    _reaper(reaper),
    _wellKnownObjects(wellKnownObjects),
    _fileCache(new FileCache()),
    _session(session)
{
    Ice::PropertiesPtr properties = database->getCommunicator()->getProperties();
    _nodeSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 30);
    _replicaSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.ReplicaSessionTimeout", 30);
}

InternalRegistryI::~InternalRegistryI()
{
}

NodeSessionPrx
InternalRegistryI::registerNode(const NodeInfo& info, const NodePrx& node, const Ice::Current& current)
{
    const Ice::LoggerPtr logger = _database->getTraceLevels()->logger;
    try
    {
	NodeSessionIPtr session = new NodeSessionI(_database, node, info, _nodeSessionTimeout);
	_reaper->add(new SessionReapable<NodeSessionI>(logger, session), _nodeSessionTimeout);
	return session->getProxy();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }
}

ReplicaSessionPrx
InternalRegistryI::registerReplica(const RegistryInfo& info,
				   const InternalRegistryPrx& prx,
				   const Ice::Current& current)
{
    const Ice::LoggerPtr logger = _database->getTraceLevels()->logger;
    try
    {
	ReplicaSessionIPtr s = new ReplicaSessionI(_database, _wellKnownObjects, info, prx, _replicaSessionTimeout);
	_reaper->add(new SessionReapable<ReplicaSessionI>(logger, s), _replicaSessionTimeout);
	return s->getProxy();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }
}

void
InternalRegistryI::registerWithReplica(const InternalRegistryPrx& replica, const Ice::Current&)
{
    _session.create(replica);
}

NodePrxSeq
InternalRegistryI::getNodes(const Ice::Current&) const
{
    NodePrxSeq nodes;
    Ice::ObjectProxySeq proxies = _database->getInternalObjectsByType(Node::ice_staticId());
    for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
	nodes.push_back(NodePrx::uncheckedCast(*p));
    }
    return nodes;
}

InternalRegistryPrxSeq
InternalRegistryI::getReplicas(const Ice::Current&) const
{
    InternalRegistryPrxSeq replicas;
    Ice::ObjectProxySeq proxies = _database->getObjectsByType(InternalRegistry::ice_staticId());
    for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
	replicas.push_back(InternalRegistryPrx::uncheckedCast(*p));
    }
    return replicas;
}

void
InternalRegistryI::shutdown(const Ice::Current& current) const
{
    _registry->shutdown();
}

Ice::StringSeq
InternalRegistryI::readLines(const string& filename, Ice::Long pos, int count, Ice::Long& newPos, 
			     const Ice::Current&) const
{
    string file;
    if(filename == "stderr")
    {
	file = _database->getCommunicator()->getProperties()->getProperty("Ice.StdErr");
	if(file.empty())
	{
	    throw FileNotAvailableException("Ice.StdErr configuration property is not set");
	}
    }
    else if(filename == "stdout")
    {
	file = _database->getCommunicator()->getProperties()->getProperty("Ice.StdOut");
	if(file.empty())
	{
	    throw FileNotAvailableException("Ice.StdOut configuration property is not set");
	}
    }
    else
    {
	throw FileNotAvailableException("unknown file");
    }

    return _fileCache->read(file, pos, count, newPos);
}
