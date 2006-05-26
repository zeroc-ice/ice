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

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class NodeSessionReapable : public Reapable
{
public:

    NodeSessionReapable(const Ice::ObjectAdapterPtr& adapter, 
			const NodeSessionIPtr& session, 
			const NodeSessionPrx& proxy) : 
	_adapter(adapter),
	_session(session),
	_proxy(proxy)
    {
    }

    virtual ~NodeSessionReapable()
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
	    out << "unexpected exception while reaping node session:\n" << ex;
	}
    }

private:

    const Ice::ObjectAdapterPtr _adapter;
    const NodeSessionIPtr _session;
    const NodeSessionPrx _proxy;
};

}

InternalRegistryI::InternalRegistryI(const DatabasePtr& database,
				     const ReapThreadPtr& reaper, 
				     const NodeObserverPrx& observer,
				     int timeout) : 
    _database(database), 
    _nodeReaper(reaper),
    _nodeObserver(observer),
    _nodeSessionTimeout(timeout)
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
    NodeSessionIPtr session = new NodeSessionI(_database, name, node, info, _nodeObserver, _nodeSessionTimeout);
    NodeSessionPrx proxy = NodeSessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _nodeReaper->add(new NodeSessionReapable(current.adapter, session, proxy));
    return proxy;
}
