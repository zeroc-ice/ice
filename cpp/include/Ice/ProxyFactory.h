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

namespace __Ice
{

class Stream;

class ProxyFactory : public Shared
{
public:
    
    ::Ice::Object_pptr stringToProxy(const std::string&);
    ::Ice::Object_pptr streamToProxy(Stream*);
    ::Ice::Object_pptr referenceToProxy(const Reference_ptr&);
    void proxyToStream(const ::Ice::Object_pptr&, Stream*);

private:

    ProxyFactory(const Instance_ptr&);
    virtual ~ProxyFactory();
    friend class Instance;

    Instance_ptr instance_;
};

}

#endif
