// Copyright (c) ZeroC, Inc.

#ifndef ICE_SELECTOR_H
#define ICE_SELECTOR_H

#include "EventHandlerF.h"
#include "Ice/InstanceF.h"
#include "Ice/StringUtil.h"
#include "Network.h"
#include "UniqueRef.h"

#include <condition_variable>
#include <map>

#if defined(ICE_USE_EPOLL)
#    include <sys/epoll.h>
#elif defined(ICE_USE_KQUEUE)
#    include <sys/event.h>
#elif defined(ICE_USE_NETWORK_FRAMEWORK)
#    include "apple/ObjectRef.h"

#    include <deque>
#    include <dispatch/dispatch.h>
#    include <mutex>
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

#elif defined(ICE_USE_EPOLL) || defined(ICE_USE_KQUEUE)

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
#    if defined(ICE_USE_KQUEUE)
        void updateSelector();
#    endif
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
#    else // ICE_USE_KQUEUE
        std::vector<struct kevent> _events;
        std::vector<struct kevent> _changes;
        int _queueFd;
#    endif
    };

#elif defined(ICE_USE_NETWORK_FRAMEWORK)

    //
    // Completion-based selector for Apple Network.framework.
    // Follows the same interface as the IOCP selector — the ThreadPool drives
    // the completion loop by calling getNextHandler() which blocks until a
    // Network.framework dispatch block posts a completion.
    //
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

        EventHandler* getNextHandler(SocketOperation&, size_t&, int&, int);

        void completed(EventHandler*, SocketOperation);

    private:
        const InstancePtr _instance;
        DispatchRef<dispatch_semaphore_t> _semaphore;

        struct CompletionEntry
        {
            EventHandler* handler;
            SocketOperation operation;
        };
        std::mutex _mutex;
        std::deque<CompletionEntry> _completionQueue;
        std::shared_ptr<SelectorCompletionToken> _completionToken;
    };

#endif
}

#ifdef __clang__
#    pragma clang diagnostic pop
#endif

#endif
