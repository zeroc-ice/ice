// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SELECTOR_H
#define ICE_SELECTOR_H

#include <IceUtil/StringUtil.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

#include <Ice/Network.h>
#include <Ice/InstanceF.h>
#include <Ice/EventHandlerF.h>

#if defined(ICE_USE_EPOLL)
#   include <sys/epoll.h>
#elif defined(ICE_USE_KQUEUE)
#   include <sys/event.h>
#elif defined(ICE_USE_IOCP)
// Nothing to include
#elif defined(ICE_USE_POLL)
#   include <sys/poll.h>
#endif

#if defined(ICE_USE_CFSTREAM)
#   include <IceUtil/RecMutex.h>
#   include <IceUtil/Thread.h>
#   include <set>

struct __CFRunLoop;
typedef struct __CFRunLoop * CFRunLoopRef;

struct __CFRunLoopSource;
typedef struct __CFRunLoopSource * CFRunLoopSourceRef;

struct __CFSocket;
typedef struct __CFSocket * CFSocketRef;
#endif

#if defined(ICE_OS_UWP)
#    include <deque>
#endif

namespace IceInternal
{

//
// Exception raised if select times out.
//
class SelectorTimeoutException
{
};


#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)

class Selector
{
#if defined(ICE_OS_UWP)
    struct SelectEvent
    {
        SelectEvent(const EventHandlerPtr& handler, SocketOperation status) : handler(handler), status(status)
        {
        }

        EventHandlerPtr handler;
        SocketOperation status;
    };
#endif

public:

    Selector(const InstancePtr&);
    ~Selector();

#ifdef ICE_USE_IOCP
    void setup(int);
#endif
    void destroy();

    void initialize(EventHandler*);
    void update(EventHandler*, SocketOperation, SocketOperation);
    void finish(EventHandler*);

    void ready(EventHandler*, SocketOperation, bool);

#ifdef ICE_USE_IOCP
    EventHandler* getNextHandler(SocketOperation&, DWORD&, int&, int);
#else
    EventHandler* getNextHandler(SocketOperation&, int);
#endif

    void completed(EventHandler*, SocketOperation);

private:

    const InstancePtr _instance;
#ifdef ICE_USE_IOCP
    HANDLE _handle;
#else
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    std::deque<SelectEvent> _events;
#endif
};

#elif defined(ICE_USE_KQUEUE) || defined(ICE_USE_EPOLL) || defined(ICE_USE_SELECT) || defined(ICE_USE_POLL)

class Selector
{
public:

    Selector(const InstancePtr&);
    ~Selector();

    void destroy();

    void initialize(EventHandler*)
    {
        // Nothing to do
    }
    void update(EventHandler*, SocketOperation, SocketOperation);
    void enable(EventHandler*, SocketOperation);
    void disable(EventHandler*, SocketOperation);
    bool finish(EventHandler*, bool);

    void ready(EventHandler*, SocketOperation, bool);

    void startSelect();
    void finishSelect(std::vector<std::pair<EventHandler*, SocketOperation> >&);
    void select(int);

private:

    void wakeup();
    void checkReady(EventHandler*);
    void updateSelector();

    const InstancePtr _instance;

    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
    bool _interrupted;
    bool _selectNow;
    int _count;
    bool _selecting;
    std::map<EventHandlerPtr, SocketOperation> _readyHandlers;

#if defined(ICE_USE_EPOLL)
    std::vector<struct epoll_event> _events;
    int _queueFd;
#elif defined(ICE_USE_KQUEUE)
    std::vector<struct kevent> _events;
    std::vector<struct kevent> _changes;
    int _queueFd;
#elif defined(ICE_USE_SELECT)
    std::vector<std::pair<EventHandler*, SocketOperation> > _changes;
    std::map<SOCKET, EventHandler*> _handlers;

    fd_set _readFdSet;
    fd_set _writeFdSet;
    fd_set _errorFdSet;
    fd_set _selectedReadFdSet;
    fd_set _selectedWriteFdSet;
    fd_set _selectedErrorFdSet;

    fd_set*
    fdSetCopy(fd_set& dest, fd_set& src)
    {
        if(src.fd_count > 0)
        {
            dest.fd_count = src.fd_count;
            memcpy(dest.fd_array, src.fd_array, sizeof(SOCKET) * src.fd_count);
            return &dest;
        }
        return 0;
    }
#elif defined(ICE_USE_POLL)
    std::vector<std::pair<EventHandler*, SocketOperation> > _changes;
    std::map<SOCKET, EventHandler*> _handlers;
    std::vector<struct pollfd> _pollFdSet;
#endif
};

#elif defined(ICE_USE_CFSTREAM)

class Selector;

class SelectorReadyCallback : public IceUtil::Shared
{
public:

    virtual ~SelectorReadyCallback() { }
    virtual void readyCallback(SocketOperation, int = 0) = 0;
};

class StreamNativeInfo : public NativeInfo
{
public:

    StreamNativeInfo(SOCKET fd) : NativeInfo(fd), _connectError(0)
    {
    }

    virtual void initStreams(SelectorReadyCallback*) = 0;
    virtual SocketOperation registerWithRunLoop(SocketOperation) = 0;
    virtual SocketOperation unregisterFromRunLoop(SocketOperation, bool) = 0;
    virtual void closeStreams() = 0;

    void setConnectError(int error)
    {
        _connectError = error;
    }

private:

    int _connectError;
};
typedef IceUtil::Handle<StreamNativeInfo> StreamNativeInfoPtr;

class EventHandlerWrapper : public SelectorReadyCallback
{
public:

    EventHandlerWrapper(EventHandler*, Selector&);
    ~EventHandlerWrapper();

    void updateRunLoop();

    virtual void readyCallback(SocketOperation, int = 0);
    void ready(SocketOperation, int);

    SocketOperation readyOp();
    bool checkReady();

    bool update(SocketOperation, SocketOperation);
    bool finish();

    bool operator<(const EventHandlerWrapper& o)
    {
        return this < &o;
    }

private:

    friend class Selector;

    EventHandlerPtr _handler;
    StreamNativeInfoPtr _streamNativeInfo;
    Selector& _selector;
    SocketOperation _ready;
    bool _finish;
    CFSocketRef _socket;
    CFRunLoopSourceRef _source;
};
typedef IceUtil::Handle<EventHandlerWrapper> EventHandlerWrapperPtr;

class Selector : IceUtil::Monitor<IceUtil::RecMutex>
{

public:

    Selector(const InstancePtr&);
    virtual ~Selector();

    void destroy();

    void initialize(EventHandler*);
    void update(EventHandler*, SocketOperation, SocketOperation);
    void enable(EventHandler*, SocketOperation);
    void disable(EventHandler*, SocketOperation);
    bool finish(EventHandler*, bool);

    void ready(EventHandler*, SocketOperation, bool);

    void startSelect();
    void finishSelect(std::vector<std::pair<EventHandler*, SocketOperation> >&);
    void select(int);

    void processInterrupt();
    void run();

private:

    void ready(EventHandlerWrapper*, SocketOperation, int = 0);
    void addReadyHandler(EventHandlerWrapper*);

    friend class EventHandlerWrapper;

    InstancePtr _instance;
    IceUtil::ThreadPtr _thread;
    CFRunLoopRef _runLoop;
    CFRunLoopSourceRef _source;
    bool _destroyed;

    std::set<EventHandlerWrapperPtr> _changes;

    std::set<EventHandlerWrapperPtr> _readyHandlers;
    std::vector<std::pair<EventHandlerWrapperPtr, SocketOperation> > _selectedHandlers;
    std::map<EventHandler*, EventHandlerWrapperPtr> _wrappers;
};

#endif

}

#endif
