// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

#include <Ice/ThreadPoolF.h>
#include <Ice/InstanceF.h>
#include <Ice/LoggerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/EventHandlerF.h>
#include <list>

#if defined(__linux) && !defined(ICE_NO_EPOLL)
#   define ICE_USE_EPOLL 1
#endif

#if defined(_WIN32)
#   include <winsock2.h>
#else
#   define SOCKET int
#   if defined(ICE_USE_EPOLL)
#       include <sys/epoll.h>
#   elif defined(__APPLE__)
#       include <sys/event.h>
#   else
#       include <sys/poll.h>
#   endif
#endif


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

    void _register(SOCKET, const EventHandlerPtr&);
    void unregister(SOCKET);
    void promoteFollower();
    void joinWithAllThreads();

    std::string prefix() const;
    
private:

    void clearInterrupt();
    void setInterrupt();

    bool run(); // Returns true if a follower should be promoted.
    void read(const EventHandlerPtr&);

    InstancePtr _instance;
    bool _destroyed;
    const std::string _prefix;

    SOCKET _maxFd;
    SOCKET _minFd;
    SOCKET _lastFd;
    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
#if defined(_WIN32)
    fd_set _fdSet;
    int _fdsInUse;
#elif defined(ICE_USE_EPOLL)
    int _epollFd;
    std::vector<struct epoll_event> _events;
#elif defined(__APPLE__)
    int _kqueueFd;
    std::vector<struct kevent> _events;
#else
    std::vector<struct pollfd> _pollFdSet;
#endif

    std::list<std::pair<SOCKET, EventHandlerPtr> > _changes; // Event handler set for addition; null for removal.

    std::map<SOCKET, EventHandlerPtr> _handlerMap;

    int _timeout;

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
