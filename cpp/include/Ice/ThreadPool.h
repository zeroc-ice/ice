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

class ThreadPoolI : public Shared, public JTCMonitorT<JTCMutex>
{
public:

    void _register(int, const EventHandler&);
    void unregister(int);
    void promoteFollower();
    void waitUntilServerFinished();
    void waitUntilFinished();
    void joinWithAllThreads();
    
private:

    ThreadPoolI(const Instance&);
    virtual ~ThreadPoolI();
    void destroy();
    friend class InstanceI; // May create and destroy ThreadPoolIs

    void clearInterrupt();
    void setInterrupt();

    void run();
    void read(const EventHandler&);

    Instance instance_;
    int lastFd_;
    int maxFd_;
    int fdIntrRead_;
    int fdIntrWrite_;
    fd_set fdSet_;
    std::vector<std::pair<int, EventHandler> > adds_;
    std::vector<int> removes_;
    std::map<int, EventHandler> handlers_;
    int servers_;
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
    std::vector<JTCThreadHandle> threads_;
};

}

#endif
