//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_FACTORY_H
#define ICE_PROXY_FACTORY_H

#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>
#include <Ice/Exception.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/Properties.h>

namespace Ice
{

class OutputStream;
class InputStream;

}

namespace IceInternal
{

class ProxyFactory
{
public:

    ProxyFactory(const InstancePtr&);

    Ice::ObjectPrxPtr stringToProxy(const std::string&) const;
    std::string proxyToString(const Ice::ObjectPrxPtr&) const;

    Ice::ObjectPrxPtr propertyToProxy(const std::string&) const;
    Ice::PropertyDict proxyToProperty(const Ice::ObjectPrxPtr&, const std::string&) const;

    Ice::ObjectPrxPtr streamToProxy(Ice::InputStream*) const;

    Ice::ObjectPrxPtr referenceToProxy(const ReferencePtr&) const;

    int checkRetryAfterException(std::exception_ptr, const ReferencePtr&, int&) const;

private:

    InstancePtr _instance;
    std::vector<int> _retryIntervals;
};

using ProxyFactoryPtr = std::shared_ptr<ProxyFactory>;

}

#endif
