// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    virtual const ::std::string& protocol() const = 0;
    virtual EndpointPtr create(const std::string&) const = 0;
    virtual EndpointPtr read(BasicStream*) const = 0;
    virtual void destroy() = 0;

protected:

    EndpointFactory();
};

}

#endif
