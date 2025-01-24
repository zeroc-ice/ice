// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_SUBSCRIBER_H
#define ICESTORM_SUBSCRIBER_H

#include "Ice/ObserverHelper.h"
#include "IceStormInternal.h"
#include "Instrumentation.h"
#include "SubscriberRecord.h"

#include <condition_variable>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace IceStorm
{
    class Instance;

    class SendQueueSizeMaxReachedException final : public Ice::LocalException
    {
    public:
        SendQueueSizeMaxReachedException(const char* file, int line)
            : Ice::LocalException(file, line, "send queue size max reached")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    class Subscriber : public std::enable_shared_from_this<Subscriber>
    {
    public:
        static std::shared_ptr<Subscriber> create(const std::shared_ptr<Instance>&, const IceStorm::SubscriberRecord&);

        [[nodiscard]] std::optional<Ice::ObjectPrx> proxy() const; // Get the per subscriber object.
        [[nodiscard]] Ice::Identity id() const;                    // Return the id of the subscriber.
        [[nodiscard]] IceStorm::SubscriberRecord record() const;   // Get the subscriber record.

        // Returns false if the subscriber should be reaped.
        bool queue(bool, EventDataSeq);
        bool reap();
        void resetIfReaped();
        [[nodiscard]] bool errored() const;

        void destroy();

        // To be called by the AMI callbacks only.
        void completed();
        void error(bool, std::exception_ptr);

        void shutdown();

        void updateObserver();

        enum SubscriberState
        {
            SubscriberStateOnline,  // Online waiting to send events.
            SubscriberStateOffline, // Offline, retrying.
            SubscriberStateError,   // Error state, awaiting reaping.
            SubscriberStateReaped   // Reaped.
        };

        virtual void flush() = 0;

    protected:
        void setState(SubscriberState);

        Subscriber(std::shared_ptr<Instance>, IceStorm::SubscriberRecord, std::optional<Ice::ObjectPrx>, int, int);

        // Immutable
        const std::shared_ptr<Instance> _instance;
        const IceStorm::SubscriberRecord _rec;             // The subscriber record.
        const int _retryCount;                             // The retryCount.
        const int _maxOutstanding;                         // The maximum number of oustanding events.
        const std::optional<Ice::ObjectPrx> _proxy;        // The per subscriber object proxy, if any.
        const std::optional<Ice::ObjectPrx> _proxyReplica; // The replicated per subscriber object proxy, if any.

        mutable std::recursive_mutex _mutex;
        std::condition_variable_any _condVar;

        bool _shutdown{false};

        SubscriberState _state{SubscriberStateOnline}; // The subscriber state.

        int _outstanding{0}; // The current number of outstanding responses.
        int _outstandingCount{
            1};               // The current number of outstanding events when batching events (only used for metrics).
        EventDataSeq _events; // The queue of events to send.

        // The next time to try sending a new event if we're offline.
        std::chrono::steady_clock::time_point _next;
        int _currentRetry{0};

        IceInternal::ObserverHelperT<IceStorm::Instrumentation::SubscriberObserver> _observer;
    };

    bool operator==(const std::shared_ptr<IceStorm::Subscriber>&, const Ice::Identity&);
    bool operator==(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
    bool operator!=(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
    bool operator<(const IceStorm::Subscriber&, const IceStorm::Subscriber&);
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#endif // SUBSCRIBER_H
