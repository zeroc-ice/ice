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

#include <Ice/ProxyFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>
#include <Ice/Shared.h>

namespace IceInternal
{

class Stream;

class ProxyFactory : public Shared
{
public:
    
    ::Ice::ObjectPrx stringToProxy(const std::string&);
    std::string proxyToString(const ::Ice::ObjectPrx&);

    ::Ice::ObjectPrx streamToProxy(Stream*);
    ::Ice::ObjectPrx referenceToProxy(const ReferencePtr&);
    void proxyToStream(const ::Ice::ObjectPrx&, Stream*);

private:

    ProxyFactory(const InstancePtr&);
    virtual ~ProxyFactory();
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
