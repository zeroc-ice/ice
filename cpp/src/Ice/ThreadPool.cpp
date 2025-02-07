// Copyright (c) ZeroC, Inc.

#include "ThreadPool.h"
#include "EventHandler.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Instance.h"
#include "Network.h"
#include "ObjectAdapterFactory.h"
#include "PropertyUtil.h"
#include "TraceLevels.h"

#if defined(__FreeBSD__)
#    include <sys/sysctl.h>
#endif

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace
{
    //
    // Exception raised by the thread pool work queue when the thread pool
    // is destroyed.
    //
    class ThreadPoolDestroyedException
    {
    };
}

IceInternal::ThreadPoolWorkQueue::ThreadPoolWorkQueue(ThreadPool& threadPool) : _threadPool(threadPool)
{
    _registered = SocketOperationRead;
}

void
IceInternal::ThreadPoolWorkQueue::destroy()
{
    // lock_guard lock(_mutex); Called with the thread pool locked
    assert(!_destroyed);
    _destroyed = true;
#if defined(ICE_USE_IOCP)
    _threadPool._selector.completed(this, SocketOperationRead);
#else
    _threadPool._selector.ready(this, SocketOperationRead, true);
#endif
}

void
IceInternal::ThreadPoolWorkQueue::queue(function<void(ThreadPoolCurrent&)> item)
{
    // lock_guard lock(_mutex); Called with the thread pool locked
    _workItems.push_back(std::move(item));
#if defined(ICE_USE_IOCP)
    _threadPool._selector.completed(this, SocketOperationRead);
#else
    if (_workItems.size() == 1)
    {
        _threadPool._selector.ready(this, SocketOperationRead, true);
    }
#endif
}

#if defined(ICE_USE_IOCP)
bool
IceInternal::ThreadPoolWorkQueue::startAsync(SocketOperation)
{
    assert(false);
    return false;
}

bool
IceInternal::ThreadPoolWorkQueue::finishAsync(SocketOperation)
{
    assert(false);
    return false;
}
#endif

void
IceInternal::ThreadPoolWorkQueue::message(ThreadPoolCurrent& current)
{
    function<void(ThreadPoolCurrent&)> workItem;
    {
        lock_guard lock(_threadPool._mutex);
        if (!_workItems.empty())
        {
            workItem = std::move(_workItems.front());
            _workItems.pop_front();
        }
#if defined(ICE_USE_IOCP)
        else
        {
            assert(_destroyed);
            _threadPool._selector.completed(this, SocketOperationRead);
        }
#else
        if (_workItems.empty() && !_destroyed)
        {
            _threadPool._selector.ready(this, SocketOperationRead, false);
        }
#endif
    }

    if (workItem)
    {
        workItem(current);
    }
    else
    {
        assert(_destroyed);
        current.ioCompleted();
        throw ThreadPoolDestroyedException();
    }
}

void
IceInternal::ThreadPoolWorkQueue::finished(ThreadPoolCurrent&, bool)
{
    assert(false);
}

string
IceInternal::ThreadPoolWorkQueue::toString() const
{
    return "work queue";
}

NativeInfoPtr
IceInternal::ThreadPoolWorkQueue::getNativeInfo()
{
    return nullptr;
}

ThreadPoolPtr
IceInternal::ThreadPool::create(const InstancePtr& instance, const string& prefix, int timeout)
{
    auto threadPool = std::shared_ptr<ThreadPool>(new ThreadPool(instance, prefix, timeout));
    threadPool->initialize();
    return threadPool;
}

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance, string prefix, int timeout)
    : _instance(instance),
      _executor(_instance->initializationData().executor),
      _prefix(std::move(prefix)),
      _selector(instance),
      _serialize(_instance->initializationData().properties->getPropertyAsInt(_prefix + ".Serialize") > 0),
      _serverIdleTime(timeout)
#if !defined(ICE_USE_IOCP)
      ,
      _nextHandler(_handlers.end())
#endif
{
    // Check for unknown thread pool properties
    validatePropertiesWithPrefix(
        _prefix,
        _instance->initializationData().properties,
        &IceInternal::PropertyNames::ThreadPoolProps);
}

void
IceInternal::ThreadPool::initialize()
{
    PropertiesPtr properties = _instance->initializationData().properties;
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int nProcessors = sysInfo.dwNumberOfProcessors;
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    static int ncpu[2] = {CTL_HW, HW_NCPU};
    int nProcessors;
    size_t sz = sizeof(nProcessors);
    if (sysctl(ncpu, 2, &nProcessors, &sz, 0, 0) == -1)
    {
        nProcessors = 1;
    }
#else
    int nProcessors = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    if (nProcessors == -1)
    {
        nProcessors = 1;
    }
#endif

    //
    // We use just one thread as the default. This is the fastest
    // possible setting, still allows one level of nesting, and
    // doesn't require to make the servants thread safe.
    //
    int size = properties->getPropertyAsIntWithDefault(_prefix + ".Size", 1);
    if (size < 1)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".Size < 1; Size adjusted to 1";
        size = 1;
    }

    int sizeMax = properties->getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
    if (sizeMax == -1)
    {
        sizeMax = nProcessors;
    }

    if (sizeMax < size)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeMax < " << _prefix << ".Size; SizeMax adjusted to Size (" << size << ")";
        sizeMax = size;
    }

    int sizeWarn = properties->getPropertyAsInt(_prefix + ".SizeWarn");
    if (sizeWarn != 0 && sizeWarn < size)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeWarn < " << _prefix << ".Size; adjusted SizeWarn to Size (" << size << ")";
        sizeWarn = size;
    }
    else if (sizeWarn > sizeMax)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeWarn > " << _prefix << ".SizeMax; adjusted SizeWarn to SizeMax (" << sizeMax << ")";
        sizeWarn = sizeMax;
    }

    int threadIdleTime = properties->getPropertyAsIntWithDefault(_prefix + ".ThreadIdleTime", 60);
    if (threadIdleTime < 0)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".ThreadIdleTime < 0; ThreadIdleTime adjusted to 0";
        threadIdleTime = 0;
    }

    const_cast<int&>(_size) = size;
    const_cast<int&>(_sizeMax) = sizeMax;
    const_cast<int&>(_sizeWarn) = sizeWarn;
    const_cast<int&>(_sizeIO) = min(sizeMax, nProcessors);
    const_cast<int&>(_threadIdleTime) = threadIdleTime;

#ifdef ICE_USE_IOCP
    _selector.setup(_sizeIO);
#endif

    _workQueue = make_shared<ThreadPoolWorkQueue>(*this);
    _selector.initialize(_workQueue.get());

    if (_instance->traceLevels()->threadPool >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->threadPoolCat);
        out << "creating " << _prefix << ": Size = " << _size << ", SizeMax = " << _sizeMax
            << ", SizeWarn = " << _sizeWarn;
    }

    try
    {
        for (int i = 0; i < _size; ++i)
        {
            auto thread = make_shared<EventHandlerThread>(shared_from_this(), nextThreadId());
            thread->start();
            _threads.insert(std::move(thread));
        }
    }
    catch (const Ice::Exception& ex)
    {
        {
            Error out(_instance->initializationData().logger);
            out << "cannot create thread for '" << _prefix << "':\n" << ex;
        }

        destroy();
        joinWithAllThreads();
        throw;
    }
}

IceInternal::ThreadPool::~ThreadPool() { assert(_destroyed); }

void
IceInternal::ThreadPool::destroy()
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        return;
    }
    _destroyed = true;
    _workQueue->destroy();
}

void
IceInternal::ThreadPool::updateObservers()
{
    lock_guard lock(_mutex);
    for (const auto& p : _threads)
    {
        p->updateObserver();
    }
}

void
IceInternal::ThreadPool::initialize(const EventHandlerPtr& handler)
{
    lock_guard lock(_mutex);
    assert(!_destroyed);
    _selector.initialize(handler.get());

    class ReadyCallbackI final : public ReadyCallback
    {
    public:
        ReadyCallbackI(ThreadPoolPtr threadPool, EventHandlerPtr handler)
            : _threadPool(std::move(threadPool)),
              _handler(std::move(handler))
        {
        }

        void ready(SocketOperation op, bool value) final { _threadPool->ready(_handler, op, value); }

    private:
        const ThreadPoolPtr _threadPool;
        const EventHandlerPtr _handler;
    };
    handler->getNativeInfo()->setReadyCallback(make_shared<ReadyCallbackI>(shared_from_this(), handler));
}

void
IceInternal::ThreadPool::update(const EventHandlerPtr& handler, SocketOperation remove, SocketOperation add)
{
    lock_guard lock(_mutex);
    assert(!_destroyed);

    // Don't remove what needs to be added
    remove = static_cast<SocketOperation>(remove & ~add);

    // Don't remove/add if already un-registered or registered
    remove = static_cast<SocketOperation>(handler->_registered & remove);
    add = static_cast<SocketOperation>(~handler->_registered & add);
    if (remove == add)
    {
        return;
    }

    _selector.update(handler.get(), remove, add);
}

bool
IceInternal::ThreadPool::finish(const EventHandlerPtr& handler, bool closeNow)
{
    lock_guard lock(_mutex);
    assert(!_destroyed);
#if !defined(ICE_USE_IOCP)
    closeNow = _selector.finish(handler.get(), closeNow); // This must be called before!
    _workQueue->queue(
        [handler, closeNow](ThreadPoolCurrent& current)
        {
            handler->finished(current, !closeNow);

            //
            // Break cyclic reference count.
            //
            if (handler->getNativeInfo())
            {
                handler->getNativeInfo()->setReadyCallback(nullptr);
            }
        });
    return closeNow;
#else
    UNREFERENCED_PARAMETER(closeNow);

    // If there are no pending asynchronous operations, we can call finish on the handler now.
    if (!handler->_pending)
    {
        _workQueue->queue(
            [handler](ThreadPoolCurrent& current)
            {
                handler->finished(current, false);

                //
                // Break cyclic reference count.
                //
                if (handler->getNativeInfo())
                {
                    handler->getNativeInfo()->setReadyCallback(nullptr);
                }
            });
        _selector.finish(handler.get());
    }
    else
    {
        handler->_finish = true;
    }
    return true; // Always close now to interrupt the pending call.
#endif
}

void
IceInternal::ThreadPool::ready(const EventHandlerPtr& handler, SocketOperation op, bool value)
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        return;
    }
    _selector.ready(handler.get(), op, value);
}

void
IceInternal::ThreadPool::executeFromThisThread(function<void()> call, const Ice::ConnectionPtr& connection)
{
    if (_executor)
    {
        try
        {
            _executor(std::move(call), connection);
        }
        catch (const std::exception& ex)
        {
            if (_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Executor") > 1)
            {
                Warning out(_instance->initializationData().logger);
                out << "executor exception:\n" << ex;
            }
        }
        catch (...)
        {
            if (_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Executor") > 1)
            {
                Warning out(_instance->initializationData().logger);
                out << "executor exception: unknown c++ exception";
            }
        }
    }
    else
    {
        call();
    }
}

void
IceInternal::ThreadPool::execute(function<void()> call, const Ice::ConnectionPtr& connection)
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _workQueue->queue(
        [self = shared_from_this(), call = std::move(call), connection](ThreadPoolCurrent& current)
        {
            current.ioCompleted(); // Promote new leader
            self->executeFromThisThread(call, connection);
        });
}

void
IceInternal::ThreadPool::joinWithAllThreads()
{
    assert(_destroyed);

    //
    // _threads is immutable after destroy() has been called,
    // therefore no synchronization is needed. (Synchronization
    // wouldn't be possible here anyway, because otherwise the other
    // threads would never terminate.)
    //
    for (const auto& thread : _threads)
    {
        thread->join();
    }
    _selector.destroy();
}

string
IceInternal::ThreadPool::prefix() const
{
    return _prefix;
}

void
IceInternal::ThreadPool::run(const EventHandlerThreadPtr& thread)
{
#if !defined(ICE_USE_IOCP)
    ThreadPoolCurrent current(shared_from_this(), thread);
    bool select = false;
    while (true)
    {
        if (current._handler)
        {
            try
            {
                current._handler->message(current);
            }
            catch (const ThreadPoolDestroyedException&)
            {
                lock_guard lock(_mutex);
                --_inUse;
                thread->setState(ThreadState::ThreadStateIdle);
                return;
            }
            catch (const exception& ex)
            {
                Error out(_instance->initializationData().logger);
                out << "exception in '" << _prefix << "':\n"
                    << ex << "\nevent handler: " << current._handler->toString();
            }
            catch (...)
            {
                Error out(_instance->initializationData().logger);
                out << "exception in '" << _prefix << "':\nevent handler: " << current._handler->toString();
            }
        }
        else if (select)
        {
            try
            {
                _selector.select(_serverIdleTime);
            }
            catch (const SelectorTimeoutException&)
            {
                lock_guard lock(_mutex);
                if (!_destroyed && _inUse == 0)
                {
                    _workQueue->queue([instance = _instance](ThreadPoolCurrent& shutdownCurrent)
                                      { shutdown(shutdownCurrent, instance); });
                }
                continue;
            }
        }

        {
            unique_lock lock(_mutex);
            if (!current._handler)
            {
                if (select)
                {
                    _selector.finishSelect(_handlers);
                    _nextHandler = _handlers.begin();
                    select = false;
                }
                else if (!current._leader && followerWait(current, lock))
                {
                    return; // Wait timed-out.
                }
            }
            else if (_sizeMax > 1)
            {
                if (!current._ioCompleted)
                {
                    //
                    // The handler didn't call ioCompleted() so we take care of decreasing
                    // the IO thread count now.
                    //
                    --_inUseIO;
                }
                else
                {
                    //
                    // If the handler called ioCompleted(), we re-enable the handler in
                    // case it was disabled and we decrease the number of thread in use.
                    //
                    if (_serialize && current._handler.get() != _workQueue.get())
                    {
                        _selector.enable(current._handler.get(), current.operation);
                    }
                    assert(_inUse > 0);
                    --_inUse;
                }

                if (!current._leader && followerWait(current, lock))
                {
                    return; // Wait timed-out.
                }
            }

            //
            // Get the next ready handler.
            //
            while (_nextHandler != _handlers.end() &&
                   !(_nextHandler->second & ~_nextHandler->first->_disabled & _nextHandler->first->_registered))
            {
                ++_nextHandler;
            }
            if (_nextHandler != _handlers.end())
            {
                current._ioCompleted = false;
                current._handler = _nextHandler->first->shared_from_this();
                current.operation = _nextHandler->second;
                ++_nextHandler;
                thread->setState(ThreadState::ThreadStateInUseForIO);
            }
            else
            {
                current._handler = nullptr;
            }

            if (!current._handler)
            {
                //
                // If there are no more ready handlers and there are still threads busy performing
                // IO, we give up leadership and promote another follower (which will perform the
                // select() only once all the IOs are completed). Otherwise, if there are no more
                // threads performing IOs, it's time to do another select().
                //
                if (_inUseIO > 0)
                {
                    promoteFollower(current);
                }
                else
                {
                    _handlers.clear();
                    _selector.startSelect();
                    select = true;
                    thread->setState(ThreadState::ThreadStateIdle);
                }
            }
            else if (_sizeMax > 1)
            {
                //
                // Increment the IO thread count and if there are still threads available
                // to perform IO and more handlers ready, we promote a follower.
                //
                ++_inUseIO;
                if (_nextHandler != _handlers.end() && _inUseIO < _sizeIO)
                {
                    promoteFollower(current);
                }
            }
        }
    }
#else
    ThreadPoolCurrent current(shared_from_this(), thread);
    while (true)
    {
        try
        {
            current._ioCompleted = false;
            current._handler =
                _selector.getNextHandler(current.operation, current._count, current._error, _threadIdleTime)
                    ->shared_from_this();
        }
        catch (const SelectorTimeoutException&)
        {
            if (_sizeMax > 1)
            {
                lock_guard lock(_mutex);

                if (_destroyed)
                {
                    continue;
                }
                else if (_inUse < static_cast<int>(_threads.size() - 1)) // If not the last idle thread, we can exit.
                {
                    BOOL hasIO = false;
                    GetThreadIOPendingFlag(GetCurrentThread(), &hasIO);
                    if (hasIO)
                    {
                        continue;
                    }

                    if (_instance->traceLevels()->threadPool >= 1)
                    {
                        Trace out(_instance->initializationData().logger, _instance->traceLevels()->threadPoolCat);
                        out << "shrinking " << _prefix << ": Size = " << (_threads.size() - 1);
                    }
                    _threads.erase(thread);
                    _workQueue->queue([thread](ThreadPoolCurrent&) { joinThread(thread); });
                    return;
                }
                else if (_inUse > 0)
                {
                    //
                    // If this is the last idle thread but there are still other threads
                    // busy dispatching, we go back waiting with _threadIdleTime. We only
                    // wait with _serverIdleTime when there's only one thread left.
                    //
                    continue;
                }
                assert(_threads.size() == 1);
            }

            try
            {
                current._handler =
                    _selector.getNextHandler(current.operation, current._count, current._error, _serverIdleTime)
                        ->shared_from_this();
            }
            catch (const SelectorTimeoutException&)
            {
                lock_guard lock(_mutex);
                if (!_destroyed)
                {
                    _workQueue->queue([instance = _instance](ThreadPoolCurrent& shutdownCurrent)
                                      { shutdown(shutdownCurrent, instance); });
                }
                continue;
            }
        }

        {
            lock_guard lock(_mutex);
            thread->setState(ThreadState::ThreadStateInUseForIO);
        }

        try
        {
            assert(current._handler);
            current._handler->message(current);
        }
        catch (const ThreadPoolDestroyedException&)
        {
            return;
        }
        catch (const exception& ex)
        {
            Error out(_instance->initializationData().logger);
            out << "exception in '" << _prefix << "':\n" << ex << "\nevent handler: " << current._handler->toString();
        }
        catch (...)
        {
            Error out(_instance->initializationData().logger);
            out << "exception in '" << _prefix << "':\nevent handler: " << current._handler->toString();
        }

        {
            lock_guard lock(_mutex);
            if (_sizeMax > 1 && current._ioCompleted)
            {
                assert(_inUse > 0);
                --_inUse;
            }
            thread->setState(ThreadState::ThreadStateIdle);
        }
    }
#endif
}

bool
IceInternal::ThreadPool::ioCompleted(ThreadPoolCurrent& current)
{
    lock_guard lock(_mutex);

    current._ioCompleted = true; // Set the IO completed flag to specify that ioCompleted() has been called.

    current._thread->setState(ThreadState::ThreadStateInUseForUser);

    if (_sizeMax > 1)
    {
#if !defined(ICE_USE_IOCP)
        --_inUseIO;

        if (!_destroyed)
        {
            if (_serialize && current._handler.get() != _workQueue.get())
            {
                _selector.disable(current._handler.get(), current.operation);
            }
        }

        if (current._leader)
        {
            //
            // If this thread is still the leader, it's time to promote a new leader.
            //
            promoteFollower(current);
        }
        else if (_promote && (_nextHandler != _handlers.end() || _inUseIO == 0))
        {
            _conditionVariable.notify_one();
        }
#endif

        assert(_inUse >= 0);
        ++_inUse;

        if (_inUse == _sizeWarn)
        {
            Warning out(_instance->initializationData().logger);
            out << "thread pool '" << _prefix << "' is running low on threads\n"
                << "Size=" << _size << ", "
                << "SizeMax=" << _sizeMax << ", "
                << "SizeWarn=" << _sizeWarn;
        }

        if (!_destroyed)
        {
            assert(_inUse <= static_cast<int>(_threads.size()));
            if (_inUse < _sizeMax && _inUse == static_cast<int>(_threads.size()))
            {
                if (_instance->traceLevels()->threadPool >= 1)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->threadPoolCat);
                    out << "growing " << _prefix << ": Size=" << _threads.size() + 1;
                }

                try
                {
                    auto thread = make_shared<EventHandlerThread>(shared_from_this(), nextThreadId());
                    thread->start();
                    _threads.insert(std::move(thread));
                }
                catch (const Ice::Exception& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "cannot create thread for '" << _prefix << "':\n" << ex;
                }
            }
        }
    }

    return _serialize && current._handler.get() != _workQueue.get();
}

#if defined(ICE_USE_IOCP)
bool
IceInternal::ThreadPool::startMessage(ThreadPoolCurrent& current)
{
    assert(current._handler->_pending & current.operation);

    if (current._handler->_started & current.operation)
    {
        assert(!(current._handler->_completed & current.operation));
        current._handler->_completed = static_cast<SocketOperation>(current._handler->_completed | current.operation);
        current._handler->_started = static_cast<SocketOperation>(current._handler->_started & ~current.operation);

        AsyncInfo* info = current._handler->getNativeInfo()->getAsyncInfo(current.operation);
        info->count = current._count;
        info->error = current._error;

        if (!current._handler->finishAsync(current.operation)) // Returns false if the handler is finished.
        {
            current._handler->_pending = static_cast<SocketOperation>(current._handler->_pending & ~current.operation);
            if (!current._handler->_pending && current._handler->_finish)
            {
                finish(current._handler, false);
            }
            return false;
        }
    }
    else if (!(current._handler->_completed & current.operation) && (current._handler->_registered & current.operation))
    {
        assert(!(current._handler->_started & current.operation));
        if (current._handler->_ready & current.operation)
        {
            return true;
        }
        else if (!current._handler->startAsync(current.operation))
        {
            current._handler->_pending = static_cast<SocketOperation>(current._handler->_pending & ~current.operation);
            if (!current._handler->_pending && current._handler->_finish)
            {
                finish(current._handler, false);
            }
            return false;
        }
        else
        {
            current._handler->_started = static_cast<SocketOperation>(current._handler->_started | current.operation);
            return false;
        }
    }

    if (current._handler->_registered & current.operation)
    {
        assert(current._handler->_completed & current.operation);
        current._handler->_completed = static_cast<SocketOperation>(current._handler->_completed & ~current.operation);
        return true;
    }
    else
    {
        current._handler->_pending = static_cast<SocketOperation>(current._handler->_pending & ~current.operation);
        if (!current._handler->_pending && current._handler->_finish)
        {
            finish(current._handler, false);
        }
        return false;
    }
}

void
IceInternal::ThreadPool::finishMessage(ThreadPoolCurrent& current)
{
    if (current._handler->_registered & current.operation && !current._handler->_finish)
    {
        assert(!(current._handler->_completed & current.operation));
        if (current._handler->_ready & current.operation)
        {
            _selector.completed(current._handler.get(), current.operation);
        }
        else if (!current._handler->startAsync(current.operation))
        {
            current._handler->_pending = static_cast<SocketOperation>(current._handler->_pending & ~current.operation);
        }
        else
        {
            assert(current._handler->_pending & current.operation);
            current._handler->_started = static_cast<SocketOperation>(current._handler->_started | current.operation);
        }
    }
    else
    {
        current._handler->_pending = static_cast<SocketOperation>(current._handler->_pending & ~current.operation);
    }

    if (!current._handler->_pending && current._handler->_finish)
    {
        finish(current._handler, false);
    }
}
#else
void
IceInternal::ThreadPool::promoteFollower(ThreadPoolCurrent& current)
{
    assert(!_promote && current._leader);
    _promote = true;
    if (_inUseIO < _sizeIO && (_nextHandler != _handlers.end() || _inUseIO == 0))
    {
        _conditionVariable.notify_one();
    }
    current._leader = false;
}

bool
IceInternal::ThreadPool::followerWait(ThreadPoolCurrent& current, unique_lock<mutex>& lock)
{
    assert(!current._leader);

    current._thread->setState(ThreadState::ThreadStateIdle);

    //
    // It's important to clear the handler before waiting to make sure that
    // resources for the handler are released now if it's finished.
    //
    current._handler = nullptr;

    //
    // Wait to be promoted and for all the IO threads to be done.
    //
    while (!_promote || _inUseIO == _sizeIO || (_nextHandler == _handlers.end() && _inUseIO > 0))
    {
        if (_threadIdleTime)
        {
            if (_conditionVariable.wait_for(lock, chrono::seconds(_threadIdleTime)) != cv_status::no_timeout)
            {
                if (!_destroyed &&
                    (!_promote || _inUseIO == _sizeIO || (_nextHandler == _handlers.end() && _inUseIO > 0)))
                {
                    if (_instance->traceLevels()->threadPool >= 1)
                    {
                        Trace out(_instance->initializationData().logger, _instance->traceLevels()->threadPoolCat);
                        out << "shrinking " << _prefix << ": Size=" << (_threads.size() - 1);
                    }
                    assert(_threads.size() > 1); // Can only be called by a waiting follower thread.
                    _threads.erase(current._thread);
                    _workQueue->queue([thread = current._thread](ThreadPoolCurrent&) { joinThread(thread); });
                    return true;
                }
            }
        }
        else
        {
            _conditionVariable.wait(lock);
        }
    }
    current._leader = true; // The current thread has become the leader.
    _promote = false;
    return false;
}
#endif

string
IceInternal::ThreadPool::nextThreadId()
{
    ostringstream os;
    os << _prefix << "-" << _nextThreadId++;
    return os.str();
}

void
IceInternal::ThreadPool::joinThread(const EventHandlerThreadPtr& thread)
{
    // No call to ioCompleted, this shouldn't block (and we don't want to cause a new thread to
    // be started).
    thread->join();
}

void
IceInternal::ThreadPool::shutdown(const ThreadPoolCurrent& current, const InstancePtr& instance)
{
    current.ioCompleted();
    try
    {
        instance->objectAdapterFactory()->shutdown();
    }
    catch (const CommunicatorDestroyedException&)
    {
    }
}

IceInternal::ThreadPool::EventHandlerThread::EventHandlerThread(ThreadPoolPtr pool, string name)
    : _name(std::move(name)),
      _pool(std::move(pool))
{
    updateObserver();
}

void
IceInternal::ThreadPool::EventHandlerThread::updateObserver()
{
    // Must be called with the thread pool mutex locked
    const CommunicatorObserverPtr& obsv = _pool->_instance->initializationData().observer;
    if (obsv)
    {
        _observer.attach(obsv->getThreadObserver(_pool->_prefix, _name, _state, _observer.get()));
    }
}

void
IceInternal::ThreadPool::EventHandlerThread::setState(Ice::Instrumentation::ThreadState s)
{
    // Must be called with the thread pool mutex locked
    if (_observer)
    {
        if (_state != s)
        {
            _observer->stateChanged(_state, s);
        }
    }
    _state = s;
}

void
IceInternal::ThreadPool::EventHandlerThread::start()
{
    assert(!_thread.joinable()); // Not started yet
    // It is safe to use this pointer here, because the thread pool keeps a reference to all threads and join them
    // before exiting.
    _thread = thread(&EventHandlerThread::run, this);
}

void
IceInternal::ThreadPool::EventHandlerThread::run()
{
    if (_pool->_instance->initializationData().threadStart)
    {
        try
        {
            _pool->_instance->initializationData().threadStart();
        }
        catch (const exception& ex)
        {
            Error out(_pool->_instance->initializationData().logger);
            out << "thread hook start() method raised an unexpected exception in '" << _pool->_prefix << "':\n" << ex;
        }
        catch (...)
        {
            Error out(_pool->_instance->initializationData().logger);
            out << "thread hook start() method raised an unexpected exception in '" << _pool->_prefix << "'";
        }
    }

    try
    {
        _pool->run(shared_from_this());
    }
    catch (const exception& ex)
    {
        Error out(_pool->_instance->initializationData().logger);
        out << "exception in '" << _pool->_prefix << "':\n" << ex;
    }
    catch (...)
    {
        Error out(_pool->_instance->initializationData().logger);
        out << "unknown exception in '" << _pool->_prefix << "'";
    }

    _observer.detach();

    if (_pool->_instance->initializationData().threadStop)
    {
        try
        {
            _pool->_instance->initializationData().threadStop();
        }
        catch (const exception& ex)
        {
            Error out(_pool->_instance->initializationData().logger);
            out << "thread hook stop() method raised an unexpected exception in '" << _pool->_prefix << "':\n" << ex;
        }
        catch (...)
        {
            Error out(_pool->_instance->initializationData().logger);
            out << "thread hook stop() method raised an unexpected exception in '" << _pool->_prefix << "'";
        }
    }

    _pool = nullptr; // Break cyclic dependency.
}

void
IceInternal::ThreadPool::EventHandlerThread::join()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

ThreadPoolCurrent::ThreadPoolCurrent(const ThreadPoolPtr& threadPool, ThreadPool::EventHandlerThreadPtr thread)
    : _threadPool(threadPool.get()),
      _thread(std::move(thread))
{
}
