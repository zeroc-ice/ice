// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ProxyFactory.h>
#include <IceE/Thread.h>
#include <IceE/Time.h>
#include <IceE/Instance.h>
#include <IceE/Proxy.h>
#include <IceE/Reference.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/LocatorInfo.h>
#include <IceE/Properties.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/LocalException.h>
#include <IceE/SafeStdio.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ProxyFactory* p) { return p; }

ObjectPrx
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    ReferencePtr ref = _instance->referenceFactory()->create(str);
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
    ReferencePtr ref = _instance->referenceFactory()->createFromProperties(prefix);
    return referenceToProxy(ref);
}

ObjectPrx
IceInternal::ProxyFactory::streamToProxy(BasicStream* s) const
{
    Identity ident;
    ident.__read(s);

    ReferencePtr ref = _instance->referenceFactory()->create(ident, s);
    return referenceToProxy(ref);
}

void
IceInternal::ProxyFactory::proxyToStream(const ObjectPrx& proxy, BasicStream* s) const
{
    if(proxy)
    {
	proxy->__reference()->getIdentity().__write(s);
	proxy->__reference()->streamWrite(s);
    }
    else
    {
	Identity ident;
	ident.__write(s);
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

void
IceInternal::ProxyFactory::checkRetryAfterException(const LocalException& ex, const ReferencePtr& ref, int& cnt) const
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->initializationData().logger;

#if defined(ICEE_HAS_LOCATOR) || defined(ICEE_HAS_ROUTER)
    const ObjectNotExistException* one = dynamic_cast<const ObjectNotExistException*>(&ex);
    if(one)
    {
#ifdef ICEE_HAS_LOCATOR
	LocatorInfoPtr li = ref->getLocatorInfo();
	if(li)
	{
	   //
	   // We retry ObjectNotExistException if the reference is indirect.
	   //
	   li->clearObjectCache(IndirectReferencePtr::dynamicCast(ref));
	}
	else
#endif
#ifdef ICEE_HAS_ROUTER
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
            if(traceLevels->retry >= 1)
            {
                Trace out(logger, traceLevels->retryCat);
                out << "retrying operation call to add proxy to router\n" << ex.toString();
            }
            return; // We must always retry, so we don't look at the retry count.
        }
	else
#endif
	{
	    //
	    // For all other cases, we don't retry  ObjectNotExistException
	    //
	    ex.ice_throw();
	}
    }
    else 
#endif
         if(dynamic_cast<const RequestFailedException*>(&ex))
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
    // maximum message size, which is represented by the the subclass
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

    ++cnt;
    assert(cnt > 0);
    
    if(cnt > static_cast<int>(_retryIntervals.size()))
    {
        if(traceLevels->retry >= 1)
        {
            Trace out(logger, traceLevels->retryCat);
            out << "cannot retry operation call because retry limit has been exceeded\n" << ex.toString();
        }
        ex.ice_throw();
    }

    int interval = _retryIntervals[cnt - 1];

    if(traceLevels->retry >= 1)
    {
        Trace out(logger, traceLevels->retryCat);
        out << "retrying operation call";
        if(interval > 0)
        {
            out << Ice::printfToString(" in %dms", interval);
        }
        out << " because of exception\n" << ex.toString();
    }

    if(interval > 0)
    {
        //
        // Sleep before retrying.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(interval));
    }
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
    string str = _instance->initializationData().properties->getPropertyWithDefault("Ice.RetryIntervals", "0");

    string::size_type beg;
    string::size_type end = 0;

    while(true)
    {
	const string delim = " \t";
    
	beg = str.find_first_not_of(delim, end);
	if(beg == string::npos)
	{
	    if(_retryIntervals.empty())
	    {
		_retryIntervals.push_back(0);
	    }
	    break;
	}

	end = str.find_first_of(delim, beg);
	if(end == string::npos)
	{
	    end = str.length();
	}
	
	if(beg == end)
	{
	    break;
	}

	string value = str.substr(beg, end - beg);
	int v = atoi(value.c_str());

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

IceInternal::ProxyFactory::~ProxyFactory()
{
}
