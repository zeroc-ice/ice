// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTANCE_H
#define INSTANCE_H

#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>
#include <IceUtil/Time.h>
#include <IceStorm/Election.h>
#include <IceStorm/Instrumentation.h>
#include <IceStorm/Util.h>

namespace IceUtil
{

class Timer;
typedef IceUtil::Handle<Timer> TimerPtr;

}

namespace IceStormElection
{

class Observers;
typedef IceUtil::Handle<Observers> ObserversPtr;

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

}

namespace IceStorm
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class TopicReaper : public IceUtil::Shared, private IceUtil::Mutex
{
public:

    void add(const std::string&);
    std::vector<std::string> consumeReapedTopics();

private:

    std::vector<std::string> _topics;
};
typedef IceUtil::Handle<TopicReaper> TopicReaperPtr;

class Instance : public IceUtil::Shared
{
public:

    enum SendQueueSizeMaxPolicy
    {
        RemoveSubscriber,
        DropEvents
    };

    Instance(const std::string&, const std::string&, const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&,
             const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr& = 0, const IceStormElection::NodePrx& = 0);

    void setNode(const IceStormElection::NodeIPtr&);

    std::string instanceName() const;
    std::string serviceName() const;
    Ice::CommunicatorPtr communicator() const;
    Ice::PropertiesPtr properties() const;
    Ice::ObjectAdapterPtr publishAdapter() const;
    Ice::ObjectAdapterPtr topicAdapter() const;
    Ice::ObjectAdapterPtr nodeAdapter() const;
    IceStormElection::ObserversPtr observers() const;
    IceStormElection::NodeIPtr node() const;
    IceStormElection::NodePrx nodeProxy() const;
    TraceLevelsPtr traceLevels() const;
    IceUtil::TimerPtr batchFlusher() const;
    IceUtil::TimerPtr timer() const;
    Ice::ObjectPrx topicReplicaProxy() const;
    Ice::ObjectPrx publisherReplicaProxy() const;
    IceStorm::Instrumentation::TopicManagerObserverPtr observer() const;
    TopicReaperPtr topicReaper() const;

    IceUtil::Time discardInterval() const;
    IceUtil::Time flushInterval() const;
    int sendTimeout() const;
    int sendQueueSizeMax() const;
    SendQueueSizeMaxPolicy sendQueueSizeMaxPolicy() const;

    void shutdown();
    virtual void destroy();

private:

    const std::string _instanceName;
    const std::string _serviceName;
    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _publishAdapter;
    const Ice::ObjectAdapterPtr _topicAdapter;
    const Ice::ObjectAdapterPtr _nodeAdapter;
    const IceStormElection::NodePrx _nodeProxy;
    const TraceLevelsPtr _traceLevels;
    const IceUtil::Time _discardInterval;
    const IceUtil::Time _flushInterval;
    const int _sendTimeout;
    const int _sendQueueSizeMax;
    const SendQueueSizeMaxPolicy _sendQueueSizeMaxPolicy;
    const Ice::ObjectPrx _topicReplicaProxy;
    const Ice::ObjectPrx _publisherReplicaProxy;
    const TopicReaperPtr _topicReaper;
    IceStormElection::NodeIPtr _node;
    IceStormElection::ObserversPtr _observers;
    IceUtil::TimerPtr _batchFlusher;
    IceUtil::TimerPtr _timer;
    IceStorm::Instrumentation::TopicManagerObserverPtr _observer;

};
typedef IceUtil::Handle<Instance> InstancePtr;

typedef IceDB::ReadWriteCursor<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStream>
        SubscriberMapRWCursor;

class PersistentInstance : public Instance
{
public:

    PersistentInstance(const std::string&, const std::string&, const Ice::CommunicatorPtr&,
                       const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr& = 0,
                       const IceStormElection::NodePrx& = 0);

    const IceDB::Env& dbEnv() const { return _dbEnv; }
    LLUMap lluMap() const { return _lluMap; }
    SubscriberMap subscriberMap() const { return _subscriberMap; }

    virtual void destroy();

private:

    IceUtilInternal::FileLock _dbLock;
    IceDB::Env _dbEnv;
    LLUMap _lluMap;
    SubscriberMap _subscriberMap;
};
typedef IceUtil::Handle<PersistentInstance> PersistentInstancePtr;

} // End namespace IceStorm

#endif
