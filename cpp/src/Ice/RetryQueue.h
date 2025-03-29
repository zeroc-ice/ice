// Copyright (c) ZeroC, Inc.

#ifndef ICE_RETRY_QUEUE_H
#define ICE_RETRY_QUEUE_H

#include "Ice/InstanceF.h"
#include "RequestHandler.h" // For CancellationHandler
#include "RetryQueueF.h"
#include "Timer.h"

#include <mutex>

namespace IceInternal
{
    class OutgoingAsyncBase;
    class ProxyOutgoingAsyncBase;

    using OutgoingAsyncBasePtr = std::shared_ptr<OutgoingAsyncBase>;
    using ProxyOutgoingAsyncBasePtr = std::shared_ptr<ProxyOutgoingAsyncBase>;

    class RetryTask : public TimerTask, public CancellationHandler, public std::enable_shared_from_this<RetryTask>
    {
    public:
        RetryTask(InstancePtr, RetryQueuePtr, ProxyOutgoingAsyncBasePtr);

        void runTimerTask() override;

        void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr) override;

        void destroy();

    private:
        const InstancePtr _instance;
        const RetryQueuePtr _queue;
        const ProxyOutgoingAsyncBasePtr _outAsync;
    };
    using RetryTaskPtr = std::shared_ptr<RetryTask>;

    class RetryQueue : public std::enable_shared_from_this<RetryQueue>
    {
    public:
        RetryQueue(InstancePtr);

        void add(const ProxyOutgoingAsyncBasePtr&, int);
        void destroy();

    private:
        void remove(const RetryTaskPtr&);
        bool cancel(const RetryTaskPtr&);
        friend class RetryTask;

        InstancePtr _instance;
        std::set<RetryTaskPtr> _requests;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
}

#endif
