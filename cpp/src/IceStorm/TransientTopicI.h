// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TRANSIENT_TOPIC_I_H
#define TRANSIENT_TOPIC_I_H

#include <IceStorm/IceStormInternal.h>

namespace IceStorm
{

// Forward declarations.
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class TransientTopicImpl : public TopicInternal, public IceUtil::Mutex
{
public:

    TransientTopicImpl(const InstancePtr&, const std::string&, const Ice::Identity&);
    ~TransientTopicImpl();

    virtual std::string getName(const Ice::Current&) const;
    virtual Ice::ObjectPrx getPublisher(const Ice::Current&) const;
    virtual Ice::ObjectPrx getNonReplicatedPublisher(const Ice::Current&) const;
    virtual void subscribe(const QoS&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual Ice::ObjectPrx subscribeAndGetPublisher(const QoS&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual void unsubscribe(const Ice::ObjectPrx&, const Ice::Current&);
    virtual TopicLinkPrx getLinkProxy(const Ice::Current&);
    virtual void link(const TopicPrx&, Ice::Int, const Ice::Current&);
    virtual void unlink(const TopicPrx&, const Ice::Current&);
    virtual LinkInfoSeq getLinkInfoSeq(const Ice::Current&) const;
    virtual Ice::IdentitySeq getSubscribers(const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);
    virtual void reap(const Ice::IdentitySeq&, const Ice::Current&);

    // Internal methods
    bool destroyed() const;
    Ice::Identity id() const;
    void publish(bool, const EventDataSeq&);

    void shutdown();

private:

    //
    // Immutable members.
    //
    const InstancePtr _instance;
    const std::string _name; // The topic name
    const Ice::Identity _id; // The topic identity

    /*const*/ Ice::ObjectPrx _publisherPrx;
    /*const*/ TopicLinkPrx _linkPrx;

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

typedef IceUtil::Handle<TransientTopicImpl> TransientTopicImplPtr;

} // End namespace IceStorm

#endif
