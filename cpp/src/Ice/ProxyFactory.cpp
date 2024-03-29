//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ProxyFactory.h"
#include "Instance.h"
#include "Ice/Proxy.h"
#include "ReferenceFactory.h"
#include "LocatorInfo.h"
#include "RouterInfo.h"
#include "Ice/OutputStream.h"
#include "Ice/InputStream.h"
#include "Ice/Properties.h"
#include "Ice/LoggerUtil.h"
#include "TraceLevels.h"
#include "Ice/LocalException.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

std::optional<ObjectPrx>
IceInternal::ProxyFactory::stringToProxy(const string& str) const
{
    ReferencePtr ref = _instance->referenceFactory()->create(str, "");
    if (ref)
    {
        return ObjectPrx::_fromReference(std::move(ref));
    }
    else
    {
        return nullopt;
    }
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
    if (ref)
    {
        return ObjectPrx::_fromReference(std::move(ref));
    }
    else
    {
        return nullopt;
    }
}

PropertyDict
IceInternal::ProxyFactory::proxyToProperty(const std::optional<ObjectPrx>& proxy, const string& prefix) const
{
    return proxy ? proxy->_getReference()->toProperty(prefix) : PropertyDict();
}

IceInternal::ProxyFactory::ProxyFactory(const InstancePtr& instance) : _instance(instance) {}
