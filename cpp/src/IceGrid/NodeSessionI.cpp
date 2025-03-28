// Copyright (c) ZeroC, Inc.

#include "NodeSessionI.h"
#include "Database.h"
#include "Ice/Ice.h"
#include "Ice/UUID.h"
#include "Topics.h"

using namespace std;
using namespace IceGrid;

shared_ptr<NodeSessionI>
NodeSessionI::create(
    const shared_ptr<Database>& database,
    NodePrx node,
    const shared_ptr<InternalNodeInfo>& info,
    std::chrono::seconds timeout,
    const LoadInfo& load)
{
    Ice::Identity nodeSessionId{Ice::generateUUID(), ""};

    shared_ptr<NodeSessionI> nodeSession(new NodeSessionI(
        database,
        node,
        info,
        timeout,
        database->getInternalAdapter()->createProxy<NodeSessionPrx>(nodeSessionId),
        load));
    try
    {
        database->getNode(info->name, true)->setSession(nodeSession);

        ObjectInfo objInfo = {node, string{Node::ice_staticId()}};
        database->addInternalObject(objInfo, true); // Add or update previous node proxy.
        database->getInternalAdapter()->add(nodeSession, nodeSessionId);
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
    NodePrx node,
    const shared_ptr<InternalNodeInfo>& info,
    std::chrono::seconds timeout,
    NodeSessionPrx proxy,
    const LoadInfo& load)
    : _database(database),
      _traceLevels(database->getTraceLevels()),
      _node(std::move(node)),
      _info(info),
      _timeout(timeout),
      _proxy(std::move(proxy)),
      _timestamp(chrono::steady_clock::now()),
      _load(load)
{
}

void
NodeSessionI::keepAlive(LoadInfo load, const Ice::Current&)
{
    lock_guard lock(_mutex);

    if (_destroy)
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }

    _timestamp = chrono::steady_clock::now();
    _load = load;

    if (_traceLevels->node > 2)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
        out << "node '" << _info->name << "' keep alive ";
        out << "(load = " << _load.avg1 << ", " << _load.avg5 << ", " << _load.avg15 << ")";
    }
}

void
NodeSessionI::setReplicaObserver(std::optional<ReplicaObserverPrx> observer, const Ice::Current& current)
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

    Ice::checkNotNull(observer, __FILE__, __LINE__, current);
    _replicaObserver = *observer;
    _database->getReplicaCache().subscribe(*observer);
}

int
NodeSessionI::getTimeout(const Ice::Current&) const
{
    return secondsToInt(_timeout);
}

optional<NodeObserverPrx>
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
    _database->waitForApplicationUpdate(application, revision, std::move(response), std::move(exception));
}

void
NodeSessionI::destroy()
{
    destroyImpl(false);
}

void
NodeSessionI::destroy(const Ice::Current&)
{
    destroy();
}

optional<chrono::steady_clock::time_point>
NodeSessionI::timestamp() const noexcept
{
    lock_guard lock(_mutex);
    if (_destroy)
    {
        return nullopt;
    }
    return _timestamp;
}

void
NodeSessionI::shutdown()
{
    destroyImpl(true);
}

const NodePrx&
NodeSessionI::getNode() const
{
    return _node;
}

const shared_ptr<InternalNodeInfo>&
NodeSessionI::getInfo() const noexcept
{
    return _info;
}

const LoadInfo&
NodeSessionI::getLoadInfo() const
{
    lock_guard lock(_mutex);
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
            throw Ice::ObjectNotExistException{__FILE__, __LINE__};
        }
        _destroy = true;
    }

    if (_traceLevels->node > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
        out << "destroying session for node '" << _info->name << "'";
        if (shutdown)
        {
            out << " because the registry is shutting down";
        }
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

    // Next we notify the observer.
    static_pointer_cast<NodeObserverTopic>(_database->getObserverTopic(TopicName::NodeObserver))->nodeDown(_info->name);

    // Unsubscribe the node replica observer.
    if (_replicaObserver)
    {
        _database->getReplicaCache().unsubscribe(*_replicaObserver);
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
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
        }
    }
}
