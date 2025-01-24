// Copyright (c) ZeroC, Inc.

#ifndef ICE_SELECTOR_H
#define ICE_SELECTOR_H

#include "EventHandlerF.h"
#include "Ice/InstanceF.h"
#include "Ice/StringUtil.h"
#include "Network.h"
#include "UniqueRef.h"

#include <map>

#if defined(ICE_USE_EPOLL)
#    include <sys/epoll.h>
#elif defined(ICE_USE_KQUEUE)
#    include <sys/event.h>
#elif defined(ICE_USE_IOCP)
// Nothing to include
#elif defined(ICE_USE_POLL)
#    include <sys/poll.h>
#endif

#include <condition_variable>

#if defined(ICE_USE_CFSTREAM)
#    include <set>
#    include <thread>

struct __CFRunLoop;
typedef struct __CFRunLoop* CFRunLoopRef;

struct __CFRunLoopSource;
typedef struct __CFRunLoopSource* CFRunLoopSourceRef;

struct __CFSocket;
typedef struct __CFSocket* CFSocketRef;
#endif

#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace IceInternal
{
    //
    // Exception raised if select times out.
    //
    class SelectorTimeoutException
    {
    };

#if defined(ICE_USE_IOCP)

    class Selector final
    {
    public:
        Selector(const InstancePtr&);

        void setup(int);
        void destroy();

        void initialize(EventHandler*);
        void update(EventHandler*, SocketOperation, SocketOperation);
        void finish(EventHandler*);

        void ready(EventHandler*, SocketOperation, bool);

        EventHandler* getNextHandler(SocketOperation&, DWORD&, int&, int);

        void completed(EventHandler*, SocketOperation);

    private:
        const InstancePtr _instance;
        HANDLE _handle;
    };

#elif defined(ICE_USE_KQUEUE) || defined(ICE_USE_EPOLL) || defined(ICE_USE_POLL)

    class Selector final
    {
    public:
        Selector(InstancePtr);

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
        void finishSelect(std::vector<std::pair<EventHandler*, SocketOperation>>&);
        void select(int);

    private:
        void wakeup();
        void checkReady(EventHandler*);
        void updateSelector();
        void updateSelectorForEventHandler(EventHandler*, SocketOperation, SocketOperation);

        const InstancePtr _instance;

        SOCKET _fdIntrRead;
        SOCKET _fdIntrWrite;
        bool _interrupted{false};
        bool _selectNow;
        int _count;
        bool _selecting;
        std::map<EventHandlerPtr, SocketOperation> _readyHandlers;

#    if defined(ICE_USE_EPOLL)
        std::vector<struct epoll_event> _events;
        int _queueFd;
#    elif defined(ICE_USE_KQUEUE)
        std::vector<struct kevent> _events;
        std::vector<struct kevent> _changes;
        int _queueFd;
#    elif defined(ICE_USE_POLL)
        std::vector<std::pair<EventHandler*, SocketOperation>> _changes;
        std::map<SOCKET, EventHandler*> _handlers;
        std::vector<struct pollfd> _pollFdSet;
#    endif
    };

#elif defined(ICE_USE_CFSTREAM)

    class Selector;

    class SelectorReadyCallback
    {
    public:
        virtual ~SelectorReadyCallback() = default;
        virtual void readyCallback(SocketOperation, int = 0) = 0;
    };

    class StreamNativeInfo : public NativeInfo
    {
    public:
        StreamNativeInfo(SOCKET fd) : NativeInfo(fd), _connectError(0) {}

        virtual void initStreams(SelectorReadyCallback*) = 0;
        virtual SocketOperation registerWithRunLoop(SocketOperation) = 0;
        virtual SocketOperation unregisterFromRunLoop(SocketOperation, bool) = 0;
        virtual void closeStreams() = 0;

        void setConnectError(int error) { _connectError = error; }

    private:
        int _connectError;
    };
    using StreamNativeInfoPtr = std::shared_ptr<StreamNativeInfo>;

    class EventHandlerWrapper final : public SelectorReadyCallback,
                                      public std::enable_shared_from_this<EventHandlerWrapper>
    {
    public:
        EventHandlerWrapper(EventHandler*, Selector&);
        ~EventHandlerWrapper();

        void updateRunLoop();

        void readyCallback(SocketOperation, int = 0) final;
        void ready(SocketOperation, int);

        SocketOperation readyOp();
        bool checkReady();

        bool update(SocketOperation, SocketOperation);
        bool finish();

        bool operator<(const EventHandlerWrapper& o) { return this < &o; }

    private:
        friend class Selector;

        EventHandlerPtr _handler;
        StreamNativeInfoPtr _streamNativeInfo;
        Selector& _selector;
        SocketOperation _ready;
        bool _finish;
        IceInternal::UniqueRef<CFSocketRef> _socket;
        IceInternal::UniqueRef<CFRunLoopSourceRef> _source;
    };
    using EventHandlerWrapperPtr = std::shared_ptr<EventHandlerWrapper>;

    class Selector final
    {
    public:
        Selector(const InstancePtr&);

        void destroy();

        void initialize(EventHandler*);
        void update(EventHandler*, SocketOperation, SocketOperation);
        void enable(EventHandler*, SocketOperation);
        void disable(EventHandler*, SocketOperation);
        bool finish(EventHandler*, bool);

        void ready(EventHandler*, SocketOperation, bool);

        void startSelect();
        void finishSelect(std::vector<std::pair<EventHandler*, SocketOperation>>&);
        void select(int);

        void processInterrupt();
        void run();

    private:
        void ready(EventHandlerWrapper*, SocketOperation, int = 0);
        void addReadyHandler(EventHandlerWrapperPtr);

        friend class EventHandlerWrapper;

        InstancePtr _instance;
        std::thread _thread;
        CFRunLoopRef _runLoop;
        IceInternal::UniqueRef<CFRunLoopSourceRef> _source;
        bool _destroyed;

        std::set<EventHandlerWrapperPtr> _changes;

        std::set<EventHandlerWrapperPtr> _readyHandlers;
        std::vector<std::pair<EventHandlerWrapperPtr, SocketOperation>> _selectedHandlers;
        std::map<EventHandler*, EventHandlerWrapperPtr> _wrappers;
        std::recursive_mutex _mutex;
        std::condition_variable_any _conditionVariable;
    };

#endif
}

#ifdef __clang__
#    pragma clang diagnostic pop
#endif

#endif
