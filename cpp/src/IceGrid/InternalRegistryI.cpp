// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceGrid/InternalRegistryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Topics.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/ReplicaSessionManager.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

template<class T>
class SessionReapable : public Reapable
{
    typedef IceUtil::Handle<T> TPtr;
    
public:
    
    SessionReapable(const Ice::ObjectAdapterPtr& adapter, const TPtr& session, const Ice::ObjectPrx& proxy) : 
	_adapter(adapter),
	_session(session),
	_proxy(proxy)
    {
    }

    virtual ~SessionReapable()
    {
    }
	
    virtual IceUtil::Time
    timestamp() const
    {
	return _session->timestamp();
    }

    virtual void
    destroy(bool destroy)
    {
	try
	{
	    //
	    // Invoke on the servant directly instead of the
	    // proxy. Invoking on the proxy might not always work if the
	    // communicator is being shutdown/destroyed. We have to create
	    // a fake "current" because the session destroy methods needs
	    // the adapter and object identity to unregister the servant
	    // from the adapter.
	    //
	    Ice::Current current;
	    if(!destroy)
	    {
		current.adapter = _adapter;
		current.id = _proxy->ice_getIdentity();
	    }
	    _session->destroy(current);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException& ex)
	{
	    Ice::Warning out(_proxy->ice_getCommunicator()->getLogger());
	    out << "unexpected exception while reaping session:\n" << ex;
	}
    }

private:

    const Ice::ObjectAdapterPtr _adapter;
    const TPtr _session;
    const Ice::ObjectPrx _proxy;
};

}

InternalRegistryI::InternalRegistryI(const DatabasePtr& database, 
				     const ReapThreadPtr& reaper,			
				     ReplicaSessionManager& session) : 
    _database(database),
    _reaper(reaper),
    _session(session)
{
}

InternalRegistryI::~InternalRegistryI()
{
}

NodeSessionPrx
InternalRegistryI::registerNode(const std::string& name,
				const NodePrx& node, 
				const NodeInfo& info,
				const Ice::Current& current)
{
    NodeSessionIPtr session = new NodeSessionI(_database, name, node, info);
    NodeSessionPrx proxy = NodeSessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(new SessionReapable<NodeSessionI>(current.adapter, session, proxy));
    return proxy;
}

ReplicaSessionPrx
InternalRegistryI::registerReplica(const std::string& name,
				   const InternalRegistryPrx& replica, 
				   const RegistryObserverPrx& observer,
				   const Ice::Current& current)
{
    ReplicaSessionIPtr session = new ReplicaSessionI(_database, name, replica, observer);
    ReplicaSessionPrx proxy = ReplicaSessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(new SessionReapable<ReplicaSessionI>(current.adapter, session, proxy));
    return proxy;
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
    try
    {
	Ice::ObjectProxySeq proxies = _database->getObjectsByType(Node::ice_staticId());
	for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
	{
	    nodes.push_back(NodePrx::uncheckedCast(*p));
	}
    }
    catch(const ObjectNotRegisteredException&)
    {
    }
    return nodes;
}

InternalRegistryPrxSeq
InternalRegistryI::getReplicas(const Ice::Current&) const
{
    InternalRegistryPrxSeq replicas;
    try
    {
	Ice::ObjectProxySeq proxies = _database->getObjectsByType(InternalRegistry::ice_staticId());
	for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
	{
	    replicas.push_back(InternalRegistryPrx::uncheckedCast(*p));
	}
    }
    catch(const ObjectNotRegisteredException&)
    {
    }
    return replicas;
}
