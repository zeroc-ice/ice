// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_FACTORY_H
#define ICEE_PROXY_FACTORY_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/ProxyFactoryF.h>
#include <IceE/InstanceF.h>
#include <IceE/ReferenceF.h>
#include <IceE/ProxyF.h>
#include <IceE/Exception.h>

namespace IceEInternal
{

class BasicStream;

class ProxyFactory : public IceE::Shared
{
public:
    
    IceE::ObjectPrx stringToProxy(const std::string&) const;
    std::string proxyToString(const IceE::ObjectPrx&) const;

    IceE::ObjectPrx streamToProxy(BasicStream*) const;
    void proxyToStream(const IceE::ObjectPrx&, BasicStream*) const;

    IceE::ObjectPrx referenceToProxy(const ReferencePtr&) const;

    void checkRetryAfterException(const IceE::LocalException&, const ReferencePtr&, int&) const;

private:

    ProxyFactory(const InstancePtr&);
    virtual ~ProxyFactory();
    friend class Instance;

    InstancePtr _instance;
    std::vector<int> _retryIntervals;
};

}

#endif
