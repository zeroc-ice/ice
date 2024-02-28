//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ENDPOINT_FACTORY_H
#define TEST_ENDPOINT_FACTORY_H

#include <Ice/EndpointFactory.h>

class EndpointFactory final : public IceInternal::EndpointFactory, public std::enable_shared_from_this<EndpointFactory>
{
public:

    EndpointFactory(const IceInternal::EndpointFactoryPtr&);

    std::int16_t type() const final;
    std::string protocol() const final;
    IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const final;
    IceInternal::EndpointIPtr read(Ice::InputStream*) const final;
    void destroy() final;

    IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

protected:

    IceInternal::EndpointFactoryPtr _factory;
};

#endif
