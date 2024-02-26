//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_FACTORY_H
#define ICE_ENDPOINT_FACTORY_H

#include <Ice/EndpointIF.h>
#include <Ice/EndpointFactoryF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>

namespace Ice
{

class InputStream;

}

namespace IceInternal
{

class ICE_API EndpointFactory
{
public:

    EndpointFactory();
    virtual ~EndpointFactory();

    virtual void initialize();
    virtual std::int16_t type() const = 0;
    virtual std::string protocol() const = 0;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const = 0;
    virtual EndpointIPtr read(Ice::InputStream*) const = 0;
    virtual void destroy() = 0;

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const = 0;
};

//
// The endpoint factory with underlying create endpoints that delegate to an underlying
// endpoint (e.g.: the SSL/WS endpoints are endpoints with underlying endpoints).
//
class ICE_API EndpointFactoryWithUnderlying : public EndpointFactory
{
public:

    EndpointFactoryWithUnderlying(const ProtocolInstancePtr&, std::int16_t);

    virtual void initialize();
    virtual std::int16_t type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

    virtual EndpointFactoryPtr cloneWithUnderlying(const ProtocolInstancePtr&, std::int16_t) const = 0;

protected:

    virtual EndpointIPtr createWithUnderlying(const EndpointIPtr&, std::vector<std::string>&, bool) const = 0;
    virtual EndpointIPtr readWithUnderlying(const EndpointIPtr&, Ice::InputStream*) const = 0;

    ProtocolInstancePtr _instance;
    const std::int16_t _type;
    EndpointFactoryPtr _underlying;
};

//
// The underlying endpoint factory creates endpoints with a factory of the given
// type. If this factory is of the EndpointFactoryWithUnderlying type, it will
// delegate to the given underlying factory (this is used by IceIAP/IceBT plugins
// for the BTS/iAPS endpoint factories).
//
class ICE_API UnderlyingEndpointFactory : public EndpointFactory
{
public:

    UnderlyingEndpointFactory(const ProtocolInstancePtr&, std::int16_t, std::int16_t);

    virtual void initialize();
    virtual std::int16_t type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

private:

    ProtocolInstancePtr _instance;
    const std::int16_t _type;
    const std::int16_t _underlying;
    EndpointFactoryPtr _factory;
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
