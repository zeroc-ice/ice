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
#include <IceUtil/Mutex.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointF.h>
#include <Ice/EndpointFactoryF.h>

namespace IceInternal
{

class BasicStream;

class EndpointFactory : public ::IceUtil::Shared
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
