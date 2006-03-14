// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TOPIC_MANAGER_I_H
#define TOPIC_MANAGER_I_H

#include <IceStorm/IceStorm.h>
#include <IceStorm/PersistentTopicMap.h>

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

    TopicManagerI(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&,
                  const TraceLevelsPtr&, const std::string&, const std::string&);
    ~TopicManagerI();

    virtual TopicPrx create(const std::string&, const Ice::Current&);
    virtual TopicPrx retrieve(const std::string&, const Ice::Current&) const;
    virtual TopicDict retrieveAll(const Ice::Current&) const;

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    void reap();

    void shutdown();

private:

    void installTopic(const std::string&, const LinkRecordDict&, bool);
  
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _topicAdapter;
    Ice::ObjectAdapterPtr _publishAdapter;
    TraceLevelsPtr _traceLevels;
    TopicIMap _topicIMap;
    FlusherPtr _flusher;
    SubscriberFactoryPtr _factory;
    std::string _envName;
    std::string _dbName;
    Freeze::ConnectionPtr _connection;
    PersistentTopicMap _topics;
};

typedef IceUtil::Handle<TopicManagerI> TopicManagerIPtr;

} // End namespace IceStorm

#endif
