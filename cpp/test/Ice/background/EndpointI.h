// Copyright (c) ZeroC, Inc.

#ifndef TEST_ENDPOINT_I_H
#define TEST_ENDPOINT_I_H

#include "Configuration.h"
#include "Ice/EndpointI.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"

class EndpointI;
using EndpointIPtr = std::shared_ptr<EndpointI>;

class EndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<EndpointI>
{
public:
    static std::int16_t TYPE_BASE;

    EndpointI(IceInternal::EndpointIPtr);

    // From EndpointI
    void streamWriteImpl(Ice::OutputStream*) const final;
    [[nodiscard]] std::int16_t type() const final;
    [[nodiscard]] const std::string& protocol() const final;
    [[nodiscard]] IceInternal::EndpointIPtr timeout(std::int32_t) const final;
    [[nodiscard]] IceInternal::EndpointIPtr connectionId(const std::string&) const final;
    [[nodiscard]] IceInternal::EndpointIPtr compress(bool) const final;
    [[nodiscard]] IceInternal::TransceiverPtr transceiver() const final;
    void connectorsAsync(
        std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
        std::function<void(std::exception_ptr)>) const final;
    [[nodiscard]] IceInternal::AcceptorPtr
    acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
    [[nodiscard]] std::vector<IceInternal::EndpointIPtr> expandHost() const final;
    [[nodiscard]] bool isLoopbackOrMulticast() const final;
    [[nodiscard]] std::shared_ptr<IceInternal::EndpointI> toPublishedEndpoint(std::string publishedHost) const final;
    [[nodiscard]] bool equivalent(const IceInternal::EndpointIPtr&) const final;

    // From TestEndpoint
    [[nodiscard]] std::string toString() const noexcept final;
    [[nodiscard]] Ice::EndpointInfoPtr getInfo() const noexcept final;
    [[nodiscard]] std::int32_t timeout() const final;
    [[nodiscard]] const std::string& connectionId() const final;
    [[nodiscard]] bool compress() const final;
    [[nodiscard]] bool datagram() const final;
    [[nodiscard]] bool secure() const final;

    bool operator==(const Ice::Endpoint&) const final;
    bool operator<(const Ice::Endpoint&) const final;

    [[nodiscard]] std::size_t hash() const noexcept final;
    [[nodiscard]] std::string options() const final;

    [[nodiscard]] IceInternal::EndpointIPtr delegate() const;
    [[nodiscard]] EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

    using IceInternal::EndpointI::connectionId;

private:
    friend class EndpointFactory;

    const IceInternal::EndpointIPtr _endpoint;
    const ConfigurationPtr _configuration;
};

#endif
