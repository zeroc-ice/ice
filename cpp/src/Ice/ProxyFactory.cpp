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
    return proxy->__reference()->toString();
}

ObjectPrx
IceInternal::ProxyFactory::streamToProxy(BasicStream* s) const
{
    Identity ident;
    ident.__read(s);

    if (ident.name.empty())
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
    if (proxy)
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
    proxy->setup(ref);
    return proxy;
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
}

IceInternal::ProxyFactory::~ProxyFactory()
{
}
