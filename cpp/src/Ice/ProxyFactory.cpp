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

std::optional<ObjectPrx>
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    ReferencePtr ref = _instance->referenceFactory()->create(str, "");
    return referenceToProxy(ref);
}

string
IceInternal::ProxyFactory::proxyToString(const optional<ObjectPrx>& proxy) const
{
    return proxy ? proxy->_getReference()->toString() : "";
}

std::optional<ObjectPrx>
IceInternal::ProxyFactory::propertyToProxy(const string& prefix) const
{
    string proxy = _instance->initializationData().properties->getProperty(prefix);
    ReferencePtr ref = _instance->referenceFactory()->create(proxy, prefix);
    return referenceToProxy(ref);
}

PropertyDict
IceInternal::ProxyFactory::proxyToProperty(const ObjectPrx& proxy, const string& prefix) const
{
    return proxy->_getReference()->toProperty(prefix);
}

std::optional<ObjectPrx>
IceInternal::ProxyFactory::referenceToProxy(const ReferencePtr& ref) const
{
    return ref ? make_optional<ObjectPrx>(ref) : nullopt;
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) :
    _instance(instance)
{
}
