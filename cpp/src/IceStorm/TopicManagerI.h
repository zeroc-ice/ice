// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    virtual TopicPrx retrieve(const std::string&, const Ice::Current&);
    virtual TopicDict retrieveAll(const Ice::Current&);
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
