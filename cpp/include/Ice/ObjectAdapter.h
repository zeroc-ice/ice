// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_H
#define ICE_OBJECT_ADAPTER_H

#include <Ice/ObjectAdapterF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/CollectorF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalException.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API ObjectAdapter : public ::__Ice::Shared, public JTCMutex
{
public:

    std::string name() const;
    Communicator_ptr communicator() const;

    void activate();
    void hold();
    void deactivate();

    void add(const Object_ptr&, const std::string&);
    void remove(const std::string&);
    Object_ptr find(const std::string&);

    std::string objectToIdentity(const Object_ptr&);
    Object_ptr proxyToObject(const Object_prx&);

    Object_prx createProxy(const std::string&);

    ::__Ice::Instance_ptr __instance() const;

private:

    ObjectAdapter(const ::__Ice::Instance_ptr&, const std::string&,
		  const std::string&);
    virtual ~ObjectAdapter();
    friend ::__Ice::ObjectAdapterFactory;

    ::__Ice::Instance_ptr instance_;
    std::string name_;
    std::vector< __Ice::CollectorFactory_ptr> collectorFactories_;
    std::map<std::string, Object_ptr> objects_;
};

}

#endif
