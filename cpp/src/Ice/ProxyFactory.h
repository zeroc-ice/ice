// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_FACTORY_H
#define ICE_PROXY_FACTORY_H

#include <IceUtil/Shared.h>
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
    
    ::Ice::ObjectPrx stringToProxy(const std::string&);
    std::string proxyToString(const ::Ice::ObjectPrx&);

    ::Ice::ObjectPrx streamToProxy(BasicStream*);
    ::Ice::ObjectPrx referenceToProxy(const ReferencePtr&);
    void proxyToStream(const ::Ice::ObjectPrx&, BasicStream*);

private:

    ProxyFactory(const InstancePtr&);
    virtual ~ProxyFactory();
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
