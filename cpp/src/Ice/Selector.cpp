// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Selector.h>
#include <Ice/EventHandler.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <IceUtil/Time.h>

using namespace std;
using namespace IceInternal;

#ifdef ICE_OS_WINRT
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;

Selector::Selector(const InstancePtr& instance) : _instance(instance)
{
}

void
Selector::destroy()
{
}

void
Selector::initialize(IceInternal::EventHandler* handler)
{
    handler->__incRef();
    handler->getNativeInfo()->setCompletedHandler(
        ref new SocketOperationCompletedHandler([=](int operation)
                                                {
                                                    completed(handler, static_cast<SocketOperation>(operation));
                                                }));
}

void
Selector::update(IceInternal::EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);
    if(add & SocketOperationRead && !(handler->_pending & SocketOperationRead))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationRead);
        completed(handler, SocketOperationRead); // Start an asynchrnous read 
    }
    else if(add & SocketOperationWrite && !(handler->_pending & SocketOperationWrite))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationWrite);
        completed(handler, SocketOperationWrite); // Start an asynchrnous write 
    }
}

void
Selector::finish(IceInternal::EventHandler* handler)
{
    handler->_registered = SocketOperationNone;
    handler->__decRef();
}

IceInternal::EventHandler* 
Selector::getNextHandler(SocketOperation& status, int timeout)
{
    Lock lock(*this);
    while(_events.empty())
    {
        if(timeout > 0)
        {
            timedWait(IceUtil::Time::seconds(timeout));
            if(_events.empty())
            {
                throw SelectorTimeoutException();  
            }
        }
        else
        {
            wait();
        }
    }
    assert(!_events.empty());
    IceInternal::EventHandler* handler = _events.front().handler;
    status = _events.front().status;
    _events.pop_front();
    return handler;
}

void 
Selector::completed(IceInternal::EventHandler* handler, SocketOperation op)
{
    Lock lock(*this);
    _events.push_back(SelectEvent(handler, op));
    notify();
}

#elif defined(ICE_USE_IOCP)

Selector::Selector(const InstancePtr& instance) : _instance(instance)
{
}

Selector::~Selector()
{
}

void
Selector::setup(int sizeIO)
{
    _handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, sizeIO);
    if(_handle == NULL)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = GetLastError();
        throw ex;
    }
}

void
Selector::destroy()
{
    CloseHandle(_handle);
}

void
Selector::initialize(EventHandler* handler)
{
    HANDLE socket = reinterpret_cast<HANDLE>(handler->getNativeInfo()->fd());
    if(CreateIoCompletionPort(socket, _handle, reinterpret_cast<ULONG_PTR>(handler), 0) == NULL)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = GetLastError();
        throw ex;
    }
    handler->__incRef();
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);
    AsyncInfo* info = 0;
    if(add & SocketOperationRead && !(handler->_pending & SocketOperationRead))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationRead);
        info = handler->getNativeInfo()->getAsyncInfo(SocketOperationRead);
    }
    else if(add & SocketOperationWrite && !(handler->_pending & SocketOperationWrite))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationWrite);
        info = handler->getNativeInfo()->getAsyncInfo(SocketOperationWrite);
    }

    if(info)
    {
        if(!PostQueuedCompletionStatus(_handle, 0, reinterpret_cast<ULONG_PTR>(handler), info))
        {
            Ice::SocketException ex(__FILE__, __LINE__);
            ex.error = GetLastError();
            throw ex;
        }
    }
}

void
Selector::finish(EventHandler* handler)
{
    handler->_registered = SocketOperationNone;
    handler->__decRef();
}

EventHandler*
Selector::getNextHandler(SocketOperation& status, int timeout)
{
    ULONG_PTR key;
    LPOVERLAPPED ol;
    DWORD count;

    if(!GetQueuedCompletionStatus(_handle, &count, &key, &ol, timeout > 0 ? timeout * 1000 : INFINITE))
    {
        int err = WSAGetLastError();
        if(ol == 0)
        {
            if(err == WAIT_TIMEOUT)
            {
                throw SelectorTimeoutException();
            }
            else
            {
                {
                    Ice::SocketException ex(__FILE__, __LINE__, err);
                    Ice::Error out(_instance->initializationData().logger);
                    out << "fatal error: couldn't dequeue packet from completion port:\n" << ex;
                }
                abort();
            }
        }
        AsyncInfo* info = static_cast<AsyncInfo*>(ol);
        status = info->status;
        info->count = SOCKET_ERROR;
        info->error = WSAGetLastError();
        return reinterpret_cast<EventHandler*>(key);
    }

    assert(ol);
    AsyncInfo* info = static_cast<AsyncInfo*>(ol);
    status = info->status;
    info->count = count;
    info->error = 0;
    return reinterpret_cast<EventHandler*>(key);
}

#elif defined(ICE_USE_KQUEUE) || defined(ICE_USE_EPOLL)

Selector::Selector(const InstancePtr& instance) : _instance(instance)
{
    _events.resize(256);
#if defined(ICE_USE_EPOLL)
    _queueFd = epoll_create(1);
    if(_queueFd < 0)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = IceInternal::getSocketErrno();
        throw ex;
    }
#else
    _queueFd = kqueue();
    if(_queueFd < 0)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    _selecting = false;
#endif
}

Selector::~Selector()
{
}

void
Selector::destroy()
{
    try
    {
        closeSocket(_queueFd);
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    SocketOperation previous = handler->_registered;
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);
    if(previous == handler->_registered)
    {
        return;
    }

    SOCKET fd = handler->getNativeInfo()->fd();
#if defined(ICE_USE_EPOLL)
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.data.ptr = handler;
    SocketOperation status = handler->_registered;
    if(handler->_disabled)
    {
        status = static_cast<SocketOperation>(status & ~handler->_disabled);
        previous = static_cast<SocketOperation>(previous & ~handler->_disabled);
    }
    event.events |= status & SocketOperationRead ? EPOLLIN : 0;
    event.events |= status & SocketOperationWrite ? EPOLLOUT : 0;
    int op;
    if(!previous && status)
    {
        op = EPOLL_CTL_ADD;
    }
    else if(previous && !status)
    {
        op = EPOLL_CTL_DEL;
    }
    else if(previous == status)
    {
        return;
    }
    else
    {
        op = EPOLL_CTL_MOD;
    }
    if(epoll_ctl(_queueFd, op, fd, &event) != 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
    }
#else // ICE_USE_KQUEUE
    if(remove & SocketOperationRead)
    {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, handler);            
        _changes.push_back(ev);
    }
    if(remove & SocketOperationWrite)
    {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, handler);
        _changes.push_back(ev);
    }
    if(add & SocketOperationRead)
    {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_READ, EV_ADD | (handler->_disabled & SocketOperationRead ? EV_DISABLE : 0), 0, 0, 
               handler);
        _changes.push_back(ev);
    }
    if(add & SocketOperationWrite)
    {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | (handler->_disabled & SocketOperationWrite ? EV_DISABLE : 0), 0, 0, 
               handler);
        _changes.push_back(ev);
    }
    if(_selecting)
    {
        updateSelector();
    }
#endif
}

void
Selector::enable(EventHandler* handler, SocketOperation status)
{
    if(!(handler->_disabled & status))
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled & ~status);

    if(handler->_registered & status)
    {
        SOCKET fd = handler->getNativeInfo()->fd();
#if defined(ICE_USE_EPOLL)
        SocketOperation previous = static_cast<SocketOperation>(handler->_registered & ~(handler->_disabled | status));
        SocketOperation newStatus = static_cast<SocketOperation>(handler->_registered & ~handler->_disabled);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.data.ptr = handler;
        event.events |= newStatus & SocketOperationRead ? EPOLLIN : 0;
        event.events |= newStatus & SocketOperationWrite ? EPOLLOUT : 0;
        if(epoll_ctl(_queueFd, previous ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &event) != 0)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
        }
#else // ICE_USE_KQUEUE
        struct kevent ev;
        EV_SET(&ev, fd, status == SocketOperationRead ? EVFILT_READ : EVFILT_WRITE, EV_ENABLE, 0, 0, handler);
        _changes.push_back(ev);
        if(_selecting)
        {
            updateSelector();
        }
#endif
    }
}

void
Selector::disable(EventHandler* handler, SocketOperation status)
{
    if(handler->_disabled & status)
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled | status);
    
    if(handler->_registered & status)
    {
        SOCKET fd = handler->getNativeInfo()->fd();
#if defined(ICE_USE_EPOLL)
        SocketOperation newStatus = static_cast<SocketOperation>(handler->_registered & ~handler->_disabled);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.data.ptr = handler;
        event.events |= newStatus & SocketOperationRead ? EPOLLIN : 0;
        event.events |= newStatus & SocketOperationWrite ? EPOLLOUT : 0;
        if(epoll_ctl(_queueFd, newStatus ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, fd, &event) != 0)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
        }
#else // ICE_USE_KQUEUE
        struct kevent ev;
        EV_SET(&ev, fd, status == SocketOperationRead ? EVFILT_READ : EVFILT_WRITE, EV_DISABLE, 0, 0, handler);
        _changes.push_back(ev);
        if(_selecting)
        {
            updateSelector();
        }
#endif
    }
}

void
Selector::finish(EventHandler* handler)
{
    if(handler->_registered)
    {
        update(handler, handler->_registered, SocketOperationNone);
    }
}

#if defined(ICE_USE_KQUEUE)
void 
Selector::updateSelector()
{
    int rs = kevent(_queueFd, &_changes[0], _changes.size(), 0, 0, 0);
    if(rs < 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
    }
    _changes.clear();
}
#endif

void 
Selector::select(vector<pair<EventHandler*, SocketOperation> >& handlers, int timeout)
{
    int ret = 0;
    while(true)
    {
#if defined(ICE_USE_EPOLL)
        ret = epoll_wait(_queueFd, &_events[0], _events.size(), timeout > 0 ? timeout * 1000 : -1);
#else
        assert(!_events.empty());
        if(timeout > 0)
        {
            struct timespec ts;
            ts.tv_sec = timeout;
            ts.tv_nsec = 0;
            ret = kevent(_queueFd, 0, 0, &_events[0], _events.size(), &ts);
        }
        else
        {
            ret = kevent(_queueFd, 0, 0, &_events[0], _events.size(), 0);
        }
#endif
        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            {
                Ice::SocketException ex(__FILE__, __LINE__, IceInternal::getSocketErrno());
                Ice::Error out(_instance->initializationData().logger);
                out << "fatal error: selector failed:\n" << ex;
            }
            abort();
        }
        break;
    }

    if(ret == 0)
    {
        throw SelectorTimeoutException();
    }

    assert(ret > 0);
    handlers.clear();
    for(int i = 0; i < ret; ++i)
    {
        pair<EventHandler*, SocketOperation> p;
#if defined(ICE_USE_EPOLL)
        struct epoll_event& ev = _events[i];
        p.first = reinterpret_cast<EventHandler*>(ev.data.ptr);
        p.second = static_cast<SocketOperation>(((ev.events & EPOLLIN) ? SocketOperationRead : SocketOperationNone) | 
                                                ((ev.events & EPOLLOUT) ? SocketOperationWrite : SocketOperationNone));
#else
        struct kevent& ev = _events[i];
        if(ev.flags & EV_ERROR)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while updating selector:\n" << IceUtilInternal::errorToString(ev.data);
            continue;
        }
        p.first = reinterpret_cast<EventHandler*>(ev.udata);
        p.second = (ev.filter == EVFILT_READ) ? SocketOperationRead : SocketOperationWrite;
#endif
        handlers.push_back(p);
    }
}

#elif defined(ICE_USE_SELECT) || defined(ICE_USE_POLL)

Selector::Selector(const InstancePtr& instance) : _instance(instance), _selecting(false), _interrupted(false)
{
    SOCKET fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
#if defined(ICE_USE_SELECT)
    FD_ZERO(&_readFdSet);
    FD_ZERO(&_writeFdSet);
    FD_ZERO(&_errorFdSet);
    FD_SET(_fdIntrRead, &_readFdSet);
#else
    struct pollfd pollFd;
    pollFd.fd = _fdIntrRead;
    pollFd.events = POLLIN;
    _pollFdSet.push_back(pollFd);
#endif
}

Selector::~Selector()
{
    try
    {
        closeSocket(_fdIntrWrite);
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }

    try
    {
        closeSocket(_fdIntrRead);
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }
}

void
Selector::destroy()
{
#if !defined(ICE_USE_SELECT) && !defined(ICE_USE_POLL)
    assert(_events.empty());
#endif
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    SocketOperation previous = handler->_registered;
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);
    if(previous == handler->_registered)
    {
        return;
    }

    updateImpl(handler);
}

void
Selector::enable(EventHandler* handler, SocketOperation status)
{
    if(!(handler->_disabled & status))
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled & ~status);

    if(handler->_registered & status)
    {
        updateImpl(handler);
    }
}

void
Selector::disable(EventHandler* handler, SocketOperation status)
{
    if(handler->_disabled & status)
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled | status);
    
    if(handler->_registered & status)
    {
        updateImpl(handler);
    }
}

void
Selector::finish(EventHandler* handler)
{
    if(handler->_registered)
    {
        update(handler, handler->_registered, SocketOperationNone);
    }
}

void
Selector::startSelect()
{
    if(_interrupted)
    {
        char c;
        while(true)
        {
            ssize_t ret;
#ifdef _WIN32
            ret = ::recv(_fdIntrRead, &c, 1, 0);
#else
            ret = ::read(_fdIntrRead, &c, 1);
#endif
            if(ret == SOCKET_ERROR)
            {
                if(interrupted())
                {
                    continue;
                }
                
                Ice::SocketException ex(__FILE__, __LINE__);
                ex.error = IceInternal::getSocketErrno();
                throw ex;
            }
            break;
        }
        _interrupted = false;

        if(!_changes.empty())
        {
            updateSelector();
        }
    }
    _selecting = true;
}

void
Selector::finishSelect()
{
    _selecting = false;
}

void 
Selector::select(vector<pair<EventHandler*, SocketOperation> >& handlers, int timeout)
{
    int ret = 0;
    while(true)
    {
#if defined(ICE_USE_SELECT)
        fd_set* rFdSet = fdSetCopy(_selectedReadFdSet, _readFdSet);
        fd_set* wFdSet = fdSetCopy(_selectedWriteFdSet, _writeFdSet);
        fd_set* eFdSet = fdSetCopy(_selectedErrorFdSet, _errorFdSet);
        if(timeout > 0)
        {
            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            ret = ::select(0, rFdSet, wFdSet, eFdSet, &tv); // The first parameter is ignored on Windows
        }
        else
        {
            ret = ::select(0, rFdSet, wFdSet, eFdSet, 0); // The first parameter is ignored on Windows
        }
#else
        ret = poll(&_pollFdSet[0], _pollFdSet.size(), timeout > 0 ? timeout * 1000 : -1);
#endif
        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            {
                Ice::SocketException ex(__FILE__, __LINE__, IceInternal::getSocketErrno());
                Ice::Error out(_instance->initializationData().logger);
                out << "fatal error: selector failed:\n" << ex;
            }
            abort();
        }
        break;
    }

    if(ret == 0)
    {
        throw SelectorTimeoutException();
    }

    assert(ret > 0);
    handlers.clear();

#if defined(ICE_USE_SELECT)
    if(_selectedReadFdSet.fd_count == 0 && _selectedWriteFdSet.fd_count == 0 && _selectedErrorFdSet.fd_count == 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "select() in selector returned " << ret << " but no filedescriptor is ready";
        return;
    }

    for(unsigned int i = 0; i < static_cast<unsigned int>(ret); ++i)
    {
        pair<EventHandler*, SocketOperation> p;

        //
        // Round robin for the filedescriptors.
        //
        SOCKET fd;
        p.second = SocketOperationNone;
        if(i < _selectedReadFdSet.fd_count)
        {
            fd = _selectedReadFdSet.fd_array[i];
            p.second = static_cast<SocketOperation>(p.second | SocketOperationRead);
        }
        else if(i < _selectedWriteFdSet.fd_count + _selectedReadFdSet.fd_count)
        {
            fd = _selectedWriteFdSet.fd_array[i - _selectedReadFdSet.fd_count];
            p.second = static_cast<SocketOperation>(p.second | SocketOperationWrite);
        }
        else
        {
            fd = _selectedErrorFdSet.fd_array[i - _selectedReadFdSet.fd_count - _selectedWriteFdSet.fd_count];
            p.second = static_cast<SocketOperation>(p.second | SocketOperationConnect);
        }

        if(fd == _fdIntrRead) // Interrupted, we have to process the interrupt before returning any handlers
        { 
            handlers.clear();
            return;
        }

        assert(_handlers.find(fd) != _handlers.end());
        p.first = _handlers[fd];
        handlers.push_back(p);
    }
#else
    if(_pollFdSet[0].revents == POLLIN) // Interrupted, we have to process the interrupt before returning any handlers
    {
        return;
    }

    for(vector<struct pollfd>::const_iterator q = _pollFdSet.begin(); q != _pollFdSet.end(); ++q)
    {
        pair<EventHandler*, SocketOperation> p;
        if(q->revents != 0)
        {
            SOCKET fd = q->fd;
            assert(fd != _fdIntrRead);
            assert(_handlers.find(fd) != _handlers.end());
            p.first = _handlers[fd];
            p.second = SocketOperationNone;
            if(q->revents & (POLLIN | POLLERR | POLLHUP))
            {
                p.second = static_cast<SocketOperation>(p.second | SocketOperationRead);
            }
            if(q->revents & POLLOUT)
            {
                p.second = static_cast<SocketOperation>(p.second | SocketOperationWrite);
            }
            assert(p.second);
            handlers.push_back(p);
        }
    }
#endif
}

void
Selector::updateImpl(EventHandler* handler)
{
    SocketOperation status = static_cast<SocketOperation>(handler->_registered & ~handler->_disabled);
    _changes.push_back(make_pair(handler, status));
    if(_selecting)
    {
        if(!_interrupted)
        {
            char c = 0;
            while(true)
            {
#ifdef _WIN32
                if(::send(_fdIntrWrite, &c, 1, 0) == SOCKET_ERROR)
#else
                if(::write(_fdIntrWrite, &c, 1) == SOCKET_ERROR)
#endif
                {
                    if(interrupted())
                    {
                        continue;
                        }
                    
                    Ice::SocketException ex(__FILE__, __LINE__);
                    ex.error = IceInternal::getSocketErrno();
                    throw ex;
                }
                break;
            }
            _interrupted = true;
        }
    }
    else
    {
        updateSelector();
    }
}

void
Selector::updateSelector()
{
    for(vector<pair<EventHandler*, SocketOperation> >::const_iterator p = _changes.begin(); p != _changes.end(); ++p)
    {
        EventHandler* handler = p->first;
        SocketOperation status = p->second;

        SOCKET fd = handler->getNativeInfo()->fd();
        if(status)
        {
#if defined(ICE_USE_SELECT)
            if(status & SocketOperationRead)
            {
                FD_SET(fd, &_readFdSet);
            }
            else
            {
                FD_CLR(fd, &_readFdSet);
            }
            if(status & SocketOperationWrite)
            {
                FD_SET(fd, &_writeFdSet);
            }
            else
            {
                FD_CLR(fd, &_writeFdSet);
            }
            if(status & SocketOperationConnect)
            {
                FD_SET(fd, &_writeFdSet);
                FD_SET(fd, &_errorFdSet);
            }
            else
            {
                FD_CLR(fd, &_writeFdSet);
                FD_CLR(fd, &_errorFdSet);
            }
            _handlers[fd] = handler;
#else
            short events = 0;
            if(status & SocketOperationRead)
            {
                events |= POLLIN;
            }
            if(status & SocketOperationWrite)
            {
                events |= POLLOUT;
            }
            map<SOCKET, EventHandler*>::const_iterator q = _handlers.find(fd);
            if(q == _handlers.end())
            {
                struct pollfd pollFd;
                pollFd.fd = fd;
                pollFd.events = events;
                pollFd.revents = 0;
                _pollFdSet.push_back(pollFd);
                _handlers.insert(make_pair(fd, handler));
            }
            else
            {
                for(vector<struct pollfd>::iterator r = _pollFdSet.begin(); r != _pollFdSet.end(); ++r)
                {
                    if(r->fd == fd)
                    {
                        r->events = events;
                        break;
                    }
                }
            }
#endif
        }
        else
        {
#if defined(ICE_USE_SELECT)
            FD_CLR(fd, &_readFdSet);
            FD_CLR(fd, &_writeFdSet);
            FD_CLR(fd, &_errorFdSet);
#else
            for(vector<struct pollfd>::iterator r = _pollFdSet.begin(); r != _pollFdSet.end(); ++r)
            {
                if(r->fd == fd)
                {
                    _pollFdSet.erase(r);
                    break;
                }
            }
#endif
            _handlers.erase(fd);
        }
    }
    _changes.clear();
}

#endif
