// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

#ifndef _WIN32
#   define SOCKET int
#endif

namespace IceInternal
{

class BasicStream;

class ThreadPool : public ::IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void _register(SOCKET, const EventHandlerPtr&);
    void unregister(SOCKET);
    void promoteFollower();
    void initiateShutdown(); // Signal-safe shutdown initiation.
    void waitUntilFinished();
    void joinWithAllThreads();
    
private:

    ThreadPool(const InstancePtr&, bool);
    virtual ~ThreadPool();
    void destroy();
    friend class Instance;

    bool clearInterrupt();
    void setInterrupt(char);

    void run();
    void read(const EventHandlerPtr&);

    InstancePtr _instance;
    bool _destroyed;
    SOCKET _maxFd;
    SOCKET _minFd;
    SOCKET _lastFd;
    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
    fd_set _fdSet;
    std::list<std::pair<SOCKET, EventHandlerPtr> > _changes; // Event handler set for addition; null for removal.
    std::map<SOCKET, EventHandlerPtr> _handlerMap;
    int _handlers;
    int _timeout;
    ::IceUtil::Mutex _threadMutex;

    class EventHandlerThread : public ::IceUtil::Thread
    {
    public:
	
	EventHandlerThread(const ThreadPoolPtr&);
	virtual void run();

    private:

	ThreadPoolPtr _pool;
    };
    friend class EventHandlerThread;

    std::vector<IceUtil::ThreadControl> _threads; // Control for all threads, running or not.
    int _threadNum; // Number of running threads.
};

}

#endif
