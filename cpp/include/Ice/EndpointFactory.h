// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_H
#define ICE_ENDPOINT_FACTORY_H

#include <IceUtil/Shared.h>
#include <Ice/EndpointF.h>
#include <Ice/EndpointFactoryF.h>

namespace IceInternal
{

class BasicStream;

class ICE_PROTOCOL_API EndpointFactory : public ::IceUtil::Shared
{
public:

    virtual ~EndpointFactory();

    virtual ::Ice::Short type() const = 0;
    virtual ::std::string protocol() const = 0;
    virtual EndpointPtr create(const std::string&) const = 0;
    virtual EndpointPtr read(BasicStream*) const = 0;
    virtual void destroy() = 0;

protected:

    EndpointFactory();
};

}

#endif
