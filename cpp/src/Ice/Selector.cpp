// Copyright (c) ZeroC, Inc.

#include "Selector.h"
#include "EventHandler.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Instance.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)
#    include <dispatch/dispatch.h>
#endif

#include <chrono>
#include <thread>

using namespace std;
using namespace IceInternal;

#if defined(ICE_USE_KQUEUE)
namespace
{
    struct timespec zeroTimeout = {0, 0};
}
#endif

#if defined(ICE_USE_IOCP)

Selector::Selector(const InstancePtr& instance) : _instance(instance) {}

void
Selector::setup(int sizeIO)
{
    _handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, sizeIO);
    if (_handle == nullptr)
    {
        throw Ice::SocketException(__FILE__, __LINE__, GetLastError());
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
    if (!handler->getNativeInfo())
    {
        return;
    }

    SOCKET socket = handler->getNativeInfo()->fd();
    if (socket != INVALID_SOCKET)
    {
        if (CreateIoCompletionPort(
                reinterpret_cast<HANDLE>(socket),
                _handle,
                reinterpret_cast<ULONG_PTR>(handler),
                0) == nullptr)
        {
            throw Ice::SocketException(__FILE__, __LINE__, GetLastError());
        }
    }
    handler->getNativeInfo()->initialize(_handle, reinterpret_cast<ULONG_PTR>(handler));
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);
    if (add & SocketOperationRead && !(handler->_pending & SocketOperationRead))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationRead);
        completed(handler, SocketOperationRead); // Start an asynchronous read
    }
    else if (add & SocketOperationWrite && !(handler->_pending & SocketOperationWrite))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationWrite);
        completed(handler, SocketOperationWrite); // Start an asynchronous write
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
    if (((handler->_ready & status) != 0) == value)
    {
        return; // Nothing to do if ready state already correctly set.
    }

    if (value)
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready | status);
    }
    else
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready & ~status);
    }
}

EventHandler*
Selector::getNextHandler(SocketOperation& status, DWORD& count, int& error, int timeout)
{
    ULONG_PTR key;
    LPOVERLAPPED ol;
    error = ERROR_SUCCESS;

    if (!GetQueuedCompletionStatus(_handle, &count, &key, &ol, timeout > 0 ? timeout * 1000 : INFINITE))
    {
        int err = WSAGetLastError();
        if (ol == nullptr)
        {
            if (err == WAIT_TIMEOUT)
            {
                throw SelectorTimeoutException();
            }
            else
            {
                // This indicates an internal error with the IOCP completion port, we log the error and abort.
                Ice::SocketException ex(__FILE__, __LINE__, err);
                Ice::Error out(_instance->initializationData().logger);
                out << "could not dequeue packet from completion port:\n" << ex;
                std::abort();
            }
        }
        AsyncInfo* info = static_cast<AsyncInfo*>(ol);
        if (info)
        {
            status = info->status;
        }
        count = 0;
        error = WSAGetLastError();
        return reinterpret_cast<EventHandler*>(key);
    }

    AsyncInfo* info = static_cast<AsyncInfo*>(ol);
    if (info)
    {
        status = info->status;
    }
    else
    {
        status = reinterpret_cast<EventHandler*>(key)->_ready;
    }
    return reinterpret_cast<EventHandler*>(key);
}

void
Selector::completed(EventHandler* handler, SocketOperation op)
{
    AsyncInfo* info = 0;
    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if (nativeInfo)
    {
        info = nativeInfo->getAsyncInfo(op);
    }
    if (!PostQueuedCompletionStatus(_handle, 0, reinterpret_cast<ULONG_PTR>(handler), info))
    {
        throw Ice::SocketException(__FILE__, __LINE__, GetLastError());
    }
}

#elif defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)

Selector::Selector(InstancePtr instance) : _instance(std::move(instance))
{
    SOCKET fds[2];
    createPipe(fds);
    _fdIntrRead = fds[0];
    _fdIntrWrite = fds[1];
    _selecting = false;

#    if defined(ICE_USE_EPOLL)
    _events.resize(256);
    _queueFd = epoll_create(1);
    if (_queueFd < 0)
    {
        throw Ice::SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
    }

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.data.ptr = nullptr;
    event.events = EPOLLIN;
    if (epoll_ctl(_queueFd, EPOLL_CTL_ADD, _fdIntrRead, &event) != 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceInternal::errorToString(IceInternal::getSocketErrno());
    }
#    else // ICE_USE_KQUEUE
    _events.resize(256);
    _queueFd = kqueue();
    if (_queueFd < 0)
    {
        throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    struct kevent ev;
    EV_SET(&ev, _fdIntrRead, EVFILT_READ, EV_ADD, 0, 0, 0);
    int rs = kevent(_queueFd, &ev, 1, nullptr, 0, nullptr);
    if (rs < 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceInternal::errorToString(IceInternal::getSocketErrno());
    }
#    endif
}

void
Selector::destroy()
{
    try
    {
        closeSocket(_queueFd);
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }

    try
    {
        closeSocket(_fdIntrWrite);
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "exception in selector while calling closeSocket():\n" << ex;
    }

    try
    {
        closeSocket(_fdIntrRead);
    }
    catch (const Ice::LocalException& ex)
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
    if (previous == handler->_registered)
    {
        return;
    }
    checkReady(handler);

    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if (nativeInfo && nativeInfo->fd() != INVALID_SOCKET)
    {
        updateSelectorForEventHandler(handler, remove, add);
    }
}

void
Selector::enable(EventHandler* handler, SocketOperation status)
{
    if (!(handler->_disabled & status))
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled & ~status);
    checkReady(handler);

    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if (!nativeInfo || nativeInfo->fd() == INVALID_SOCKET)
    {
        return;
    }

    if (handler->_registered & status)
    {
#    if defined(ICE_USE_EPOLL)
        SOCKET fd = nativeInfo->fd();
        auto previous = static_cast<SocketOperation>(handler->_registered & ~(handler->_disabled | status));
        auto newStatus = static_cast<SocketOperation>(handler->_registered & ~handler->_disabled);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.data.ptr = handler;
        if (newStatus & SocketOperationRead)
        {
            event.events |= EPOLLIN;
        }
        if (newStatus & SocketOperationWrite)
        {
            event.events |= EPOLLOUT;
        }
        if (epoll_ctl(_queueFd, previous ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &event) != 0)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while updating selector:\n" << IceInternal::errorToString(IceInternal::getSocketErrno());
        }
#    else // ICE_USE_KQUEUE
        struct kevent ev;
        SOCKET fd = handler->getNativeInfo()->fd();
        EV_SET(&ev, fd, status == SocketOperationRead ? EVFILT_READ : EVFILT_WRITE, EV_ENABLE, 0, 0, handler);
        _changes.push_back(ev);
        if (_selecting)
        {
            updateSelector();
        }
#    endif
    }
}

void
Selector::disable(EventHandler* handler, SocketOperation status)
{
    if (handler->_disabled & status)
    {
        return;
    }
    handler->_disabled = static_cast<SocketOperation>(handler->_disabled | status);
    checkReady(handler);

    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if (!nativeInfo || nativeInfo->fd() == INVALID_SOCKET)
    {
        return;
    }

    if (handler->_registered & status)
    {
#    if defined(ICE_USE_EPOLL)
        SOCKET fd = nativeInfo->fd();
        auto newStatus = static_cast<SocketOperation>(handler->_registered & ~handler->_disabled);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.data.ptr = handler;
        if (newStatus & SocketOperationRead)
        {
            event.events |= EPOLLIN;
        }
        if (newStatus & SocketOperationWrite)
        {
            event.events |= EPOLLOUT;
        }
        if (epoll_ctl(_queueFd, newStatus ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, fd, &event) != 0)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "error while updating selector:\n" << IceInternal::errorToString(IceInternal::getSocketErrno());
        }
#    else // ICE_USE_KQUEUE
        SOCKET fd = nativeInfo->fd();
        struct kevent ev;
        EV_SET(&ev, fd, status == SocketOperationRead ? EVFILT_READ : EVFILT_WRITE, EV_DISABLE, 0, 0, handler);
        _changes.push_back(ev);
        if (_selecting)
        {
            updateSelector();
        }
#    endif
    }
}

bool
Selector::finish(EventHandler* handler, bool closeNow)
{
    if (handler->_registered)
    {
        update(handler, handler->_registered, SocketOperationNone);
    }

#    if defined(ICE_USE_KQUEUE)
    if (closeNow && !_changes.empty())
    {
        //
        // Update selector now to remove the FD from the kqueue if
        // we're going to close it now. This isn't necessary for
        // epoll since we always update the epoll FD immediately.
        //
        updateSelector();
    }
#    endif

    return closeNow;
}

void
Selector::ready(EventHandler* handler, SocketOperation status, bool value)
{
    if (((handler->_ready & status) != 0) == value)
    {
        return; // Nothing to do if ready state already correctly set.
    }

    if (status & SocketOperationConnect)
    {
        NativeInfoPtr nativeInfo = handler->getNativeInfo();
        if (nativeInfo && nativeInfo->newFd() && handler->_registered)
        {
            // If new FD is set after connect, register the FD with the selector.
            updateSelectorForEventHandler(handler, SocketOperationNone, handler->_registered);
        }
    }

    if (value)
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
    if (_selecting && !_interrupted)
    {
        char c = 0;
        while (true)
        {
            if (::write(_fdIntrWrite, &c, 1) == SOCKET_ERROR)
            {
                if (interrupted())
                {
                    continue;
                }

                throw Ice::SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            break;
        }
        _interrupted = true;
    }
}

void
Selector::startSelect()
{
    if (_interrupted)
    {
        char c;
        while (true)
        {
            ssize_t ret = ::read(_fdIntrRead, &c, 1);
            if (ret == SOCKET_ERROR)
            {
                if (interrupted())
                {
                    continue;
                }
                throw Ice::SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            break;
        }
        _interrupted = false;
    }

#    if defined(ICE_USE_KQUEUE)
    if (!_changes.empty())
    {
        updateSelector();
    }
#    endif
    _selecting = true;

    //
    // If there are ready handlers, don't block in select, just do a non-blocking
    // select to retrieve new ready handlers from the selector.
    //
    _selectNow = !_readyHandlers.empty();
}

void
Selector::finishSelect(vector<pair<EventHandler*, SocketOperation>>& handlers)
{
    _selecting = false;

    assert(handlers.empty());

    for (int i = 0; i < _count; ++i)
    {
        pair<EventHandler*, SocketOperation> p;

#    if defined(ICE_USE_EPOLL)
        struct epoll_event& ev = _events[i];
        p.first = reinterpret_cast<EventHandler*>(ev.data.ptr);
        p.second = static_cast<SocketOperation>(
            ((ev.events & (EPOLLIN | EPOLLERR)) ? SocketOperationRead : SocketOperationNone) |
            ((ev.events & (EPOLLOUT | EPOLLERR)) ? SocketOperationWrite : SocketOperationNone));
#    else // ICE_USE_KQUEUE
        struct kevent& ev = _events[static_cast<size_t>(i)];
        if (ev.flags & EV_ERROR)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "selector returned error:\n" << IceInternal::errorToString(static_cast<int>(ev.data));
            continue;
        }
        p.first = reinterpret_cast<EventHandler*>(ev.udata);
        p.second = (ev.filter == EVFILT_READ) ? SocketOperationRead : SocketOperationWrite;
#    endif
        if (!p.first)
        {
            continue; // Interrupted
        }

        auto q = _readyHandlers.find(p.first->shared_from_this());

        if (q != _readyHandlers.end()) // Handler will be added by the loop below
        {
            q->second = p.second; // We just remember which operations are ready here.
        }
        else
        {
            handlers.push_back(p);
        }
    }

    for (auto& readyHandler : _readyHandlers)
    {
        pair<EventHandler*, SocketOperation> p;
        p.first = readyHandler.first.get();
        p.second = static_cast<SocketOperation>(p.first->_ready & ~p.first->_disabled & p.first->_registered);
        p.second = static_cast<SocketOperation>(p.second | readyHandler.second);
        if (p.second)
        {
            handlers.push_back(p);
        }

        //
        // Reset the operation, it's only used by this method to temporarily store the socket status
        // return by the select operation above.
        //
        readyHandler.second = SocketOperationNone;
    }
}

void
Selector::select(int timeout)
{
    if (_selectNow)
    {
        timeout = 0;
    }
    else if (timeout > 0)
    {
        // kqueue uses seconds, epoll uses milliseconds
#    ifdef ICE_USE_EPOLL
        timeout = timeout * 1000;
#    endif
    }
    else
    {
        timeout = -1;
    }

    int spuriousWakeup = 0;
    while (true)
    {
#    if defined(ICE_USE_EPOLL)
        _count = epoll_wait(_queueFd, _events.data(), _events.size(), timeout);
#    else // ICE_USE_KQUEUE
        assert(!_events.empty());
        if (timeout >= 0)
        {
            timespec ts{.tv_sec = timeout, .tv_nsec = 0};
            _count = kevent(_queueFd, nullptr, 0, _events.data(), static_cast<int>(_events.size()), &ts);
        }
        else
        {
            _count = kevent(_queueFd, nullptr, 0, _events.data(), static_cast<int>(_events.size()), nullptr);
        }
#    endif

        if (_count == SOCKET_ERROR)
        {
            if (interrupted())
            {
                continue;
            }

            // This indicates an internal error with the selector, we log the error and abort.
            Ice::SocketException ex(__FILE__, __LINE__, IceInternal::getSocketErrno());
            Ice::Error out(_instance->initializationData().logger);
            out << "selector failed:\n" << ex;
            std::abort();
        }
        else if (_count == 0 && timeout < 0)
        {
            if (++spuriousWakeup > 100)
            {
                spuriousWakeup = 0;
                _instance->initializationData().logger->warning("spurious selector wakeup");
            }
            std::this_thread::sleep_for(1ms);
            continue;
        }
        break;
    }

    if (_count == 0 && !_selectNow)
    {
        throw SelectorTimeoutException();
    }
}

void
Selector::checkReady(EventHandler* handler)
{
    if (handler->_ready & ~handler->_disabled & handler->_registered)
    {
        _readyHandlers.insert(make_pair(handler->shared_from_this(), SocketOperationNone));
        wakeup();
    }
    else
    {
        auto p = _readyHandlers.find(handler->shared_from_this());
        if (p != _readyHandlers.end())
        {
            _readyHandlers.erase(p);
        }
    }
}

#    if defined(ICE_USE_KQUEUE)
void
Selector::updateSelector()
{
    int rs = kevent(
        _queueFd,
        _changes.data(),
        static_cast<int>(_changes.size()),
        _changes.data(),
        static_cast<int>(_changes.size()),
        &zeroTimeout);
    if (rs < 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceInternal::errorToString(IceInternal::getSocketErrno());
    }
    else
    {
        for (int i = 0; i < rs; ++i)
        {
            //
            // Check for errors, we ignore EINPROGRESS that started showing up with macOS Sierra
            // and which occurs when another thread removes the FD from the kqueue (see ICE-7419).
            //
            if (_changes[static_cast<size_t>(i)].flags & EV_ERROR &&
                _changes[static_cast<size_t>(i)].data != EINPROGRESS)
            {
                Ice::Error out(_instance->initializationData().logger);
                out << "error while updating selector:\n"
                    << IceInternal::errorToString(static_cast<int>(_changes[static_cast<size_t>(i)].data));
            }
        }
    }
    _changes.clear();
}
#    endif

void
Selector::updateSelectorForEventHandler(
    EventHandler* handler,
    [[maybe_unused]] SocketOperation remove,
    [[maybe_unused]] SocketOperation add)
{
#    if defined(ICE_USE_EPOLL)
    SocketOperation previous = handler->_registered;
    previous = static_cast<SocketOperation>(previous & ~add);
    previous = static_cast<SocketOperation>(previous | remove);
    SOCKET fd = handler->getNativeInfo()->fd();
    assert(fd != INVALID_SOCKET);
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.data.ptr = handler;
    SocketOperation status = handler->_registered;
    if (handler->_disabled)
    {
        status = static_cast<SocketOperation>(status & ~handler->_disabled);
        previous = static_cast<SocketOperation>(previous & ~handler->_disabled);
    }
    if (status & SocketOperationRead)
    {
        event.events |= EPOLLIN;
    }
    if (status & SocketOperationWrite)
    {
        event.events |= EPOLLOUT;
    }
    int op;
    if (!previous && status)
    {
        op = EPOLL_CTL_ADD;
    }
    else if (previous && !status)
    {
        op = EPOLL_CTL_DEL;
    }
    else if (previous == status)
    {
        return;
    }
    else
    {
        op = EPOLL_CTL_MOD;
    }
    if (epoll_ctl(_queueFd, op, fd, &event) != 0)
    {
        Ice::Error out(_instance->initializationData().logger);
        out << "error while updating selector:\n" << IceInternal::errorToString(IceInternal::getSocketErrno());
    }
#    else // ICE_USE_KQUEUE
    SOCKET fd = handler->getNativeInfo()->fd();
    assert(fd != INVALID_SOCKET);
    if (remove & SocketOperationRead)
    {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, handler);
        _changes.push_back(ev);
    }
    if (remove & SocketOperationWrite)
    {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, handler);
        _changes.push_back(ev);
    }
    if (add & SocketOperationRead)
    {
        struct kevent ev;
        EV_SET(
            &ev,
            fd,
            EVFILT_READ,
            EV_ADD | (handler->_disabled & SocketOperationRead ? EV_DISABLE : 0),
            0,
            0,
            handler);
        _changes.push_back(ev);
    }
    if (add & SocketOperationWrite)
    {
        struct kevent ev;
        EV_SET(
            &ev,
            fd,
            EVFILT_WRITE,
            EV_ADD | (handler->_disabled & SocketOperationWrite ? EV_DISABLE : 0),
            0,
            0,
            handler);
        _changes.push_back(ev);
    }
    if (_selecting)
    {
        updateSelector();
    }
#    endif
    checkReady(handler);
}

#elif defined(ICE_USE_NETWORK_FRAMEWORK)

//
// Completion-based selector for Apple Network.framework.
//
// This follows the same pattern as the IOCP selector: the ThreadPool calls
// getNextHandler() which blocks until a completion is posted. Network.framework
// dispatch blocks call completed() to post completions to the queue.
//

Selector::Selector(const InstancePtr& instance) : _instance(instance), _queue(nullptr), _semaphore(nullptr) {}

void
Selector::setup(int)
{
    _queue = dispatch_queue_create("com.zeroc.ice.network-framework", DISPATCH_QUEUE_CONCURRENT);
    _semaphore = dispatch_semaphore_create(0);
}

void
Selector::destroy()
{
    if (_queue)
    {
        dispatch_release(_queue);
        _queue = nullptr;
    }
    if (_semaphore)
    {
        dispatch_release(_semaphore);
        _semaphore = nullptr;
    }
}

void
Selector::initialize(EventHandler* handler)
{
    NativeInfoPtr nativeInfo = handler->getNativeInfo();
    if (nativeInfo)
    {
        nativeInfo->initialize(this, handler);
    }
}

void
Selector::update(EventHandler* handler, SocketOperation remove, SocketOperation add)
{
    handler->_registered = static_cast<SocketOperation>(handler->_registered & ~remove);
    handler->_registered = static_cast<SocketOperation>(handler->_registered | add);

    if (add & SocketOperationConnect && !(handler->_pending & SocketOperationConnect))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationConnect);
        completed(handler, SocketOperationConnect);
    }
    if (add & SocketOperationRead && !(handler->_pending & SocketOperationRead))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationRead);
        completed(handler, SocketOperationRead);
    }
    if (add & SocketOperationWrite && !(handler->_pending & SocketOperationWrite))
    {
        handler->_pending = static_cast<SocketOperation>(handler->_pending | SocketOperationWrite);
        completed(handler, SocketOperationWrite);
    }
}

void
Selector::finish(EventHandler* handler)
{
    handler->_registered = SocketOperationNone;
    handler->_finish = false;
}

void
Selector::ready(EventHandler* handler, SocketOperation status, bool value)
{
    if (((handler->_ready & status) != 0) == value)
    {
        return;
    }

    if (value)
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready | status);
    }
    else
    {
        handler->_ready = static_cast<SocketOperation>(handler->_ready & ~status);
    }
}

EventHandler*
Selector::getNextHandler(SocketOperation& status, size_t& count, int& error, int timeout)
{
    long result;
    if (timeout > 0)
    {
        result = dispatch_semaphore_wait(
            _semaphore,
            dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(timeout) * static_cast<int64_t>(NSEC_PER_SEC)));
    }
    else
    {
        result = dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
    }

    if (result != 0)
    {
        throw SelectorTimeoutException();
    }

    lock_guard lock(_mutex);
    assert(!_completionQueue.empty());
    CompletionEntry entry = _completionQueue.front();
    _completionQueue.pop_front();

    status = entry.operation;
    count = 0;
    error = 0;
    return entry.handler;
}

void
Selector::completed(EventHandler* handler, SocketOperation op)
{
    {
        lock_guard lock(_mutex);
        _completionQueue.push_back({handler, op});
    }
    dispatch_semaphore_signal(_semaphore);
}

#endif
