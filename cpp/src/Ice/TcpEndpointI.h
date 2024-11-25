//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

        Ice::EndpointInfoPtr getInfo() const noexcept final;

        std::int32_t timeout() const final;
        EndpointIPtr timeout(std::int32_t) const final;
        bool compress() const final;
        EndpointIPtr compress(bool) const final;
        bool datagram() const final;

        std::shared_ptr<EndpointI> toPublishedEndpoint(std::string publishedHost) const final;

        TransceiverPtr transceiver() const final;
        AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        std::string options() const final;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

        std::size_t hash() const noexcept final;

        TcpEndpointIPtr endpoint(const TcpAcceptorPtr&) const;

        using IPEndpointI::connectionId;

    protected:
        bool checkOption(const std::string&, const std::string&, const std::string&) final;

        ConnectorPtr createConnector(const Address&, const NetworkProxyPtr&) const final;
        IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const final;

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
        TcpEndpointFactory(const ProtocolInstancePtr&);
        ~TcpEndpointFactory();

        std::int16_t type() const final;
        std::string protocol() const final;
        EndpointIPtr create(std::vector<std::string>&, bool) const final;
        EndpointIPtr read(Ice::InputStream*) const final;

        EndpointFactoryPtr clone(const ProtocolInstancePtr&) const final;

    private:
        const ProtocolInstancePtr _instance;
    };
}

#endif
