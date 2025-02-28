// Copyright (c) ZeroC, Inc.

#ifndef ICE_UNKNOWN_ENDPOINT_I_H
#define ICE_UNKNOWN_ENDPOINT_I_H

#include "EndpointFactory.h"
#include "EndpointI.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"

#include <optional>

namespace IceInternal
{
    class OpaqueEndpointI final : public EndpointI, public std::enable_shared_from_this<OpaqueEndpointI>
    {
    public:
        OpaqueEndpointI(std::vector<std::string>&);
        OpaqueEndpointI(std::int16_t, Ice::InputStream*);

        void streamWrite(Ice::OutputStream*) const final;
        [[nodiscard]] Ice::EndpointInfoPtr getInfo() const noexcept final;
        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] const std::string& protocol() const final;

        [[nodiscard]] std::int32_t timeout() const final;
        [[nodiscard]] EndpointIPtr timeout(std::int32_t) const final;
        [[nodiscard]] const std::string& connectionId() const final;
        [[nodiscard]] EndpointIPtr connectionId(const std::string&) const final;
        [[nodiscard]] bool compress() const final;
        [[nodiscard]] EndpointIPtr compress(bool) const final;
        [[nodiscard]] bool datagram() const final;
        [[nodiscard]] bool secure() const final;

        [[nodiscard]] TransceiverPtr transceiver() const final;
        void connectorsAsync(
            std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const final;
        [[nodiscard]] AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        [[nodiscard]] std::vector<EndpointIPtr> expandHost() const final;
        [[nodiscard]] bool isLoopbackOrMulticast() const final;
        [[nodiscard]] std::shared_ptr<EndpointI> toPublishedEndpoint(std::string publishedHost) const final;
        [[nodiscard]] bool equivalent(const EndpointIPtr&) const final;
        [[nodiscard]] std::size_t hash() const noexcept final;
        [[nodiscard]] std::string options() const final;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

        using EndpointI::connectionId;

    protected:
        void streamWriteImpl(Ice::OutputStream*) const final;
        bool checkOption(const std::string&, const std::string&, const std::string&) final;

    private:
        //
        // All members are const, because endpoints are immutable.
        //
        std::int16_t _type;
        Ice::EncodingVersion _rawEncoding; // The encoding used for _rawBytes
        const std::vector<std::byte> _rawBytes;
    };
}

#endif
