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

InternalRegistryI::InternalRegistryI(const DatabasePtr& database, const ReapThreadPtr& reaper) : 
    _database(database), 
    _reaper(reaper)
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
				   const ReplicaInfo& info,
				   const Ice::Current& current)
{
    ReplicaSessionIPtr session = new ReplicaSessionI(_database, name, replica, info);
    ReplicaSessionPrx proxy = ReplicaSessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(new SessionReapable<ReplicaSessionI>(current.adapter, session, proxy));
    return proxy;
}
