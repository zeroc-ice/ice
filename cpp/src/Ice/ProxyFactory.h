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

    std::optional<Ice::ObjectPrx> stringToProxy(const std::string&) const;
    std::string proxyToString(const std::optional<Ice::ObjectPrx>&) const;

    std::optional<Ice::ObjectPrx> propertyToProxy(const std::string&) const;
    Ice::PropertyDict proxyToProperty(const Ice::ObjectPrx&, const std::string&) const;

    std::optional<Ice::ObjectPrx> referenceToProxy(const ReferencePtr&) const;

private:

    InstancePtr _instance;
};

using ProxyFactoryPtr = std::shared_ptr<ProxyFactory>;

}

#endif
