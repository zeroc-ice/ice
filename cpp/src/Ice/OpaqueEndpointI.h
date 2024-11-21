//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        Ice::EndpointInfoPtr getInfo() const noexcept final;
        std::int16_t type() const final;
        const std::string& protocol() const final;

        std::int32_t timeout() const final;
        EndpointIPtr timeout(std::int32_t) const final;
        const std::string& connectionId() const final;
        EndpointIPtr connectionId(const std::string&) const final;
        bool compress() const final;
        EndpointIPtr compress(bool) const final;
        bool datagram() const final;
        bool secure() const final;

        TransceiverPtr transceiver() const final;
        void connectorsAsync(
            Ice::EndpointSelectionType,
            std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const final;
        AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        std::vector<EndpointIPtr> expandHost() const final;
        bool isLoopbackOrMulticast() const final;
        std::shared_ptr<EndpointI> withPublishedHost(std::string host) const final;
        bool equivalent(const EndpointIPtr&) const final;
        std::size_t hash() const noexcept final;
        std::string options() const final;

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
