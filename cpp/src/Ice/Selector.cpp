// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#ifdef ICE_USE_CFSTREAM
#   include <CoreFoundation/CoreFoundation.h>
#   include <CoreFoundation/CFStream.h>
#endif

using namespace std;
using namespace IceInternal;

#if defined(ICE_USE_KQUEUE)
namespace
{
struct timespec zeroTimeout = { 0, 0 };
}
#endif

#if defined(ICE_OS_UWP)
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
#endif

#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)

Selector::Selector(const InstancePtr& instance) : _instance(instance)
{
}

Selector::~Selector()
{
}

#ifdef ICE_USE_IOCP
void
Selector::setup(int sizeIO)
{
    _handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, ICE_NULLPTR, 0, sizeIO);
    if(_handle == ICE_NULLPTR)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = GetLastError();
        throw ex;
    }
}
#endif

void
Selector::destroy()
{
#ifdef ICE_USE_IOCP
    CloseHandle(_handle);
#endif
}

void
Selector::initialize(EventHandler* handler)
{
    if(!handler->getNativeInfo())
    {
        return;
    }
#ifdef ICE_USE_IOCP
    HANDLE socket = reinterpret_cast<HANDLE>(handler->getNativeInfo()->fd());
    if(CreateIoCompletionPort(socket, _handle, reinterpret_cast<ULONG_PTR>(handler), 0) == ICE_NULLPTR)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = GetLastError();
        throw ex;
    }
    handler->getNativeInfo()->initialize(_handle, reinterpret_cast<ULONG_PTR>(handler));
#else
    EventHandlerPtr h = ICE_GET_SHARED_FROM_THIS(handler);
    handler->getNativeInfo()->setCompletedHandler(
        ref new SocketOperationCompletedHandler(
            [=](int operation)
            {
                //
                // Use the reference counted handler to ensure it's not
                // destroyed as long as the callback lambda exists.
                //
                completed(h.get(), static_cast<SocketOperation>(operation));
            }));
#endif
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
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
    handler->_finish = false; // Ensures that finished() is only called once on the event handler.
}

void
Selector::ready(EventHandler* handler, SocketOperation status, bool value)
{
    if(((handler->_ready & status) != 0) == value)
    {
        return; // Nothing to do if ready state already correctly set.
    }

    if(value)
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready | status);
    }
    else
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready & ~status);
    }
}

EventHandler*
#ifdef ICE_USE_IOCP
Selector::getNextHandler(SocketOperation& status, DWORD& count, int& error, int timeout)
#else
Selector::getNextHandler(SocketOperation& status, int timeout)
#endif
{
#ifdef ICE_USE_IOCP
    ULONG_PTR key;
    LPOVERLAPPED ol;
    error = 0;

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
                Ice::SocketException ex(__FILE__, __LINE__, err);
                Ice::Error out(_instance->initializationData().logger);
                out << "couldn't dequeue packet from completion port:\n" << ex;
                IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(5)); // Sleep 5s to avoid looping
            }
        }
        AsyncInfo* info = static_cast<AsyncInfo*>(ol);
        if(info)
        {
            status = info->status;
        }
        count = SOCKET_ERROR;
        error = WSAGetLastError();
        return reinterpret_cast<EventHandler*>(key);
    }

    AsyncInfo* info = static_cast<AsyncInfo*>(ol);
    if(info)
    {
        status = info->status;
    }
    else
    {
        status = reinterpret_cast<EventHandler*>(key)->_ready;
    }
    return reinterpret_cast<EventHandler*>(key);
#else
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    while(_events.empty())
    {
        if(timeout > 0)
        {
            _monitor.timedWait(IceUtil::Time::seconds(timeout));
            if(_events.empty())
            {
                throw SelectorTimeoutException();
            }
        }
        else
        {
            _monitor.wait();
        }
    }
    assert(!_events.empty());
    IceInternal::EventHandlerPtr handler = _events.front().handler;
    const SelectEvent& event = _events.front();
    status = event.status;
    _events.pop_front();
    return handler.get();
#endif
}

void
Selector::completed(EventHandler* handler, SocketOperation op)
{
#ifdef ICE_USE_IOCP
    AsyncInfo* info = 0;
    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if(nativeInfo)
    {
        info = nativeInfo->getAsyncInfo(op);
    }
    if(!PostQueuedCompletionStatus(_handle, 0, reinterpret_cast<ULONG_PTR>(handler), info))
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = GetLastError();
        throw ex;
    }
#else
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    _events.push_back(SelectEvent(handler->shared_from_this(), op));
    _monitor.notify();
#endif
}

#elif defined(ICE_USE_KQUEUE) || defined(ICE_USE_EPOLL) || defined(ICE_USE_SELECT) || defined(ICE_USE_POLL)

Selector::Selector(const InstancePtr& instance) : _instance(instance), _interrupted(false)
{
    SOCKET fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    _selecting = false;

#if defined(ICE_USE_EPOLL)
    _events.resize(256);
    _queueFd = epoll_create(1);
    if(_queueFd < 0)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = IceInternal::getSocketErrno();
        throw ex;
    }

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.data.ptr = 0;
    event.events = EPOLLIN;
    if(epoll_ctl(_queueFd, EPOLL_CTL_ADD, _fdIntrRead, &event) != 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
    }
#elif defined(ICE_USE_KQUEUE)
    _events.resize(256);
    _queueFd = kqueue();
    if(_queueFd < 0)
    {
        Ice::SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    struct kevent ev;
    EV_SET(&ev, _fdIntrRead, EVFILT_READ, EV_ADD, 0, 0, 0);
    int rs = kevent(_queueFd, &ev, 1, 0, 0, 0);
    if(rs < 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
    }
#elif defined(ICE_USE_SELECT)
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
}

void
Selector::destroy()
{
#if defined(ICE_USE_KQUEUE) || defined(ICE_USE_EPOLL)
    try
    {
        closeSocket(_queueFd);
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }
#endif

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
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    SocketOperation previous = handler->_registered;
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);
    if(previous == handler->_registered)
    {
        return;
    }
    checkReady(handler);

    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if(nativeInfo && nativeInfo->fd() != INVALID_SOCKET)
    {
        updateSelectorForEventHandler(handler, remove, add);
    }
}

void
Selector::enable(EventHandler* handler, SocketOperation status)
{
    if(!(handler->_disabled & status))
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled & ~status);
    checkReady(handler);

    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if(!nativeInfo || nativeInfo->fd() == INVALID_SOCKET)
    {
        return;
    }

    if(handler->_registered & status)
    {
#if defined(ICE_USE_EPOLL)
        SOCKET fd = nativeInfo->fd();
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
#elif defined(ICE_USE_KQUEUE)
        struct kevent ev;
        SOCKET fd = handler->getNativeInfo()->fd();
        EV_SET(&ev, fd, status == SocketOperationRead ? EVFILT_READ : EVFILT_WRITE, EV_ENABLE, 0, 0, handler);
        _changes.push_back(ev);
        if(_selecting)
        {
            updateSelector();
        }
#else
        _changes.push_back(make_pair(handler, static_cast<SocketOperation>(handler->_registered & ~handler->_disabled)));
        wakeup();
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
    checkReady(handler);

    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if(!nativeInfo || nativeInfo->fd() == INVALID_SOCKET)
    {
        return;
    }

    if(handler->_registered & status)
    {
#if defined(ICE_USE_EPOLL)
        SOCKET fd = nativeInfo->fd();
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
#elif defined(ICE_USE_KQUEUE)
        SOCKET fd = nativeInfo->fd();
        struct kevent ev;
        EV_SET(&ev, fd, status == SocketOperationRead ? EVFILT_READ : EVFILT_WRITE, EV_DISABLE, 0, 0, handler);
        _changes.push_back(ev);
        if(_selecting)
        {
            updateSelector();
        }
#else
        _changes.push_back(make_pair(handler, static_cast<SocketOperation>(handler->_registered & ~handler->_disabled)));
        wakeup();
#endif
    }
}

bool
Selector::finish(EventHandler* handler, bool closeNow)
{
    if(handler->_registered)
    {
        update(handler, handler->_registered, SocketOperationNone);
#if !defined(ICE_USE_EPOLL) && !defined(ICE_USE_KQUEUE)
        return false; // Don't close now if selecting
#endif
    }

#if defined(ICE_USE_KQUEUE)
    if(closeNow && !_changes.empty())
    {
        //
        // Update selector now to remove the FD from the kqueue if
        // we're going to close it now. This isn't necessary for
        // epoll since we always update the epoll FD immediately.
        //
        updateSelector();
    }
#elif !defined(ICE_USE_EPOLL)
    if(!_changes.empty())
    {
        return false;
    }
#endif

    return closeNow;
}

void
Selector::ready(EventHandler* handler, SocketOperation status, bool value)
{
    if(((handler->_ready & status) != 0) == value)
    {
        return; // Nothing to do if ready state already correctly set.
    }

    if(status & SocketOperationConnect)
    {
        NativeInfoPtr nativeInfo = handler->getNativeInfo();
        if(nativeInfo && nativeInfo->newFd() && handler->_registered)
        {
            // If new FD is set after connect, register the FD with the selector.
            updateSelectorForEventHandler(handler, SocketOperationNone, handler->_registered);
        }
    }

    if(value)
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready | status);
    }
    else
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready & ~status);
    }
    checkReady(handler);
}

void
Selector::wakeup()
{
    if(_selecting && !_interrupted)
    {
        char c = 0;
        while(true)
        {
            if(::write(_fdIntrWrite, &c, 1) == SOCKET_ERROR)
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

void
Selector::startSelect()
{
    if(_interrupted)
    {
        char c;
        while(true)
        {
            ssize_t ret = ::read(_fdIntrRead, &c, 1);
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
    }

#if !defined(ICE_USE_EPOLL)
    if(!_changes.empty())
    {
        updateSelector();
    }
#endif
    _selecting = true;

    //
    // If there are ready handlers, don't block in select, just do a non-blocking
    // select to retrieve new ready handlers from the Java selector.
    //
    _selectNow = !_readyHandlers.empty();
}

void
Selector::finishSelect(vector<pair<EventHandler*, SocketOperation> >& handlers)
{
    _selecting = false;

    assert(handlers.empty());

#if defined(ICE_USE_POLL) || defined(ICE_USE_SELECT)
    if(_interrupted) // Interrupted, we have to process the interrupt before returning any handlers
    {
        return;
    }
#endif

#if defined(ICE_USE_POLL)
    for(vector<struct pollfd>::const_iterator r = _pollFdSet.begin(); r != _pollFdSet.end(); ++r)
#else
    for(int i = 0; i < _count; ++i)
#endif
    {
        pair<EventHandler*, SocketOperation> p;

#if defined(ICE_USE_EPOLL)
        struct epoll_event& ev = _events[i];
        p.first = reinterpret_cast<EventHandler*>(ev.data.ptr);
        p.second = static_cast<SocketOperation>(((ev.events & (EPOLLIN | EPOLLERR)) ?
                                                 SocketOperationRead : SocketOperationNone) |
                                                ((ev.events & (EPOLLOUT | EPOLLERR)) ?
                                                 SocketOperationWrite : SocketOperationNone));
#elif defined(ICE_USE_KQUEUE)
        struct kevent& ev = _events[i];
        if(ev.flags & EV_ERROR)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "selector returned error:\n" << IceUtilInternal::errorToString(ev.data);
            continue;
        }
        p.first = reinterpret_cast<EventHandler*>(ev.udata);
        p.second = (ev.filter == EVFILT_READ) ? SocketOperationRead : SocketOperationWrite;
#elif defined(ICE_USE_SELECT)
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

        assert(fd != _fdIntrRead);
        p.first = _handlers[fd];
#else
        if(r->revents == 0)
        {
            continue;
        }

        SOCKET fd = r->fd;
        assert(_handlers.find(fd) != _handlers.end());
        p.first = _handlers[fd];
        p.second = SocketOperationNone;
        if(r->revents & (POLLIN | POLLERR | POLLHUP))
        {
            p.second = static_cast<SocketOperation>(p.second | SocketOperationRead);
        }
        if(r->revents & (POLLOUT | POLLERR | POLLHUP))
        {
            p.second = static_cast<SocketOperation>(p.second | SocketOperationWrite);
        }
        assert(p.second);
#endif
        if(!p.first)
        {
            continue; // Interrupted
        }

        map<EventHandlerPtr, SocketOperation>::iterator q = _readyHandlers.find(ICE_GET_SHARED_FROM_THIS(p.first));

        if(q != _readyHandlers.end()) // Handler will be added by the loop below
        {
            q->second = p.second; // We just remember which operations are ready here.
        }
        else
        {
            handlers.push_back(p);
        }
    }

    for(map<EventHandlerPtr, SocketOperation>::iterator q = _readyHandlers.begin(); q != _readyHandlers.end(); ++q)
    {
        pair<EventHandler*, SocketOperation> p;
        p.first = q->first.get();
        p.second = static_cast<SocketOperation>(p.first->_ready & ~p.first->_disabled & p.first->_registered);
        p.second = static_cast<SocketOperation>(p.second | q->second);
        if(p.second)
        {
            handlers.push_back(p);
        }

        //
        // Reset the operation, it's only used by this method to temporarly store the socket status
        // return by the select operation above.
        //
        q->second = SocketOperationNone;
    }
}

void
Selector::select(int timeout)
{
    if(_selectNow)
    {
        timeout = 0;
    }
    else if(timeout > 0)
    {
        timeout = timeout * 1000;
    }
    else
    {
        timeout = -1;
    }

    int spuriousWakeup = 0;
    while(true)
    {
#if defined(ICE_USE_EPOLL)
        _count = epoll_wait(_queueFd, &_events[0], _events.size(), timeout);
#elif defined(ICE_USE_KQUEUE)
        assert(!_events.empty());
        if(timeout >= 0)
        {
            struct timespec ts;
            ts.tv_sec = timeout;
            ts.tv_nsec = 0;
            _count = kevent(_queueFd, 0, 0, &_events[0], _events.size(), &ts);
        }
        else
        {
            _count = kevent(_queueFd, 0, 0, &_events[0], _events.size(), 0);
        }
#elif defined(ICE_USE_SELECT)
        fd_set* rFdSet = fdSetCopy(_selectedReadFdSet, _readFdSet);
        fd_set* wFdSet = fdSetCopy(_selectedWriteFdSet, _writeFdSet);
        fd_set* eFdSet = fdSetCopy(_selectedErrorFdSet, _errorFdSet);
        if(timeout >= 0)
        {
            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            _count = ::select(0, rFdSet, wFdSet, eFdSet, &tv); // The first parameter is ignored on Windows
        }
        else
        {
            _count = ::select(0, rFdSet, wFdSet, eFdSet, 0); // The first parameter is ignored on Windows
        }
#else
        _count = poll(&_pollFdSet[0], _pollFdSet.size(), timeout);
#endif

        if(_count == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            Ice::SocketException ex(__FILE__, __LINE__, IceInternal::getSocketErrno());
            Ice::Error out(_instance->initializationData().logger);
            out << "selector failed:\n" << ex;
            IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(5)); // Sleep 5s to avoid looping
        }
        else if(_count == 0 && timeout < 0)
        {
            if(++spuriousWakeup > 100)
            {
                spuriousWakeup = 0;
                _instance->initializationData().logger->warning("spurious selector wakeup");
            }
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
            continue;
        }
        break;
    }

    if(_count == 0 && !_selectNow)
    {
        throw SelectorTimeoutException();
    }
}

void
Selector::checkReady(EventHandler* handler)
{
    if(handler->_ready & ~handler->_disabled & handler->_registered)
    {
        _readyHandlers.insert(make_pair(ICE_GET_SHARED_FROM_THIS(handler), SocketOperationNone));
        wakeup();
    }
    else
    {
        map<EventHandlerPtr, SocketOperation>::iterator p = _readyHandlers.find(ICE_GET_SHARED_FROM_THIS(handler));
        if(p != _readyHandlers.end())
        {
            _readyHandlers.erase(p);
        }
    }
}

void
Selector::updateSelector()
{
#if defined(ICE_USE_KQUEUE)
    int rs = kevent(_queueFd, &_changes[0], _changes.size(), &_changes[0], _changes.size(), &zeroTimeout);
    if(rs < 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceUtilInternal::errorToString(IceInternal::getSocketErrno());
    }
    else
    {
        for(int i = 0; i < rs; ++i)
        {
            //
            // Check for errors, we ignore EINPROGRESS that started showing up with macOS Sierra
            // and which occurs when another thread removes the FD from the kqueue (see ICE-7419).
            //
            if(_changes[i].flags & EV_ERROR && _changes[i].data != EINPROGRESS)
            {
                Ice::Error out(_instance->initializationData().logger);
                out << "error while updating selector:\n" << IceUtilInternal::errorToString(_changes[i].data);
            }
        }
    }
    _changes.clear();
#elif !defined(ICE_USE_EPOLL)
    assert(!_selecting);

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
#endif
}

void
Selector::updateSelectorForEventHandler(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
#if defined(ICE_USE_EPOLL)
    SocketOperation previous = handler->_registered;
    previous = static_cast<SocketOperation>(previous & ~add);
    previous = static_cast<SocketOperation>(previous | remove);
    SOCKET fd = handler->getNativeInfo()->fd();
    assert(fd != INVALID_SOCKET);
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
#elif defined(ICE_USE_KQUEUE)
    SOCKET fd = handler->getNativeInfo()->fd();
    assert(fd != INVALID_SOCKET);
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
#else
    _changes.push_back(make_pair(handler, static_cast<SocketOperation>(handler->_registered & ~handler->_disabled)));
    wakeup();
#endif
    checkReady(handler);
}

#elif defined(ICE_USE_CFSTREAM)

namespace
{

void selectorInterrupt(void* info)
{
    reinterpret_cast<Selector*>(info)->processInterrupt();
}

void eventHandlerSocketCallback(CFSocketRef, CFSocketCallBackType callbackType, CFDataRef, const void* d, void* info)
{
    if(callbackType == kCFSocketReadCallBack)
    {
        reinterpret_cast<EventHandlerWrapper*>(info)->readyCallback(SocketOperationRead);
    }
    else if(callbackType == kCFSocketWriteCallBack)
    {
        reinterpret_cast<EventHandlerWrapper*>(info)->readyCallback(SocketOperationWrite);
    }
    else if(callbackType == kCFSocketConnectCallBack)
    {
        reinterpret_cast<EventHandlerWrapper*>(info)->readyCallback(SocketOperationConnect,
                                                                    d ? *reinterpret_cast<const SInt32*>(d) : 0);
    }
}

class SelectorHelperThread : public IceUtil::Thread
{
public:

    SelectorHelperThread(Selector& selector) : _selector(selector)
    {
    }

    virtual void run()
    {
        _selector.run();
    }

private:

    Selector& _selector;
};

CFOptionFlags
toCFCallbacks(SocketOperation op)
{
    CFOptionFlags cbs = 0;
    if(op & SocketOperationRead)
    {
        cbs |= kCFSocketReadCallBack;
    }
    if(op & SocketOperationWrite)
    {
        cbs |= kCFSocketWriteCallBack;
    }
    if(op & SocketOperationConnect)
    {
        cbs |= kCFSocketConnectCallBack;
    }
    return cbs;
}

}

EventHandlerWrapper::EventHandlerWrapper(EventHandler* handler, Selector& selector) :
    _handler(ICE_GET_SHARED_FROM_THIS(handler)),
    _streamNativeInfo(StreamNativeInfoPtr::dynamicCast(handler->getNativeInfo())),
    _selector(selector),
    _ready(SocketOperationNone),
    _finish(false)
{
    if(_streamNativeInfo)
    {
        _streamNativeInfo->initStreams(this);
    }
    else if(handler->getNativeInfo())
    {
        SOCKET fd = handler->getNativeInfo()->fd();
        CFSocketContext ctx = { 0, this, 0, 0, 0 };
        _socket.reset(CFSocketCreateWithNative(kCFAllocatorDefault,
                                               fd,
                                               kCFSocketReadCallBack |
                                               kCFSocketWriteCallBack |
                                               kCFSocketConnectCallBack,
                                               eventHandlerSocketCallback,
                                               &ctx));

        // Disable automatic re-enabling of callbacks and closing of the native socket.
        CFSocketSetSocketFlags(_socket.get(), 0);
        CFSocketDisableCallBacks(_socket.get(), kCFSocketReadCallBack | kCFSocketWriteCallBack | kCFSocketConnectCallBack);
        _source.reset(CFSocketCreateRunLoopSource(kCFAllocatorDefault, _socket.get(), 0));
    }
}

EventHandlerWrapper::~EventHandlerWrapper()
{
}

void
EventHandlerWrapper::updateRunLoop()
{
    SocketOperation op = _handler->_registered;
    assert(!op || !_finish);

    if(_socket)
    {
        CFSocketDisableCallBacks(_socket.get(), kCFSocketReadCallBack | kCFSocketWriteCallBack | kCFSocketConnectCallBack);
        if(op)
        {
            CFSocketEnableCallBacks(_socket.get(), toCFCallbacks(op));
        }

        if(op && !CFRunLoopContainsSource(CFRunLoopGetCurrent(), _source.get(), kCFRunLoopDefaultMode))
        {
            CFRunLoopAddSource(CFRunLoopGetCurrent(), _source.get(), kCFRunLoopDefaultMode);
        }
        else if(!op && CFRunLoopContainsSource(CFRunLoopGetCurrent(), _source.get(), kCFRunLoopDefaultMode))
        {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), _source.get(), kCFRunLoopDefaultMode);
        }

        if(_finish)
        {
            CFSocketInvalidate(_socket.get());
        }
    }
    else
    {
        SocketOperation readyOp = _streamNativeInfo->registerWithRunLoop(op);
        if(!(op & (SocketOperationWrite | SocketOperationConnect)) || _ready & SocketOperationWrite)
        {
            _streamNativeInfo->unregisterFromRunLoop(SocketOperationWrite, false);
        }

        if(!(op & (SocketOperationRead | SocketOperationConnect)) || _ready & SocketOperationRead)
        {
            _streamNativeInfo->unregisterFromRunLoop(SocketOperationRead, false);
        }

        if(readyOp)
        {
            ready(readyOp, 0);
        }

        if(_finish)
        {
            _streamNativeInfo->closeStreams();
        }
    }
}

void
EventHandlerWrapper::readyCallback(SocketOperation op, int error)
{
    _selector.ready(this, op, error);
}

void
EventHandlerWrapper::ready(SocketOperation op, int error)
{
    if(!_socket)
    {
        //
        // Unregister the stream from the runloop as soon as we got the callback. This is
        // required to allow thread pool thread to perform read/write operations on the
        // stream (which can't be used from another thread than the run loop thread if
        // it's registered with a run loop).
        //
        op = _streamNativeInfo->unregisterFromRunLoop(op, error != 0);
    }

    op = static_cast<SocketOperation>(_handler->_registered & op);
    if(!op || _ready & op)
    {
        return;
    }

    if(_socket)
    {
        if(op & SocketOperationConnect)
        {
            _streamNativeInfo->setConnectError(error);
        }
    }

    _ready = static_cast<SocketOperation>(_ready | op);
    checkReady();
}

bool
EventHandlerWrapper::checkReady()
{
    if((_ready | _handler->_ready) & ~_handler->_disabled & _handler->_registered)
    {
        _selector.addReadyHandler(this);
        return false;
    }
    else
    {
        return _handler->getNativeInfo() && !_finish;
    }
}

SocketOperation
EventHandlerWrapper::readyOp()
{
    assert(!(~_handler->_registered & _ready));
    SocketOperation op = static_cast<SocketOperation>(~_handler->_disabled & (_ready | _handler->_ready));
    _ready = static_cast<SocketOperation>(~op & _ready);
    return op;
}

bool
EventHandlerWrapper::update(SocketOperation remove, SocketOperation add)
{
    SocketOperation previous = _handler->_registered;
    _handler->_registered = static_cast<SocketOperation>(_handler->_registered & ~remove);
    _handler->_registered = static_cast<SocketOperation>(_handler->_registered | add);
    if(previous == _handler->_registered)
    {
        return false;
    }

    // Clear ready flags which might not be valid anymore.
    _ready = static_cast<SocketOperation>(_ready & _handler->_registered);
    return _handler->getNativeInfo();
}

bool
EventHandlerWrapper::finish()
{
    _finish = true;
    _ready = SocketOperationNone;
    _handler->_registered = SocketOperationNone;
    return _handler->getNativeInfo();
}

Selector::Selector(const InstancePtr& instance) : _instance(instance), _destroyed(false)
{
    CFRunLoopSourceContext ctx;
    memset(&ctx, 0, sizeof(CFRunLoopSourceContext));
    ctx.info = this;
    ctx.perform = selectorInterrupt;
    _source.reset(CFRunLoopSourceCreate(0, 0, &ctx));
    _runLoop = 0;

    _thread = new SelectorHelperThread(*this);
    _thread->start();

    Lock sync(*this);
    while(!_runLoop)
    {
        wait();
    }
}

Selector::~Selector()
{
}

void
Selector::destroy()
{
    {
        Lock sync(*this);

        //
        // Make sure any pending changes are processed to ensure remaining
        // streams/sockets are closed.
        //
        _destroyed = true;
        CFRunLoopSourceSignal(_source.get());
        CFRunLoopWakeUp(_runLoop);

        while(!_changes.empty())
        {
            CFRunLoopSourceSignal(_source.get());
            CFRunLoopWakeUp(_runLoop);

            wait();
        }
    }

    _thread->getThreadControl().join();
    _thread = 0;

    Lock sync(*this);
    _source.reset(0);

    //assert(_wrappers.empty());
    _readyHandlers.clear();
    _selectedHandlers.clear();
}

void
Selector::initialize(EventHandler* handler)
{
    Lock sync(*this);
    _wrappers[handler] = new EventHandlerWrapper(handler, *this);
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    Lock sync(*this);
    const EventHandlerWrapperPtr& wrapper = _wrappers[handler];
    if(wrapper->update(remove, add))
    {
        _changes.insert(wrapper);
        notify();
    }
}

void
Selector::enable(EventHandler* handler, SocketOperation op)
{
    Lock sync(*this);
    if(!(handler->_disabled & op))
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled & ~op);

    if(handler->_registered & op)
    {
        _wrappers[handler]->checkReady();
    }
}

void
Selector::disable(EventHandler* handler, SocketOperation op)
{
    Lock sync(*this);
    if(handler->_disabled & op)
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled | op);
}

bool
Selector::finish(EventHandler* handler, bool closeNow)
{
    Lock sync(*this);
    std::map<EventHandler*, EventHandlerWrapperPtr>::iterator p = _wrappers.find(handler);
    assert(p != _wrappers.end());
    EventHandlerWrapperPtr wrapper = p->second;
    if(wrapper->finish())
    {
        _changes.insert(wrapper);
        notify();
    }
    _wrappers.erase(p);
    return closeNow;
}

void
Selector::ready(EventHandler* handler, SocketOperation status, bool value)
{
    if(((handler->_ready & status) != 0) == value)
    {
        return; // Nothing to do if ready state already correctly set.
    }

    if(value)
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready | status);
    }
    else
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready & ~status);
    }

    Lock sync(*this);
    std::map<EventHandler*, EventHandlerWrapperPtr>::iterator p = _wrappers.find(handler);
    assert(p != _wrappers.end());
    p->second->checkReady();
}

void
Selector::startSelect()
{
    Lock sync(*this);

    //
    // Re-enable callbacks for previously selected handlers.
    //
    vector<pair<EventHandlerWrapperPtr, SocketOperation> >::const_iterator p;
    for(p = _selectedHandlers.begin(); p != _selectedHandlers.end(); ++p)
    {
        if(p->first->checkReady())
        {
            _changes.insert(p->first);
        }
    }
    _selectedHandlers.clear();
}

void
Selector::finishSelect(std::vector<std::pair<EventHandler*, SocketOperation> >& handlers)
{
    Lock sync(*this);
    handlers.clear();
    for(set<EventHandlerWrapperPtr>::const_iterator p = _readyHandlers.begin(); p != _readyHandlers.end(); ++p)
    {
        SocketOperation op = (*p)->readyOp();
        if(op)
        {
            _selectedHandlers.push_back(pair<EventHandlerWrapperPtr, SocketOperation>(*p, op));
            handlers.push_back(pair<EventHandler*, SocketOperation>((*p)->_handler.get(), op));
        }
    }
    _readyHandlers.clear();
}

void
Selector::select(int timeout)
{
    //
    // Wait for handlers to be ready.
    //
    Lock sync(*this);
    while(!_destroyed)
    {
        while(!_changes.empty())
        {
            CFRunLoopSourceSignal(_source.get());
            CFRunLoopWakeUp(_runLoop);

            wait();
        }

        if(_readyHandlers.empty())
        {
            if(timeout > 0)
            {
                if(!timedWait(IceUtil::Time::seconds(timeout)))
                {
                    break;
                }
            }
            else
            {
                wait();
            }
        }

        if(_changes.empty())
        {
            break;
        }
    }
}

void
Selector::processInterrupt()
{
    Lock sync(*this);
    if(!_changes.empty())
    {
        for(set<EventHandlerWrapperPtr>::const_iterator p = _changes.begin(); p != _changes.end(); ++p)
        {
            (*p)->updateRunLoop();
        }
        _changes.clear();
        notify();
    }
    if(_destroyed)
    {
        CFRunLoopStop(_runLoop);
    }
}

void
Selector::run()
{
    {
        Lock sync(*this);
        _runLoop = CFRunLoopGetCurrent();
        notify();
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), _source.get(), kCFRunLoopDefaultMode);
    CFRunLoopRun();
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), _source.get(), kCFRunLoopDefaultMode);
}

void
Selector::ready(EventHandlerWrapper* wrapper, SocketOperation op, int error)
{
    Lock sync(*this);
    wrapper->ready(op, error);
}

void
Selector::addReadyHandler(EventHandlerWrapper* wrapper)
{
    // Called from ready()
    _readyHandlers.insert(wrapper);
    if(_readyHandlers.size() == 1)
    {
        notify();
    }
}

#endif
