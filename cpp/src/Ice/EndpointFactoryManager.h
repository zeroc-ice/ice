// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_MANAGER_H
#define ICE_ENDPOINT_FACTORY_MANAGER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointF.h>
#include <Ice/EndpointFactoryF.h>
#include <Ice/EndpointFactoryManagerF.h>

namespace IceInternal
{

class BasicStream;

class EndpointFactoryManager : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    void add(const EndpointFactoryPtr&);
    EndpointFactoryPtr get(::Ice::Short) const;
    EndpointPtr create(const std::string&) const;
    EndpointPtr read(BasicStream*) const;

private:

    EndpointFactoryManager(const InstancePtr&);
    void destroy();
    friend class Instance;

    InstancePtr _instance;
    std::vector<EndpointFactoryPtr> _factories;
};

}

#endif
