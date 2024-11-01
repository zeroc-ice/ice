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

        DataStormContract::NodePrx getNodeWithExistingConnection(DataStormContract::NodePrx, const Ice::ConnectionPtr&);

        DataStormContract::NodePrx getProxy() const { return _proxy; }

        std::shared_ptr<Instance> getInstance() const
        {
            auto instance = _instance.lock();
            assert(instance);
            return instance;
        }

        DataStormContract::PublisherSessionPrx getPublisherForwarder() const { return _publisherForwarder; }

        DataStormContract::SubscriberSessionPrx getSubscriberForwarder() const { return _subscriberForwarder; }

    private:
        std::shared_ptr<SubscriberSessionI> createSubscriberSessionServant(DataStormContract::NodePrx);

        std::shared_ptr<PublisherSessionI> createPublisherSessionServant(DataStormContract::NodePrx);

        void forwardToSubscribers(const Ice::ByteSeq&, const Ice::Current&) const;
        void forwardToPublishers(const Ice::ByteSeq&, const Ice::Current&) const;

        mutable std::mutex _mutex;
        mutable std::condition_variable _cond;
        std::weak_ptr<Instance> _instance;
        DataStormContract::NodePrx _proxy;
        DataStormContract::SubscriberSessionPrx _subscriberForwarder;
        DataStormContract::PublisherSessionPrx _publisherForwarder;
        std::map<Ice::Identity, std::shared_ptr<SubscriberSessionI>> _subscribers;
        std::map<Ice::Identity, std::shared_ptr<PublisherSessionI>> _publishers;
        std::map<Ice::Identity, std::shared_ptr<SubscriberSessionI>> _subscriberSessions;
        std::map<Ice::Identity, std::shared_ptr<PublisherSessionI>> _publisherSessions;
        std::int64_t _nextSubscriberSessionId;
        std::int64_t _nextPublisherSessionId;
    };
}
#endif
