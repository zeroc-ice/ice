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

#include <IceStorm/IceStorm.h>
#include <IceStorm/TopicIF.h>
#include <IceStorm/FlusherF.h>
#include <IceStorm/TraceLevelsF.h>

namespace IceStorm
{

class TopicSubscribers;
typedef IceUtil::Handle<TopicSubscribers> TopicSubscribersPtr;

class TopicI : public Topic
{
public:

    TopicI(const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&, const Ice::LoggerPtr&, const std::string&,
	   const FlusherPtr&);
    ~TopicI();

    virtual std::string getName();
    virtual Ice::ObjectPrx getPublisher();
    virtual void destroy();

    // Internal methods
    bool destroyed() const;
    void subscribe(const Ice::ObjectPrx&, const std::string&, const QoS&);
    void unsubscribe(const std::string&);

private:

    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    Ice::LoggerPtr _logger;

    // Immutable
    std::string _name;

    FlusherPtr _flusher;

    JTCMutex _destroyedMutex;
    bool _destroyed;

    TopicSubscribersPtr _subscribers;

    // Immutable
    Ice::ObjectPtr _publisher;
    Ice::ObjectPrx _obj;
    //Ice::ServantLocatorPtr _locator;
};

} // End namespace IceStorm

#endif
