//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "RequestHandlerCache.h"
#include "ConnectionI.h"
#include "Reference.h"
#include "RequestHandler.h"
#include "Instance.h"
#include "LocatorInfo.h"
#include "RouterInfo.h"
#include "TraceLevels.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

int
checkRetryAfterException(std::exception_ptr ex, const ReferencePtr& ref, int& cnt)
{
    const InstancePtr& instance = ref->getInstance();

    TraceLevelsPtr traceLevels = instance->traceLevels();
    LoggerPtr logger = instance->initializationData().logger;

    //
    // We don't retry batch requests because the exception might have
    // caused all the requests batched with the connection to be
    // aborted and we want the application to be notified.
    //
    if (ref->isBatch())
    {
        rethrow_exception(ex);
    }

    //
    // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
    // the connection and the request will fail with the exception.
    //
    if (dynamic_cast<const FixedReference*>(ref.get()))
    {
        rethrow_exception(ex);
    }

    bool isCloseConnectionException = false;
    string errorMessage;
    try
    {
        rethrow_exception(ex);
    }
    catch (const ObjectNotExistException& one)
    {
        if (ref->getRouterInfo() && one.operation == "ice_add_proxy")
        {
            //
            // If we have a router, an ObjectNotExistException with an
            // operation name "ice_add_proxy" indicates to the client
            // that the router isn't aware of the proxy (for example,
            // because it was evicted by the router). In this case, we
            // must *always* retry, so that the missing proxy is added
            // to the router.
            //

            ref->getRouterInfo()->clearCache(ref);

            if(traceLevels->retry >= 1)
            {
                Trace out(logger, traceLevels->retryCat);
                out << "retrying operation call to add proxy to router\n" << one;
            }

            return 0; // We must always retry, so we don't look at the retry count.
        }
        else if(ref->isIndirect())
        {
            //
            // We retry ObjectNotExistException if the reference is
            // indirect.
            //

            if(ref->isWellKnown())
            {
                LocatorInfoPtr li = ref->getLocatorInfo();
                if(li)
                {
                    li->clearCache(ref);
                }
            }
        }
        else
        {
            //
            // For all other cases, we don't retry
            // ObjectNotExistException.
            //
            throw;
        }
    }
    catch (const RequestFailedException&)
    {
        //
        // We don't retry other *NotExistException, which are all
        // derived from RequestFailedException.
        //
        throw;
    }
    catch (const MarshalException&)
    {
        //
        // There is no point in retrying an operation that resulted in a
        // MarshalException. This must have been raised locally (because
        // if it happened in a server it would result in an
        // UnknownLocalException instead), which means there was a problem
        // in this process that will not change if we try again.
        //
        // The most likely cause for a MarshalException is exceeding the
        // maximum message size, which is represented by the subclass
        // MemoryLimitException. For example, a client can attempt to send
        // a message that exceeds the maximum memory size, or accumulate
        // enough batch requests without flushing that the maximum size is
        // reached.
        //
        // This latter case is especially problematic, because if we were
        // to retry a batch request after a MarshalException, we would in
        // fact silently discard the accumulated requests and allow new
        // batch requests to accumulate. If the subsequent batched
        // requests do not exceed the maximum message size, it appears to
        // the client that all of the batched requests were accepted, when
        // in reality only the last few are actually sent.
        //
        throw;
    }
    catch (const CommunicatorDestroyedException&)
    {
        throw;
    }
    catch (const ObjectAdapterDeactivatedException&)
    {
        throw;
    }
    catch (const Ice::ConnectionManuallyClosedException&)
    {
        throw;
    }
    catch (const InvocationTimeoutException&)
    {
        throw;
    }
    catch (const InvocationCanceledException&)
    {
        throw;
    }
    catch (const CloseConnectionException& e)
    {
        isCloseConnectionException = true;
        errorMessage = e.what();
        // and retry
    }
    catch (const std::exception& e)
    {
        errorMessage = e.what();
        // We retry on all other exceptions!
    }

    ++cnt;
    assert(cnt > 0);

    const auto& retryIntervals = instance->retryIntervals();
    int interval = -1;
    if(cnt == static_cast<int>(retryIntervals.size() + 1) && isCloseConnectionException)
    {
        //
        // A close connection exception is always retried at least once, even if the retry
        // limit is reached.
        //
        interval = 0;
    }
    else if(cnt > static_cast<int>(retryIntervals.size()))
    {
        if(traceLevels->retry >= 1)
        {
            Trace out(logger, traceLevels->retryCat);
            out << "cannot retry operation call because retry limit has been exceeded\n" << errorMessage;
        }
        rethrow_exception(ex);
    }
    else
    {
        interval = retryIntervals[static_cast<size_t>(cnt - 1)];
    }

    if(traceLevels->retry >= 1)
    {
        Trace out(logger, traceLevels->retryCat);
        out << "retrying operation call";
        if(interval > 0)
        {
            out << " in " << interval << "ms";
        }
        out << " because of exception\n" << errorMessage;
    }
    return interval;
}

}

RequestHandlerCache::RequestHandlerCache(const ReferencePtr& reference) :
    _reference(reference),
    _cacheConnection(reference->getCacheConnection())
{
}

IceInternal::RequestHandlerPtr
RequestHandlerCache::getRequestHandler()
{
    if (_cacheConnection)
    {
        lock_guard<mutex> lock(_mutex);
        if (_cachedRequestHandler)
        {
            return _cachedRequestHandler;
        }
    }

    auto handler = _reference->getRequestHandler();
    if (_cacheConnection)
    {
        lock_guard<mutex> lock(_mutex);
        if (!_cachedRequestHandler)
        {
            _cachedRequestHandler = handler;
        }
        // else discard handler
        return _cachedRequestHandler;
    }
    else
    {
        return handler;
    }
}

ConnectionPtr
RequestHandlerCache::getCachedConnection()
{
    if (_cacheConnection)
    {
        RequestHandlerPtr handler;
        {
            lock_guard<mutex> lock(_mutex);
            handler = _cachedRequestHandler;
        }
        if (handler)
        {
            return handler->getConnection();
        }
    }
    return nullptr;
}

void
RequestHandlerCache::clearCachedRequestHandler(const RequestHandlerPtr& handler)
{
    if (_cacheConnection)
    {
        lock_guard<mutex> lock(_mutex);
        if (handler == _cachedRequestHandler)
        {
            _cachedRequestHandler = nullptr;
        }
    }
}

int
RequestHandlerCache::handleException(
    std::exception_ptr ex,
    const RequestHandlerPtr& handler,
    OperationMode mode,
    bool sent,
    int& cnt)
{
    clearCachedRequestHandler(handler);

    //
    // We only retry local exceptions, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once".
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    //
    try
    {
        rethrow_exception(ex);
    }
    catch (const Ice::LocalException& localEx)
    {
        if (!sent ||
                mode == OperationMode::Nonmutating || mode == OperationMode::Idempotent ||
                dynamic_cast<const CloseConnectionException*>(&localEx) ||
                dynamic_cast<const ObjectNotExistException*>(&localEx))
        {
            try
            {
                return checkRetryAfterException(ex, _reference, cnt);
            }
            catch (const CommunicatorDestroyedException&)
            {
                //
                // The communicator is already destroyed, so we cannot retry.
                //
                rethrow_exception(ex);
            }
        }
        else
        {
            throw; // Retry could break at-most-once semantics, don't retry.
        }
    }
}
