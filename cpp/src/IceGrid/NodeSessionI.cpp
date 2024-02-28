//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UUID.h>
#include <Ice/Ice.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

    class PatcherFeedbackI : public PatcherFeedback, public std::enable_shared_from_this<PatcherFeedback>
    {
    public:
        PatcherFeedbackI(
            const string& node,
            const shared_ptr<NodeSessionI>& session,
            const Ice::Identity id,
            const shared_ptr<PatcherFeedbackAggregator>& aggregator)
            : _node(node),
              _session(session),
              _id(id),
              _aggregator(aggregator)
        {
        }

        void finished(const Ice::Current&)
        {
            _aggregator->finished(_node);
            _session->removeFeedback(shared_from_this(), _id);
        }

        virtual void failed(string reason, const Ice::Current& = Ice::Current())
        {
            _aggregator->failed(_node, std::move(reason));
            _session->removeFeedback(shared_from_this(), _id);
        }

    private:
        const std::string _node;
        const shared_ptr<NodeSessionI> _session;
        const Ice::Identity _id;
        const shared_ptr<PatcherFeedbackAggregator> _aggregator;
    };

}

PatcherFeedbackAggregator::PatcherFeedbackAggregator(
    Ice::Identity id,
    const shared_ptr<TraceLevels>& traceLevels,
    const string& type,
    const string& name,
    int nodeCount)
    : _id(id),
      _traceLevels(traceLevels),
      _type(type),
      _name(name),
      _count(nodeCount)
{
}

PatcherFeedbackAggregator::~PatcherFeedbackAggregator() {}

void
PatcherFeedbackAggregator::finished(const string& node)
{
    lock_guard lock(_mutex);
    if (_successes.find(node) != _successes.end() || _failures.find(node) != _failures.end())
    {
        return;
    }

    if (_traceLevels->patch > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
        out << "finished patching of " << _type << " `" << _name << "' on node `" << node << "'";
    }

    _successes.insert(node);
    checkIfDone();
}

void
PatcherFeedbackAggregator::failed(string node, const string& failure)
{
    lock_guard lock(_mutex);
    if (_successes.find(node) != _successes.end() || _failures.find(node) != _failures.end())
    {
        return;
    }

    if (_traceLevels->patch > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
        out << "patching of " << _type << " `" << _name << "' on node `" << node << "' failed:\n" << failure;
    }

    _failures.insert(node);
    _reasons.push_back("patch on node `" + node + "' failed:\n" + failure);
    checkIfDone();
}

void
PatcherFeedbackAggregator::checkIfDone()
{
    if (static_cast<int>(_successes.size() + _failures.size()) == _count)
    {
        if (!_failures.empty())
        {
            try
            {
                sort(_reasons.begin(), _reasons.end());
                throw PatchException(_reasons);
            }
            catch (...)
            {
                exception(current_exception());
            }
        }
        else
        {
            response();
        }
    }
}

shared_ptr<NodeSessionI>
NodeSessionI::create(
    const shared_ptr<Database>& database,
    const NodePrxPtr& node,
    const shared_ptr<InternalNodeInfo>& info,
    std::chrono::seconds timeout,
    const LoadInfo& load)
{
    shared_ptr<NodeSessionI> nodeSession(new NodeSessionI(database, node, info, timeout, load));
    try
    {
        database->getNode(info->name, true)->setSession(nodeSession);

        ObjectInfo objInfo = {node, string{Node::ice_staticId()}};
        database->addInternalObject(objInfo, true); // Add or update previous node proxy.

        nodeSession->_proxy =
            Ice::uncheckedCast<NodeSessionPrx>(database->getInternalAdapter()->addWithUUID(nodeSession));
    }
    catch (const NodeActiveException&)
    {
        throw;
    }
    catch (const std::exception&)
    {
        try
        {
            database->removeInternalObject(node->ice_getIdentity());
        }
        catch (const ObjectNotRegisteredException&)
        {
        }

        database->getNode(info->name)->setSession(nullptr);

        throw;
    }

    return nodeSession;
}

NodeSessionI::NodeSessionI(
    const shared_ptr<Database>& database,
    const NodePrxPtr& node,
    const shared_ptr<InternalNodeInfo>& info,
    std::chrono::seconds timeout,
    const LoadInfo& load)
    : _database(database),
      _traceLevels(database->getTraceLevels()),
      _node(node),
      _info(info),
      _timeout(timeout),
      _timestamp(chrono::steady_clock::now()),
      _load(load),
      _destroy(false)
{
}

void
NodeSessionI::keepAlive(LoadInfo load, const Ice::Current&)
{
    lock_guard lock(_mutex);

    if (_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = chrono::steady_clock::now();
    _load = std::move(load);

    if (_traceLevels->node > 2)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
        out << "node `" << _info->name << "' keep alive ";
        out << "(load = " << _load.avg1 << ", " << _load.avg5 << ", " << _load.avg15 << ")";
    }
}

void
NodeSessionI::setReplicaObserver(ReplicaObserverPrxPtr observer, const Ice::Current&)
{
    lock_guard lock(_mutex);

    if (_destroy)
    {
        return;
    }
    else if (_replicaObserver) // This might happen on activation of the node.
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
    return secondsToInt(_timeout);
}

NodeObserverPrxPtr
NodeSessionI::getObserver(const Ice::Current&) const
{
    return dynamic_pointer_cast<NodeObserverTopic>(_database->getObserverTopic(TopicName::NodeObserver))
        ->getPublisher();
}

void
NodeSessionI::loadServersAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current&) const
{
    //
    // No need to wait for the servers to be loaded. If we were
    // waiting, we would have to figure out an appropriate timeout for
    // calling this method since each load() call might take time to
    // complete.
    //
    response();

    //
    // Get the server proxies to load them on the node.
    //
    auto servers = _database->getNode(_info->name)->getServers();
    for (const auto& server : servers)
    {
        server->sync();
        server->waitForSyncNoThrow(1s); // Don't wait too long.
    }
}

Ice::StringSeq
NodeSessionI::getServers(const Ice::Current&) const
{
    auto servers = _database->getNode(_info->name)->getServers();
    Ice::StringSeq names;
    for (const auto& server : servers)
    {
        names.push_back(server->getId());
    }
    return names;
}

void
NodeSessionI::waitForApplicationUpdateAsync(
    std::string application,
    int revision,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Ice::Current&) const
{
    _database->waitForApplicationUpdate(
        std::move(application), std::move(revision), std::move(response), std::move(exception));
}

void
NodeSessionI::destroy(const Ice::Current&)
{
    destroyImpl(false);
}

chrono::steady_clock::time_point
NodeSessionI::timestamp() const
{
    lock_guard lock(_mutex);
    if (_destroy)
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
NodeSessionI::patch(
    const shared_ptr<PatcherFeedbackAggregator>& aggregator,
    const string& application,
    const string& server,
    const shared_ptr<InternalDistributionDescriptor>& dist,
    bool shutdown)
{
    Ice::Identity id;
    id.category = _database->getInstanceName();
    id.name = Ice::generateUUID();

    auto obj = make_shared<PatcherFeedbackI>(_info->name, shared_from_this(), id, aggregator);
    try
    {
        auto feedback = Ice::uncheckedCast<PatcherFeedbackPrx>(_database->getInternalAdapter()->add(obj, id));
        _node->patch(feedback, application, server, dist, shutdown);

        lock_guard lock(_mutex);
        if (_destroy)
        {
            throw NodeUnreachableException(_info->name, "node is down");
        }
        _feedbacks.insert(obj);
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "node unreachable:\n" << ex;
        obj->failed(os.str());
    }
}

const NodePrxPtr&
NodeSessionI::getNode() const
{
    return _node;
}

const shared_ptr<InternalNodeInfo>&
NodeSessionI::getInfo() const
{
    return _info;
}

const LoadInfo&
NodeSessionI::getLoadInfo() const
{
    lock_guard lock(_mutex);
    return _load;
}

NodeSessionPrxPtr
NodeSessionI::getProxy() const
{
    return _proxy;
}

bool
NodeSessionI::isDestroyed() const
{
    lock_guard lock(_mutex);
    return _destroy;
}

void
NodeSessionI::destroyImpl(bool shutdown)
{
    {
        lock_guard lock(_mutex);
        if (_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _destroy = true;
    }

    ServerEntrySeq servers = _database->getNode(_info->name)->getServers();
    for (const auto& server : servers)
    {
        server->unsync();
    }

    //
    // If the registry isn't being shutdown we remove the node
    // internal proxy from the database.
    //
    if (!shutdown)
    {
        _database->removeInternalObject(_node->ice_getIdentity());
    }

    //
    // Next we notify the observer.
    //
    static_pointer_cast<NodeObserverTopic>(_database->getObserverTopic(TopicName::NodeObserver))->nodeDown(_info->name);

    //
    // Unsubscribe the node replica observer.
    //
    if (_replicaObserver)
    {
        _database->getReplicaCache().unsubscribe(_replicaObserver);
        _replicaObserver = nullopt;
    }

    //
    // Finally, we clear the session, this must be done last. As soon
    // as the node entry session is set to 0 another session might be
    // created.
    //
    _database->getNode(_info->name)->setSession(nullptr);

    if (!shutdown)
    {
        try
        {
            _database->getInternalAdapter()->remove(_proxy->ice_getIdentity());
        }
        catch (const Ice::ObjectAdapterDeactivatedException&)
        {
        }
    }
}

void
NodeSessionI::removeFeedback(const shared_ptr<PatcherFeedback>& feedback, const Ice::Identity& id)
{
    try
    {
        _database->getInternalAdapter()->remove(id);
    }
    catch (const Ice::LocalException&)
    {
    }

    lock_guard lock(_mutex);
    _feedbacks.erase(feedback);
}
