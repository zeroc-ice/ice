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

class ICE_API ProxyFactoryI : public Shared
{
public:
    
    ::Ice::ObjectProxy stringToProxy(const std::string&);
    ::Ice::ObjectProxy streamToProxy(Stream*);
    ::Ice::ObjectProxy referenceToProxy(const Reference&);
    void proxyToStream(const ::Ice::ObjectProxy&, Stream*);

private:

    ProxyFactoryI(const Instance&);
    virtual ~ProxyFactoryI();
    friend class InstanceI; // May create and destroy ProxyFactoryIs

    Instance instance_;
};

}

#endif
