// Copyright (c) ZeroC, Inc.

#include "ObjectAdapterFactory.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Object.h"
#include "Ice/Router.h"
#include "Ice/UUID.h"
#include "ObjectAdapterI.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::ObjectAdapterFactory::shutdown() noexcept
{
    list<shared_ptr<ObjectAdapterI>> adapters;

    {
        lock_guard lock(_mutex);

        //
        // Ignore shutdown requests if the object adapter factory has
        // already been shut down.
        //
        if (!_instance)
        {
            return;
        }

        adapters = _adapters;

        _instance = nullptr;
        _communicator = nullptr;

        _conditionVariable.notify_all();
    }

    // Deactivate outside the thread synchronization, to avoid deadlocks.
    for (const auto& adapter : adapters)
    {
        adapter->deactivate();
    }
}

void
IceInternal::ObjectAdapterFactory::waitForShutdown() noexcept
{
    list<shared_ptr<ObjectAdapterI>> adapters;
    {
        unique_lock lock(_mutex);
        if (_shutdownCompleted)
        {
            return;
        }

        // First we wait for the shutdown of the factory itself.
        _conditionVariable.wait(lock, [this] { return !_instance; });
        adapters = _adapters;
    }

    // Now we wait for deactivation of each object adapter.
    for (const auto& adapter : adapters)
    {
        adapter->waitForDeactivate();
    }

    {
        lock_guard lock(_mutex);
        _shutdownCompleted = true;
    }
}

void
IceInternal::ObjectAdapterFactory::waitForShutdownAsync(function<void()> completed) noexcept
{
    assert(completed); // the caller (Communicator) makes sure completed is callable.

    bool executeCallback = false;
    {
        lock_guard lock(_mutex);
        if (_shutdownCompleted)
        {
            executeCallback = true; // execute outside the lock
        }
        else
        {
            // Start the shutdown completed thread if there is no callback yet.
            if (_shutdownCompletedCallbacks.empty())
            {
                assert(!_shutdownCompletedThread.joinable());
                _shutdownCompletedThread = std::thread{[self = shared_from_this()]()
                                                       {
                                                           self->waitForShutdown();
                                                           vector<function<void()>> callbacks;
                                                           {
                                                               lock_guard lg(self->_mutex);
                                                               callbacks = std::move(self->_shutdownCompletedCallbacks);
                                                           }

                                                           for (const auto& callback : callbacks)
                                                           {
                                                               callback();
                                                           }
                                                       }};
            }

            _shutdownCompletedCallbacks.push_back(std::move(completed));
        }
    }

    if (executeCallback)
    {
        completed();
    }
}

bool
IceInternal::ObjectAdapterFactory::isShutdown() const noexcept
{
    lock_guard lock(_mutex);
    return _instance == nullptr;
}

void
IceInternal::ObjectAdapterFactory::destroy() noexcept
{
    //
    // First wait for shutdown to finish.
    //
    waitForShutdown();

    list<shared_ptr<ObjectAdapterI>> adapters;
    std::thread shutdownCompletedThread;
    {
        lock_guard lock(_mutex);
        // We make a copy because we want all callers of destroy to wait for the destruction of these adapters.
        adapters = _adapters;

        shutdownCompletedThread = std::move(_shutdownCompletedThread); // take ownership
    }

    if (shutdownCompletedThread.joinable())
    {
        if (shutdownCompletedThread.get_id() == std::this_thread::get_id())
        {
            // destroy is called from a shutdown completed callback - this is fine, but the thread can't join itself.
            shutdownCompletedThread.detach();
        }
        else
        {
            shutdownCompletedThread.join();
        }
    }

    // Now we destroy each object adapter.
    for (const auto& adapter : adapters)
    {
        adapter->destroy();
    }

    {
        lock_guard lock(_mutex);
        _adapters.clear();
    }
}

void
IceInternal::ObjectAdapterFactory::updateObservers(void (ObjectAdapterI::*fn)())
{
    list<shared_ptr<ObjectAdapterI>> adapters;

    {
        lock_guard lock(_mutex);
        adapters = _adapters;
    }

    for (const auto& adapter : adapters)
    {
        (adapter.get()->*fn)();
    }
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(
    string name, // NOLINT(performance-unnecessary-value-param)
    optional<Ice::RouterPrx> router,
    optional<Ice::SSL::ServerAuthenticationOptions> serverAuthenticationOptions)
{
    shared_ptr<ObjectAdapterI> adapter;
    {
        lock_guard lock(_mutex);

        if (!_instance)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        if (name.empty())
        {
            string uuid = Ice::generateUUID();
            adapter = make_shared<ObjectAdapterI>(
                _instance,
                _communicator,
                shared_from_this(),
                uuid,
                true,
                std::move(serverAuthenticationOptions));
        }
        else
        {
            if (_adapterNamesInUse.find(name) != _adapterNamesInUse.end())
            {
                throw AlreadyRegisteredException(__FILE__, __LINE__, "object adapter", name);
            }
            adapter = make_shared<ObjectAdapterI>(
                _instance,
                _communicator,
                shared_from_this(),
                name,
                false,
                std::move(serverAuthenticationOptions));
            _adapterNamesInUse.insert(name);
        }
    }

    //
    // Must be called outside the synchronization since initialize can make client invocations
    // on the router if it's set.
    //
    bool initialized = false;
    try
    {
        adapter->initialize(std::move(router));
        initialized = true;

        lock_guard lock(_mutex);
        if (!_instance)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }
        _adapters.push_back(adapter);
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        if (initialized)
        {
            adapter->destroy();
        }
        throw;
    }
    catch (const std::exception&)
    {
        if (!name.empty())
        {
            lock_guard lock(_mutex);
            _adapterNamesInUse.erase(name);
        }
        throw;
    }

    return adapter;
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::findObjectAdapter(const ReferencePtr& reference)
{
    list<shared_ptr<ObjectAdapterI>> adapters;
    {
        lock_guard lock(_mutex);

        if (!_instance)
        {
            return nullptr;
        }

        adapters = _adapters;
    }

    for (const auto& adapter : adapters)
    {
        try
        {
            if (adapter->isLocal(reference))
            {
                return adapter;
            }
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            // Ignore.
        }
    }

    return nullptr;
}

void
IceInternal::ObjectAdapterFactory::removeObjectAdapter(const ObjectAdapterPtr& adapter)
{
    lock_guard lock(_mutex);

    if (!_instance)
    {
        return;
    }

    for (auto p = _adapters.begin(); p != _adapters.end(); ++p)
    {
        if (*p == adapter)
        {
            _adapters.erase(p);
            break;
        }
    }
    _adapterNamesInUse.erase(adapter->getName());
}

void
IceInternal::ObjectAdapterFactory::flushAsyncBatchRequests(
    const CommunicatorFlushBatchAsyncPtr& outAsync,
    CompressBatch compressBatch) const
{
    list<shared_ptr<ObjectAdapterI>> adapters;
    {
        lock_guard lock(_mutex);

        adapters = _adapters;
    }

    for (const auto& adapter : adapters)
    {
        adapter->flushAsyncBatchRequests(outAsync, compressBatch);
    }
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(InstancePtr instance, CommunicatorPtr communicator)
    : _instance(std::move(instance)),
      _communicator(std::move(communicator))
{
}

IceInternal::ObjectAdapterFactory::~ObjectAdapterFactory()
{
    assert(!_instance);
    assert(!_communicator);
    assert(_adapters.empty());
}
