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

#include <Ice/ThreadPoolF.h>
#include <Ice/InstanceF.h>
#include <Ice/EventHandlerF.h>
#include <Ice/Shared.h>
#include <map>

namespace __Ice
{

class Stream;

class ThreadPool : public Shared, public JTCMonitorT<JTCMutex>
{
public:

    void _register(int, const EventHandler_ptr&);
    void unregister(int);
    void promoteFollower();
    void waitUntilServerFinished();
    void waitUntilFinished();
    void joinWithAllThreads();
    
private:

    ThreadPool(const Instance_ptr&);
    virtual ~ThreadPool();
    void destroy();
    friend class Instance;

    void clearInterrupt();
    void setInterrupt();

    void run();
    void read(const EventHandler_ptr&);

    Instance_ptr instance_;
    int lastFd_;
    int maxFd_;
    int fdIntrRead_;
    int fdIntrWrite_;
    fd_set fdSet_;
    std::vector<std::pair<int, EventHandler_ptr> > adds_;
    std::vector<int> removes_;
    std::map<int, EventHandler_ptr> handlers_;
    int servers_;
    JTCMutex threadMutex_;

    class EventHandlerThread : public JTCThread
    {
    public:
	
	EventHandlerThread(ThreadPool_ptr pool) : pool_(pool) { }
	virtual void run();

    private:

	ThreadPool_ptr pool_;
    };
    friend class EventHandlerThread;
    std::vector<JTCThreadHandle> threads_;
};

}

#endif
