// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

namespace IceInternal
{

class BasicStream;

class ProxyFactory : public ::IceUtil::Shared
{
public:
    
    ::Ice::ObjectPrx stringToProxy(const std::string&) const;
    std::string proxyToString(const ::Ice::ObjectPrx&) const;

    ::Ice::ObjectPrx streamToProxy(BasicStream*) const;
    void proxyToStream(const ::Ice::ObjectPrx&, BasicStream*) const;

    ::Ice::ObjectPrx referenceToProxy(const ReferencePtr&) const;

    const ::std::vector<int>& getRetryIntervals() const;

private:

    ProxyFactory(const InstancePtr&);
    virtual ~ProxyFactory();
    friend class Instance;

    InstancePtr _instance;
    ::std::vector<int> _retryIntervals;
};

}

#endif
