// Copyright (c) ZeroC, Inc.

#ifndef TEST_ENDPOINT_FACTORY_H
#define TEST_ENDPOINT_FACTORY_H

#include "Ice/EndpointFactory.h"

class EndpointFactory final : public IceInternal::EndpointFactory, public std::enable_shared_from_this<EndpointFactory>
{
public:
    EndpointFactory(IceInternal::EndpointFactoryPtr);

    [[nodiscard]] std::int16_t type() const final;
    [[nodiscard]] std::string protocol() const final;
    IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const final;
    IceInternal::EndpointIPtr read(Ice::InputStream*) const final;

    [[nodiscard]] IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

protected:
    const IceInternal::EndpointFactoryPtr _factory;
};

#endif
