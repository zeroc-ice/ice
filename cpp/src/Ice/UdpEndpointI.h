// Copyright (c) ZeroC, Inc.

#ifndef ICE_UDP_ENDPOINT_I_H
#define ICE_UDP_ENDPOINT_I_H

#include "EndpointFactory.h"
#include "IPEndpointI.h"
#include "Ice/Config.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Network.h"

#include <optional>

namespace IceInternal
{
    class UdpEndpointI final : public IPEndpointI
    {
    public:
        UdpEndpointI(
            const ProtocolInstancePtr&,
            const std::string&,
            std::int32_t,
            const Address&,
            std::string,
            std::int32_t,
            const std::string&,
            bool);
        UdpEndpointI(const ProtocolInstancePtr&);
        UdpEndpointI(const ProtocolInstancePtr&, Ice::InputStream*);

        void streamWriteImpl(Ice::OutputStream*) const final;

        [[nodiscard]] Ice::EndpointInfoPtr getInfo() const noexcept final;

        [[nodiscard]] std::int32_t timeout() const final;
        [[nodiscard]] EndpointIPtr timeout(std::int32_t) const final;
        [[nodiscard]] bool compress() const final;
        [[nodiscard]] EndpointIPtr compress(bool) const final;
        [[nodiscard]] bool datagram() const final;

        [[nodiscard]] std::shared_ptr<EndpointI> toPublishedEndpoint(std::string publishedHost) const final;

        [[nodiscard]] TransceiverPtr transceiver() const final;
        [[nodiscard]] AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        [[nodiscard]] std::string options() const final;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

        [[nodiscard]] std::size_t hash() const noexcept final;

        [[nodiscard]] UdpEndpointIPtr endpoint(const UdpTransceiverPtr&) const;

        using IPEndpointI::connectionId;

        void initWithOptions(std::vector<std::string>&, bool) final;

    protected:
        bool checkOption(const std::string&, const std::string&, const std::string&) final;

        [[nodiscard]] ConnectorPtr createConnector(const Address&, const NetworkProxyPtr&) const final;
        [[nodiscard]] IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const final;

    private:
        //
        // All members are const, because endpoints are immutable.
        //
        const std::int32_t _mcastTtl;
        const std::string _mcastInterface;
        const bool _compress;
    };

    class UdpEndpointFactory final : public EndpointFactory
    {
    public:
        UdpEndpointFactory(ProtocolInstancePtr);
        ~UdpEndpointFactory() override;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string protocol() const final;
        EndpointIPtr create(std::vector<std::string>&, bool) const final;
        EndpointIPtr read(Ice::InputStream*) const final;

        [[nodiscard]] EndpointFactoryPtr clone(const ProtocolInstancePtr&) const final;

    private:
        const ProtocolInstancePtr _instance;
    };
}

#endif
