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

#include <IceUtil/Shared.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <map>

namespace IceInternal
{

class ObjectAdapterFactory : public ::IceUtil::Shared, public JTCMutex
{
public:

    void shutdown();
    ::Ice::ObjectAdapterPtr createObjectAdapter(const std::string&,
						 const std::string&);
    ::Ice::ObjectPtr proxyToObject(const ::Ice::ObjectPrx&);

private:

    ObjectAdapterFactory(const InstancePtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    std::map<std::string, ::Ice::ObjectAdapterPtr> _adapters;
};

}

#endif
