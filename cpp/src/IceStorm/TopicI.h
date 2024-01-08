//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TOPIC_I_H
#define TOPIC_I_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/Election.h>
#include <IceStorm/Instrumentation.h>
#include <IceStorm/Util.h>
#include <Ice/ObserverHelper.h>
#include <list>

namespace IceStorm
{

// Forward declarations
class PersistentInstance;
class Subscriber;

class TopicImpl
{
public:

    static std::shared_ptr<TopicImpl> create(std::shared_ptr<PersistentInstance>,
                                             const std::string&,
                                             const Ice::Identity&,
                                             const SubscriberRecordSeq&);

    std::string getName() const;
    std::shared_ptr<Ice::ObjectPrx> getPublisher() const;
    std::shared_ptr<Ice::ObjectPrx> getNonReplicatedPublisher() const;
    std::shared_ptr<Ice::ObjectPrx> subscribeAndGetPublisher(QoS, std::shared_ptr<Ice::ObjectPrx>);
    void unsubscribe(const std::shared_ptr<Ice::ObjectPrx>&);
    std::shared_ptr<TopicLinkPrx> getLinkProxy();
    void link(const std::shared_ptr<TopicPrx>&, int);
    void unlink(const std::shared_ptr<TopicPrx>&);
    LinkInfoSeq getLinkInfoSeq() const;
    Ice::IdentitySeq getSubscribers() const;
    void reap(const Ice::IdentitySeq&);
    void destroy();

    IceStormElection::TopicContent getContent() const;

    void update(const SubscriberRecordSeq&);

    // Internal methods
    bool destroyed() const;
    Ice::Identity id() const;
    std::shared_ptr<TopicPrx> proxy() const;
    void shutdown();
    void publish(bool, const EventDataSeq&);

    // Observer methods.
    void observerAddSubscriber(const IceStormElection::LogUpdate&, const SubscriberRecord&);
    void observerRemoveSubscriber(const IceStormElection::LogUpdate&, const Ice::IdentitySeq&);
    void observerDestroyTopic(const IceStormElection::LogUpdate&);

    std::shared_ptr<Ice::Object> getServant() const;

    void updateObserver();
    void updateSubscriberObservers();

private:

    TopicImpl(std::shared_ptr<PersistentInstance>, const std::string&, const Ice::Identity&, const SubscriberRecordSeq&);

    IceStormElection::LogUpdate destroyInternal(const IceStormElection::LogUpdate&, bool);
    void removeSubscribers(const Ice::IdentitySeq&);

    //
    // Immutable members.
    //
    const std::shared_ptr<Ice::ObjectPrx> _publisherReplicaProxy;
    const std::shared_ptr<PersistentInstance> _instance;
    const std::string _name; // The topic name
    const Ice::Identity _id; // The topic identity

    IceInternal::ObserverHelperT<IceStorm::Instrumentation::TopicObserver> _observer;

    std::shared_ptr<Ice::ObjectPrx> _publisherPrx; // The actual publisher proxy.
    std::shared_ptr<TopicLinkPrx> _linkPrx; // The link proxy.

    std::shared_ptr<Ice::Object> _servant; // The topic implementation servant.

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

    bool _destroyed; // Has this Topic been destroyed?

    LLUMap _lluMap;
    SubscriberMap _subscriberMap;
};

} // End namespace IceStorm

#endif
