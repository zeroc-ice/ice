// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_THREAD_POOL_H
#define ICE_THREAD_POOL_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
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

class ThreadPool : public ::IceUtil::Shared, public IceUtil::Mutex
{
public:

    ThreadPool(const InstancePtr&, int, int);
    virtual ~ThreadPool();

    void destroy();

    void _register(SOCKET, const EventHandlerPtr&);
    void unregister(SOCKET);
    void promoteFollower();
    void initiateShutdown(); // Signal-safe shutdown initiation.
    void joinWithAllThreads();
    
private:

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
    int _timeout;
    ::IceUtil::Mutex _threadMutex;
    bool _multipleThreads;

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
};

}

#endif
