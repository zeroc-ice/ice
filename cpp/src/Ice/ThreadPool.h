// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_THREAD_POOL_H
#define ICE_THREAD_POOL_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h>

#include <Ice/Config.h>
#include <Ice/Dispatcher.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/InstanceF.h>
#include <Ice/LoggerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/EventHandler.h>
#include <Ice/Selector.h>
#include <Ice/BasicStream.h>
#include <Ice/ObserverHelper.h>

#include <set>
#include <list>

namespace IceInternal
{

class ThreadPoolCurrent;

class ThreadPoolWorkQueue;
typedef IceUtil::Handle<ThreadPoolWorkQueue> ThreadPoolWorkQueuePtr;

class ThreadPoolWorkItem : virtual public IceUtil::Shared
{
public:
    
    virtual void execute(ThreadPoolCurrent&) = 0;
};
typedef IceUtil::Handle<ThreadPoolWorkItem> ThreadPoolWorkItemPtr;

class DispatchWorkItem : public ThreadPoolWorkItem, public Ice::DispatcherCall
{
public:

    DispatchWorkItem();
    DispatchWorkItem(const Ice::ConnectionPtr& connection);
 
    const Ice::ConnectionPtr& 
    getConnection()
    {
        return _connection;
    }

private:

    virtual void execute(ThreadPoolCurrent&);

    const Ice::ConnectionPtr _connection;
};
typedef IceUtil::Handle<DispatchWorkItem> DispatchWorkItemPtr;

class ThreadPool : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
    class EventHandlerThread : public IceUtil::Thread
    {
    public:
        
        EventHandlerThread(const ThreadPoolPtr&, const std::string&);
        virtual void run();

        void updateObserver();
        void setState(Ice::Instrumentation::ThreadState);
        
    private:

        ThreadPoolPtr _pool;
        ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
        Ice::Instrumentation::ThreadState _state;
    };
    typedef IceUtil::Handle<EventHandlerThread> EventHandlerThreadPtr;

public:

    ThreadPool(const InstancePtr&, const std::string&, int);
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

    void dispatchFromThisThread(const DispatchWorkItemPtr&);
    void dispatch(const DispatchWorkItemPtr&);

    void joinWithAllThreads();

    std::string prefix() const;

private:

    void run(const EventHandlerThreadPtr&);

    bool ioCompleted(ThreadPoolCurrent&);

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    bool startMessage(ThreadPoolCurrent&);
    void finishMessage(ThreadPoolCurrent&);
#else
    void promoteFollower(ThreadPoolCurrent&);
    bool followerWait(ThreadPoolCurrent&);
#endif

    std::string nextThreadId();

    const InstancePtr _instance;
    const Ice::DispatcherPtr _dispatcher;
    ThreadPoolWorkQueuePtr _workQueue;
    bool _destroyed;
    const std::string _prefix;
    Selector _selector;
    int _nextThreadId;

    friend class EventHandlerThread;
    friend class ThreadPoolCurrent;

    const int _size; // Number of threads that are pre-created.
    const int _sizeIO; // Maximum number of threads that can concurrently perform IO.
    const int _sizeMax; // Maximum number of threads.
    const int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
    const bool _serialize; // True if requests need to be serialized over the connection.
    const bool _hasPriority;
    const int _priority;
    const int _serverIdleTime;
    const int _threadIdleTime;
    const size_t _stackSize;

    std::set<EventHandlerThreadPtr> _threads; // All threads, running or not.
    int _inUse; // Number of threads that are currently in use.
#if !defined(ICE_USE_IOCP) && !defined(ICE_OS_WINRT)
    int _inUseIO; // Number of threads that are currently performing IO.
    std::vector<std::pair<EventHandler*, SocketOperation> > _handlers;
    std::vector<std::pair<EventHandler*, SocketOperation> >::const_iterator _nextHandler;
    std::set<EventHandler*> _pendingHandlers;
#endif

    bool _promote;
};

class ThreadPoolCurrent
{
public:

    ThreadPoolCurrent(const InstancePtr&, const ThreadPoolPtr&, const ThreadPool::EventHandlerThreadPtr&);

    SocketOperation operation;
    BasicStream stream; // A per-thread stream to be used by event handlers for optimization.

    bool ioCompleted() const
    {
        return _threadPool->ioCompleted(const_cast<ThreadPoolCurrent&>(*this));
    }

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    bool startMessage()
    {
        return _threadPool->startMessage(const_cast<ThreadPoolCurrent&>(*this));
    }

    void finishMessage()
    {
        _threadPool->finishMessage(const_cast<ThreadPoolCurrent&>(*this));
    }
#else
    bool ioReady()
    {
        return (_handler->_registered & operation) != 0;
    }
#endif

    void dispatchFromThisThread(const DispatchWorkItemPtr& workItem)
    {
        _threadPool->dispatchFromThisThread(workItem);
    }

private:

    ThreadPool* _threadPool;
    ThreadPool::EventHandlerThreadPtr _thread;
    EventHandlerPtr _handler;
    bool _ioCompleted;
#if !defined(ICE_USE_IOCP) && !defined(ICE_OS_WINRT)
    bool _leader;
#else
    DWORD _count;
    int _error;
#endif
    friend class ThreadPool;
};

class ThreadPoolWorkQueue : public EventHandler, public IceUtil::Mutex
{
public:

    ThreadPoolWorkQueue(const InstancePtr&, Selector&);
    ~ThreadPoolWorkQueue();

    void destroy();
    void queue(const ThreadPoolWorkItemPtr&);

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    bool startAsync(SocketOperation);
    bool finishAsync(SocketOperation);
#endif

    virtual void message(ThreadPoolCurrent&);
    virtual void finished(ThreadPoolCurrent&, bool);
    virtual std::string toString() const;
    virtual NativeInfoPtr getNativeInfo();
    virtual void postMessage();

private:

    const InstancePtr _instance;
    Selector& _selector;
    bool _destroyed;
#ifdef ICE_USE_IOCP
    AsyncInfo _info;
#elif !defined(ICE_OS_WINRT)
    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
#endif
    std::list<ThreadPoolWorkItemPtr> _workItems;
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
#if !defined(ICE_USE_IOCP) && !defined(ICE_OS_WINRT)
template<class T> class ThreadPoolMessage
{
public:

    class IOScope
    {
    public:

        IOScope(ThreadPoolMessage<T>& message) : _message(message)
        {
            // Nothing to do.
        }

        ~IOScope()
        {
            // Nothing to do.
        }

        operator bool()
        {
            return _message._current.ioReady(); // Ensure the handler is still interested in the operation.
        }

        void completed()
        {
            _message._current.ioCompleted();
        }

    private:
        
        ThreadPoolMessage<T>& _message;
    };
    friend class IOScope;

    ThreadPoolMessage(ThreadPoolCurrent& current, const T&) : _current(current)
    {
    }

    ~ThreadPoolMessage()
    {
        // Nothing to do.
    }

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
            if(_finish)
            {
                // This must be called with the handler locked. 
                _message._current.finishMessage();
            }
        }

        operator bool()
        {
            return _finish;
        }

        void
        completed()
        {
            //
            // Call finishMessage once IO is completed only if serialization is not enabled.
            // Otherwise, finishMessage will be called when the event handler is done with 
            // the message (it will be called from ~ThreadPoolMessage below).
            //
            assert(_finish);
            if(_message._current.ioCompleted())
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
    
    ThreadPoolMessage(ThreadPoolCurrent& current, const T& mutex) : 
        _current(current), _mutex(mutex), _finish(false)
    {
    }
            
    ~ThreadPoolMessage()
    {
        if(_finish)
        {
            //
            // A ThreadPoolMessage instance must be created outside the synchronization
            // of the event handler. We need to lock the event handler here to call 
            // finishMessage.
            //
#if defined(__MINGW32__)
            IceUtil::LockT<T> sync(_mutex);
#else
            IceUtil::LockT<typename T> sync(_mutex);
#endif
            _current.finishMessage();
        }
    }

private:
    
    ThreadPoolCurrent& _current;
    const T& _mutex;
    bool _finish;
};
#endif

};


#endif
