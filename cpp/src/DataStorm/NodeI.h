// Copyright (c) ZeroC, Inc.

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

    class NodeI final : public virtual DataStormContract::AsyncNode, public std::enable_shared_from_this<NodeI>
    {
    public:
        NodeI(const std::shared_ptr<Instance>&, std::string);
        ~NodeI() final;

        void init();
        void destroy(bool);

        void initiateCreateSessionAsync(
            std::optional<DataStormContract::NodePrx>,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void createSessionAsync(
            std::optional<DataStormContract::NodePrx>,
            std::optional<DataStormContract::SubscriberSessionPrx>,
            bool,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void confirmCreateSessionAsync(
            std::optional<DataStormContract::NodePrx>,
            std::optional<DataStormContract::PublisherSessionPrx>,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void createSubscriberSession(
            DataStormContract::NodePrx,
            const Ice::ConnectionPtr&,
            const std::shared_ptr<PublisherSessionI>&);

        void createPublisherSession(
            const DataStormContract::NodePrx&,
            const Ice::ConnectionPtr&,
            std::shared_ptr<SubscriberSessionI>);

        void retrySubscriberSessionCreation(
            const DataStormContract::NodePrx&,
            const std::shared_ptr<SubscriberSessionI>&,
            std::exception_ptr);

        void retryPublisherSessionCreation(
            const DataStormContract::NodePrx&,
            const std::shared_ptr<PublisherSessionI>&,
            std::exception_ptr);

        void removeSubscriberSession(
            const DataStormContract::NodePrx&,
            const std::shared_ptr<SubscriberSessionI>&,
            std::exception_ptr);

        void removePublisherSession(
            const DataStormContract::NodePrx&,
            const std::shared_ptr<PublisherSessionI>&,
            std::exception_ptr);

        [[nodiscard]] Ice::ConnectionPtr getSessionConnection(std::string_view) const;

        [[nodiscard]] std::shared_ptr<SessionI> getSession(const Ice::Identity&) const;

        [[nodiscard]] DataStormContract::NodePrx getNodeWithExistingConnection(
            const std::shared_ptr<Instance>& instance,
            const DataStormContract::NodePrx& node,
            const Ice::ConnectionPtr& connection);

        [[nodiscard]] DataStormContract::NodePrx getProxy() const { return _proxy; }

        [[nodiscard]] DataStormContract::PublisherSessionPrx getPublisherForwarder() const
        {
            return _publisherForwarder;
        }

        [[nodiscard]] DataStormContract::SubscriberSessionPrx getSubscriberForwarder() const
        {
            return _subscriberForwarder;
        }

    private:
        [[nodiscard]] std::shared_ptr<SubscriberSessionI>
        createSubscriberSessionServant(const DataStormContract::NodePrx&);

        [[nodiscard]] std::shared_ptr<PublisherSessionI>
        createPublisherSessionServant(const DataStormContract::NodePrx&);

        void forwardToSubscribers(const Ice::ByteSeq&, const Ice::Current&) const;
        void forwardToPublishers(const Ice::ByteSeq&, const Ice::Current&) const;

        std::weak_ptr<Instance> _instance;
        mutable std::mutex _mutex;
        std::int64_t _nextPublisherSessionId{0};
        std::int64_t _nextSubscriberSessionId{0};

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
