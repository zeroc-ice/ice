// Copyright (c) ZeroC, Inc.

#ifndef ICE_TCP_ENDPOINT_I_H
#define ICE_TCP_ENDPOINT_I_H

#include "EndpointFactory.h"
#include "IPEndpointI.h"
#include "Ice/Config.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Network.h" // for IceIternal::Address

namespace IceInternal
{
    class TcpEndpointI final : public IPEndpointI
    {
    public:
        TcpEndpointI(
            const ProtocolInstancePtr&,
            const std::string&,
            std::int32_t,
            const Address&,
            std::int32_t,
            const std::string&,
            bool);
        TcpEndpointI(const ProtocolInstancePtr&);
        TcpEndpointI(const ProtocolInstancePtr&, Ice::InputStream*);

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

        [[nodiscard]] TcpEndpointIPtr endpoint(const TcpAcceptorPtr&) const;

        using IPEndpointI::connectionId;

    protected:
        bool checkOption(const std::string&, const std::string&, const std::string&) final;

        [[nodiscard]] ConnectorPtr createConnector(const Address&, const NetworkProxyPtr&) const final;
        [[nodiscard]] IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const final;

    private:
        //
        // All members are const, because endpoints are immutable.
        //
        const std::int32_t _timeout;
        const bool _compress;
    };

    class TcpEndpointFactory final : public EndpointFactory
    {
    public:
        TcpEndpointFactory(ProtocolInstancePtr);
        ~TcpEndpointFactory() override;

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
