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
