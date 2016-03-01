// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    
    Ice::ObjectPrx stringToProxy(const std::string&) const;
    std::string proxyToString(const Ice::ObjectPrx&) const;

    Ice::ObjectPrx propertyToProxy(const std::string&) const;
    Ice::PropertyDict proxyToProperty(const Ice::ObjectPrx&, const std::string&) const;

    Ice::ObjectPrx streamToProxy(BasicStream*) const;
    void proxyToStream(const Ice::ObjectPrx&, BasicStream*) const;

    Ice::ObjectPrx referenceToProxy(const ReferencePtr&) const;

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
