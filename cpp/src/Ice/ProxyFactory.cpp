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
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/BasicStream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ProxyFactory* p) { p->__incRef(); }
void IceInternal::decRef(ProxyFactory* p) { p->__decRef(); }

ObjectPrx
IceInternal::ProxyFactory::stringToProxy(const string& s)
{
    ReferencePtr reference = new Reference(_instance, s);
    return referenceToProxy(reference);
}

string
IceInternal::ProxyFactory::proxyToString(const ObjectPrx& proxy)
{
    return proxy->__reference()->toString();
}

ObjectPrx
IceInternal::ProxyFactory::streamToProxy(BasicStream* s)
{
    string identity;
    s->read(identity);

    if (identity.empty())
    {
	return 0;
    }
    else
    {
	ReferencePtr reference = new Reference(identity, s);
	return referenceToProxy(reference);
    }
}

ObjectPrx
IceInternal::ProxyFactory::referenceToProxy(const ReferencePtr& reference)
{
    ObjectPrx proxy = new ::IceProxy::Ice::Object;
    proxy->setup(reference);
    return proxy;
}

void
IceInternal::ProxyFactory::proxyToStream(const ObjectPrx& proxy, BasicStream* s)
{
    if (proxy)
    {
	s->write(proxy->__reference()->identity);
	proxy->__reference()->streamWrite(s);
    }
    else
    {
	s->write("");
    }
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance)
    : _instance(instance)
{
}

IceInternal::ProxyFactory::~ProxyFactory()
{
}
