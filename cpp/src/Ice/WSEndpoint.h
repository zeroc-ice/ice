// Copyright (c) ZeroC, Inc.

#ifndef ICE_WS_ENDPOINT_I_H
#define ICE_WS_ENDPOINT_I_H

#include "EndpointFactory.h"
#include "EndpointI.h"
#include "IPEndpointIF.h"
#include "Ice/Endpoint.h"
#include "Ice/InstanceF.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "ProtocolInstance.h"

#include <optional>

namespace IceInternal
{
    class WSEndpoint final : public EndpointI, public std::enable_shared_from_this<WSEndpoint>
    {
    public:
        WSEndpoint(ProtocolInstancePtr, EndpointIPtr, std::string);
        WSEndpoint(ProtocolInstancePtr, EndpointIPtr, std::vector<std::string>&);
        WSEndpoint(ProtocolInstancePtr, EndpointIPtr, Ice::InputStream*);

        void streamWriteImpl(Ice::OutputStream*) const final;

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
        void connectorsAsync(std::function<void(std::vector<ConnectorPtr>)>, std::function<void(std::exception_ptr)>)
            const final;
        [[nodiscard]] AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        [[nodiscard]] std::vector<EndpointIPtr> expandHost() const final;
        [[nodiscard]] bool isLoopbackOrMulticast() const final;
        [[nodiscard]] std::shared_ptr<EndpointI> toPublishedEndpoint(std::string publishedHost) const final;
        [[nodiscard]] bool equivalent(const EndpointIPtr&) const final;
        [[nodiscard]] std::size_t hash() const noexcept final;
        [[nodiscard]] std::string options() const final;

        [[nodiscard]] WSEndpointPtr endpoint(const EndpointIPtr&) const;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

    protected:
        bool checkOption(const std::string&, const std::string&, const std::string&) override;

    private:
        //
        // All members are const, because endpoints are immutable.
        //
        const ProtocolInstancePtr _instance;
        const EndpointIPtr _delegate;
        const std::string _resource;
    };

    class ICE_API WSEndpointFactory final : public EndpointFactoryWithUnderlying
    {
    public:
        WSEndpointFactory(const ProtocolInstancePtr&, std::int16_t);

        [[nodiscard]] EndpointFactoryPtr cloneWithUnderlying(const ProtocolInstancePtr&, std::int16_t) const final;

    protected:
        EndpointIPtr createWithUnderlying(const EndpointIPtr&, std::vector<std::string>&, bool) const final;
        EndpointIPtr readWithUnderlying(const EndpointIPtr&, Ice::InputStream*) const final;
    };
}

#endif
