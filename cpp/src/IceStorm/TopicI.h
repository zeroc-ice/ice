// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TOPIC_I_H
#define TOPIC_I_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/Election.h>
#include <IceStorm/Instrumentation.h>
#include <Ice/ObserverHelper.h>
#include <Freeze/Freeze.h>
#include <list>

namespace IceStorm
{

// Forward declarations
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class TopicImpl : public IceUtil::Shared
{
public:

    TopicImpl(const InstancePtr&, const std::string&, const Ice::Identity&, const SubscriberRecordSeq&);
    ~TopicImpl();

    std::string getName() const;
    Ice::ObjectPrx getPublisher() const;
    Ice::ObjectPrx getNonReplicatedPublisher() const;
    Ice::ObjectPrx subscribeAndGetPublisher(const QoS&, const Ice::ObjectPrx&);
    void unsubscribe(const Ice::ObjectPrx&);
    TopicLinkPrx getLinkProxy();
    void link(const TopicPrx&, Ice::Int);
    void unlink(const TopicPrx&);
    LinkInfoSeq getLinkInfoSeq() const;
    Ice::IdentitySeq getSubscribers() const;
    void reap(const Ice::IdentitySeq&);
    void destroy();

    IceStormElection::TopicContent getContent() const;

    void update(const SubscriberRecordSeq&);

    // Internal methods
    bool destroyed() const;
    Ice::Identity id() const;
    TopicPrx proxy() const;
    void shutdown();
    void publish(bool, const EventDataSeq&);

    // Observer methods.
    void observerAddSubscriber(const IceStormElection::LogUpdate&, const SubscriberRecord&);
    void observerRemoveSubscriber(const IceStormElection::LogUpdate&, const Ice::IdentitySeq&);
    void observerDestroyTopic(const IceStormElection::LogUpdate&);

    Ice::ObjectPtr getServant() const;

    void updateObserver();
    void updateSubscriberObservers();

private:

    IceStormElection::LogUpdate destroyInternal(const IceStormElection::LogUpdate&, bool);
    void removeSubscribers(const Ice::IdentitySeq&);

    //
    // Immutable members.
    //
    const Ice::ObjectPrx _publisherReplicaProxy;
    const InstancePtr _instance;
    const Freeze::ConnectionPtr _connection;
    const std::string _name; // The topic name
    const Ice::Identity _id; // The topic identity
    const std::string _envName;

    IceInternal::ObserverHelperT<IceStorm::Instrumentation::TopicObserver> _observer;

    /*const*/ Ice::ObjectPrx _publisherPrx; // The actual publisher proxy.
    /*const*/ TopicLinkPrx _linkPrx; // The link proxy.

    Ice::ObjectPtr _servant; // The topic implementation servant.

    // Mutex protecting the subscribers.
    IceUtil::Mutex _subscribersMutex;

    //
    // We keep a vector of subscribers since the optimized behaviour
    // should be publishing events, not searching through the list of
    // subscribers for a particular subscriber. I tested
    // vector/list/map and although there was little difference vector
    // was the fastest of the three.
    //
    std::vector<SubscriberPtr> _subscribers;

    bool _destroyed; // Has this Topic been destroyed?
};

typedef IceUtil::Handle<TopicImpl> TopicImplPtr;

} // End namespace IceStorm

#endif
