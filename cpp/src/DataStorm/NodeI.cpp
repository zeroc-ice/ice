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

namespace
{
    // TODO convert to a middleware
    class DispatchInterceptorI : public Ice::Object
    {
    public:
        DispatchInterceptorI(shared_ptr<NodeI> node, shared_ptr<CallbackExecutor> executor)
            : _node(std::move(node)),
              _executor(std::move(executor))
        {
        }

        void dispatch(Ice::IncomingRequest& request, std::function<void(Ice::OutgoingResponse)> sendResponse) final
        {
            if (auto session = _node->getSession(request.current().id))
            {
                session->dispatch(request, std::move(sendResponse));
                _executor->flush();
            }
            else
            {
                throw Ice::ObjectNotExistException{__FILE__, __LINE__};
            }
        }

    private:
        shared_ptr<NodeI> _node;
        shared_ptr<CallbackExecutor> _executor;
    };
}

NodeI::NodeI(const shared_ptr<Instance>& instance)
    : _instance(instance),
      _proxy{instance->getObjectAdapter()->createProxy<NodePrx>({Ice::generateUUID(), ""})},
      // The subscriber and publisher collocated forwarders are initalized here to avoid using a nullable proxy. These
      // objects are only used after the node is initialized and are removed in destroy implementation.
      _subscriberForwarder{instance->getCollocatedForwarder()->add<SubscriberSessionPrx>(
          [this](Ice::ByteSeq inParams, const Ice::Current& current) { forwardToSubscribers(inParams, current); })},
      _publisherForwarder{instance->getCollocatedForwarder()->add<PublisherSessionPrx>(
          [this](Ice::ByteSeq inParams, const Ice::Current& current) { forwardToPublishers(inParams, current); })},
      _nextSubscriberSessionId{0},
      _nextPublisherSessionId{0}
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
    auto instance = getInstance();

    auto adapter = instance->getObjectAdapter();
    adapter->add<NodePrx>(self, _proxy->ice_getIdentity());

    auto interceptor = make_shared<DispatchInterceptorI>(self, instance->getCallbackExecutor());
    adapter->addDefaultServant(interceptor, "s");
    adapter->addDefaultServant(interceptor, "p");
}

void
NodeI::destroy(bool ownsCommunicator)
{
    unique_lock<mutex> lock(_mutex);

    auto instance = getInstance();
    assert(instance);
    if (instance)
    {
        instance->getCollocatedForwarder()->remove(_subscriberForwarder->ice_getIdentity());
        instance->getCollocatedForwarder()->remove(_publisherForwarder->ice_getIdentity());
    }

    if (!ownsCommunicator)
    {
        //
        // Notifies peer sessions of the disconnection.
        //
        for (const auto& [_, subscriber] : _subscribers)
        {
            if (auto session = subscriber->getSession())
            {
                try
                {
                    // Notify subscriber session of the disconnection, don't need to wait for the result.
                    session->disconnectedAsync(nullptr);
                }
                catch (const Ice::LocalException&)
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
                catch (const Ice::LocalException&)
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
NodeI::initiateCreateSession(optional<NodePrx> publisher, const Ice::Current& current)
{
    Ice::checkNotNull(publisher, __FILE__, __LINE__, current);
    // Create a session with the given publisher.
    createPublisherSession(std::move(*publisher), current.con, nullptr);
}

void
NodeI::createSession(
    optional<NodePrx> subscriber,
    optional<SubscriberSessionPrx> subscriberSession,
    bool fromRelay,
    const Ice::Current& current)
{
    Ice::checkNotNull(subscriber, __FILE__, __LINE__, current);
    Ice::checkNotNull(subscriberSession, __FILE__, __LINE__, current);

    shared_ptr<PublisherSessionI> session;
    try
    {
        NodePrx s = *subscriber;
        if (fromRelay)
        {
            //
            // If the call is from a relay, we check if we already have a connection to this node
            // and eventually re-use it. Otherwise, we'll try to establish a connection to the node
            // if it has endpoints. If it doesn't, we'll re-use the current connection to send the
            // confirmation.
            //
            s = getNodeWithExistingConnection(s, current.con);
        }
        else if (current.con)
        {
            s = s->ice_fixed(current.con);
        }

        unique_lock<mutex> lock(_mutex);
        session = createPublisherSessionServant(*subscriber);
        if (!session || session->checkSession())
        {
            return; // Shutting down or already connected
        }

        auto self = shared_from_this();
        s->ice_getConnectionAsync(
            [=, this](auto connection) mutable
            {
                if (session->checkSession())
                {
                    return;
                }

                if (connection && !connection->getAdapter())
                {
                    connection->setAdapter(getInstance()->getObjectAdapter());
                }

                if (connection)
                {
                    subscriberSession = subscriberSession->ice_fixed(connection);
                }

                try
                {
                    // Must be called before connected
                    s->confirmCreateSessionAsync(
                        _proxy,
                        session->getProxy<PublisherSessionPrx>(),
                        nullptr,
                        [=](auto ex) { self->removePublisherSession(*subscriber, session, ex); });
                    assert(!s->ice_getCachedConnection() || s->ice_getCachedConnection() == connection);
                    session->connected(
                        *subscriberSession,
                        connection,
                        getInstance()->getTopicFactory()->getTopicWriters());
                }
                catch (const Ice::LocalException&)
                {
                    removePublisherSession(*subscriber, session, current_exception());
                }
            },
            [=](auto ex) { self->removePublisherSession(*subscriber, session, ex); });
    }
    catch (const Ice::LocalException&)
    {
        removePublisherSession(*subscriber, session, current_exception());
    }
}

void
NodeI::confirmCreateSession(
    optional<NodePrx> publisher,
    optional<PublisherSessionPrx> publisherSession,
    const Ice::Current& current)
{
    Ice::checkNotNull(publisher, __FILE__, __LINE__, current);
    Ice::checkNotNull(publisherSession, __FILE__, __LINE__, current);

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

    if (current.con && publisherSession->ice_getEndpoints().empty() && publisherSession->ice_getAdapterId().empty())
    {
        publisherSession = publisherSession->ice_fixed(current.con);
    }

    session->connected(*publisherSession, current.con, getInstance()->getTopicFactory()->getTopicReaders());
}

void
NodeI::createSubscriberSession(
    NodePrx subscriber,
    const Ice::ConnectionPtr& connection,
    const shared_ptr<PublisherSessionI>& session)
{
    try
    {
        subscriber = getNodeWithExistingConnection(subscriber, connection);

        auto self = shared_from_this();
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif
        subscriber->ice_getConnectionAsync(
            [=, this](auto connection)
            {
                if (connection && !connection->getAdapter())
                {
                    connection->setAdapter(getInstance()->getObjectAdapter());
                }
                subscriber->initiateCreateSessionAsync(
                    _proxy,
                    nullptr,
                    [=](auto ex) { self->removePublisherSession(subscriber, session, ex); });
            },
            [=](auto ex) { self->removePublisherSession(subscriber, session, ex); });
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    }
    catch (const Ice::LocalException&)
    {
        removePublisherSession(subscriber, session, current_exception());
    }
}

void
NodeI::createPublisherSession(NodePrx publisher, const Ice::ConnectionPtr& con, shared_ptr<SubscriberSessionI> session)
{
    try
    {
        auto p = getNodeWithExistingConnection(publisher, con);

        unique_lock<mutex> lock(_mutex);
        if (!session)
        {
            session = createSubscriberSessionServant(publisher);
            if (!session)
            {
                return; // Shutting down.
            }
        }

        auto self = shared_from_this();
        p->ice_getConnectionAsync(
            [=, this](auto connection)
            {
                if (session->checkSession())
                {
                    return;
                }

                if (connection && !connection->getAdapter())
                {
                    connection->setAdapter(getInstance()->getObjectAdapter());
                }

                try
                {
                    p->createSessionAsync(
                        _proxy,
                        session->getProxy<SubscriberSessionPrx>(),
                        false,
                        nullptr,
                        [=](exception_ptr ex) { self->removeSubscriberSession(publisher, session, ex); });
                }
                catch (const Ice::LocalException&)
                {
                    removeSubscriberSession(publisher, session, current_exception());
                }
            },
            [=](exception_ptr ex) { self->removeSubscriberSession(publisher, session, ex); });
    }
    catch (const Ice::LocalException&)
    {
        removeSubscriberSession(publisher, session, current_exception());
    }
}

void
NodeI::removeSubscriberSession(NodePrx node, const shared_ptr<SubscriberSessionI>& session, exception_ptr ex)
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
NodeI::removePublisherSession(NodePrx node, const shared_ptr<PublisherSessionI>& session, exception_ptr ex)
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

Ice::ConnectionPtr
NodeI::getSessionConnection(string_view id) const
{
    auto session = getSession(Ice::stringToIdentity(id));
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
NodeI::getSession(const Ice::Identity& ident) const
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
NodeI::createSubscriberSessionServant(NodePrx node)
{
    // Called with mutex locked
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
                shared_from_this(),
                node,
                getInstance()->getObjectAdapter()->createProxy<SessionPrx>({to_string(id), "s"})->ice_oneway());
            session->init();
            _subscribers.emplace(node->ice_getIdentity(), session);
            _subscriberSessions.emplace(session->getProxy()->ice_getIdentity(), session);
            return session;
        }
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
            return nullptr;
        }
    }
}

shared_ptr<PublisherSessionI>
NodeI::createPublisherSessionServant(NodePrx node)
{
    // Called with mutex locked
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
                shared_from_this(),
                node,
                getInstance()->getObjectAdapter()->createProxy<SessionPrx>({to_string(id), "p"})->ice_oneway());
            session->init();
            _publishers.emplace(node->ice_getIdentity(), session);
            _publisherSessions.emplace(session->getProxy()->ice_getIdentity(), session);
            return session;
        }
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
            return nullptr;
        }
    }
}

void
NodeI::forwardToSubscribers(const Ice::ByteSeq& inParams, const Ice::Current& current) const
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
NodeI::forwardToPublishers(const Ice::ByteSeq& inParams, const Ice::Current& current) const
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
NodeI::getNodeWithExistingConnection(NodePrx node, const Ice::ConnectionPtr& con)
{
    Ice::ConnectionPtr connection;

    // If the node has a session with this node, use a bi-dir proxy associated with node session's connection.
    auto instance = _instance.lock();
    if (instance)
    {
        if (auto nodeSession = instance->getNodeSessionManager()->getSession(node->ice_getIdentity()))
        {
            connection = nodeSession->getConnection();
        }
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
        connection = con;
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
