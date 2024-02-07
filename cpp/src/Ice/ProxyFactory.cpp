//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <Ice/ProxyFactory.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/LocatorInfo.h>
#include <Ice/RouterInfo.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/LocalException.h>
#include <Ice/OutgoingAsync.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

ObjectPrxPtr
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    ReferencePtr ref = _instance->referenceFactory()->create(str, "");
    return referenceToProxy(ref);
}

string
IceInternal::ProxyFactory::proxyToString(const ObjectPrxPtr& proxy) const
{
    if(proxy)
    {
        return proxy->_getReference()->toString();
    }
    else
    {
        return "";
    }
}

ObjectPrxPtr
IceInternal::ProxyFactory::propertyToProxy(const string& prefix) const
{
    string proxy = _instance->initializationData().properties->getProperty(prefix);
    ReferencePtr ref = _instance->referenceFactory()->create(proxy, prefix);
    return referenceToProxy(ref);
}

PropertyDict
IceInternal::ProxyFactory::proxyToProperty(const ObjectPrxPtr& proxy, const string& prefix) const
{
    if(proxy)
    {
        return proxy->_getReference()->toProperty(prefix);
    }
    else
    {
        return PropertyDict();
    }
}

ObjectPrxPtr
IceInternal::ProxyFactory::streamToProxy(InputStream* s) const
{
    Identity ident;
    s->read(ident);

    ReferencePtr ref = _instance->referenceFactory()->create(ident, s);
    return referenceToProxy(ref);
}

ObjectPrxPtr
IceInternal::ProxyFactory::referenceToProxy(const ReferencePtr& ref) const
{
    if(ref)
    {
        return make_shared<ObjectPrx>(ref);
    }
    else
    {
        return nullptr;
    }
}

int
IceInternal::ProxyFactory::checkRetryAfterException(std::exception_ptr ex, const ReferencePtr& ref, int& cnt) const
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->initializationData().logger;

    //
    // We don't retry batch requests because the exception might have
    // caused all the requests batched with the connection to be
    // aborted and we want the application to be notified.
    //
    if(ref->getMode() == Reference::ModeBatchOneway || ref->getMode() == Reference::ModeBatchDatagram)
    {
        rethrow_exception(ex);
    }

    //
    // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
    // the connection and the request will fail with the exception.
    //
    if(dynamic_cast<const FixedReference*>(ref.get()))
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
        if(ref->getRouterInfo() && one.operation == "ice_add_proxy")
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

    int interval = -1;
    if(cnt == static_cast<int>(_retryIntervals.size() + 1) && isCloseConnectionException)
    {
        //
        // A close connection exception is always retried at least once, even if the retry
        // limit is reached.
        //
        interval = 0;
    }
    else if(cnt > static_cast<int>(_retryIntervals.size()))
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
        interval = _retryIntervals[static_cast<size_t>(cnt - 1)];
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

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
    StringSeq retryValues = _instance->initializationData().properties->getPropertyAsList("Ice.RetryIntervals");
    if(retryValues.size() == 0)
    {
        _retryIntervals.push_back(0);
    }
    else
    {
        for(StringSeq::const_iterator p = retryValues.begin(); p != retryValues.end(); ++p)
        {
            istringstream value(*p);

            int v;
            if(!(value >> v) || !value.eof())
            {
                v = 0;
            }

            //
            // If -1 is the first value, no retry and wait intervals.
            //
            if(v == -1 && _retryIntervals.empty())
            {
                break;
            }

            _retryIntervals.push_back(v > 0 ? v : 0);
        }
    }
}
