//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "NodeI.h"
#include "CallbackExecutor.h"
#include "Instance.h"
#include "NodeSessionI.h"
#include "NodeSessionManager.h"
#include "SessionI.h"
#include "TopicFactoryI.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

namespace
{
    class SessionDispatcher : public Object
    {
    public:
        SessionDispatcher(shared_ptr<NodeI> node, shared_ptr<CallbackExecutor> executor)
            : _node(std::move(node)),
              _executor(std::move(executor))
        {
        }

        void dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse) final
        {
            if (auto session = _node->getSession(request.current().id))
            {
                session->dispatch(request, std::move(sendResponse));
                _executor->flush();
            }
            else
            {
                throw ObjectNotExistException{__FILE__, __LINE__};
            }
        }

    private:
        shared_ptr<NodeI> _node;
        shared_ptr<CallbackExecutor> _executor;
    };
}

NodeI::NodeI(const shared_ptr<Instance>& instance, std::string name)
    : _instance(instance),
      _proxy{instance->getObjectAdapter()->createProxy<NodePrx>(Identity{.name = std::move(name), .category = ""})},
      // The subscriber and publisher collocated forwarders are initalized here to avoid using a nullable proxy. These
      // objects are only used after the node is initialized and are removed in destroy implementation.
      _publisherForwarder{instance->getCollocatedForwarder()->add<PublisherSessionPrx>(
          [this](const ByteSeq& inParams, const Current& current) { forwardToPublishers(inParams, current); })},
      _subscriberForwarder{instance->getCollocatedForwarder()->add<SubscriberSessionPrx>(
          [this](const ByteSeq& inParams, const Current& current) { forwardToSubscribers(inParams, current); })}
{
}

NodeI::~NodeI()
{
    assert(_subscribers.empty());
    assert(_publishers.empty());
    assert(_subscriberSessions.empty());
    assert(_publisherSessions.empty());
}

void
NodeI::init()
{
    auto self = shared_from_this();
    auto instance = _instance.lock();
    assert(instance);

    auto adapter = instance->getObjectAdapter();
    adapter->add<NodePrx>(self, _proxy->ice_getIdentity());

    // Register the SessionDispatcher object as the default servant for subscriber and publisher sessions.
    // The "s" category handles subscriber sessions, and the "p" category handles publisher sessions.
    auto interceptor = make_shared<SessionDispatcher>(self, instance->getCallbackExecutor());
    adapter->addDefaultServant(interceptor, "s");
    adapter->addDefaultServant(interceptor, "p");
}

void
NodeI::destroy(bool ownsCommunicator)
{
    unique_lock<mutex> lock(_mutex);

    auto instance = _instance.lock();
    assert(instance);
    instance->getCollocatedForwarder()->remove(_subscriberForwarder->ice_getIdentity());
    instance->getCollocatedForwarder()->remove(_publisherForwarder->ice_getIdentity());

    if (!ownsCommunicator)
    {
        // Notifies peer sessions of the disconnection.
        for (const auto& [_, subscriber] : _subscribers)
        {
            if (auto session = subscriber->getSession())
            {
                try
                {
                    // Notify subscriber session of the disconnection, don't need to wait for the result.
                    session->disconnectedAsync(nullptr);
                }
                catch (const LocalException&)
                {
                }
            }
        }

        for (const auto& [_, publisher] : _publishers)
        {
            if (auto session = publisher->getSession())
            {
                try
                {
                    // Notify publisher session of the disconnection, don't need to wait for the result.
                    session->disconnectedAsync(nullptr);
                }
                catch (const LocalException&)
                {
                }
            }
        }
    }
    _subscribers.clear();
    _publishers.clear();
    _subscriberSessions.clear();
    _publisherSessions.clear();
}

void
NodeI::initiateCreateSession(optional<NodePrx> publisher, const Current& current)
{
    checkNotNull(publisher, __FILE__, __LINE__, current);
    // Create a session with the given publisher.
    createPublisherSession(*publisher, current.con, nullptr);
}

void
NodeI::createSession(
    optional<NodePrx> subscriber,
    optional<SubscriberSessionPrx> subscriberSession,
    bool fromRelay,
    const Current& current)
{
    checkNotNull(subscriber, __FILE__, __LINE__, current);
    checkNotNull(subscriberSession, __FILE__, __LINE__, current);

    auto instance = _instance.lock();
    assert(instance);

    shared_ptr<PublisherSessionI> session;
    try
    {
        NodePrx s = *subscriber;
        if (fromRelay && subscriberSession->ice_getIdentity().category == "s")
        {
            // If the request originates from a relay and the relay does not host a forwarder for the subscriber node,
            // check if there is an existing connection to the subscriber node and reuse it if available. Otherwise,
            // attempt to establish a new connection.
            s = getNodeWithExistingConnection(instance, s, nullptr);
        }
        else if (current.con)
        {
            // If the request originates from a relay hosting a forwarder for the subscriber node, or directly from the
            // subscriber node itself, use the current connection.
            //
            // This ensures that the confirmCreateSession request is not sent over a new connection, which in the relay
            // case could lead to sending a confirmation for a session that has already been closed by a close
            // connection callback.
            s = s->ice_fixed(current.con);
        }
        // else collocated call.

        unique_lock<mutex> lock(_mutex);
        session = createPublisherSessionServant(*subscriber);
        if (!session || session->checkSession())
        {
            return; // Shutting down or already connected
        }

        s->ice_getConnectionAsync(
            [=, self = shared_from_this()](const auto& connection) mutable
            {
                if (session->checkSession())
                {
                    return;
                }

                if (connection)
                {
                    if (!connection->getAdapter())
                    {
                        connection->setAdapter(instance->getObjectAdapter());
                    }

                    // Use a fixed proxy to ensure the request is sent using the connection configured with the OA.
                    s = s->ice_fixed(connection);
                    subscriberSession = subscriberSession->ice_fixed(connection);
                }

                try
                {
                    // Must be called before connected
                    s->confirmCreateSessionAsync(
                        self->_proxy,
                        uncheckedCast<PublisherSessionPrx>(session->getProxy()),
                        nullptr,
                        [self, subscriber, session](auto ex)
                        { self->removePublisherSession(*subscriber, session, ex); });

                    // Session::connected informs the subscriber session of all the topic writers in the current node.
                    session->connected(*subscriberSession, connection, instance->getTopicFactory()->getTopicWriters());
                }
                catch (const LocalException&)
                {
                    self->removePublisherSession(*subscriber, session, current_exception());
                }
            },
            [self = shared_from_this(), session, subscriber](exception_ptr ex)
            { self->removePublisherSession(*subscriber, session, ex); });
    }
    catch (const LocalException&)
    {
        removePublisherSession(*subscriber, session, current_exception());
    }
}

void
NodeI::confirmCreateSession(
    optional<NodePrx> publisher,
    optional<PublisherSessionPrx> publisherSession,
    const Current& current)
{
    checkNotNull(publisher, __FILE__, __LINE__, current);
    checkNotNull(publisherSession, __FILE__, __LINE__, current);

    unique_lock<mutex> lock(_mutex);
    auto p = _subscribers.find(publisher->ice_getIdentity());
    if (p == _subscribers.end())
    {
        return;
    }

    auto session = p->second;
    if (session->checkSession())
    {
        return;
    }

    // If the publisher session is hosted on a relay, current.con represents the connection to the relay.
    // Otherwise, it represents the connection to the publisher node. In both cases, a fixed proxy is used
    // to ensure the session is no longer used once the connection is closed.
    if (current.con)
    {
        publisherSession = publisherSession->ice_fixed(current.con);
    }
    // else collocated call.

    auto instance = _instance.lock();
    assert(instance);

    // Session::connected informs the publisher session of all the topic readers in the current node.
    session->connected(*publisherSession, current.con, instance->getTopicFactory()->getTopicReaders());
}

void
NodeI::createSubscriberSession(
    NodePrx subscriber,
    const ConnectionPtr& subscriberConnection,
    const shared_ptr<PublisherSessionI>& session)
{
    auto instance = _instance.lock();
    assert(instance);

    try
    {
        subscriber = getNodeWithExistingConnection(instance, subscriber, subscriberConnection);

        subscriber->ice_getConnectionAsync(
            [self = shared_from_this(), instance, session, subscriber](const auto& connection)
            {
                auto s = subscriber;
                if (connection)
                {
                    if (!connection->getAdapter())
                    {
                        connection->setAdapter(instance->getObjectAdapter());
                    }

                    // Use a fixed proxy to ensure the request is sent using the connection configured with the OA.
                    s = s->ice_fixed(connection);
                }

                s->initiateCreateSessionAsync(
                    self->_proxy,
                    nullptr,
                    [self, session, subscriber](exception_ptr ex)
                    { self->removePublisherSession(subscriber, session, ex); });
            },
            [subscriber, session, self = shared_from_this()](exception_ptr ex)
            { self->removePublisherSession(subscriber, session, ex); });
    }
    catch (const LocalException&)
    {
        removePublisherSession(subscriber, session, current_exception());
    }
}

void
NodeI::createPublisherSession(
    const NodePrx& publisher,
    const ConnectionPtr& publisherConnection,
    shared_ptr<SubscriberSessionI> session)
{
    auto instance = _instance.lock();
    assert(instance);

    try
    {
        auto p = getNodeWithExistingConnection(instance, publisher, publisherConnection);

        unique_lock<mutex> lock(_mutex);
        if (!session)
        {
            session = createSubscriberSessionServant(publisher);
            if (!session)
            {
                return; // Shutting down.
            }
            else if (session->checkSession())
            {
                return; // Already connected.
            }
        }

        p->ice_getConnectionAsync(
            [self = shared_from_this(), instance, session, publisher, p](const auto& connection) mutable
            {
                if (session->checkSession())
                {
                    return; // Already connected.
                }

                if (connection)
                {
                    if (!connection->getAdapter())
                    {
                        connection->setAdapter(instance->getObjectAdapter());
                    }

                    // Use a fixed proxy to ensure the request is sent using the connection configured with the OA.
                    p = p->ice_fixed(connection);
                }

                try
                {
                    p->createSessionAsync(
                        self->_proxy,
                        uncheckedCast<SubscriberSessionPrx>(session->getProxy()),
                        false,
                        nullptr,
                        [=](exception_ptr ex) { self->removeSubscriberSession(publisher, session, ex); });
                }
                catch (const LocalException&)
                {
                    self->removeSubscriberSession(publisher, session, current_exception());
                }
            },
            [publisher, session, self = shared_from_this()](exception_ptr ex)
            { self->removeSubscriberSession(publisher, session, ex); });
    }
    catch (const LocalException&)
    {
        removeSubscriberSession(publisher, session, current_exception());
    }
}

void
NodeI::removeSubscriberSession(const NodePrx& node, const shared_ptr<SubscriberSessionI>& session, exception_ptr ex)
{
    if (session && !session->retry(node, ex))
    {
        unique_lock<mutex> lock(_mutex);
        auto sessionNode = session->getNode();
        if (!session->checkSession() && node->ice_getIdentity() == sessionNode->ice_getIdentity())
        {
            auto p = _subscribers.find(sessionNode->ice_getIdentity());
            if (p != _subscribers.end() && p->second == session)
            {
                _subscribers.erase(p);
                _subscriberSessions.erase(session->getProxy()->ice_getIdentity());
                session->destroyImpl(ex);
            }
        }
    }
}

void
NodeI::removePublisherSession(const NodePrx& node, const shared_ptr<PublisherSessionI>& session, exception_ptr ex)
{
    if (session && !session->retry(node, ex))
    {
        unique_lock<mutex> lock(_mutex);
        auto sessionNode = session->getNode();
        if (!session->checkSession() && node->ice_getIdentity() == sessionNode->ice_getIdentity())
        {
            auto p = _publishers.find(sessionNode->ice_getIdentity());
            if (p != _publishers.end() && p->second == session)
            {
                _publishers.erase(p);
                _publisherSessions.erase(session->getProxy()->ice_getIdentity());
                session->destroyImpl(ex);
            }
        }
    }
}

ConnectionPtr
NodeI::getSessionConnection(string_view id) const
{
    auto session = getSession(stringToIdentity(id));
    if (session)
    {
        return session->getConnection();
    }
    else
    {
        return nullptr;
    }
}

shared_ptr<SessionI>
NodeI::getSession(const Identity& ident) const
{
    unique_lock<mutex> lock(_mutex);
    if (ident.category == "s")
    {
        auto p = _subscriberSessions.find(ident);
        if (p != _subscriberSessions.end())
        {
            return p->second;
        }
    }
    else if (ident.category == "p")
    {
        auto p = _publisherSessions.find(ident);
        if (p != _publisherSessions.end())
        {
            return p->second;
        }
    }
    return nullptr;
}

shared_ptr<SubscriberSessionI>
NodeI::createSubscriberSessionServant(const NodePrx& node)
{
    // Called with mutex locked
    auto instance = _instance.lock();
    assert(instance);
    auto p = _subscribers.find(node->ice_getIdentity());
    if (p != _subscribers.end())
    {
        p->second->setNode(node);
        return p->second;
    }
    else
    {
        try
        {
            int64_t id = ++_nextSubscriberSessionId;
            auto session = make_shared<SubscriberSessionI>(
                instance,
                shared_from_this(),
                node,
                instance->getObjectAdapter()
                    ->createProxy<SessionPrx>(Identity{.name = to_string(id), .category = "s"})
                    ->ice_oneway());
            session->init();
            _subscribers.emplace(node->ice_getIdentity(), session);
            _subscriberSessions.emplace(session->getProxy()->ice_getIdentity(), session);
            return session;
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            return nullptr;
        }
    }
}

shared_ptr<PublisherSessionI>
NodeI::createPublisherSessionServant(const NodePrx& node)
{
    // Called with mutex locked
    auto instance = _instance.lock();
    assert(instance);
    auto p = _publishers.find(node->ice_getIdentity());
    if (p != _publishers.end())
    {
        p->second->setNode(node);
        return p->second;
    }
    else
    {
        try
        {
            int64_t id = ++_nextPublisherSessionId;
            auto session = make_shared<PublisherSessionI>(
                instance,
                shared_from_this(),
                node,
                instance->getObjectAdapter()
                    ->createProxy<SessionPrx>(Identity{.name = to_string(id), .category = "p"})
                    ->ice_oneway());
            session->init();
            _publishers.emplace(node->ice_getIdentity(), session);
            _publisherSessions.emplace(session->getProxy()->ice_getIdentity(), session);
            return session;
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            return nullptr;
        }
    }
}

void
NodeI::forwardToSubscribers(const ByteSeq& inParams, const Current& current) const
{
    // Forward the invocation to all subscribers with an active session, don't need to wait for the result.
    lock_guard<mutex> lock(_mutex);
    assert(current.id == _subscriberForwarder->ice_getIdentity());
    for (const auto& [_, subscriber] : _subscribers)
    {
        if (optional<SessionPrx> session = subscriber->getSession())
        {
            session->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
        }
    }
}

void
NodeI::forwardToPublishers(const ByteSeq& inParams, const Current& current) const
{
    // Forward the invocation to all publishers with an active session, don't need to wait for the result.
    assert(current.id == _publisherForwarder->ice_getIdentity());
    for (const auto& [_, publisher] : _publishers)
    {
        if (optional<SessionPrx> session = publisher->getSession())
        {
            session->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
        }
    }
}

NodePrx
NodeI::getNodeWithExistingConnection(
    const shared_ptr<Instance>& instance,
    const NodePrx& node,
    const ConnectionPtr& newConnection)
{
    ConnectionPtr connection;

    // If the node has a session with this node, use a fixed proxy associated with node session's connection.
    if (auto nodeSession = instance->getNodeSessionManager()->getSession(node->ice_getIdentity()))
    {
        connection = nodeSession->getConnection();
    }

    // Otherwise, check if the node already has a session established and use the connection from the session.
    {
        lock_guard<mutex> lock(_mutex);
        auto p = _subscribers.find(node->ice_getIdentity());
        if (p != _subscribers.end())
        {
            connection = p->second->getConnection();
        }

        auto q = _publishers.find(node->ice_getIdentity());
        if (q != _publishers.end())
        {
            connection = q->second->getConnection();
        }
    }

    // Make sure the connection is still valid.
    if (connection)
    {
        try
        {
            connection->throwException();
        }
        catch (...)
        {
            connection = nullptr;
        }
    }

    if (!connection && node->ice_getEndpoints().empty() && node->ice_getAdapterId().empty())
    {
        connection = newConnection;
    }

    if (connection)
    {
        if (!connection->getAdapter())
        {
            connection->setAdapter(instance->getObjectAdapter());
        }
        return node->ice_fixed(connection);
    }

    // Ensure that the returned proxy doesn't have a cached connection.
    return node->ice_connectionCached(false)->ice_connectionCached(true);
}
