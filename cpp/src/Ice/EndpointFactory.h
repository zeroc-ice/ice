// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDPOINT_FACTORY_H
#define ICE_ENDPOINT_FACTORY_H

#include "EndpointFactoryF.h"
#include "EndpointIF.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Config.h"
#include "Ice/Plugin.h"
#include "ProtocolInstanceF.h"

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
        [[nodiscard]] virtual std::int16_t type() const = 0;
        [[nodiscard]] virtual std::string protocol() const = 0;
        virtual EndpointIPtr create(std::vector<std::string>&, bool) const = 0;
        virtual EndpointIPtr read(Ice::InputStream*) const = 0;

        [[nodiscard]] virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const = 0;
    };

    //
    // The endpoint factory with underlying create endpoints that delegate to an underlying
    // endpoint (e.g.: the SSL/WS endpoints are endpoints with underlying endpoints).
    //
    class ICE_API EndpointFactoryWithUnderlying : public EndpointFactory
    {
    public:
        EndpointFactoryWithUnderlying(ProtocolInstancePtr, std::int16_t);

        void initialize() override;
        [[nodiscard]] std::int16_t type() const override;
        [[nodiscard]] std::string protocol() const override;
        EndpointIPtr create(std::vector<std::string>&, bool) const override;
        EndpointIPtr read(Ice::InputStream*) const override;

        [[nodiscard]] EndpointFactoryPtr clone(const ProtocolInstancePtr&) const override;

        [[nodiscard]] virtual EndpointFactoryPtr
        cloneWithUnderlying(const ProtocolInstancePtr&, std::int16_t) const = 0;

    protected:
        virtual EndpointIPtr createWithUnderlying(const EndpointIPtr&, std::vector<std::string>&, bool) const = 0;
        virtual EndpointIPtr readWithUnderlying(const EndpointIPtr&, Ice::InputStream*) const = 0;

        const ProtocolInstancePtr _instance;
        const std::int16_t _type;
        EndpointFactoryPtr _underlying;
    };

    /// The underlying endpoint factory creates endpoints with a factory of the given type.
    /// If this factory is of the @c EndpointFactoryWithUnderlying type,
    /// it will delegate to the given underlying factory.
    /// @remark This is used by IceIAP/IceBT plugins for the BTS/iAPS endpoint factories.
    class ICE_API UnderlyingEndpointFactory : public EndpointFactory
    {
    public:
        UnderlyingEndpointFactory(ProtocolInstancePtr, std::int16_t, std::int16_t);

        void initialize() override;
        [[nodiscard]] std::int16_t type() const override;
        [[nodiscard]] std::string protocol() const override;
        EndpointIPtr create(std::vector<std::string>&, bool) const override;
        EndpointIPtr read(Ice::InputStream*) const override;

        [[nodiscard]] EndpointFactoryPtr clone(const ProtocolInstancePtr&) const override;

    private:
        const ProtocolInstancePtr _instance;
        const std::int16_t _type;
        const std::int16_t _underlying;
        EndpointFactoryPtr _factory;
    };

    class ICE_API EndpointFactoryPlugin : public Ice::Plugin
    {
    public:
        EndpointFactoryPlugin(const Ice::CommunicatorPtr&, const EndpointFactoryPtr&);

        void initialize() override;
        void destroy() override;
    };
}

#endif
