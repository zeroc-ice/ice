// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TOPIC_MANAGER_I_H
#define TOPIC_MANAGER_I_H

#include <IceStorm/IceStorm.h>
#include <IceStorm/StringBoolDict.h>

namespace IceStorm
{

//
// Forward declarations.
//
class TopicI;
typedef IceUtil::Handle<TopicI> TopicIPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Flusher;
typedef IceUtil::Handle<Flusher> FlusherPtr;
 
class SubscriberFactory;
typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

//
// Map of TopicImplementation objects.
//
typedef std::map<std::string, TopicIPtr> TopicIMap;

//
// TopicManager implementation.
//
class TopicManagerI : public TopicManager, public IceUtil::Mutex
{
public:

    TopicManagerI(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const TraceLevelsPtr&,
		  const Freeze::DBEnvironmentPtr&, const Freeze::DBPtr&);
    ~TopicManagerI();

    virtual TopicPrx create(const std::string&, const Ice::Current&);
    virtual TopicPrx retrieve(const std::string&, const Ice::Current&) const;
    virtual TopicDict retrieveAll(const Ice::Current&) const;
    virtual void subscribe(const QoS&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual void unsubscribe(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    void reap();

private:

    void installTopic(const std::string&, const std::string&, bool);

    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    TopicIMap _topicIMap;
    FlusherPtr _flusher;
    SubscriberFactoryPtr _factory;
    Freeze::DBEnvironmentPtr _dbEnv;
    StringBoolDict _topics;
};

typedef IceUtil::Handle<TopicManagerI> TopicManagerIPtr;

} // End namespace IceStorm

#endif
