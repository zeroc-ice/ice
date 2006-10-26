// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TOPIC_I_H
#define TOPIC_I_H

#include <IceUtil/RecMutex.h>
#include <IceStorm/IceStormInternal.h>
#include <IceStorm/PersistentTopicMap.h>
#include <IceStorm/PersistentUpstreamMap.h>
#include <list>

namespace IceStorm
{

class Event;
typedef IceUtil::Handle<Event> EventPtr;

class TopicSubscribers;
typedef IceUtil::Handle<TopicSubscribers> TopicSubscribersPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class SubscriberFactory;
typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

typedef std::list<SubscriberPtr> SubscriberList;

class KeepAliveThread;
typedef IceUtil::Handle<KeepAliveThread> KeepAliveThreadPtr;

class TopicSubscribers : public IceUtil::Shared
{
public:

    TopicSubscribers(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    virtual ~TopicSubscribers();

    void add(const SubscriberPtr&);
    void remove(const Ice::ObjectPrx&);
    void publish(const EventPtr&);
    SubscriberList clearErrorList();

private:
	
    Ice::CommunicatorPtr _communicator;
    TraceLevelsPtr _traceLevels;

    //
    // TODO: Should there be a map from identity to subscriber?
    //
    IceUtil::Mutex _subscribersMutex;
    SubscriberList _subscribers;

    //
    // Set of subscribers that have encountered an error.
    //
    IceUtil::Mutex _errorMutex;
    SubscriberList _error;
};

class TopicI : public TopicInternal
{
public:

    TopicI(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&, const KeepAliveThreadPtr&,
	   const std::string&, const LinkRecordDict&, const SubscriberFactoryPtr&, const std::string&,
	   const std::string&);
    ~TopicI();

    virtual std::string getName(const Ice::Current&) const;
    virtual Ice::ObjectPrx getPublisher(const Ice::Current&) const;
    virtual void subscribe(const QoS&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual Ice::ObjectPrx subscribeAndGetPublisher(const QoS&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual void unsubscribe(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);
    virtual void link(const TopicPrx&, Ice::Int, const Ice::Current&);
    virtual void unlink(const TopicPrx&, const Ice::Current&);
    virtual void unlinkByName(const std::string&, const Ice::Current&);
    virtual LinkInfoSeq getLinkInfoSeq(const Ice::Current&) const;

    virtual TopicLinkPrx getLinkProxy(const Ice::Current&);
    virtual void linkNotification(const std::string&, const TopicUpstreamLinkPrx&, const Ice::Current&);
    virtual void unlinkNotification(const std::string&, const TopicUpstreamLinkPrx&, const Ice::Current&);

    // Internal methods
    bool destroyed() const;

    void reap();

private:

    //
    // Immutable members.
    //
    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _adapter;
    const TraceLevelsPtr _traceLevels;
    const KeepAliveThreadPtr _keepAlive;
    std::string _name; // The topic name
    SubscriberFactoryPtr _factory;

    Ice::ObjectPtr _publisher; // Publisher & associated proxy
    Ice::ObjectPrx _publisherPrx;

    Ice::ObjectPtr _link; // TopicLink & associated proxy
    TopicLinkPrx _linkPrx;

    TopicSubscribersPtr _subscribers; // Set of Subscribers

    Freeze::ConnectionPtr _connection;

    // The set of downstream topics.
    PersistentTopicMap _topics;
    IceStorm::LinkRecordDict _topicRecord;
    IceUtil::RecMutex _topicRecordMutex;

    // The set of upstream topics.
    PersistentUpstreamMap _upstream;
    IceStorm::TopicUpstreamLinkPrxSeq _upstreamRecord;
    IceUtil::RecMutex _upstreamRecordMutex;

    bool _destroyed; // Has this Topic been destroyed?
};
typedef IceUtil::Handle<TopicI> TopicIPtr;

} // End namespace IceStorm

#endif
