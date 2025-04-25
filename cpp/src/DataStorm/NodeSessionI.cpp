// Copyright (c) ZeroC, Inc.

#include "NodeSessionI.h"
#include "ConnectionManager.h"
#include "Instance.h"
#include "NodeSessionManager.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

namespace
{
    // The NodeForwarder class forwards calls to a Node that lacks a public endpoint.
    //
    // This class implements the Slice DataContract::Node interface by forwarding calls to the target Node object
    // using the connection established during the creation of the NodeSession object.
    //
    // The NodeForwarder wraps the node and session proxy parameters passed to the DataContract::Node operations
    // in forwarder proxies, which handle forwarding to the corresponding target objects.
    class NodeForwarder : public Node, public enable_shared_from_this<NodeForwarder>
    {
    public:
        NodeForwarder(
            shared_ptr<NodeSessionManager> nodeSessionManager,
            const shared_ptr<NodeSessionI>& nodeSession,
            NodePrx node)
            : _nodeSessionManager(std::move(nodeSessionManager)),
              _nodeSession(nodeSession),
              _node(std::move(node))
        {
        }

        void initiateCreateSessionAsync(
            optional<NodePrx> publisher,
            function<void()> response,
            function<void(exception_ptr)> exception,
            const Current& current) final
        {
            checkNotNull(publisher, __FILE__, __LINE__, current);
            if (auto nodeSession = _nodeSession.lock())
            {
                try
                {
                    optional<SessionPrx> sessionPrx;
                    updateNodeAndSessionProxy(*publisher, sessionPrx, current);
                    // Forward the call to the target Node.
                    _node->initiateCreateSessionAsync(publisher, response, exception);
                }
                catch (const CommunicatorDestroyedException&)
                {
                    exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
                }
            }
        }

        void createSessionAsync(
            optional<NodePrx> subscriber,
            optional<SubscriberSessionPrx> subscriberSession,
            bool /* fromRelay */,
            function<void()> response,
            function<void(exception_ptr)> exception,
            const Current& current) final
        {
            checkNotNull(subscriber, __FILE__, __LINE__, current);
            checkNotNull(subscriberSession, __FILE__, __LINE__, current);

            bool subscriberIsHostedOnRelay =
                (subscriber->ice_getEndpoints().empty() && subscriber->ice_getAdapterId().empty());

            if (auto nodeSession = _nodeSession.lock())
            {
                try
                {
                    optional<SubscriberSessionPrx> subscriberSessionForwarder = subscriberSession;
                    updateNodeAndSessionProxy(*subscriber, subscriberSessionForwarder, current);

                    // Keep track of the subscriber session with the NodeSession, the NodeSession will use this proxy
                    // to inform the subscriber of the disconnection if the target publisher is disconnected.
                    nodeSession->addSession(
                        subscriberIsHostedOnRelay ? subscriberSession->ice_fixed(current.con) : *subscriberSession);

                    // Forward the call to the target Node.
                    _node->createSessionAsync(subscriber, subscriberSessionForwarder, true, response, exception);
                }
                catch (const CommunicatorDestroyedException&)
                {
                    exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
                }
            }
        }

        void confirmCreateSessionAsync(
            optional<NodePrx> publisher,
            optional<PublisherSessionPrx> publisherSession,
            function<void()> response,
            function<void(exception_ptr)> exception,
            const Current& current) final
        {
            checkNotNull(publisher, __FILE__, __LINE__, current);
            checkNotNull(publisherSession, __FILE__, __LINE__, current);

            if (auto nodeSession = _nodeSession.lock())
            {
                bool publisherIsHostedOnRelay =
                    (publisher->ice_getEndpoints().empty() && publisher->ice_getAdapterId().empty());
                try
                {
                    optional<PublisherSessionPrx> publisherSessionForwarder = publisherSession;
                    updateNodeAndSessionProxy(*publisher, publisherSessionForwarder, current);

                    // Keep track of the publisher session with the NodeSession, the NodeSession will use this proxy
                    // to inform the publisher of the disconnection if the target subscriber is disconnected.
                    nodeSession->addSession(
                        publisherIsHostedOnRelay ? publisherSession->ice_fixed(current.con) : *publisherSession);
                    // Forward the request to the target subscriber.
                    _node->confirmCreateSessionAsync(publisher, publisherSessionForwarder, response, exception);
                }
                catch (const CommunicatorDestroyedException&)
                {
                    exception(make_exception_ptr(SessionCreationException{SessionCreationError::NodeShutdown}));
                }
            }
        }

    private:
        /// This helper method is used to replace the Node and Session proxy parameters with forwarder proxies.
        ///
        /// Before forwarding a request to the target Node, this method is called and creates the required forwarders
        /// to ensure that the target can call back to the source node and session objects using the provided proxy.
        ///
        /// @tparam T The type of the session being updated.
        /// @param node The proxy for the Node, which may be replaced with a forwarder if the Node lacks a public
        /// endpoint.
        /// @param session The optional session proxy, which may be replaced with a forwarder if the Node lacks a public
        /// endpoint.
        /// @param current A reference to the current object of the request.
        template<typename T> void updateNodeAndSessionProxy(NodePrx& node, optional<T>& session, const Current& current)
        {
            if (node->ice_getEndpoints().empty() && node->ice_getAdapterId().empty())
            {
                shared_ptr<NodeSessionI> nodeSession = _nodeSessionManager->createOrGet(node, current.con, false);
                node = nodeSession->getPublicNode();
                if (session)
                {
                    session = nodeSession->forwarder(*session);
                }
            }
        }

        const shared_ptr<NodeSessionManager> _nodeSessionManager;
        const weak_ptr<NodeSessionI> _nodeSession;
        const NodePrx _node;
    };
}

NodeSessionI::NodeSessionI(
    shared_ptr<Instance> instance,
    NodePrx node,
    ConnectionPtr connection,
    bool forwardAnnouncements)
    : _instance(std::move(instance)),
      _node(std::move(node)),
      _connection(std::move(connection))
{
    if (forwardAnnouncements)
    {
        _lookup = _connection->createProxy<LookupPrx>(Identity{.name = "Lookup", .category = "DataStorm"});
    }
}

void
NodeSessionI::init()
{
    // When the target node doesn't have a public endpoint, we create a NodeForwarder object to forward calls to the
    // target and assign it to the publicNode member, otherwise the publicNode member is set to the target node.
    if (_node->ice_getEndpoints().empty() && _node->ice_getAdapterId().empty())
    {
        _publicNode = _instance->getObjectAdapter()->add<NodePrx>(
            make_shared<NodeForwarder>(
                _instance->getNodeSessionManager(),
                shared_from_this(),
                _node->ice_fixed(_connection)),
            _node->ice_getIdentity());
    }
    else
    {
        _publicNode = _node;
    }

    if (_instance->getTraceLevels()->session > 0)
    {
        Trace out(_instance->getTraceLevels()->logger, _instance->getTraceLevels()->sessionCat);
        out << "created node session (peer = '" << _publicNode << "'):\n" << _connection->toString();
    }
}

void
NodeSessionI::destroy()
{
    lock_guard<mutex> lock(_mutex);
    try
    {
        if (_publicNode != _node)
        {
            // Remove the NodeForwarder object from the object adapter.
            _instance->getObjectAdapter()->remove(_publicNode->ice_getIdentity());
        }

        for (const auto& [_, session] : _sessions)
        {
            // Notify sessions of the disconnection, don't need to wait for the result.
            session->disconnectedAsync(nullptr);
        }
    }
    catch (const ObjectAdapterDestroyedException&)
    {
    }
    catch (const CommunicatorDestroyedException&)
    {
    }

    if (_instance->getTraceLevels()->session > 0)
    {
        Trace out(_instance->getTraceLevels()->logger, _instance->getTraceLevels()->sessionCat);
        out << "destroyed node session (peer = '" << _publicNode << "')";
    }
}

void
NodeSessionI::addSession(SessionPrx session)
{
    lock_guard<mutex> lock(_mutex);
    Identity id = session->ice_getIdentity();
    _sessions.insert_or_assign(std::move(id), std::move(session));
}
