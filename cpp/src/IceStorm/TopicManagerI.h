// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

//
// Map of TopicI objects.
//
class TopicI;
typedef IceUtil::Handle<TopicI> TopicIPtr;

typedef std::map<std::string, TopicIPtr> TopicIMap;

//
// TopicManager implementation.
//
class TopicManagerI : public TopicManager, public IceUtil::Mutex
{
public:

    TopicManagerI(const InstancePtr&,
                  const Ice::ObjectAdapterPtr&,
                  const std::string&,
                  const std::string&);
    ~TopicManagerI();

    virtual TopicPrx create(const std::string&, const Ice::Current&);
    virtual TopicPrx retrieve(const std::string&, const Ice::Current&) const;
    virtual TopicDict retrieveAll(const Ice::Current&) const;

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    void reap();

    void shutdown();

private:

    TopicPrx installTopic(const std::string&, const Ice::Identity&, const LinkRecordSeq&, bool);
  
    const InstancePtr _instance;
    const Ice::ObjectAdapterPtr _topicAdapter;
    const std::string _envName;
    const std::string _dbName;
    const Freeze::ConnectionPtr _connection;
    PersistentTopicMap _topics;

    TopicIMap _topicIMap;
};

typedef IceUtil::Handle<TopicManagerI> TopicManagerIPtr;

} // End namespace IceStorm

#endif
