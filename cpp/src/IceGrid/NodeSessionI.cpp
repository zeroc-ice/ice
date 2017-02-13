// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UUID.h>
#include <Ice/Ice.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class PatcherFeedbackI : public PatcherFeedback
{
public:

    PatcherFeedbackI(const string& node, 
                     const NodeSessionIPtr& session, 
                     const Ice::Identity id,
                     const PatcherFeedbackAggregatorPtr& aggregator) :
        _node(node),
        _session(session),
        _id(id),
        _aggregator(aggregator)
    {
    }

    void finished(const Ice::Current&)
    {
        _aggregator->finished(_node);
        _session->removeFeedback(this, _id);
    }

    virtual void failed(const string& reason, const Ice::Current& = Ice::Current())
    {
        _aggregator->failed(_node, reason);
        _session->removeFeedback(this, _id);
    }

private:

    const std::string _node;
    const NodeSessionIPtr _session;
    const Ice::Identity _id;
    const PatcherFeedbackAggregatorPtr _aggregator;
};

};

PatcherFeedbackAggregator::PatcherFeedbackAggregator(Ice::Identity id,
                                                     const TraceLevelsPtr& traceLevels,
                                                     const string& type,
                                                     const string& name,
                                                     int nodeCount) : 
    _id(id),
    _traceLevels(traceLevels),
    _type(type),
    _name(name),
    _count(nodeCount)
{
}

PatcherFeedbackAggregator::~PatcherFeedbackAggregator()
{
}

void
PatcherFeedbackAggregator::finished(const string& node)
{
    Lock sync(*this);
    if(_successes.find(node) != _successes.end() || _failures.find(node) != _failures.end())
    {
        return;
    }
    
    if(_traceLevels->patch > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
        out << "finished patching of " << _type << " `" << _name << "' on node `" << node << "'";
    }
    
    _successes.insert(node);
    checkIfDone();
}

void
PatcherFeedbackAggregator::failed(const string& node, const string& failure)
{
    Lock sync(*this);
    if(_successes.find(node) != _successes.end() || _failures.find(node) != _failures.end())
    {
        return;
    }
    
    if(_traceLevels->patch > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
        out << "patching of " << _type << " `" << _name << "' on node `" << node <<"' failed:\n" << failure;
    }
    
    _failures.insert(node);
    _reasons.push_back("patch on node `" + node + "' failed:\n" + failure);
    checkIfDone();
}

void
PatcherFeedbackAggregator::checkIfDone()
{
    if(static_cast<int>(_successes.size() + _failures.size()) == _count)
    {
        if(!_failures.empty())
        {
            sort(_reasons.begin(), _reasons.end());
            PatchException ex;
            ex.reasons = _reasons;
            exception(ex);
        }
        else
        {
            response();
        }
    }
}

NodeSessionI::NodeSessionI(const DatabasePtr& database, 
                           const NodePrx& node, 
                           const InternalNodeInfoPtr& info,
                           int timeout,
                           const LoadInfo& load) :
    _database(database),
    _traceLevels(database->getTraceLevels()),
    _node(node),
    _info(info),
    _timeout(timeout),
    _timestamp(IceUtil::Time::now(IceUtil::Time::Monotonic)),
    _load(load),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
        _database->getNode(info->name, true)->setSession(this);

        ObjectInfo objInfo;
        objInfo.type = Node::ice_staticId();
        objInfo.proxy = _node;
        _database->addInternalObject(objInfo, true); // Add or update previous node proxy.

        _proxy = NodeSessionPrx::uncheckedCast(_database->getInternalAdapter()->addWithUUID(this));
    }
    catch(const NodeActiveException&)
    {
        __setNoDelete(false);
        throw;
    }
    catch(...)
    {
        try
        {
            _database->removeInternalObject(_node->ice_getIdentity());
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        _database->getNode(info->name)->setSession(0);

        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

void
NodeSessionI::keepAlive(const LoadInfo& load, const Ice::Current&)
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = IceUtil::Time::now(IceUtil::Time::Monotonic);
    _load = load;

    if(_traceLevels->node > 2)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
        out << "node `" << _info->name << "' keep alive ";
        out << "(load = " << _load.avg1 << ", " << _load.avg5 << ", " << _load.avg15 << ")";
    }
}

void
NodeSessionI::setReplicaObserver(const ReplicaObserverPrx& observer, const Ice::Current&)
{
    Lock sync(*this);
    if(_destroy)
    {
        return;
    }
    else if(_replicaObserver) // This might happen on activation of the node.
    {
        assert(_replicaObserver == observer);
        return;
    }

    _replicaObserver = observer;
    _database->getReplicaCache().subscribe(observer);
}

int
NodeSessionI::getTimeout(const Ice::Current&) const
{
    return _timeout;
}

NodeObserverPrx
NodeSessionI::getObserver(const Ice::Current&) const
{
    return NodeObserverTopicPtr::dynamicCast(_database->getObserverTopic(NodeObserverTopicName))->getPublisher();
}

void
NodeSessionI::loadServers_async(const AMD_NodeSession_loadServersPtr& amdCB, const Ice::Current&) const
{
    //
    // No need to wait for the servers to be loaded. If we were
    // waiting, we would have to figure out an appropriate timeout for
    // calling this method since each load() call might take time to
    // complete.
    //
    amdCB->ice_response();

    //
    // Get the server proxies to load them on the node.
    //
    ServerEntrySeq servers = _database->getNode(_info->name)->getServers();
    for(ServerEntrySeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
        (*p)->sync();
        (*p)->waitForSyncNoThrow(1); // Don't wait too long.
    }
}

Ice::StringSeq
NodeSessionI::getServers(const Ice::Current&) const
{
    ServerEntrySeq servers =  _database->getNode(_info->name)->getServers();
    Ice::StringSeq names;
    for(ServerEntrySeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
        names.push_back((*p)->getId());
    }
    return names;
}

void
NodeSessionI::waitForApplicationUpdate_async(const AMD_NodeSession_waitForApplicationUpdatePtr& cb, 
                                             const std::string& application, 
                                             int revision, 
                                             const Ice::Current&) const
{
    _database->waitForApplicationUpdate(cb, application, revision);
}

void
NodeSessionI::destroy(const Ice::Current&)
{
    destroyImpl(false);
}

IceUtil::Time
NodeSessionI::timestamp() const
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return _timestamp;
}

void
NodeSessionI::shutdown()
{
    destroyImpl(true);
}

void
NodeSessionI::patch(const PatcherFeedbackAggregatorPtr& aggregator, 
                    const string& application,
                    const string& server,
                    const InternalDistributionDescriptorPtr& dist, 
                    bool shutdown)
{
    Ice::Identity id;
    id.category = _database->getInstanceName();
    id.name = Ice::generateUUID();

    PatcherFeedbackPtr obj = new PatcherFeedbackI(_info->name, this, id, aggregator);
    try
    {
        PatcherFeedbackPrx feedback = PatcherFeedbackPrx::uncheckedCast(_database->getInternalAdapter()->add(obj, id));
        _node->patch(feedback, application, server, dist, shutdown);

        Lock sync(*this);
        if(_destroy)
        {
            throw NodeUnreachableException(_info->name, "node is down");
        }
        _feedbacks.insert(obj);
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "node unreachable:\n" << ex;
        obj->failed(os.str());
    }
}

const NodePrx&
NodeSessionI::getNode() const
{
    return _node;
}

const InternalNodeInfoPtr&
NodeSessionI::getInfo() const
{
    return _info;
}

const LoadInfo&
NodeSessionI::getLoadInfo() const
{
    Lock sync(*this);
    return _load;
}

NodeSessionPrx
NodeSessionI::getProxy() const
{
    return _proxy;
}

bool
NodeSessionI::isDestroyed() const
{
    Lock sync(*this);
    return _destroy;
}

void
NodeSessionI::destroyImpl(bool shutdown)
{
    {
        Lock sync(*this);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }       
        _destroy = true;
    }

    ServerEntrySeq servers = _database->getNode(_info->name)->getServers();
    for_each(servers.begin(), servers.end(), IceUtil::voidMemFun(&ServerEntry::unsync));

    //
    // If the registry isn't being shutdown we remove the node
    // internal proxy from the database.
    // 
    if(!shutdown)
    {
        _database->removeInternalObject(_node->ice_getIdentity());
    }

    //
    // Next we notify the observer.
    //
    NodeObserverTopicPtr::dynamicCast(_database->getObserverTopic(NodeObserverTopicName))->nodeDown(_info->name);

    //
    // Unsubscribe the node replica observer.
    //
    if(_replicaObserver)
    {
        _database->getReplicaCache().unsubscribe(_replicaObserver);
        _replicaObserver = 0;
    }

    //
    // Finally, we clear the session, this must be done last. As soon
    // as the node entry session is set to 0 another session might be
    // created.
    //
    _database->getNode(_info->name)->setSession(0);

    //
    // Clean up the patcher feedback servants (this will call back
    // removeFeedback so we need to use a temporary set).
    //
    set<PatcherFeedbackPtr> feedbacks;
    _feedbacks.swap(feedbacks);
    for(set<PatcherFeedbackPtr>::const_iterator p = feedbacks.begin(); p != feedbacks.end(); ++p)
    {
        (*p)->failed("node is down");
    }

    if(!shutdown)
    {
        try
        {
            _database->getInternalAdapter()->remove(_proxy->ice_getIdentity());
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
        }
    }
}

void
NodeSessionI::removeFeedback(const PatcherFeedbackPtr& feedback, const Ice::Identity& id)
{
    try
    {
        _database->getInternalAdapter()->remove(id);
    }
    catch(const Ice::LocalException&)
    {
    }
    {
        Lock sync(*this);
        _feedbacks.erase(feedback);
    }
}

