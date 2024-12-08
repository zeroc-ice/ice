//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_NODEI_H
#define DATASTORM_NODEI_H

#include "DataStorm/Contract.h"
#include "DataStorm/InternalI.h"
#include "ForwarderManager.h"

#include "Ice/Ice.h"

#include <set>

namespace DataStormI
{
    class Instance;
    class SessionI;
    class PublisherSessionI;
    class SubscriberSessionI;

    class NodeI final : public virtual DataStormContract::Node, public std::enable_shared_from_this<NodeI>
    {
    public:
        NodeI(const std::shared_ptr<Instance>&);
        ~NodeI() final;

        void init();
        void destroy(bool);

        void initiateCreateSession(std::optional<DataStormContract::NodePrx>, const Ice::Current&) final;

        void createSession(
            std::optional<DataStormContract::NodePrx>,
            std::optional<DataStormContract::SubscriberSessionPrx>,
            bool,
            const Ice::Current&) final;

        void confirmCreateSession(
            std::optional<DataStormContract::NodePrx>,
            std::optional<DataStormContract::PublisherSessionPrx>,
            const Ice::Current&) final;

        void createSubscriberSession(
            DataStormContract::NodePrx,
            const Ice::ConnectionPtr&,
            const std::shared_ptr<PublisherSessionI>&);

        void createPublisherSession(
            DataStormContract::NodePrx,
            const Ice::ConnectionPtr&,
            std::shared_ptr<SubscriberSessionI>);

        void removeSubscriberSession(
            DataStormContract::NodePrx,
            const std::shared_ptr<SubscriberSessionI>&,
            std::exception_ptr);

        void removePublisherSession(
            DataStormContract::NodePrx,
            const std::shared_ptr<PublisherSessionI>&,
            std::exception_ptr);

        Ice::ConnectionPtr getSessionConnection(std::string_view) const;

        std::shared_ptr<SessionI> getSession(const Ice::Identity&) const;

        DataStormContract::NodePrx getNodeWithExistingConnection(
            const std::shared_ptr<Instance>& instance,
            DataStormContract::NodePrx node,
            const Ice::ConnectionPtr& connection);

        DataStormContract::NodePrx getProxy() const { return _proxy; }

        DataStormContract::PublisherSessionPrx getPublisherForwarder() const { return _publisherForwarder; }

        DataStormContract::SubscriberSessionPrx getSubscriberForwarder() const { return _subscriberForwarder; }

    private:
        std::shared_ptr<SubscriberSessionI> createSubscriberSessionServant(DataStormContract::NodePrx);

        std::shared_ptr<PublisherSessionI> createPublisherSessionServant(DataStormContract::NodePrx);

        void forwardToSubscribers(const Ice::ByteSeq&, const Ice::Current&) const;
        void forwardToPublishers(const Ice::ByteSeq&, const Ice::Current&) const;

        std::weak_ptr<Instance> _instance;
        mutable std::mutex _mutex;
        std::int64_t _nextPublisherSessionId;
        std::int64_t _nextSubscriberSessionId;

        // The proxy for this node.
        DataStormContract::NodePrx _proxy;

        // A map of all publisher sessions, indexed by the identity of the peer node.
        std::map<Ice::Identity, std::shared_ptr<PublisherSessionI>> _publishers;

        // A proxy to a colocated publisher session object that forwards requests to all active publisher sessions.
        DataStormContract::PublisherSessionPrx _publisherForwarder;

        // A map of all publisher sessions, indexed by the identity of each session.
        std::map<Ice::Identity, std::shared_ptr<PublisherSessionI>> _publisherSessions;

        // A map of all subscriber sessions, indexed by the identity of the peer node.
        std::map<Ice::Identity, std::shared_ptr<SubscriberSessionI>> _subscribers;

        // A proxy to a colocated subscriber session object that forwards requests to all active subscriber sessions.
        DataStormContract::SubscriberSessionPrx _subscriberForwarder;

        // A map of all subscriber sessions, indexed by the identity of each session.
        std::map<Ice::Identity, std::shared_ptr<SubscriberSessionI>> _subscriberSessions;
    };
}
#endif
