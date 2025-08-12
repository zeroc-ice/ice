// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_INSTANCE_H
#define ICESTORM_INSTANCE_H

#include "Election.h"
#include "Ice/CommunicatorF.h"
#include "Ice/ObjectAdapterF.h"
#include "Ice/PropertiesF.h"
#include "Instrumentation.h"
#include "Util.h"

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

        Instance(
            std::string,
            Ice::CommunicatorPtr,
            Ice::ObjectAdapterPtr,
            Ice::ObjectAdapterPtr,
            Ice::ObjectAdapterPtr = nullptr,
            std::optional<IceStormElection::NodePrx> = std::nullopt);

        virtual ~Instance();

        void setNode(std::shared_ptr<IceStormElection::NodeI>);

        [[nodiscard]] std::string instanceName() const;
        [[nodiscard]] Ice::CommunicatorPtr communicator() const;
        [[nodiscard]] Ice::PropertiesPtr properties() const;
        [[nodiscard]] Ice::ObjectAdapterPtr publishAdapter() const;
        [[nodiscard]] Ice::ObjectAdapterPtr topicAdapter() const;
        [[nodiscard]] Ice::ObjectAdapterPtr nodeAdapter() const;
        [[nodiscard]] std::shared_ptr<IceStormElection::Observers> observers() const;
        [[nodiscard]] std::shared_ptr<IceStormElection::NodeI> node() const;
        [[nodiscard]] std::optional<IceStormElection::NodePrx> nodeProxy() const;
        [[nodiscard]] std::shared_ptr<TraceLevels> traceLevels() const;
        [[nodiscard]] IceInternal::TimerPtr timer() const;
        [[nodiscard]] std::optional<Ice::ObjectPrx> topicReplicaProxy() const;
        [[nodiscard]] std::optional<Ice::ObjectPrx> publisherReplicaProxy() const;
        [[nodiscard]] std::shared_ptr<IceStorm::Instrumentation::TopicManagerObserver> observer() const;
        [[nodiscard]] std::shared_ptr<TopicReaper> topicReaper() const;

        [[nodiscard]] std::chrono::seconds discardInterval() const;
        [[nodiscard]] std::chrono::milliseconds flushInterval() const;
        [[nodiscard]] std::chrono::milliseconds sendTimeout() const;
        [[nodiscard]] int sendQueueSizeMax() const;
        [[nodiscard]] SendQueueSizeMaxPolicy sendQueueSizeMaxPolicy() const;

        void shutdown() noexcept;
        virtual void destroy() noexcept;

    private:
        const std::string _instanceName;
        const Ice::CommunicatorPtr _communicator;
        const Ice::ObjectAdapterPtr _publishAdapter;
        const Ice::ObjectAdapterPtr _topicAdapter;
        const Ice::ObjectAdapterPtr _nodeAdapter;
        const std::optional<IceStormElection::NodePrx> _nodeProxy;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::chrono::seconds _discardInterval;
        const std::chrono::milliseconds _flushInterval;
        const std::chrono::milliseconds _sendTimeout;
        const int _sendQueueSizeMax;
        const SendQueueSizeMaxPolicy _sendQueueSizeMaxPolicy{RemoveSubscriber};
        const std::optional<Ice::ObjectPrx> _topicReplicaProxy;
        const std::optional<Ice::ObjectPrx> _publisherReplicaProxy;
        const std::shared_ptr<TopicReaper> _topicReaper;
        std::shared_ptr<IceStormElection::NodeI> _node;
        std::shared_ptr<IceStormElection::Observers> _observers;
        IceInternal::TimerPtr _timer;
        std::shared_ptr<IceStorm::Instrumentation::TopicManagerObserver> _observer;
    };

    using SubscriberMapRWCursor =
        IceDB::ReadWriteCursor<SubscriberRecordKey, SubscriberRecord, IceDB::IceContext, Ice::OutputStream>;

    class PersistentInstance final : public Instance
    {
    public:
        PersistentInstance(
            const std::string&,
            Ice::CommunicatorPtr,
            Ice::ObjectAdapterPtr,
            Ice::ObjectAdapterPtr,
            Ice::ObjectAdapterPtr = nullptr,
            std::optional<IceStormElection::NodePrx> = std::nullopt);

        [[nodiscard]] const IceDB::Env& dbEnv() const { return _dbEnv; }
        [[nodiscard]] LLUMap lluMap() const { return _lluMap; }
        [[nodiscard]] SubscriberMap subscriberMap() const { return _subscriberMap; }

        void destroy() noexcept override;

    private:
        IceInternal::FileLock _dbLock;
        IceDB::Env _dbEnv;
        LLUMap _lluMap;
        SubscriberMap _subscriberMap;
    };

} // End namespace IceStorm

#endif
