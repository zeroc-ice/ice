// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

namespace IceInternal
{

class BasicStream;

class ProxyFactory : public IceUtil::Shared
{
public:
    
    Ice::ObjectPrx stringToProxy(const std::string&) const;
    std::string proxyToString(const Ice::ObjectPrx&) const;

    Ice::ObjectPrx streamToProxy(BasicStream*) const;
    void proxyToStream(const Ice::ObjectPrx&, BasicStream*) const;

    Ice::ObjectPrx referenceToProxy(const ReferencePtr&) const;

    void checkRetryAfterException(const Ice::LocalException&, int&) const;

private:

    ProxyFactory(const InstancePtr&);
    virtual ~ProxyFactory();
    friend class Instance;

    InstancePtr _instance;
    std::vector<int> _retryIntervals;
};

}

#endif
