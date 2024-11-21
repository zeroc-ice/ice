//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ENDPOINT_I_H
#define TEST_ENDPOINT_I_H

#include "Configuration.h"
#include "Ice/EndpointI.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Test.h"

class EndpointI;
using EndpointIPtr = std::shared_ptr<EndpointI>;

class EndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<EndpointI>
{
public:
    static std::int16_t TYPE_BASE;

    EndpointI(const IceInternal::EndpointIPtr&);

    // From EndpointI
    void streamWriteImpl(Ice::OutputStream*) const final;
    std::int16_t type() const final;
    const std::string& protocol() const final;
    IceInternal::EndpointIPtr timeout(std::int32_t) const final;
    IceInternal::EndpointIPtr connectionId(const std::string&) const final;
    IceInternal::EndpointIPtr compress(bool) const final;
    IceInternal::TransceiverPtr transceiver() const final;
    void connectorsAsync(
        Ice::EndpointSelectionType,
        std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
        std::function<void(std::exception_ptr)>) const final;
    IceInternal::AcceptorPtr
    acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
    std::vector<IceInternal::EndpointIPtr> expandHost() const final;
    bool isLoopbackOrMulticast() const final;
    std::shared_ptr<IceInternal::EndpointI> withPublishedHost(std::string host) const final;
    bool equivalent(const IceInternal::EndpointIPtr&) const final;

    // From TestEndpoint
    std::string toString() const noexcept final;
    Ice::EndpointInfoPtr getInfo() const noexcept final;
    std::int32_t timeout() const final;
    const std::string& connectionId() const final;
    bool compress() const final;
    bool datagram() const final;
    bool secure() const final;

    bool operator==(const Ice::Endpoint&) const final;
    bool operator<(const Ice::Endpoint&) const final;

    std::size_t hash() const noexcept final;
    std::string options() const final;

    IceInternal::EndpointIPtr delegate() const;
    EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

    using IceInternal::EndpointI::connectionId;

private:
    friend class EndpointFactory;

    const IceInternal::EndpointIPtr _endpoint;
    const ConfigurationPtr _configuration;
};

#endif
