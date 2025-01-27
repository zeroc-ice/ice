// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_TOPIC_I_H
#define ICESTORM_TOPIC_I_H

#include "Election.h"
#include "Ice/ObserverHelper.h"
#include "IceStormInternal.h"
#include "Instrumentation.h"
#include "Util.h"

#include <list>

namespace IceStorm
{
    // Forward declarations
    class PersistentInstance;
    class Subscriber;

    class TopicImpl
    {
    public:
        static std::shared_ptr<TopicImpl> create(
            const std::shared_ptr<PersistentInstance>&,
            const std::string&,
            const Ice::Identity&,
            const SubscriberRecordSeq&);

        [[nodiscard]] std::string getName() const;
        [[nodiscard]] Ice::ObjectPrx getPublisher() const;
        [[nodiscard]] Ice::ObjectPrx getNonReplicatedPublisher() const;
        std::optional<Ice::ObjectPrx> subscribeAndGetPublisher(QoS, Ice::ObjectPrx);
        void unsubscribe(const Ice::ObjectPrx&);
        TopicLinkPrx getLinkProxy();
        void link(const TopicPrx&, int);
        void unlink(const TopicPrx&);
        [[nodiscard]] LinkInfoSeq getLinkInfoSeq() const;
        [[nodiscard]] Ice::IdentitySeq getSubscribers() const;
        void reap(const Ice::IdentitySeq&);
        void destroy();

        [[nodiscard]] IceStormElection::TopicContent getContent() const;

        void update(const SubscriberRecordSeq&);

        // Internal methods
        [[nodiscard]] bool destroyed() const;
        [[nodiscard]] Ice::Identity id() const;
        [[nodiscard]] TopicPrx proxy() const;
        void shutdown();
        void publish(bool, const EventDataSeq&);

        // Observer methods.
        void observerAddSubscriber(const IceStormElection::LogUpdate&, const SubscriberRecord&);
        void observerRemoveSubscriber(const IceStormElection::LogUpdate&, const Ice::IdentitySeq&);
        void observerDestroyTopic(const IceStormElection::LogUpdate&);

        [[nodiscard]] Ice::ObjectPtr getServant() const;

        void updateObserver();
        void updateSubscriberObservers();

    private:
        TopicImpl(std::shared_ptr<PersistentInstance>, std::string, Ice::Identity, const SubscriberRecordSeq&);

        IceStormElection::LogUpdate destroyInternal(const IceStormElection::LogUpdate&, bool);
        void removeSubscribers(const Ice::IdentitySeq&);

        //
        // Immutable members.
        //
        const std::shared_ptr<PersistentInstance> _instance;
        const std::string _name; // The topic name
        const Ice::Identity _id; // The topic identity

        IceInternal::ObserverHelperT<IceStorm::Instrumentation::TopicObserver> _observer;

        std::optional<Ice::ObjectPrx> _publisherPrx; // The actual publisher proxy.
        std::optional<TopicLinkPrx> _linkPrx;        // The link proxy.

        Ice::ObjectPtr _servant; // The topic implementation servant.

        // Mutex protecting the subscribers.
        mutable std::mutex _subscribersMutex;

        //
        // We keep a vector of subscribers since the optimized behaviour
        // should be publishing events, not searching through the list of
        // subscribers for a particular subscriber. I tested
        // vector/list/map and although there was little difference vector
        // was the fastest of the three.
        //
        std::vector<std::shared_ptr<Subscriber>> _subscribers;

        bool _destroyed{false}; // Has this Topic been destroyed?

        LLUMap _lluMap;
        SubscriberMap _subscriberMap;
    };

} // End namespace IceStorm

#endif
