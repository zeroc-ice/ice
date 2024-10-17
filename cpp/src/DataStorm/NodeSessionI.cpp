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

    class NodeForwarderI : public Node, public enable_shared_from_this<NodeForwarderI>
    {
    public:
        NodeForwarderI(
            shared_ptr<NodeSessionManager> nodeSessionManager,
            shared_ptr<NodeSessionI> session,
            optional<NodePrx> node)
            : _nodeSessionManager(std::move(nodeSessionManager)),
              _session(std::move(session)),
              _node(std::move(node))
        {
        }

        virtual void initiateCreateSession(optional<NodePrx> publisher, const Ice::Current& current) override
        {
            if (publisher == nullopt)
            {
                return;
            }

            auto session = _session.lock();
            if (!session)
            {
                return;
            }

            try
            {
                optional<SessionPrx> session;
                updateNodeAndSessionProxy(*publisher, session, current);
                // TODO check the return value?
                auto _ = _node->initiateCreateSessionAsync(publisher);
            }
            catch (const Ice::ObjectAdapterDestroyedException&)
            {
            }
            catch (const Ice::CommunicatorDestroyedException&)
            {
            }
        }

        virtual void createSession(
            optional<NodePrx> subscriber,
            optional<SubscriberSessionPrx> subscriberSession,
            bool /* fromRelay */,
            const Ice::Current& current) override
        {
            if (subscriber == nullopt || subscriberSession == nullopt)
            {
                return;
            }

            auto session = _session.lock();
            if (!session)
            {
                return;
            }

            try
            {
                updateNodeAndSessionProxy(*subscriber, subscriberSession, current);
                session->addSession(subscriberSession);
                // TODO check the return value?
                auto _ = _node->createSessionAsync(subscriber, subscriberSession, true);
            }
            catch (const Ice::ObjectAdapterDestroyedException&)
            {
            }
            catch (const Ice::CommunicatorDestroyedException&)
            {
            }
        }

        virtual void confirmCreateSession(
            optional<NodePrx> publisher,
            optional<PublisherSessionPrx> publisherSession,
            const Ice::Current& current) override
        {
            if (publisher == nullopt || publisherSession == nullopt)
            {
                return;
            }

            auto session = _session.lock();
            if (!session)
            {
                return;
            }
            try
            {
                updateNodeAndSessionProxy(*publisher, publisherSession, current);
                session->addSession(publisherSession);
                // TODO check the return value?
                auto _ = _node->confirmCreateSessionAsync(publisher, publisherSession);
            }
            catch (const Ice::ObjectAdapterDestroyedException&)
            {
            }
            catch (const Ice::CommunicatorDestroyedException&)
            {
            }
        }

    private:
        template<typename T>
        void updateNodeAndSessionProxy(NodePrx& node, optional<T>& session, const Ice::Current& current)
        {
            if (node->ice_getEndpoints().empty() && node->ice_getAdapterId().empty())
            {
                auto peerSession = _nodeSessionManager->createOrGet(node, current.con, false);
                assert(peerSession);
                node = peerSession->getPublicNode();
                if (session)
                {
                    session = peerSession->getSessionForwarder(session);
                }
            }
        }

        const shared_ptr<NodeSessionManager> _nodeSessionManager;
        const weak_ptr<NodeSessionI> _session;
        const optional<NodePrx> _node;
    };

}

NodeSessionI::NodeSessionI(
    shared_ptr<Instance> instance,
    optional<NodePrx> node,
    Ice::ConnectionPtr connection,
    bool forwardAnnouncements)
    : _instance(std::move(instance)),
      _traceLevels(_instance->getTraceLevels()),
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
    if (_node->ice_getEndpoints().empty() && _node->ice_getAdapterId().empty())
    {
        auto bidirNode = _node->ice_fixed(_connection);
        auto fwd = make_shared<NodeForwarderI>(_instance->getNodeSessionManager(), shared_from_this(), bidirNode);
        _publicNode = _instance->getObjectAdapter()->add<NodePrx>(fwd, _node->ice_getIdentity());
    }
    else
    {
        _publicNode = _node;
    }

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels, _traceLevels->sessionCat);
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
            _instance->getObjectAdapter()->remove(_publicNode->ice_getIdentity());
        }

        for (const auto& session : _sessions)
        {
            // TODO check the return value?
            auto _ = session.second->disconnectedAsync();
        }
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
    }

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels, _traceLevels->sessionCat);
        out << "destroyed node session (peer = `" << _publicNode << "')";
    }
}

void
NodeSessionI::addSession(optional<SessionPrx> session)
{
    lock_guard<mutex> lock(_mutex);
    _sessions[session->ice_getIdentity()] = std::move(session);
}

optional<SessionPrx>
NodeSessionI::forwarder(optional<SessionPrx> session) const
{
    auto id = session->ice_getIdentity();
    auto proxy = _instance->getObjectAdapter()->createProxy<SessionPrx>(
        {id.name + '-' + _node->ice_getIdentity().name, id.category + 'f'});
    return proxy->ice_oneway();
}
