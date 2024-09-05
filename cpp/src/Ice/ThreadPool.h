//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_THREAD_POOL_H
#define ICE_THREAD_POOL_H

#include "EventHandler.h"
#include "Ice/Config.h"
#include "Ice/InputStream.h"
#include "Ice/InstanceF.h"
#include "Ice/Logger.h"
#include "Ice/ObserverHelper.h"
#include "Ice/PropertiesF.h"
#include "Selector.h"
#include "ThreadPoolF.h"

#include <list>
#include <set>
#include <thread>

namespace IceInternal
{
    class ThreadPoolCurrent;

    class ThreadPoolWorkQueue;
    using ThreadPoolWorkQueuePtr = std::shared_ptr<ThreadPoolWorkQueue>;

    class ThreadPool : public std::enable_shared_from_this<ThreadPool>
    {
    public:
        class EventHandlerThread final : public std::enable_shared_from_this<EventHandlerThread>
        {
        public:
            EventHandlerThread(const ThreadPoolPtr&, const std::string&);
            void start();
            void run();
            void join();

            void updateObserver();
            void setState(Ice::Instrumentation::ThreadState);

        private:
            std::string _name;
            ThreadPoolPtr _pool;
            ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
            Ice::Instrumentation::ThreadState _state;
            std::thread _thread;
        };
        using EventHandlerThreadPtr = std::shared_ptr<EventHandlerThread>;

        static ThreadPoolPtr create(const InstancePtr&, const std::string&, int);

        virtual ~ThreadPool();

        void destroy();

        void updateObservers();

        void initialize(const EventHandlerPtr&);
        void _register(const EventHandlerPtr& handler, SocketOperation status)
        {
            update(handler, SocketOperationNone, status);
        }
        void update(const EventHandlerPtr&, SocketOperation, SocketOperation);
        void unregister(const EventHandlerPtr& handler, SocketOperation status)
        {
            update(handler, status, SocketOperationNone);
        }
        bool finish(const EventHandlerPtr&, bool);
        void ready(const EventHandlerPtr&, SocketOperation, bool);

        void executeFromThisThread(std::function<void()>, const Ice::ConnectionPtr&);
        void execute(std::function<void()>, const Ice::ConnectionPtr&);

        void joinWithAllThreads();

        std::string prefix() const;

    private:
        ThreadPool(const InstancePtr&, const std::string&, int);
        void initialize();

        void run(const EventHandlerThreadPtr&);

        void ioCompleted(ThreadPoolCurrent&);

#if defined(ICE_USE_IOCP)
        bool startMessage(ThreadPoolCurrent&);
        void finishMessage(ThreadPoolCurrent&);
#else
        void promoteFollower(ThreadPoolCurrent&);
        bool followerWait(ThreadPoolCurrent&, std::unique_lock<std::mutex>&);
#endif

        std::string nextThreadId();

        static void joinThread(const EventHandlerThreadPtr&);
        static void shutdown(const ThreadPoolCurrent&, const InstancePtr&);

        const InstancePtr _instance;

        std::function<void(std::function<void()>, const Ice::ConnectionPtr&)> _executor;

        ThreadPoolWorkQueuePtr _workQueue;
        bool _destroyed;
        const std::string _prefix;
        Selector _selector;
        int _nextThreadId;

        friend class EventHandlerThread;
        friend class ThreadPoolCurrent;
        friend class ThreadPoolWorkQueue;

        const int _size;     // Number of threads that are pre-created.
        const int _sizeIO;   // Maximum number of threads that can concurrently perform IO.
        const int _sizeMax;  // Maximum number of threads.
        const int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
        const bool _hasPriority;
        const int _priority;
        const int _serverIdleTime;
        const int _threadIdleTime;
        const size_t _stackSize;

        std::set<EventHandlerThreadPtr> _threads; // All threads, running or not.
        int _inUse;                               // Number of threads that are currently in use.
#if !defined(ICE_USE_IOCP)
        int _inUseIO; // Number of threads that are currently performing IO.
        std::vector<std::pair<EventHandler*, SocketOperation>> _handlers;
        std::vector<std::pair<EventHandler*, SocketOperation>>::const_iterator _nextHandler;
#endif

        bool _promote;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };

    class ThreadPoolCurrent
    {
    public:
        ThreadPoolCurrent(const ThreadPoolPtr&, const ThreadPool::EventHandlerThreadPtr&);

        SocketOperation operation;

        void ioCompleted() const { _threadPool->ioCompleted(const_cast<ThreadPoolCurrent&>(*this)); }

#if defined(ICE_USE_IOCP)
        bool startMessage() { return _threadPool->startMessage(const_cast<ThreadPoolCurrent&>(*this)); }

        void finishMessage() { _threadPool->finishMessage(const_cast<ThreadPoolCurrent&>(*this)); }
#else
        bool ioReady() { return (_handler->_registered & operation) != 0; }
#endif

    private:
        ThreadPool* _threadPool;
        ThreadPool::EventHandlerThreadPtr _thread;
        EventHandlerPtr _handler;
        bool _ioCompleted;
#if !defined(ICE_USE_IOCP)
        bool _leader;
#else
        DWORD _count;
        int _error;
#endif
        friend class ThreadPool;
    };

    class ThreadPoolWorkQueue : public EventHandler
    {
    public:
        ThreadPoolWorkQueue(ThreadPool&);

        void destroy();
        void queue(std::function<void(ThreadPoolCurrent&)>);

#if defined(ICE_USE_IOCP)
        bool startAsync(SocketOperation);
        bool finishAsync(SocketOperation);
#endif

        virtual void message(ThreadPoolCurrent&);
        virtual void finished(ThreadPoolCurrent&, bool);
        virtual std::string toString() const;
        virtual NativeInfoPtr getNativeInfo();

    private:
        ThreadPool& _threadPool;
        bool _destroyed;
        std::list<std::function<void(ThreadPoolCurrent&)>> _workItems;
    };

    //
    // The ThreadPoolMessage class below hides the IOCP implementation details from
    // the event handler implementations. Only event handler implementation that
    // require IO need to use this class.
    //
    class ThreadPoolMessage
    {
    public:
        ThreadPoolMessage(ThreadPoolCurrent& current)
            :
#if defined(ICE_USE_IOCP)
              _current(current),
              _ioReady(_current.startMessage())
#else
              _current(current)
#endif
        {
        }

        ~ThreadPoolMessage()
        {
#if defined(ICE_USE_IOCP)
            if (_ioReady)
            {
                _current.finishMessage();
            }
#endif
        }

        void ioCompleted() { _current.ioCompleted(); }

        bool ioReady()
        {
#if defined(ICE_USE_IOCP)
            return _ioReady;
#else
            return _current.ioReady();
#endif
        }

    private:
        ThreadPoolCurrent& _current;
#if defined(ICE_USE_IOCP)
        const bool _ioReady;
#endif
    };
}

#endif
