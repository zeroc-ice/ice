//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INSTANCE_H
#define INSTANCE_H

#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>
#include <IceStorm/Election.h>
#include <IceStorm/Instrumentation.h>
#include <IceStorm/Util.h>

namespace IceStormElection
{

class Observers;
class NodeI;

}

namespace IceStorm
{

class TraceLevels;

class TopicReaper
{
public:

    void add(const std::string&);
    std::vector<std::string> consumeReapedTopics();

private:

    std::vector<std::string> _topics;

    std::mutex _mutex;
};

class Instance
{
public:

    enum SendQueueSizeMaxPolicy
    {
        RemoveSubscriber,
        DropEvents
    };

    Instance(const std::string&, const std::string&, std::shared_ptr<Ice::Communicator>,
             std::shared_ptr<Ice::ObjectAdapter>, std::shared_ptr<Ice::ObjectAdapter>,
             std::shared_ptr<Ice::ObjectAdapter> = nullptr,
             std::shared_ptr<IceStormElection::NodePrx> = nullptr);

    virtual ~Instance();

    void setNode(std::shared_ptr<IceStormElection::NodeI>);

    std::string instanceName() const;
    std::string serviceName() const;
    std::shared_ptr<Ice::Communicator> communicator() const;
    std::shared_ptr<Ice::Properties> properties() const;
    std::shared_ptr<Ice::ObjectAdapter> publishAdapter() const;
    std::shared_ptr<Ice::ObjectAdapter> topicAdapter() const;
    std::shared_ptr<Ice::ObjectAdapter> nodeAdapter() const;
    std::shared_ptr<IceStormElection::Observers> observers() const;
    std::shared_ptr<IceStormElection::NodeI> node() const;
    std::shared_ptr<IceStormElection::NodePrx> nodeProxy() const;
    std::shared_ptr<TraceLevels> traceLevels() const;
    IceUtil::TimerPtr timer() const;
    std::shared_ptr<Ice::ObjectPrx> topicReplicaProxy() const;
    std::shared_ptr<Ice::ObjectPrx> publisherReplicaProxy() const;
    std::shared_ptr<IceStorm::Instrumentation::TopicManagerObserver> observer() const;
    std::shared_ptr<TopicReaper> topicReaper() const;

    std::chrono::seconds discardInterval() const;
    std::chrono::milliseconds flushInterval() const;
    std::chrono::milliseconds sendTimeout() const;
    int sendQueueSizeMax() const;
    SendQueueSizeMaxPolicy sendQueueSizeMaxPolicy() const;

    void shutdown();
    virtual void destroy();

private:

    const std::string _instanceName;
    const std::string _serviceName;
    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::shared_ptr<Ice::ObjectAdapter> _publishAdapter;
    const std::shared_ptr<Ice::ObjectAdapter> _topicAdapter;
    const std::shared_ptr<Ice::ObjectAdapter> _nodeAdapter;
    const std::shared_ptr<IceStormElection::NodePrx> _nodeProxy;
    const std::shared_ptr<TraceLevels> _traceLevels;
    const std::chrono::seconds _discardInterval;
    const std::chrono::milliseconds _flushInterval;
    const std::chrono::milliseconds _sendTimeout;
    const int _sendQueueSizeMax;
    const SendQueueSizeMaxPolicy _sendQueueSizeMaxPolicy;
    const std::shared_ptr<Ice::ObjectPrx> _topicReplicaProxy;
    const std::shared_ptr<Ice::ObjectPrx> _publisherReplicaProxy;
    const std::shared_ptr<TopicReaper> _topicReaper;
    std::shared_ptr<IceStormElection::NodeI> _node;
    std::shared_ptr<IceStormElection::Observers> _observers;
    IceUtil::TimerPtr _timer;
    std::shared_ptr<IceStorm::Instrumentation::TopicManagerObserver> _observer;
};

using SubscriberMapRWCursor = IceDB::ReadWriteCursor<SubscriberRecordKey,
                                                     SubscriberRecord,
                                                     IceDB::IceContext,
                                                     Ice::OutputStream>;

class PersistentInstance final : public Instance
{
public:

    PersistentInstance(const std::string&, const std::string&, std::shared_ptr<Ice::Communicator>,
                       std::shared_ptr<Ice::ObjectAdapter>, std::shared_ptr<Ice::ObjectAdapter>,
                       std::shared_ptr<Ice::ObjectAdapter> = nullptr,
                       std::shared_ptr<IceStormElection::NodePrx> = nullptr);

    const IceDB::Env& dbEnv() const { return _dbEnv; }
    LLUMap lluMap() const { return _lluMap; }
    SubscriberMap subscriberMap() const { return _subscriberMap; }

    void destroy() override;

private:

    IceUtilInternal::FileLock _dbLock;
    IceDB::Env _dbEnv;
    LLUMap _lluMap;
    SubscriberMap _subscriberMap;
};

} // End namespace IceStorm

#endif
