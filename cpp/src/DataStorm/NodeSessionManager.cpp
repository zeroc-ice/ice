// Copyright (c) ZeroC, Inc.

#include "NodeSessionManager.h"
#include "ConnectionManager.h"
#include "Instance.h"
#include "NodeI.h"
#include "NodeSessionI.h"
#include "TopicFactoryI.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

namespace
{
    class SessionForwarder : public Blobject
    {
    public:
        SessionForwarder(shared_ptr<NodeSessionManager> nodeSessionManager)
            : _nodeSessionManager(std::move(nodeSessionManager))
        {
        }

        bool ice_invoke(ByteSeq inParams, ByteSeq&, const Current& current) final
        {
            auto pos = current.id.name.find('-');
            if (pos != string::npos && pos < current.id.name.length())
            {
                if (auto session = _nodeSessionManager->getSession(
                        Identity{.name = current.id.name.substr(pos + 1), .category = ""}))
                {
                    // Forward the call to the target session, don't need to wait for the result.
                    Identity id{.name = current.id.name.substr(0, pos), .category = current.id.category.substr(0, 1)};
                    session->getConnection()
                        ->createProxy(std::move(id))
                        ->ice_invokeAsync(
                            current.operation,
                            current.mode,
                            inParams,
                            nullptr,
                            nullptr,
                            nullptr,
                            current.ctx);
                    return true;
                }
            }
            throw ObjectNotExistException{__FILE__, __LINE__};
        }

    private:
        const shared_ptr<NodeSessionManager> _nodeSessionManager;
    };
}

NodeSessionManager::NodeSessionManager(const shared_ptr<Instance>& instance, const shared_ptr<NodeI>& node)
    : _instance(instance),
      _traceLevels(instance->getTraceLevels()),
      _nodePrx(node->getProxy()),
      _forwardToMulticast(
          instance->getCommunicator()->getProperties()->getIcePropertyAsInt(
              "DataStorm.Node.Server.ForwardDiscoveryToMulticast") > 0),
      _forwarder(instance->getCollocatedForwarder()->add<LookupPrx>(
          [this](const ByteSeq& inParams, const Current& current) { forward(inParams, current); }))
{
}

void
NodeSessionManager::init()
{
    auto instance = _instance.lock();
    assert(instance);
    auto sessionForwader = make_shared<SessionForwarder>(shared_from_this());
    instance->getObjectAdapter()->addDefaultServant(sessionForwader, "sf");
    instance->getObjectAdapter()->addDefaultServant(sessionForwader, "pf");

    auto communicator = instance->getCommunicator();
    const string connectTo = communicator->getProperties()->getIceProperty("DataStorm.Node.ConnectTo");
    if (!connectTo.empty())
    {
        connect(LookupPrx{communicator, "DataStorm/Lookup:" + connectTo}, _nodePrx);
    }
}

shared_ptr<NodeSessionI>
NodeSessionManager::createOrGet(NodePrx node, const ConnectionPtr& newConnection, bool forwardAnnouncements)
{
    unique_lock<mutex> lock(_mutex);

    auto p = _sessions.find(node->ice_getIdentity());
    if (p != _sessions.end())
    {
        // If called with a new connection we destroy the node session before creating a new one that uses the new
        // connection
        if (p->second->getConnection() != newConnection)
        {
            p->second->destroy();
            _sessions.erase(p);
        }
        else
        {
            return p->second;
        }
    }

    auto instance = _instance.lock();
    assert(instance);

    assert(newConnection->getAdapter() == instance->getObjectAdapter());

    auto session = make_shared<NodeSessionI>(instance, node, newConnection, forwardAnnouncements);
    session->init();
    _sessions.emplace(node->ice_getIdentity(), session);

    // Register a callback with the connection manager to destroy the session when the connection is closed.
    instance->getConnectionManager()->add(
        newConnection,
        make_shared<NodePrx>(node),
        [self = shared_from_this(), node = std::move(node)](const ConnectionPtr& connection, exception_ptr) mutable
        { self->destroySession(connection, node); });

    return session;
}

void
NodeSessionManager::announceTopicReader(const string& topic, NodePrx node, const ConnectionPtr& connection) const
{
    unique_lock<mutex> lock(_mutex);
    if (connection && node->ice_getIdentity() == _nodePrx->ice_getIdentity())
    {
        return; // Ignore requests from self
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        if (connection)
        {
            out << "topic reader '" << topic << "' announced (peer = '" << node << "')";
        }
        else
        {
            out << "announcing topic reader '" << topic << "' (peer = '" << node << "')";
        }
    }

    auto p = _sessions.find(node->ice_getIdentity());
    node = p != _sessions.end() ? p->second->getPublicNode() : node;

    // Set the exclude connection to prevent forwarding the announcement back to the sender.
    _exclude = connection;
    // Forward the announcement to all known nodes, including nodes with an active session and those we are connected
    // to. This is a collocated, synchronous call.
    _forwarder->announceTopicReader(topic, node);
    _exclude = nullptr;

    lock.unlock();

    // Forward the announcement to the multicast lookup if:
    // - It is a local announcement, or
    // - It comes from a non-multicast lookup and multicast-forwarding is enabled.
    if (!connection || (_forwardToMulticast && connection->type() != "udp"))
    {
        auto instance = _instance.lock();
        if (instance && instance->getLookup())
        {
            instance->getLookup()->announceTopicReaderAsync(topic, node, nullptr);
        }
    }
}

void
NodeSessionManager::announceTopicWriter(const string& topic, NodePrx node, const ConnectionPtr& connection) const
{
    unique_lock<mutex> lock(_mutex);
    if (connection && node->ice_getIdentity() == _nodePrx->ice_getIdentity())
    {
        return; // Ignore requests from self
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        if (connection)
        {
            out << "topic writer '" << topic << "' announced (peer = '" << node << "')";
        }
        else
        {
            out << "announcing topic writer '" << topic << "' (peer = '" << node << "')";
        }
    }

    auto p = _sessions.find(node->ice_getIdentity());
    node = p != _sessions.end() ? p->second->getPublicNode() : node;

    // Set the exclude connection to prevent forwarding the announcement back to the sender.
    _exclude = connection;
    // Forward the announcement to all known nodes, including nodes with an active session and those we are connected
    // to. This is a collocated, synchronous call.
    _forwarder->announceTopicWriter(topic, node);
    _exclude = nullptr;

    lock.unlock();

    // Forward the announcement to the multicast lookup if:
    // - It is a local announcement, or
    // - It comes from a non-multicast lookup and multicast-forwarding is enabled.
    if (!connection || (_forwardToMulticast && connection->type() != "udp"))
    {
        auto instance = _instance.lock();
        if (instance && instance->getLookup())
        {
            instance->getLookup()->announceTopicWriterAsync(topic, node, nullptr);
        }
    }
}

void
NodeSessionManager::announceTopics(
    const StringSeq& readers,
    const StringSeq& writers,
    NodePrx node,
    const ConnectionPtr& connection) const
{
    unique_lock<mutex> lock(_mutex);
    if (connection && node->ice_getIdentity() == _nodePrx->ice_getIdentity())
    {
        return; // Ignore requests from self
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        if (connection)
        {
            if (!readers.empty())
            {
                out << "topic reader(s) '" << readers << "' announced (peer = '" << node << "')";
            }
            if (!writers.empty())
            {
                out << "topic writer(s) '" << writers << "' announced (peer = '" << node << "')";
            }
        }
        else
        {
            if (!readers.empty())
            {
                out << "announcing topic reader(s) '" << readers << "' (peer = '" << node << "')";
            }
            if (!writers.empty())
            {
                out << "announcing topic writer(s) '" << writers << "' (peer = '" << node << "')";
            }
        }
    }

    auto p = _sessions.find(node->ice_getIdentity());
    node = p != _sessions.end() ? p->second->getPublicNode() : node;

    // Set the exclude connection to prevent forwarding the announcement back to the sender.
    _exclude = connection;
    // Forward the announcement to all known nodes, including nodes with an active session and those we are connected
    // to. This is a collocated, synchronous call.
    _forwarder->announceTopics(readers, writers, node);
    _exclude = nullptr;

    lock.unlock();

    // Forward the announcement to the multicast lookup if:
    // - It is a local announcement, or
    // - It comes from a non-multicast lookup and multicast-forwarding is enabled.
    if (!connection || (_forwardToMulticast && connection->type() != "udp"))
    {
        auto instance = _instance.lock();
        if (instance && instance->getLookup())
        {
            instance->getLookup()->announceTopicsAsync(readers, writers, node, nullptr);
        }
    }
}

shared_ptr<NodeSessionI>
NodeSessionManager::getSession(const Identity& node) const
{
    unique_lock<mutex> lock(_mutex);
    auto p = _sessions.find(node);
    return p != _sessions.end() ? p->second : nullptr;
}

void
NodeSessionManager::forward(const ByteSeq& inParams, const Current& current) const
{
    // Called while holding the mutex lock to ensure _exclude is not updated concurrently.

    // Forward the call to all nodes that have an active session, don't need to wait for the result.
    for (const auto& [_, session] : _sessions)
    {
        if (session->getConnection() != _exclude)
        {
            if (auto lookup = session->getLookup())
            {
                lookup->ice_invokeAsync(
                    current.operation,
                    current.mode,
                    inParams,
                    nullptr,
                    nullptr,
                    nullptr,
                    current.ctx);
            }
        }
    }

    // Forward the call to the connectedTo node, don't need to wait for the result.
    if (_connectedTo && (*_connectedTo)->ice_getCachedConnection() != _exclude)
    {
        (*_connectedTo)
            ->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
    }
}

void
NodeSessionManager::connect(const LookupPrx& lookup, const NodePrx& proxy)
{
    auto instance = _instance.lock();
    if (!instance)
    {
        // Ignore the Node is being shutdown.
        return;
    }

    try
    {
        lookup->createSessionAsync(
            proxy,
            [self = shared_from_this(), lookup](optional<NodePrx> node)
            {
                // createSession must return a non null proxy.
                assert(node);
                self->connected(*node, lookup);
            },
            [self = shared_from_this(), lookup](exception_ptr) { self->disconnected(lookup); });
    }
    catch (const CommunicatorDestroyedException&)
    {
        // Ignore node is being shutdown.
    }
    catch (const exception&)
    {
        disconnected(lookup);
    }
}

void
NodeSessionManager::connected(const NodePrx& node, const LookupPrx& lookup)
{
    unique_lock<mutex> lock(_mutex);
    auto instance = _instance.lock();
    if (!instance)
    {
        // Ignore the Node is being shutdown.
        return;
    }

    auto p = _sessions.find(node->ice_getIdentity());
    auto connection = p != _sessions.end() ? p->second->getConnection() : lookup->ice_getCachedConnection();

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << "established node session (peer = '" << node << "'):\n" << connection->toString();
    }

    instance->getConnectionManager()->add(
        connection,
        make_shared<LookupPrx>(lookup),
        [self = shared_from_this(), node, lookup](const ConnectionPtr&, exception_ptr)
        { self->disconnected(node, lookup); });

    _connectedTo = p == _sessions.end() ? lookup : lookup->ice_fixed(connection);

    auto readerNames = instance->getTopicFactory()->getTopicReaderNames();
    auto writerNames = instance->getTopicFactory()->getTopicWriterNames();
    if (!readerNames.empty() || !writerNames.empty())
    {
        try
        {
            _connectedTo->announceTopicsAsync(readerNames, writerNames, _nodePrx, nullptr);
        }
        catch (const CommunicatorDestroyedException&)
        {
            // Ignore node is being shutdown.
        }
    }
}

void
NodeSessionManager::disconnected(const NodePrx& node, const LookupPrx& lookup)
{
    unique_lock<mutex> lock(_mutex);
    auto instance = _instance.lock();
    if (instance)
    {
        _retryCount = 0;
        if (_traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "disconnected node session (peer = '" << node << "')";
        }
        _connectedTo = nullopt;
        lock.unlock();
        connect(lookup, _nodePrx);
    }
}

void
NodeSessionManager::disconnected(const LookupPrx& lookup)
{
    unique_lock<mutex> lock(_mutex);
    auto instance = _instance.lock();
    if (instance)
    {
        instance->scheduleTimerTask(
            [self = shared_from_this(), lookup] { self->connect(lookup, self->_nodePrx); },
            instance->getRetryDelay(_retryCount++));
    }
}

void
NodeSessionManager::destroySession(const ConnectionPtr& connection, const NodePrx& node)
{
    unique_lock<mutex> lock(_mutex);
    // Destroy the connection if the session is still using it, otherwise the node has already
    // replaced its NodeSession and it is using a new connection.
    auto p = _sessions.find(node->ice_getIdentity());
    if (p != _sessions.end() && p->second->getConnection() == connection)
    {
        p->second->destroy();
        _sessions.erase(p);
    }
}
