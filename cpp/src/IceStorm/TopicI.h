// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TOPIC_I_H
#define TOPIC_I_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/IdentityLinkDict.h>

#include <IceStorm/SubscriberFactory.h>

namespace IceStorm
{

//
// Forward declarations.
//
class TopicSubscribers;
typedef IceUtil::Handle<TopicSubscribers> TopicSubscribersPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class SubscriberFactory;
typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

//
// TopicInternal implementation.
//
class TopicI : public TopicInternal, public JTCRecursiveMutex
{
public:

    TopicI(const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&, const std::string&, const SubscriberFactoryPtr&,
	   const Freeze::DBPtr&);
    ~TopicI();

    virtual std::string getName(const Ice::Current&);
    virtual Ice::ObjectPrx getPublisher(const Ice::Current&);
    virtual void destroy(const Ice::Current&);
    virtual void link(const TopicPrx&, Ice::Int, const Ice::Current&);
    virtual void unlink(const TopicPrx&, const Ice::Current&);
    virtual LinkInfoSeq getLinkInfoSeq(const Ice::Current&);

    virtual TopicLinkPrx getLinkProxy(const Ice::Current&);

    // Internal methods
    bool destroyed() const;
    void subscribe(const Ice::ObjectPrx&, const std::string&, const QoS&);
    void unsubscribe(const std::string&);

    void reap();

private:

    //
    // Immutable members.
    //
    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    std::string _name; // The topic name
    SubscriberFactoryPtr _factory;

    Ice::ObjectPtr _publisher; // Publisher & associated proxy
    Ice::ObjectPrx _publisherPrx;

    Ice::ObjectPtr _link; // TopicLink & associated proxy
    TopicLinkPrx _linkPrx;

    //
    // Mutable members. Protected by *this
    //
    bool _destroyed; // Has this Topic been destroyed?

    TopicSubscribersPtr _subscribers; // Set of Subscribers

    IdentityLinkDict _links; // The database of Topic links
    Freeze::DBPtr _linksDb;
};

typedef IceUtil::Handle<TopicI> TopicIPtr;

} // End namespace IceStorm

#endif
