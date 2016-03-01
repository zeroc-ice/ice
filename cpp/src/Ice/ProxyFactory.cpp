// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <Ice/ProxyFactory.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/LocatorInfo.h>
#include <Ice/RouterInfo.h>
#include <Ice/BasicStream.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/LocalException.h>
#include <Ice/OutgoingAsync.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ProxyFactory* p) { return p; }

ObjectPrx
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    ReferencePtr ref = _instance->referenceFactory()->create(str, "");
    return referenceToProxy(ref);
}

string
IceInternal::ProxyFactory::proxyToString(const ObjectPrx& proxy) const
{
    if(proxy)
    {
        return proxy->__reference()->toString();
    }
    else
    {
        return "";
    }
}

ObjectPrx
IceInternal::ProxyFactory::propertyToProxy(const string& prefix) const
{
    string proxy = _instance->initializationData().properties->getProperty(prefix);
    ReferencePtr ref = _instance->referenceFactory()->create(proxy, prefix);
    return referenceToProxy(ref);
}

PropertyDict
IceInternal::ProxyFactory::proxyToProperty(const ObjectPrx& proxy, const string& prefix) const
{
    if(proxy)
    {
        return proxy->__reference()->toProperty(prefix);
    }
    else
    {
        return PropertyDict();
    }
}

ObjectPrx
IceInternal::ProxyFactory::streamToProxy(BasicStream* s) const
{
    Identity ident;
    s->read(ident);

    ReferencePtr ref = _instance->referenceFactory()->create(ident, s);
    return referenceToProxy(ref);
}

void
IceInternal::ProxyFactory::proxyToStream(const ObjectPrx& proxy, BasicStream* s) const
{
    if(proxy)
    {
        s->write(proxy->__reference()->getIdentity());
        proxy->__reference()->streamWrite(s);
    }
    else
    {
        Identity ident;
        s->write(ident);
    }
}

ObjectPrx
IceInternal::ProxyFactory::referenceToProxy(const ReferencePtr& ref) const
{
    if(ref)
    {
        ObjectPrx proxy = new ::IceProxy::Ice::Object;
        proxy->setup(ref);
        return proxy;
    }
    else
    {
        return 0;
    }
}

int
IceInternal::ProxyFactory::checkRetryAfterException(const LocalException& ex, const ReferencePtr& ref, int& cnt) const
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
        ex.ice_throw();
    }

    const ObjectNotExistException* one = dynamic_cast<const ObjectNotExistException*>(&ex);
    if(one)
    {
        if(ref->getRouterInfo() && one->operation == "ice_add_proxy")
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
                out << "retrying operation call to add proxy to router\n" << ex;
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
            ex.ice_throw();
        }
    }
    else if(dynamic_cast<const RequestFailedException*>(&ex))
    {
        //
        // We don't retry other *NotExistException, which are all
        // derived from RequestFailedException.
        //
        ex.ice_throw();
    }

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
    if(dynamic_cast<const MarshalException*>(&ex))
    {
        ex.ice_throw();
    }

    //
    // Don't retry if the communicator is destroyed or object adapter
    // deactivated.
    //
    if(dynamic_cast<const CommunicatorDestroyedException*>(&ex) ||
       dynamic_cast<const ObjectAdapterDeactivatedException*>(&ex))
    {
        ex.ice_throw();
    }

    //
    // Don't retry invocation timeouts.
    //
    if(dynamic_cast<const InvocationTimeoutException*>(&ex) || dynamic_cast<const InvocationCanceledException*>(&ex))
    {
        ex.ice_throw();
    }

    ++cnt;
    assert(cnt > 0);

    int interval = -1;
    if(cnt == static_cast<int>(_retryIntervals.size() + 1) && dynamic_cast<const CloseConnectionException*>(&ex))
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
            out << "cannot retry operation call because retry limit has been exceeded\n" << ex;
        }
        ex.ice_throw();
    }
    else
    {
        interval = _retryIntervals[cnt - 1];
    }

    if(traceLevels->retry >= 1)
    {
        Trace out(logger, traceLevels->retryCat);
        out << "retrying operation call";
        if(interval > 0)
        {
            out << " in " << interval << "ms";
        }
        out << " because of exception\n" << ex;
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

IceInternal::ProxyFactory::~ProxyFactory()
{
}
