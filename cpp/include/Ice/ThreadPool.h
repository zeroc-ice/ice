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
#include <queue>
#include <list>

namespace __Ice
{

class ICE_API ThreadPoolI : public Shared, public JTCMutex
{
public:

    void _register(const EventHandler&);
    void unregister(int);
    void reregister(int);
    void joinWithAllThreads();
    
private:

    ThreadPoolI(const ThreadPoolI&);
    void operator=(const ThreadPoolI&);

    ThreadPoolI(const Instance&);
    virtual ~ThreadPoolI();
    void destroy();
    friend class InstanceI; // May create and destroy ThreadPoolIs

    void clearInterrupt();
    void setInterrupt();

    void run();

    Instance instance_;
    int lastFd_;
    int maxFd_;
    int fdIntrRead_;
    int fdIntrWrite_;
    fd_set fdSet_;
    std::list<EventHandler> newHandlers_;
    std::queue<int> removes_;
    std::vector<int> adds_;
    std::list<EventHandler> handlers_;
    JTCMutex threadMutex_;

    class EventHandlerThread : public JTCThread
    {
    public:
	
	EventHandlerThread(ThreadPool pool) : pool_(pool) { }
	virtual void run();

    private:

	ThreadPool pool_;
    };
    friend class EventHandlerThread;
    std::list<JTCThreadHandle> threads_;
};

}

#endif
