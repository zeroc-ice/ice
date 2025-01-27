// Copyright (c) ZeroC, Inc.

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
            EventHandlerThread(ThreadPoolPtr, std::string);
            void start();
            void run();
            void join();

            void updateObserver();
            void setState(Ice::Instrumentation::ThreadState);

        private:
            std::string _name;
            ThreadPoolPtr _pool;
            ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
            Ice::Instrumentation::ThreadState _state{Ice::Instrumentation::ThreadState::ThreadStateIdle};
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

        [[nodiscard]] std::string prefix() const;

    private:
        ThreadPool(const InstancePtr&, std::string, int);
        void initialize();

        void run(const EventHandlerThreadPtr&);

        bool ioCompleted(ThreadPoolCurrent&);

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
        bool _destroyed{false};
        const std::string _prefix;
        Selector _selector;
        int _nextThreadId{0};

        friend class EventHandlerThread;
        friend class ThreadPoolCurrent;
        friend class ThreadPoolWorkQueue;

        const int _size{0};     // Number of threads that are pre-created.
        const int _sizeIO{0};   // Maximum number of threads that can concurrently perform IO.
        const int _sizeMax{0};  // Maximum number of threads.
        const int _sizeWarn{0}; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
        const bool _serialize;  // True if requests need to be serialized over the connection.
        const int _serverIdleTime;
        const int _threadIdleTime{0};

        std::set<EventHandlerThreadPtr> _threads; // All threads, running or not.
        int _inUse{0};                            // Number of threads that are currently in use.
#if !defined(ICE_USE_IOCP)
        int _inUseIO{0}; // Number of threads that are currently performing IO.
        std::vector<std::pair<EventHandler*, SocketOperation>> _handlers;
        std::vector<std::pair<EventHandler*, SocketOperation>>::const_iterator _nextHandler;
#endif

        bool _promote{true};
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };

    class ThreadPoolCurrent
    {
    public:
        ThreadPoolCurrent(const ThreadPoolPtr&, ThreadPool::EventHandlerThreadPtr);

        SocketOperation operation{SocketOperationNone};

        bool ioCompleted() const // NOLINT(modernize-use-nodiscard)
        {
            return _threadPool->ioCompleted(const_cast<ThreadPoolCurrent&>(*this));
        }

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
        bool _ioCompleted{false};
#if !defined(ICE_USE_IOCP)
        bool _leader{false};
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

        void message(ThreadPoolCurrent&) override;
        void finished(ThreadPoolCurrent&, bool) override;
        [[nodiscard]] std::string toString() const override;
        NativeInfoPtr getNativeInfo() override;

    private:
        ThreadPool& _threadPool;
        bool _destroyed{false};
        std::list<std::function<void(ThreadPoolCurrent&)>> _workItems;
    };

//
// The ThreadPoolMessage class below hides the IOCP implementation details from
// the event handler implementations. Only event handler implementation that
// require IO need to use this class.
//
// An instance of the IOScope subclass must be created within the synchronization
// of the event handler. It takes care of calling startMessage/finishMessage for
// the IOCP implementation and ensures that finishMessage isn't called multiple
// times.
//
#if !defined(ICE_USE_IOCP)
    template<class T> class ThreadPoolMessage
    {
    public:
        class IOScope
        {
        public:
            IOScope(ThreadPoolMessage<T>& message) : _message(message) {}

            operator bool()
            {
                return _message._current.ioReady(); // Ensure the handler is still interested in the operation.
            }

            void completed() { _message._current.ioCompleted(); }

        private:
            ThreadPoolMessage<T>& _message;
        };
        friend class IOScope;

        ThreadPoolMessage(ThreadPoolCurrent& current, const T&) : _current(current) {}

    private:
        ThreadPoolCurrent& _current;
    };

#else

    template<class T> class ThreadPoolMessage
    {
    public:
        class IOScope
        {
        public:
            IOScope(ThreadPoolMessage& message) : _message(message)
            {
                // This must be called with the handler locked.
                _finish = _message._current.startMessage();
            }

            ~IOScope()
            {
                if (_finish)
                {
                    // This must be called with the handler locked.
                    _message._current.finishMessage();
                }
            }

            operator bool() { return _finish; }

            void completed()
            {
                //
                // Call finishMessage once IO is completed only if serialization is not enabled.
                // Otherwise, finishMessage will be called when the event handler is done with
                // the message (it will be called from ~ThreadPoolMessage below).
                //
                assert(_finish);
                if (_message._current.ioCompleted())
                {
                    _finish = false;
                    _message._finish = true;
                }
            }

        private:
            ThreadPoolMessage& _message;
            bool _finish;
        };
        friend class IOScope;

        ThreadPoolMessage(ThreadPoolCurrent& current, const T& eventHandler)
            : _current(current),
              _eventHandler(eventHandler),
              _finish(false)
        {
        }

        ~ThreadPoolMessage()
        {
            if (_finish)
            {
                //
                // A ThreadPoolMessage instance must be created outside the synchronization
                // of the event handler. We need to lock the event handler here to call
                // finishMessage.
                //
                std::lock_guard lock(_eventHandler._mutex);
                _current.finishMessage();
            }
        }

    private:
        ThreadPoolCurrent& _current;
        const T& _eventHandler;
        bool _finish;
    };
#endif

};

#endif
