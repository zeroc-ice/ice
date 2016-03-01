// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_H
#define ICE_ENDPOINT_FACTORY_H

#include <IceUtil/Shared.h>
#include <Ice/EndpointIF.h>
#include <Ice/EndpointFactoryF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>

namespace IceInternal
{

class BasicStream;

class ICE_API EndpointFactory : public ::IceUtil::Shared
{
public:

    virtual ~EndpointFactory();

    virtual ::Ice::Short type() const = 0;
    virtual ::std::string protocol() const = 0;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const = 0;
    virtual EndpointIPtr read(BasicStream*) const = 0;
    virtual void destroy() = 0;

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const = 0;

protected:

    EndpointFactory();
};

class ICE_API EndpointFactoryPlugin : public Ice::Plugin
{
public:

    EndpointFactoryPlugin(const Ice::CommunicatorPtr&, const EndpointFactoryPtr&);

    virtual void initialize();
    virtual void destroy();
};

}

#endif
