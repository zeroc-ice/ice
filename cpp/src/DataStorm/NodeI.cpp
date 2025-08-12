// Copyright (c) ZeroC, Inc.

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

    // The instance class owns this object, so it is guaranteed to be available.
    auto instance = _instance.lock();
    assert(instance);

    auto adapter = instance->getObjectAdapter();
    adapter->add<NodePrx>(self, _proxy->ice_getIdentity());

    // Register the SessionDispatcher object as the default servant for subscriber sessions.
    // The interceptor handles requests sent to subscriber session facets by publishers using a sample filter.
    adapter->addDefaultServant(make_shared<SessionDispatcher>(self, instance->getCallbackExecutor()), "s");
}

void
NodeI::destroy(bool ownsCommunicator)
{
    unique_lock<mutex> lock(_mutex);

    // The instance class owns this object, so it is guaranteed to be available.
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
NodeI::initiateCreateSessionAsync(
    optional<NodePrx> publisher,
    function<void()> response,
    function<void(std::exception_ptr)> exception,
    const Current& current)
{
    try
    {
        checkNotNull(publisher, __FILE__, __LINE__, current);
        // Create a session with the given publisher.
        createPublisherSession(*publisher, current.con, nullptr);
        response();
    }
    catch (const SessionCreationException&)
    {
        exception(current_exception());
    }
    catch (const CommunicatorDestroyedException&)
    {
        // The node is shutting down, don't retry.
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        // The node is shutting down, don't retry.
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
    }
    catch (const LocalException&)
    {
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::Internal}));
    }
}

void
NodeI::createSessionAsync(
    optional<NodePrx> subscriber,
    optional<SubscriberSessionPrx> subscriberSession,
    bool fromRelay,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Current& current)
{
    checkNotNull(subscriber, __FILE__, __LINE__, current);
    checkNotNull(subscriberSession, __FILE__, __LINE__, current);

    // The instance class owns this object, so it is guaranteed to be available.
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

        auto traceLevels = instance->getTraceLevels();

        unique_lock<mutex> lock(_mutex);
        session = createPublisherSessionServant(*subscriber);
        s->ice_getConnectionAsync(
            [=, self = shared_from_this()](const auto& connection) mutable
            {
                if (session->checkSession())
                {
                    // The session is already connected.
                    if (traceLevels->session > 2)
                    {
                        Trace out(traceLevels->logger, traceLevels->sessionCat);
                        out << "node '" << current.id << "' is ignoring '" << current.operation << "' request from '"
                            << subscriber << "' because session '" << session->getId() << "' is already connected to: '"
                            << session->getSession() << "'";
                    }
                    exception(
                        std::make_exception_ptr(SessionCreationException{SessionCreationError::AlreadyConnected}));
                    return;
                }
                response();

                if (connection)
                {
                    // Use a fixed proxy to ensure the request is sent using the connection registered by connected
                    // with the connection manager.
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
                        { self->retryPublisherSessionCreation(*subscriber, session, ex); });

                    // Session::connected informs the subscriber session of all the topic writers in the current node.
                    session->connected(*subscriberSession, connection, instance->getTopicFactory()->getTopicWriters());
                }
                catch (const CommunicatorDestroyedException&)
                {
                    // The node is shutting down, don't retry.
                }
                catch (const ObjectAdapterDestroyedException&)
                {
                    // The node is shutting down, don't retry.
                }
                catch (const LocalException&)
                {
                    self->retryPublisherSessionCreation(*subscriber, session, current_exception());
                }
            },
            [self = shared_from_this(), session, subscriber, exception](exception_ptr ex)
            {
                self->retryPublisherSessionCreation(*subscriber, session, ex);
                exception(make_exception_ptr(SessionCreationException{SessionCreationError::Internal}));
            });
    }
    catch (const CommunicatorDestroyedException&)
    {
        // The node is shutting down, don't retry.
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        // The node is shutting down, don't retry.
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
    }
    catch (const LocalException&)
    {
        assert(session);
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::Internal}));
        retryPublisherSessionCreation(*subscriber, session, current_exception());
    }
}

void
NodeI::confirmCreateSessionAsync(
    optional<NodePrx> publisher,
    optional<PublisherSessionPrx> publisherSession,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Current& current)
{
    checkNotNull(publisher, __FILE__, __LINE__, current);
    checkNotNull(publisherSession, __FILE__, __LINE__, current);

    unique_lock<mutex> lock(_mutex);

    // The instance class owns this object, so it is guaranteed to be available.
    auto instance = _instance.lock();
    assert(instance);

    auto traceLevels = instance->getTraceLevels();

    auto p = _subscribers.find(publisher->ice_getIdentity());
    if (p == _subscribers.end())
    {
        if (traceLevels->session > 2)
        {
            Trace out(traceLevels->logger, traceLevels->sessionCat);
            out << "node '" << current.id << "' is ignoring '" << current.operation << "' request from publisher '"
                << publisher->ice_getIdentity() << "' because no corresponding subscriber session was found";
        }
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::SessionNotFound}));
        return;
    }

    auto session = p->second;
    if (session->checkSession())
    {
        // The session is already connected.
        if (traceLevels->session > 2)
        {
            Trace out(traceLevels->logger, traceLevels->sessionCat);
            out << "node '" << current.id << "' is ignoring '" << current.operation << "' request from '" << publisher
                << "' because session '" << session->getId() << "' is already connected to: '" << session->getSession()
                << "'";
        }
        exception(make_exception_ptr(SessionCreationException{SessionCreationError::AlreadyConnected}));
        return;
    }
    response();

    // If the publisher session is hosted on a relay, current.con represents the connection to the relay.
    // Otherwise, it represents the connection to the publisher node. In both cases, a fixed proxy is used
    // to ensure the session is no longer used once the connection is closed.
    if (current.con)
    {
        publisherSession = publisherSession->ice_fixed(current.con);
    }
    // else collocated call.

    // Session::connected informs the publisher session of all the topic readers in the current node.
    session->connected(*publisherSession, current.con, instance->getTopicFactory()->getTopicReaders());
}

void
NodeI::createSubscriberSession(
    NodePrx subscriber,
    const ConnectionPtr& subscriberConnection,
    const shared_ptr<PublisherSessionI>& session)
{
    // The instance class owns this object, so it is guaranteed to be available.
    auto instance = _instance.lock();
    assert(instance);

    // The publisher session is null when we are creating a new session, in response to a topic reader announcement. It
    // is not null when we are attempting to reconnect an existing session.

    try
    {
        subscriber = getNodeWithExistingConnection(instance, subscriber, subscriberConnection);

        subscriber->initiateCreateSessionAsync(
            _proxy,
            nullptr,
            [self = shared_from_this(), session, subscriber](exception_ptr ex)
            {
                if (session)
                {
                    self->retryPublisherSessionCreation(subscriber, session, ex);
                }
                // Else node is shutting down, or the session was established by a concurrent call.
            });
    }
    catch (const CommunicatorDestroyedException&)
    {
        // The node is shutting down, don't retry.
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        // The node is shutting down, don't retry.
    }
    catch (const LocalException&)
    {
        if (session)
        {
            retryPublisherSessionCreation(subscriber, session, current_exception());
        }
        // Else node is shutting down, or the session was established by a concurrent call.
    }
}

void
NodeI::createPublisherSession(
    const NodePrx& publisher,
    const ConnectionPtr& publisherConnection,
    shared_ptr<SubscriberSessionI> session)
{
    // The instance class owns this object, so it is guaranteed to be available.
    auto instance = _instance.lock();
    assert(instance);

    auto traceLevels = instance->getTraceLevels();

    try
    {
        auto p = getNodeWithExistingConnection(instance, publisher, publisherConnection);

        unique_lock<mutex> lock(_mutex);
        if (!session)
        {
            session = createSubscriberSessionServant(publisher);
            if (session->checkSession())
            {
                // The session is already connected.
                if (traceLevels->session > 2)
                {
                    Trace out(traceLevels->logger, traceLevels->sessionCat);
                    out << "node '" << _proxy->ice_getIdentity()
                        << "' is ignoring 'createPublisherSession' request from '" << publisher << "' because session '"
                        << session->getId() << "' is already connected to: '" << session->getSession() << "'";
                }
                throw SessionCreationException{SessionCreationError::AlreadyConnected};
            }
        }

        assert(session);
        p->createSessionAsync(
            _proxy,
            uncheckedCast<SubscriberSessionPrx>(session->getProxy()),
            false,
            nullptr,
            [self = shared_from_this(), publisher, session](exception_ptr ex)
            { self->retrySubscriberSessionCreation(publisher, session, ex); });
    }
    catch (const CommunicatorDestroyedException&)
    {
        // The node is shutting down, don't retry.
        throw SessionCreationException{SessionCreationError::NodeShutdown};
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        // The node is shutting down, don't retry.
        throw SessionCreationException{SessionCreationError::NodeShutdown};
    }
    catch (const LocalException&)
    {
        retrySubscriberSessionCreation(publisher, session, current_exception());
        throw SessionCreationException{SessionCreationError::Internal};
    }
}

void
NodeI::retrySubscriberSessionCreation(
    const NodePrx& node,
    const shared_ptr<SubscriberSessionI>& session,
    exception_ptr ex)
{
    try
    {
        rethrow_exception(ex);
    }
    catch (const SessionCreationException& sessionCreationException)
    {
        if (sessionCreationException.error == SessionCreationError::AlreadyConnected && session->checkSession())
        {
            // A concurrent session attempt from the peer succeeded, no need to retry.
            return;
        }
        // else let Session::retry handle the exception
    }
    catch (...)
    {
        // Let Session::retry handle the exception.
    }

    if (!session->retry(node, ex))
    {
        removeSubscriberSession(node, session, ex);
    }
}

void
NodeI::removeSubscriberSession(const NodePrx& node, const shared_ptr<SubscriberSessionI>& session, exception_ptr ex)
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

void
NodeI::retryPublisherSessionCreation(
    const NodePrx& node,
    const shared_ptr<PublisherSessionI>& session,
    exception_ptr ex)
{
    try
    {
        rethrow_exception(ex);
    }
    catch (const SessionCreationException& sessionCreationException)
    {
        if (sessionCreationException.error == SessionCreationError::AlreadyConnected && session->checkSession())
        {
            // A concurrent session attempt from the peer succeeded, no need to retry.
            return;
        }
        // else let Session::retry handle the exception
    }
    catch (...)
    {
        // Let Session::retry handle the exception.
    }

    if (!session->retry(node, ex))
    {
        removePublisherSession(node, session, ex);
    }
}

void
NodeI::removePublisherSession(const NodePrx& node, const shared_ptr<PublisherSessionI>& session, exception_ptr ex)
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

    // The instance class owns this object, so it is guaranteed to be available.
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
}

shared_ptr<PublisherSessionI>
NodeI::createPublisherSessionServant(const NodePrx& node)
{
    // Called with mutex locked

    // The instance class owns this object, so it is guaranteed to be available.
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

    // If the proxy is well-known and there isn't an existing connection, use the new connection.
    if (!connection && node->ice_getEndpoints().empty() && node->ice_getAdapterId().empty())
    {
        connection = newConnection;
    }

    if (connection)
    {
        return node->ice_fixed(connection);
    }

    // Ensure that the returned proxy doesn't have a cached connection.
    return node->ice_connectionCached(false)->ice_connectionCached(true);
}
