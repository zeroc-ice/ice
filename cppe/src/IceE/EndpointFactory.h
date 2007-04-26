// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ENDPOINT_FACTORY_H
#define ICEE_ENDPOINT_FACTORY_H

#include <IceE/EndpointFactoryF.h>
#include <IceE/EndpointF.h>
#include <IceE/InstanceF.h>
#include <IceE/Shared.h>

namespace IceInternal
{

class BasicStream;

class EndpointFactory : public ::IceUtil::Shared
{
public:

    ~EndpointFactory();

    EndpointPtr create(const std::string&) const;
    EndpointPtr read(BasicStream*) const;
    void destroy();

protected:

    EndpointFactory(const InstancePtr&);
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
