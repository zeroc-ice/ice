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

    class TraceLevels;
    class Instance;
    class SessionI;
    class PublisherSessionI;
    class SubscriberSessionI;

    class NodeI final : virtual public DataStormContract::Node, public std::enable_shared_from_this<NodeI>
    {
    public:
        NodeI(const std::shared_ptr<Instance>&);
        virtual ~NodeI();

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
            std::optional<DataStormContract::NodePrx>,
            const Ice::ConnectionPtr&,
            const std::shared_ptr<PublisherSessionI>&);

        void createPublisherSession(
            std::optional<DataStormContract::NodePrx>,
            const Ice::ConnectionPtr&,
            std::shared_ptr<SubscriberSessionI>);

        void removeSubscriberSession(
            std::optional<DataStormContract::NodePrx>,
            const std::shared_ptr<SubscriberSessionI>&,
            const std::exception_ptr&);

        void removePublisherSession(
            std::optional<DataStormContract::NodePrx>,
            const std::shared_ptr<PublisherSessionI>&,
            const std::exception_ptr&);

        Ice::ConnectionPtr getSessionConnection(const std::string&) const;

        std::shared_ptr<SessionI> getSession(const Ice::Identity&) const;

        std::optional<DataStormContract::NodePrx>
        getNodeWithExistingConnection(std::optional<DataStormContract::NodePrx>, const Ice::ConnectionPtr&);

        std::optional<DataStormContract::NodePrx> getProxy() const { return _proxy; }

        std::shared_ptr<Instance> getInstance() const
        {
            auto instance = _instance.lock();
            assert(instance);
            return instance;
        }

        std::optional<DataStormContract::PublisherSessionPrx> getPublisherForwarder() const
        {
            return _publisherForwarder;
        }

        std::optional<DataStormContract::SubscriberSessionPrx> getSubscriberForwarder() const
        {
            return _subscriberForwarder;
        }

    private:
        std::shared_ptr<SubscriberSessionI> createSubscriberSessionServant(std::optional<DataStormContract::NodePrx>);

        std::shared_ptr<PublisherSessionI> createPublisherSessionServant(std::optional<DataStormContract::NodePrx>);

        void forward(const Ice::ByteSeq&, const Ice::Current&) const;

        mutable std::mutex _mutex;
        mutable std::condition_variable _cond;
        std::weak_ptr<Instance> _instance;
        std::optional<DataStormContract::NodePrx> _proxy;
        std::optional<DataStormContract::SubscriberSessionPrx> _subscriberForwarder;
        std::optional<DataStormContract::PublisherSessionPrx> _publisherForwarder;
        std::map<Ice::Identity, std::shared_ptr<SubscriberSessionI>> _subscribers;
        std::map<Ice::Identity, std::shared_ptr<PublisherSessionI>> _publishers;
        std::map<Ice::Identity, std::shared_ptr<SubscriberSessionI>> _subscriberSessions;
        std::map<Ice::Identity, std::shared_ptr<PublisherSessionI>> _publisherSessions;
        std::int64_t _nextSubscriberSessionId;
        std::int64_t _nextPublisherSessionId;
    };

}
#endif
