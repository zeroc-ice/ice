//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "NodeSessionI.h"
#include "ConnectionManager.h"
#include "Instance.h"
#include "NodeSessionManager.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;

namespace
{
    // The NodeForwarder class is used to forward calls to a Node that doesn't have a public endpoint.
    // It implements the Slice DataContract::Node interface by forwarding calls to the target Node object.
    class NodeForwarder : public Node, public enable_shared_from_this<NodeForwarder>
    {
    public:
        NodeForwarder(shared_ptr<NodeSessionManager> nodeSessionManager, shared_ptr<NodeSessionI> nodeSession, NodePrx node)
            : _nodeSessionManager(std::move(nodeSessionManager)),
              _nodeSession(std::move(nodeSession)),
              _node(std::move(node))
        {
        }

        void initiateCreateSession(optional<NodePrx> publisher, const Ice::Current& current) final
        {
            Ice::checkNotNull(publisher, __FILE__, __LINE__, current);
            if (auto nodeSession = _nodeSession.lock())
            {
                try
                {
                    optional<SessionPrx> sessionPrx;
                    updateNodeAndSessionProxy(*publisher, sessionPrx, current);
                    // Forward the call to the target Node object, don't need to wait for the result.
                    _node->initiateCreateSessionAsync(publisher, nullptr);
                }
                catch (const Ice::CommunicatorDestroyedException&)
                {
                }
            }
        }

        void createSession(
            optional<NodePrx> subscriber,
            optional<SubscriberSessionPrx> subscriberSession,
            bool /* fromRelay */,
            const Ice::Current& current) final
        {
            Ice::checkNotNull(subscriber, __FILE__, __LINE__, current);
            Ice::checkNotNull(subscriberSession, __FILE__, __LINE__, current);

            if (auto nodeSession = _nodeSession.lock())
            {
                try
                {
                    updateNodeAndSessionProxy(*subscriber, subscriberSession, current);
                    nodeSession->addSession(*subscriberSession);
                    // Forward the call to the target Node object, don't need to wait for the result.
                    _node->createSessionAsync(subscriber, subscriberSession, true, nullptr);
                }
                catch (const Ice::CommunicatorDestroyedException&)
                {
                }
            }
        }

        void confirmCreateSession(
            optional<NodePrx> publisher,
            optional<PublisherSessionPrx> publisherSession,
            const Ice::Current& current) final
        {
            Ice::checkNotNull(publisher, __FILE__, __LINE__, current);
            Ice::checkNotNull(publisherSession, __FILE__, __LINE__, current);

            if (auto nodeSession = _nodeSession.lock())
            {
                try
                {
                    updateNodeAndSessionProxy(*publisher, publisherSession, current);
                    nodeSession->addSession(*publisherSession);
                    // Forward the call to the target Node object, don't need to wait for the result.
                    _node->confirmCreateSessionAsync(publisher, publisherSession, nullptr);
                }
                catch (const Ice::CommunicatorDestroyedException&)
                {
                }
            }
        }

    private:
        // This helper method is used to replace the Node and Session proxies with forwarders when the calling Node
        // doesn't have a public endpoint.
        template<typename T>
        void updateNodeAndSessionProxy(NodePrx& node, optional<T>& session, const Ice::Current& current)
        {
            if (node->ice_getEndpoints().empty() && node->ice_getAdapterId().empty())
            {
                shared_ptr<NodeSessionI> nodeSession = _nodeSessionManager->createOrGet(node, current.con, false);
                assert(nodeSession);
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
    Ice::ConnectionPtr connection,
    bool forwardAnnouncements)
    : _instance(std::move(instance)),
      _node(std::move(node)),
      _connection(std::move(connection))
{
    if (forwardAnnouncements)
    {
        _lookup = _connection->createProxy<LookupPrx>({"Lookup", "DataStorm"});
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
        Trace out(_instance->getTraceLevels(), _instance->getTraceLevels()->sessionCat);
        out << "created node session (peer = `" << _publicNode << "'):\n" << _connection->toString();
    }
}

void
NodeSessionI::destroy()
{
    lock_guard<mutex> lock(_mutex);
    _destroyed = true;

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
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
    }

    if (_instance->getTraceLevels()->session > 0)
    {
        Trace out(_instance->getTraceLevels(), _instance->getTraceLevels()->sessionCat);
        out << "destroyed node session (peer = `" << _publicNode << "')";
    }
}

void
NodeSessionI::addSession(SessionPrx session)
{
    lock_guard<mutex> lock(_mutex);
    _sessions[session->ice_getIdentity()] = std::move(session);
}
