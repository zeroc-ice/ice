// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    EventHandlerPtr h = handler;
    handler->__incRef();
    handler->getNativeInfo()->setCompletedHandler(
        ref new SocketOperationCompletedHandler([=](int operation)
                                                {
                                                    //
                                                    // Use the reference counted handler to ensure it's not
                                                    // destroyed as long as the callback lambda exists.
                                                    //
                                                    completed(h, static_cast<SocketOperation>(operation));
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
    handler->_finish = false; // Ensures that finished() is only called once on the event handler.
    handler->__decRef();
}

IceInternal::EventHandlerPtr
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
    IceInternal::EventHandlerPtr handler = _events.front().handler;
    const SelectEvent& event = _events.front();
    status = event.status;
    _events.pop_front();
    return handler;
}

void
Selector::completed(const IceInternal::EventHandlerPtr& handler, SocketOperation op)
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
    handler->getNativeInfo()->initialize(_handle, reinterpret_cast<ULONG_PTR>(handler));
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
Selector::finish(IceInternal::EventHandler* handler)
{
    handler->_registered = SocketOperationNone;
    handler->__decRef();
}

EventHandler*
Selector::getNextHandler(SocketOperation& status, DWORD& count, int& error, int timeout)
{
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
        count = SOCKET_ERROR;
        error = WSAGetLastError();
        return reinterpret_cast<EventHandler*>(key);
    }

    assert(ol);
    AsyncInfo* info = static_cast<AsyncInfo*>(ol);
    status = info->status;
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

bool
Selector::finish(EventHandler* handler, bool closeNow)
{
    if(handler->_registered)
    {
        update(handler, handler->_registered, SocketOperationNone);
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
#endif
    return closeNow;
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
    for(int i = 0; i < ret; ++i)
    {
        pair<EventHandler*, SocketOperation> p;
#if defined(ICE_USE_EPOLL)
        struct epoll_event& ev = _events[i];
        p.first = reinterpret_cast<EventHandler*>(ev.data.ptr);
        p.second = static_cast<SocketOperation>(((ev.events & (EPOLLIN | EPOLLERR)) ?
                                                 SocketOperationRead : SocketOperationNone) |
                                                ((ev.events & (EPOLLOUT | EPOLLERR)) ?
                                                 SocketOperationWrite : SocketOperationNone));
#else
        struct kevent& ev = _events[i];
        if(ev.flags & EV_ERROR)
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "selector returned error:\n" << IceUtilInternal::errorToString(ev.data);
            continue;
        }
        p.first = reinterpret_cast<EventHandler*>(ev.udata);
        p.second = (ev.filter == EVFILT_READ) ? SocketOperationRead : SocketOperationWrite;
#endif
        handlers.push_back(p);
    }
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

EventHandlerWrapper::EventHandlerWrapper(const EventHandlerPtr& handler, Selector& selector) :
    _handler(handler),
    _nativeInfo(StreamNativeInfoPtr::dynamicCast(handler->getNativeInfo())),
    _selector(selector),
    _ready(SocketOperationNone),
    _finish(false)
{
    if(!StreamNativeInfoPtr::dynamicCast(handler->getNativeInfo()))
    {
        SOCKET fd = handler->getNativeInfo()->fd();
        CFSocketContext ctx = { 0, this, 0, 0, 0 };
        _socket = CFSocketCreateWithNative(kCFAllocatorDefault,
                                           fd,
                                           kCFSocketReadCallBack |
                                           kCFSocketWriteCallBack |
                                           kCFSocketConnectCallBack,
                                           eventHandlerSocketCallback,
                                           &ctx);

        // Disable automatic re-enabling of callbacks and closing of the native socket.
        CFSocketSetSocketFlags(_socket, 0);
        CFSocketDisableCallBacks(_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack | kCFSocketConnectCallBack);
        _source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, _socket, 0);
    }
    else
    {
        _socket = 0;
        _source = 0;
        _nativeInfo->initStreams(this);
    }
}

EventHandlerWrapper::~EventHandlerWrapper()
{
    if(_socket)
    {
        CFRelease(_socket);
        CFRelease(_source);
    }
}

void
EventHandlerWrapper::updateRunLoop()
{
    SocketOperation op = _handler->_registered;
    assert(!op || !_finish);

    if(_socket)
    {
        CFSocketDisableCallBacks(_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack | kCFSocketConnectCallBack);
        if(op)
        {
            CFSocketEnableCallBacks(_socket, toCFCallbacks(op));
        }

        if(op && !CFRunLoopContainsSource(CFRunLoopGetCurrent(), _source, kCFRunLoopDefaultMode))
        {
            CFRunLoopAddSource(CFRunLoopGetCurrent(), _source, kCFRunLoopDefaultMode);
        }
        else if(!op && CFRunLoopContainsSource(CFRunLoopGetCurrent(), _source, kCFRunLoopDefaultMode))
        {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), _source, kCFRunLoopDefaultMode);
        }

        if(_finish)
        {
            CFSocketInvalidate(_socket);
        }
    }
    else
    {
        SocketOperation readyOp = _nativeInfo->registerWithRunLoop(op);
        if(!(op & (SocketOperationWrite | SocketOperationConnect)) || _ready & SocketOperationWrite)
        {
            _nativeInfo->unregisterFromRunLoop(SocketOperationWrite, false);
        }

        if(!(op & (SocketOperationRead | SocketOperationConnect)) || _ready & SocketOperationRead)
        {
            _nativeInfo->unregisterFromRunLoop(SocketOperationRead, false);
        }

        if(readyOp)
        {
            ready(readyOp, 0);
        }

        if(_finish)
        {
            _nativeInfo->closeStreams();
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
        op = _nativeInfo->unregisterFromRunLoop(op, error != 0);
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
            _nativeInfo->setConnectError(error);
        }
    }

    _ready = static_cast<SocketOperation>(_ready | op);
    if(!(_handler->_disabled & op))
    {
        _selector.addReadyHandler(this);
    }
}

void
EventHandlerWrapper::checkReady()
{
    if(_ready & _handler->_registered)
    {
        _selector.addReadyHandler(this);
    }
}

SocketOperation
EventHandlerWrapper::readyOp()
{
    assert(!(~_handler->_registered & _ready));
    SocketOperation op = static_cast<SocketOperation>(~_handler->_disabled & _ready);
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
    return true;
}

void
EventHandlerWrapper::finish()
{
    _finish = true;
    _ready = SocketOperationNone;
    _handler->_registered = SocketOperationNone;
}

Selector::Selector(const InstancePtr& instance) : _instance(instance), _destroyed(false)
{
    CFRunLoopSourceContext ctx;
    memset(&ctx, 0, sizeof(CFRunLoopSourceContext));
    ctx.info = this;
    ctx.perform = selectorInterrupt;
    _source = CFRunLoopSourceCreate(0, 0, &ctx);
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
    Lock sync(*this);

    //
    // Make sure any pending changes are processed to ensure remaining
    // streams/sockets are closed.
    //
    _destroyed = true;
    while(!_changes.empty())
    {
        CFRunLoopSourceSignal(_source);
        CFRunLoopWakeUp(_runLoop);

        wait();
    }

    _thread->getThreadControl().join();
    _thread = 0;

    CFRelease(_source);

    assert(_wrappers.empty());
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
    assert(wrapper);
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
    wrapper->finish();
    _wrappers.erase(p);
    _changes.insert(wrapper);
    notify();
    return closeNow;
}

void
Selector::select(std::vector<std::pair<EventHandler*, SocketOperation> >& handlers, int timeout)
{
    Lock sync(*this);

    //
    // Re-enable callbacks for previously selected handlers.
    //
    if(!_selectedHandlers.empty())
    {
        vector<pair<EventHandlerWrapperPtr, SocketOperation> >::const_iterator p;
        for(p = _selectedHandlers.begin(); p != _selectedHandlers.end(); ++p)
        {
            if(!p->first->_finish)
            {
                _changes.insert(p->first);
            }
        }
        _selectedHandlers.clear();
    }

    //
    // Wait for handlers to be ready.
    //
    handlers.clear();
    while(_selectedHandlers.empty())
    {
        while(!_changes.empty())
        {
            CFRunLoopSourceSignal(_source);
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

        if(!_changes.empty())
        {
            continue; // Make sure to process the changes first.
        }

        for(vector<EventHandlerWrapperPtr>::const_iterator p = _readyHandlers.begin(); p != _readyHandlers.end(); ++p)
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
Selector::ready(EventHandlerWrapper* wrapper, SocketOperation op, int error)
{
    Lock sync(*this);
    wrapper->ready(op, error);
}

void
Selector::addReadyHandler(EventHandlerWrapper* wrapper)
{
    // Called from ready()
    _readyHandlers.push_back(wrapper);
    if(_readyHandlers.size() == 1)
    {
        notify();
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

    CFRunLoopAddSource(CFRunLoopGetCurrent(), _source, kCFRunLoopDefaultMode);
    CFRunLoopRun();
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), _source, kCFRunLoopDefaultMode);
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

bool
Selector::finish(EventHandler* handler, bool closeNow)
{
    if(handler->_registered)
    {
        update(handler, handler->_registered, SocketOperationNone);
        return false; // Don't close now if selecting.
    }
    return closeNow;
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
