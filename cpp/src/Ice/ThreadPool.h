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
#include <list>

#ifndef WIN32
#   define SOCKET int
#endif

namespace IceInternal
{

class BasicStream;

class ThreadPool : public ::IceUtil::Shared, public JTCMonitorT<JTCMutex>
{
public:

    void _register(SOCKET, const EventHandlerPtr&);
    void unregister(SOCKET);
    void promoteFollower();
    void initiateServerShutdown(); // Signal-safe shutdown initiation.
    void waitUntilServerFinished();
    void waitUntilFinished();
    void joinWithAllThreads();
    void setMaxConnections(int);
    int getMaxConnections();
    
private:

    ThreadPool(const InstancePtr&);
    virtual ~ThreadPool();
    void destroy();
    friend class Instance;

    bool clearInterrupt();
    void setInterrupt();

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
    std::vector<std::pair<SOCKET, EventHandlerPtr> > _adds;
    std::vector<SOCKET> _removes;
    std::map<SOCKET, EventHandlerPtr> _handlerMap;
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
    std::vector<JTCThreadHandle> _threads; // Handles for all threads, running or not.
    int _threadNum; // Number of running threads.
    int _maxConnections; // Maximum number of connections. If set to zero, the number of connections is not limited.
};

}

#endif
