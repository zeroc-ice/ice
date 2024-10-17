//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_INSTANCE_H
#define DATASTORM_INSTANCE_H

#include "DataStorm/Config.h"
#include "DataStorm/Contract.h"
#include "Ice/Ice.h"

#include <cmath>
#include <mutex>

namespace DataStormI
{
    class TopicFactoryI;
    class ConnectionManager;
    class NodeSessionManager;
    class TraceLevels;
    class ForwarderManager;
    class NodeI;
    class CallbackExecutor;

    class Instance : public std::enable_shared_from_this<Instance>
    {
    public:
        Instance(const Ice::CommunicatorPtr&);

        void init();

        std::shared_ptr<ConnectionManager> getConnectionManager() const
        {
            assert(_connectionManager);
            return _connectionManager;
        }

        std::shared_ptr<NodeSessionManager> getNodeSessionManager() const
        {
            assert(_nodeSessionManager);
            return _nodeSessionManager;
        }

        Ice::CommunicatorPtr getCommunicator() const
        {
            assert(_communicator);
            return _communicator;
        }

        Ice::ObjectAdapterPtr getObjectAdapter() const
        {
            assert(_adapter);
            return _adapter;
        }

        std::shared_ptr<ForwarderManager> getCollocatedForwarder() const
        {
            assert(_collocatedForwarder);
            return _collocatedForwarder;
        }

        std::optional<DataStormContract::LookupPrx> getLookup() const { return _lookup; }

        std::shared_ptr<TopicFactoryI> getTopicFactory() const
        {
            assert(_topicFactory);
            return _topicFactory;
        }

        std::shared_ptr<TraceLevels> getTraceLevels() const
        {
            assert(_traceLevels);
            return _traceLevels;
        }

        std::shared_ptr<NodeI> getNode() const
        {
            assert(_node);
            return _node;
        }

        std::shared_ptr<CallbackExecutor> getCallbackExecutor() const
        {
            assert(_executor);
            return _executor;
        }

        Ice::TimerPtr getTimer() const
        {
            assert(_timer);
            return _timer;
        }

        std::chrono::milliseconds getRetryDelay(int count) const
        {
            return _retryDelay * static_cast<int>(std::pow(_retryMultiplier, std::min(count, _retryCount)));
        }

        int getRetryCount() const { return _retryCount; }

        void shutdown();
        bool isShutdown() const;
        void checkShutdown() const;
        void waitForShutdown() const;

        void destroy(bool);

    private:
        std::shared_ptr<TopicFactoryI> _topicFactory;
        std::shared_ptr<ConnectionManager> _connectionManager;
        std::shared_ptr<NodeSessionManager> _nodeSessionManager;
        std::shared_ptr<ForwarderManager> _collocatedForwarder;
        std::shared_ptr<NodeI> _node;
        Ice::CommunicatorPtr _communicator;
        Ice::ObjectAdapterPtr _adapter;
        Ice::ObjectAdapterPtr _collocatedAdapter;
        Ice::ObjectAdapterPtr _multicastAdapter;
        std::optional<DataStormContract::LookupPrx> _lookup;
        std::shared_ptr<TraceLevels> _traceLevels;
        std::shared_ptr<CallbackExecutor> _executor;
        Ice::TimerPtr _timer;
        std::chrono::milliseconds _retryDelay;
        int _retryMultiplier;
        int _retryCount;

        mutable std::mutex _mutex;
        mutable std::condition_variable _cond;
        bool _shutdown;
    };
}
#endif
