// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

void IceInternal::incRef(ProxyFactory* p) { p->__incRef(); }
void IceInternal::decRef(ProxyFactory* p) { p->__decRef(); }

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
    //
    // We retry ObjectNotExistException if the reference is
    // indirect. Otherwise, we don't retry other *NotExistException,
    // which are all derived from RequestFailedException.
    //
#ifdef ICEE_HAS_LOCATOR
    if(dynamic_cast<const ObjectNotExistException*>(&ex))
    {
	IndirectReferencePtr ir = IndirectReferencePtr::dynamicCast(ref);
	if(!ir || !ir->getLocatorInfo())
	{
	    ex.ice_throw();
	}
	ir->getLocatorInfo()->clearObjectCache(ir);
    }
    else
#endif
    if(dynamic_cast<const RequestFailedException*>(&ex))
    {
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
    
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->logger();
    
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
        out << "re-trying operation call";
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
    string str = _instance->properties()->getPropertyWithDefault("Ice.RetryIntervals", "0");

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
