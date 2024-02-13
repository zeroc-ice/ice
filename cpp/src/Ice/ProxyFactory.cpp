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

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
}
