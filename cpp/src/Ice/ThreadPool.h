// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
#include <Ice/ThreadPoolF.h>
#include <Ice/InstanceF.h>
#include <Ice/LoggerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/EventHandlerF.h>
#include <Ice/Selector.h>
#include <list>

namespace IceInternal
{

class BasicStream;

class ThreadPool : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ThreadPool(const InstancePtr&, const std::string&, int);
    virtual ~ThreadPool();

    void destroy();

    void incFdsInUse();
    void decFdsInUse();

    void _register(const EventHandlerPtr&);
    void unregister(const EventHandlerPtr&);
    void finish(const EventHandlerPtr&);
    void execute(const ThreadPoolWorkItemPtr&);

    void promoteFollower(EventHandler* = 0);
    void joinWithAllThreads();

    std::string prefix() const;
    
private:

    bool run(); // Returns true if a follower should be promoted.
    bool read(const EventHandlerPtr&);

    InstancePtr _instance;
    bool _destroyed;
    const std::string _prefix;

    Selector<EventHandler> _selector;

    std::list<ThreadPoolWorkItemPtr> _workItems;
    std::list<EventHandlerPtr> _finished;

    class EventHandlerThread : public IceUtil::Thread
    {
    public:
        
        EventHandlerThread(const ThreadPoolPtr&);
        virtual void run();

    private:

        ThreadPoolPtr _pool;
    };
    friend class EventHandlerThread;

    const int _size; // Number of threads that are pre-created.
    const int _sizeMax; // Maximum number of threads.
    const int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
    const bool _serialize; // True if requests need to be serialized over the connection.
 
    const size_t _stackSize;

    std::vector<IceUtil::ThreadPtr> _threads; // All threads, running or not.
    int _running; // Number of running threads.
    int _inUse; // Number of threads that are currently in use.
    double _load; // Current load in number of threads.

    bool _promote;

    const bool _warnUdp;
};

}

#endif
