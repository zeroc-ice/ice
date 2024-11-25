//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "NodeSessionManager.h"
#include "ConnectionManager.h"
#include "ForwarderManager.h"
#include "Instance.h"
#include "NodeI.h"
#include "NodeSessionI.h"
#include "TopicFactoryI.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;

namespace
{
    class SessionForwarder : public Ice::Blobject
    {
    public:
        SessionForwarder(shared_ptr<NodeSessionManager> nodeSessionManager)
            : _nodeSessionManager(std::move(nodeSessionManager))
        {
        }

        bool ice_invoke(Ice::ByteSeq inParams, Ice::ByteSeq&, const Ice::Current& current) final
        {
            auto pos = current.id.name.find('-');
            if (pos != string::npos && pos < current.id.name.length())
            {
                if (auto session = _nodeSessionManager->getSession(current.id.name.substr(pos + 1)))
                {
                    // Forward the call to the target session, don't need to wait for the result.
                    auto id = Ice::Identity{current.id.name.substr(0, pos), current.id.category.substr(0, 1)};
                    session->getConnection()->createProxy(id)->ice_invokeAsync(
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
            throw Ice::ObjectNotExistException{__FILE__, __LINE__};
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
      _retryCount(0),
      _forwarder(instance->getCollocatedForwarder()->add<LookupPrx>(
          [this](Ice::ByteSeq inParams, const Ice::Current& current) { forward(inParams, current); }))
{
}

void
NodeSessionManager::init()
{
    auto instance = getInstance();
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
NodeSessionManager::createOrGet(NodePrx node, const Ice::ConnectionPtr& connection, bool forwardAnnouncements)
{
    unique_lock<mutex> lock(_mutex);

    auto p = _sessions.find(node->ice_getIdentity());
    if (p != _sessions.end())
    {
        if (p->second->getConnection() != connection)
        {
            p->second->destroy();
            _sessions.erase(p);
        }
        else
        {
            return p->second;
        }
    }

    auto instance = getInstance();

    if (!connection->getAdapter())
    {
        connection->setAdapter(instance->getObjectAdapter());
    }

    auto session = make_shared<NodeSessionI>(instance, node, connection, forwardAnnouncements);
    session->init();
    _sessions.emplace(node->ice_getIdentity(), session);

    // Register a callback with the connection manager to destroy the session when the connection is closed.
    instance->getConnectionManager()->add(
        connection,
        make_shared<NodePrx>(node),
        [self = shared_from_this(), node](const Ice::ConnectionPtr&, std::exception_ptr)
        { self->destroySession(std::move(node)); });

    return session;
}

void
NodeSessionManager::announceTopicReader(const string& topic, NodePrx node, const Ice::ConnectionPtr& connection) const
{
    unique_lock<mutex> lock(_mutex);
    if (connection && node->ice_getIdentity() == _nodePrx->ice_getIdentity())
    {
        return; // Ignore requests from self
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels, _traceLevels->sessionCat);
        if (connection)
        {
            out << "topic reader `" << topic << "' announced (peer = `" << node << "')";
        }
        else
        {
            out << "announcing topic reader `" << topic << "' (peer = `" << node << "')";
        }
    }

    auto p = _sessions.find(node->ice_getIdentity());
    auto nodePrx = p != _sessions.end() ? p->second->getPublicNode() : node;

    _exclude = connection;
    _forwarder->announceTopicReader(topic, nodePrx);

    lock.unlock();

    if (!connection || (_forwardToMulticast && connection->type() != "udp"))
    {
        auto instance = _instance.lock();
        if (instance && instance->getLookup())
        {
            instance->getLookup()->announceTopicReaderAsync(topic, nodePrx, nullptr);
        }
    }
}

void
NodeSessionManager::announceTopicWriter(const string& topic, NodePrx node, const Ice::ConnectionPtr& connection) const
{
    unique_lock<mutex> lock(_mutex);
    if (connection && node->ice_getIdentity() == _nodePrx->ice_getIdentity())
    {
        return; // Ignore requests from self
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels, _traceLevels->sessionCat);
        if (connection)
        {
            out << "topic writer `" << topic << "' announced (peer = `" << node << "')";
        }
        else
        {
            out << "announcing topic writer `" << topic << "' (peer = `" << node << "')";
        }
    }

    _exclude = connection;
    auto p = _sessions.find(node->ice_getIdentity());
    if (p != _sessions.end())
    {
        node = p->second->getPublicNode();
    }
    _forwarder->announceTopicWriter(topic, node);

    lock.unlock();

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
    const Ice::StringSeq& readers,
    const Ice::StringSeq& writers,
    NodePrx node,
    const Ice::ConnectionPtr& connection) const
{
    unique_lock<mutex> lock(_mutex);
    if (connection && node->ice_getIdentity() == _nodePrx->ice_getIdentity())
    {
        return; // Ignore requests from self
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels, _traceLevels->sessionCat);
        if (connection)
        {
            if (!readers.empty())
            {
                out << "topic reader(s) `" << readers << "' announced (peer = `" << node << "')";
            }
            if (!writers.empty())
            {
                out << "topic writer(s) `" << writers << "' announced (peer = `" << node << "')";
            }
        }
        else
        {
            if (!readers.empty())
            {
                out << "announcing topic reader(s) `" << readers << "' (peer = `" << node << "')";
            }
            if (!writers.empty())
            {
                out << "announcing topic writer(s) `" << writers << "' (peer = `" << node << "')";
            }
        }
    }

    _exclude = connection;
    auto p = _sessions.find(node->ice_getIdentity());
    auto nodePrx = p != _sessions.end() ? p->second->getPublicNode() : node;
    _forwarder->announceTopics(readers, writers, nodePrx);

    lock.unlock();

    if (!connection || (_forwardToMulticast && connection->type() != "udp"))
    {
        auto instance = _instance.lock();
        if (instance && instance->getLookup())
        {
            instance->getLookup()->announceTopicsAsync(readers, writers, nodePrx, nullptr);
        }
    }
}

shared_ptr<NodeSessionI>
NodeSessionManager::getSession(const Ice::Identity& node) const
{
    unique_lock<mutex> lock(_mutex);
    auto p = _sessions.find(node);
    return p != _sessions.end() ? p->second : nullptr;
}

void
NodeSessionManager::forward(const Ice::ByteSeq& inParams, const Ice::Current& current) const
{
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

    // Forward the call to all nodes we are connected to, don't need to wait for the result.
    for (const auto& [_, lookup] : _connectedTo)
    {
        if (lookup.second->ice_getCachedConnection() != _exclude)
        {
            lookup.second
                ->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
        }
    }
}

void
NodeSessionManager::connect(LookupPrx lookup, NodePrx proxy)
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
            [=, self = shared_from_this()](optional<NodePrx> node)
            {
                // createSession must return a non null proxy.
                assert(node);
                if (node)
                {
                    self->connected(*node, lookup);
                }
                else
                {
                    self->disconnected(lookup);
                }
            },
            [=, self = shared_from_this()](std::exception_ptr) { self->disconnected(lookup); });
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        // Ignore node is being shutdown.
    }
    catch (const std::exception&)
    {
        disconnected(lookup);
    }
}

void
NodeSessionManager::connected(NodePrx node, LookupPrx lookup)
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
    if (!connection->getAdapter())
    {
        connection->setAdapter(instance->getObjectAdapter());
    }

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels, _traceLevels->sessionCat);
        out << "established node session (peer = `" << node << "'):\n" << connection->toString();
    }

    instance->getConnectionManager()->add(
        connection,
        make_shared<LookupPrx>(lookup),
        [=, self = shared_from_this()](const Ice::ConnectionPtr&, std::exception_ptr)
        { self->disconnected(node, lookup); });

    if (p != _sessions.end())
    {
        lookup = lookup->ice_fixed(connection);
    }
    _connectedTo.emplace(node->ice_getIdentity(), make_pair(node, lookup));

    auto readerNames = instance->getTopicFactory()->getTopicReaderNames();
    auto writerNames = instance->getTopicFactory()->getTopicWriterNames();
    if (!readerNames.empty() || !writerNames.empty())
    {
        try
        {
            lookup->announceTopicsAsync(readerNames, writerNames, _nodePrx, nullptr);
        }
        catch (const Ice::CommunicatorDestroyedException&)
        {
            // Ignore node is being shutdown.
        }
    }
}

void
NodeSessionManager::disconnected(NodePrx node, LookupPrx lookup)
{
    unique_lock<mutex> lock(_mutex);
    auto instance = _instance.lock();
    if (instance)
    {
        _retryCount = 0;
        if (_traceLevels->session > 0)
        {
            Trace out(_traceLevels, _traceLevels->sessionCat);
            out << "disconnected node session (peer = `" << node << "')";
        }
        _connectedTo.erase(node->ice_getIdentity());
        lock.unlock();
        connect(lookup, _nodePrx);
    }
}

void
NodeSessionManager::disconnected(LookupPrx lookup)
{
    unique_lock<mutex> lock(_mutex);
    auto instance = _instance.lock();
    if (instance)
    {
        instance->scheduleTimerTask(
            [=, self = shared_from_this()] { self->connect(lookup, self->_nodePrx); },
            instance->getRetryDelay(_retryCount++));
    }
}

void
NodeSessionManager::destroySession(optional<NodePrx> node)
{
    unique_lock<mutex> lock(_mutex);
    auto p = _sessions.find(node->ice_getIdentity());
    if (p != _sessions.end())
    {
        p->second->destroy();
        _sessions.erase(p);
    }
}
