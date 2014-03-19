// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

#if defined(ICE_OS_WINRT)
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

#if defined(ICE_OS_WINRT)

struct SelectEvent
{
    SelectEvent(IceInternal::EventHandler* handler, SocketOperation status) : handler(handler), status(status)
    {
    }

    IceInternal::EventHandler* handler;
    SocketOperation status;
};

class Selector : IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Selector(const InstancePtr&);

    void destroy();

    void initialize(IceInternal::EventHandler*);
    void update(IceInternal::EventHandler*, SocketOperation, SocketOperation);    
    void finish(IceInternal::EventHandler*);

    IceInternal::EventHandler* getNextHandler(SocketOperation&, int);
    
    void completed(IceInternal::EventHandler*, SocketOperation);

private:

    const InstancePtr _instance;
    std::deque<SelectEvent> _events;
};

#elif defined(ICE_USE_IOCP)

class Selector
{
public:

    Selector(const InstancePtr&);
    ~Selector();

    void setup(int);
    void destroy();    

    void initialize(EventHandler*);
    void update(EventHandler*, SocketOperation, SocketOperation);
    void finish(EventHandler*);

    EventHandler* getNextHandler(SocketOperation&, DWORD&, int&, int);

    HANDLE getIOCPHandle() { return _handle; } 
    
private:

    const InstancePtr _instance;
    HANDLE _handle;
};

#elif defined(ICE_USE_KQUEUE) || defined(ICE_USE_EPOLL)

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
    void finish(EventHandler*);

#if defined(ICE_USE_KQUEUE)
    void updateSelector();
#endif    

    void
    startSelect()
    {
#ifdef ICE_USE_KQUEUE
        _selecting = true;
        if(!_changes.empty())
        {
            updateSelector();
        }
#endif
    }

    void
    finishSelect()
    {
#ifdef ICE_USE_KQUEUE
        _selecting = false;
#endif
    }

    void select(std::vector<std::pair<EventHandler*, SocketOperation> >&, int);

private:

    const InstancePtr _instance;
#if defined(ICE_USE_EPOLL)
    std::vector<struct epoll_event> _events;
#else
    std::vector<struct kevent> _events;
    std::vector<struct kevent> _changes;
    bool _selecting;
#endif
    int _queueFd;
};

#elif defined(ICE_USE_SELECT) || defined(ICE_USE_POLL)

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
    void finish(EventHandler*);

    void startSelect();
    void finishSelect();
    void select(std::vector<std::pair<EventHandler*, SocketOperation> >&, int);

private:

    void updateSelector();
    void updateImpl(EventHandler*);

    const InstancePtr _instance;

    SOCKET _fdIntrRead;
    SOCKET _fdIntrWrite;
    bool _selecting;
    bool _interrupted;

    std::vector<std::pair<EventHandler*, SocketOperation> > _changes;
    std::map<SOCKET, EventHandler*> _handlers;

#if defined(ICE_USE_SELECT)
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
#else
    std::vector<struct pollfd> _pollFdSet;
#endif
};

#endif

}

#endif
