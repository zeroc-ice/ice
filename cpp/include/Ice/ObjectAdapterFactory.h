// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_FACTORY_H
#define ICE_OBJECT_ADAPTER_FACTORY_H

#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/Shared.h>
#include <map>

namespace __Ice
{

class ObjectAdapterFactory : public Shared, public JTCMutex
{
public:

    void shutdown();
    ::Ice::ObjectAdapter_ptr createObjectAdapter(const std::string&,
						 const std::string&);

private:

    ObjectAdapterFactory(const Instance_ptr&);
    void destroy();
    friend class Instance;

    Instance_ptr instance_;
    std::map<std::string, ::Ice::ObjectAdapter_ptr> adapters_;
};

}

#endif
