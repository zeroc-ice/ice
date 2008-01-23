// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SELECTOR_THREAD_H
#define ICE_SELECTOR_THREAD_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Timer.h>

#include <Ice/Config.h>
#include <Ice/SelectorThreadF.h>
#include <Ice/InstanceF.h>
#include <Ice/Selector.h>

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

#include <deque>

namespace IceInternal
{

class SocketReadyCallback : public IceUtil::TimerTask
{
public:
    
    //
    // The selector thread unregisters the callback when socketReady returns SocketStatus.Finished.
    //
    virtual SocketStatus socketReady(bool) = 0;

    //
    // The selector thread doesn't unregister the callback when sockectTimeout is called; socketTimeout
    // must unregister the callback either explicitly with unregister() or by shutting down the socket 
    // (if necessary).
    //
    virtual void socketTimeout() = 0;

private:

    void
    runTimerTask()
    {
        this->socketTimeout();
    }
    
    friend class SelectorThread;
};
typedef IceUtil::Handle<SocketReadyCallback> SocketReadyCallbackPtr;

class SelectorThread : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SelectorThread(const InstancePtr&);
    virtual ~SelectorThread();

    void destroy();

    void incFdsInUse();
    void decFdsInUse();

    void _register(SOCKET, const SocketReadyCallbackPtr&, SocketStatus status, int timeout);
    void unregister(SOCKET);
    void joinWithThread();

private:

    void run();

    struct SocketInfo
    {
        SOCKET fd;
        SocketReadyCallbackPtr cb;
        SocketStatus status;
        int timeout;
        
        SocketInfo(SOCKET fd, const SocketReadyCallbackPtr& cb, SocketStatus status, int timeout)
        {
            this->fd = fd;
            this->cb = cb;
            this->status = status;
            this->timeout = timeout;
        }
    };

    class HelperThread : public IceUtil::Thread
    {
    public:
        
        HelperThread(const SelectorThreadPtr&);
        virtual void run();

    private:

        SelectorThreadPtr _selectorThread;
    };
    friend class HelperThread;

    InstancePtr _instance;
    bool _destroyed;

    SOCKET _maxFd;
    SOCKET _minFd;
    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
    Selector _selector;

    std::deque<SocketInfo> _changes;

    IceUtil::ThreadPtr _thread;
    IceUtil::TimerPtr _timer;
};

}

#endif
