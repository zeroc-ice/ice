// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ProxyFactory.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ProxyFactory* p) { p->__incRef(); }
void IceInternal::decRef(ProxyFactory* p) { p->__decRef(); }

ObjectPrx
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    if(str.empty())
    {
	return 0;
    }
    else
    {
	ReferencePtr ref = _instance->referenceFactory()->create(str);
	return referenceToProxy(ref);
    }
}

string
IceInternal::ProxyFactory::proxyToString(const ObjectPrx& proxy) const
{
    static const string nilProxy("");
    if(proxy)
    {
	return proxy->__reference()->toString();
    }
    else
    {
	return nilProxy;
    }
}

ObjectPrx
IceInternal::ProxyFactory::streamToProxy(BasicStream* s) const
{
    Identity ident;
    ident.__read(s);

    if(ident.name.empty())
    {
	return 0;
    }
    else
    {
	ReferencePtr ref = _instance->referenceFactory()->create(ident, s);
	return referenceToProxy(ref);
    }
}

void
IceInternal::ProxyFactory::proxyToStream(const ObjectPrx& proxy, BasicStream* s) const
{
    if(proxy)
    {
	proxy->__reference()->identity.__write(s);
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
    ObjectPrx proxy = new ::IceProxy::Ice::Object;
    proxy->setup(ref, _retryIntervals);
    return proxy;
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
    string str = _instance->properties()->getPropertyWithDefault("Ice.RetryIntervals", "0");

    string::size_type beg;
    string::size_type end = 0;

    while(true)
    {
	static const string delim = " \t";
    
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
