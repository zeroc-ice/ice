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
#include <Ice/ThreadPoolF.h>
#include <Ice/InstanceF.h>
#include <Ice/EventHandlerF.h>

namespace IceInternal
{

class Stream;

class ThreadPool : public ::IceUtil::Shared, public JTCMonitorT<JTCMutex>
{
public:

    void _register(int, const EventHandlerPtr&);
    void unregister(int);
    void promoteFollower();
    void waitUntilServerFinished();
    void waitUntilFinished();
    void joinWithAllThreads();
    
private:

    ThreadPool(const InstancePtr&);
    virtual ~ThreadPool();
    void destroy();
    friend class Instance;

    void clearInterrupt();
    void setInterrupt();

    void run();
    void read(const EventHandlerPtr&);

    InstancePtr _instance;
    int _maxFd;
    int _minFd;
    int _lastFd;
    int _fdIntrRead;
    int _fdIntrWrite;
    fd_set _fdSet;
    std::vector<std::pair<int, EventHandlerPtr> > _adds;
    std::vector<int> _removes;
    std::map<int, EventHandlerPtr> _handlers;
    int _servers;
    int _timeout;
    JTCMutex _threadMutex;

    class EventHandlerThread : public JTCThread
    {
    public:
	
	EventHandlerThread(ThreadPoolPtr pool) : _pool(pool) { }
	virtual void run();

    private:

	ThreadPoolPtr _pool;
    };
    friend class EventHandlerThread;
    std::vector<JTCThreadHandle> _threads;
};

}

#endif
