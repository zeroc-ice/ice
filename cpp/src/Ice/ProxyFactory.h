// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROXY_FACTORY_H
#define ICE_PROXY_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>
#include <Ice/Exception.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/Properties.h>

namespace IceInternal
{

class BasicStream;

class ProxyFactory : public IceUtil::Shared
{
public:
    
    Ice::ObjectPrxPtr stringToProxy(const std::string&) const;
    std::string proxyToString(const Ice::ObjectPrxPtr&) const;

    Ice::ObjectPrxPtr propertyToProxy(const std::string&) const;
    Ice::PropertyDict proxyToProperty(const Ice::ObjectPrxPtr&, const std::string&) const;

    Ice::ObjectPrxPtr streamToProxy(BasicStream*) const;
    void proxyToStream(const Ice::ObjectPrxPtr&, BasicStream*) const;

    Ice::ObjectPrxPtr referenceToProxy(const ReferencePtr&) const;

    int checkRetryAfterException(const Ice::LocalException&, const ReferencePtr&, int&) const;

private:

    ProxyFactory(const InstancePtr&);
    virtual ~ProxyFactory();
    friend class Instance;

    InstancePtr _instance;
    std::vector<int> _retryIntervals;
};

}

#endif
