// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TOPIC_MANAGER_I_H
#define TOPIC_MANAGER_I_H

#include <IceStorm/IceStorm.h>

#include <IceStorm/Replica.h>
#include <IceStorm/Election.h>
#include <IceStorm/Instrumentation.h>
#include <IceStorm/Util.h>

#include <IceUtil/RecMutex.h>

namespace IceStorm
{

//
// Forward declarations.
//
class PersistentInstance;
typedef IceUtil::Handle<PersistentInstance> PersistentInstancePtr;

class TopicImpl;
typedef IceUtil::Handle<TopicImpl> TopicImplPtr;

//
// TopicManager implementation.
//
class TopicManagerImpl : public IceStormElection::Replica,
                         public IceStorm::Instrumentation::ObserverUpdater,
                         public IceUtil::RecMutex
{
public:

    TopicManagerImpl(const PersistentInstancePtr&);

    // TopicManager methods.
    TopicPrx create(const std::string&);
    TopicPrx retrieve(const std::string&) const;
    TopicDict retrieveAll() const;

    // Observer methods.
    void observerInit(const IceStormElection::LogUpdate&, const IceStormElection::TopicContentSeq&);
    void observerCreateTopic(const IceStormElection::LogUpdate&, const std::string&);
    void observerDestroyTopic(const IceStormElection::LogUpdate&, const std::string&);
    void observerAddSubscriber(const IceStormElection::LogUpdate&, const std::string&,
                               const IceStorm::SubscriberRecord&);
    void observerRemoveSubscriber(const IceStormElection::LogUpdate&, const std::string&, const Ice::IdentitySeq&);

    // Sync methods.
    void getContent(IceStormElection::LogUpdate&, IceStormElection::TopicContentSeq&);

    // Replica methods.
    virtual IceStormElection::LogUpdate getLastLogUpdate() const;
    virtual void sync(const Ice::ObjectPrx&);
    virtual void initMaster(const std::set<IceStormElection::GroupNodeInfo>&, const IceStormElection::LogUpdate&);
    virtual Ice::ObjectPrx getObserver() const;
    virtual Ice::ObjectPrx getSync() const;

    void reap();

    void shutdown();

    Ice::ObjectPtr getServant() const;

private:

    void updateTopicObservers();
    void updateSubscriberObservers();

    TopicPrx installTopic(const std::string&, const Ice::Identity&, bool,
                          const IceStorm::SubscriberRecordSeq& = IceStorm::SubscriberRecordSeq());

    const PersistentInstancePtr _instance;

    std::map<std::string, TopicImplPtr> _topics;

    Ice::ObjectPtr _managerImpl;
    Ice::ObjectPtr _observerImpl;
    Ice::ObjectPrx _observer;
    Ice::ObjectPtr _syncImpl;
    Ice::ObjectPrx _sync;

    LLUMap _lluMap;
    SubscriberMap _subscriberMap;
};
typedef IceUtil::Handle<TopicManagerImpl> TopicManagerImplPtr;

} // End namespace IceStorm

#endif
