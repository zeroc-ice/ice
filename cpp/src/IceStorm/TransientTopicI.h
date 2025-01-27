// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_TRANSIENT_TOPIC_I_H
#define ICESTORM_TRANSIENT_TOPIC_I_H

#include "IceStormInternal.h"

namespace IceStorm
{
    // Forward declarations.
    class Instance;
    class Subscriber;

    class TransientTopicImpl : public TopicInternal
    {
    public:
        static std::shared_ptr<TransientTopicImpl>
        create(const std::shared_ptr<Instance>&, const std::string&, const Ice::Identity&);

        [[nodiscard]] std::string getName(const Ice::Current&) const override;
        [[nodiscard]] std::optional<Ice::ObjectPrx> getNonReplicatedPublisher(const Ice::Current&) const override;
        [[nodiscard]] std::optional<Ice::ObjectPrx> getPublisher(const Ice::Current&) const override;
        std::optional<Ice::ObjectPrx>
        subscribeAndGetPublisher(QoS, std::optional<Ice::ObjectPrx>, const Ice::Current&) override;
        void unsubscribe(std::optional<Ice::ObjectPrx>, const Ice::Current&) override;
        std::optional<TopicLinkPrx> getLinkProxy(const Ice::Current&) override;
        void link(std::optional<TopicPrx>, int, const Ice::Current&) override;
        void unlink(std::optional<TopicPrx>, const Ice::Current&) override;
        [[nodiscard]] LinkInfoSeq getLinkInfoSeq(const Ice::Current&) const override;
        [[nodiscard]] Ice::IdentitySeq getSubscribers(const Ice::Current&) const override;
        void destroy(const Ice::Current&) override;
        void reap(Ice::IdentitySeq, const Ice::Current&) override;

        // Internal methods
        [[nodiscard]] bool destroyed() const;
        [[nodiscard]] Ice::Identity id() const;
        void publish(bool, const EventDataSeq&);

        void shutdown();

    private:
        TransientTopicImpl(std::shared_ptr<Instance>, std::string, Ice::Identity);

        //
        // Immutable members.
        //
        const std::shared_ptr<Instance> _instance;
        const std::string _name; // The topic name
        const Ice::Identity _id; // The topic identity

        std::optional<Ice::ObjectPrx> _publisherPrx;
        std::optional<TopicLinkPrx> _linkPrx;

        //
        // We keep a vector of subscribers since the optimized behaviour
        // should be publishing events, not searching through the list of
        // subscribers for a particular subscriber. I tested
        // vector/list/map and although there was little difference vector
        // was the fastest of the three.
        //
        std::vector<std::shared_ptr<Subscriber>> _subscribers;

        bool _destroyed{false}; // Has this Topic been destroyed?

        mutable std::mutex _mutex;
    };

} // End namespace IceStorm

#endif
