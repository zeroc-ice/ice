// Copyright (c) ZeroC, Inc.

#ifndef ICESSL_ENDPOINT_I_H
#define ICESSL_ENDPOINT_I_H

#include "../EndpointFactory.h"
#include "../EndpointI.h"
#include "../IPEndpointI.h"
#include "../Network.h"
#include "Ice/SSL/EndpointInfo.h"
#include "SSLEngineF.h"
#include "SSLInstanceF.h"

namespace Ice::SSL
{
    class EndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<EndpointI>
    {
    public:
        EndpointI(InstancePtr, IceInternal::EndpointIPtr);

        void streamWriteImpl(Ice::OutputStream*) const final;

        [[nodiscard]] Ice::EndpointInfoPtr getInfo() const noexcept final;
        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] const std::string& protocol() const final;

        [[nodiscard]] std::int32_t timeout() const final;
        [[nodiscard]] IceInternal::EndpointIPtr timeout(std::int32_t) const final;
        [[nodiscard]] const std::string& connectionId() const final;
        [[nodiscard]] IceInternal::EndpointIPtr connectionId(const std::string&) const final;
        [[nodiscard]] bool compress() const final;
        [[nodiscard]] IceInternal::EndpointIPtr compress(bool) const final;
        [[nodiscard]] bool datagram() const final;
        [[nodiscard]] bool secure() const final;

        [[nodiscard]] IceInternal::TransceiverPtr transceiver() const final;
        void connectorsAsync(
            std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const final;
        [[nodiscard]] IceInternal::AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        [[nodiscard]] std::vector<IceInternal::EndpointIPtr> expandHost() const final;
        [[nodiscard]] bool isLoopbackOrMulticast() const final;
        [[nodiscard]] std::shared_ptr<IceInternal::EndpointI>
        toPublishedEndpoint(std::string publishedHost) const final;
        [[nodiscard]] bool equivalent(const IceInternal::EndpointIPtr&) const final;
        [[nodiscard]] std::size_t hash() const noexcept final;
        [[nodiscard]] std::string options() const final;

        [[nodiscard]] EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

    protected:
        bool checkOption(const std::string&, const std::string&, const std::string&) final;

    private:
        //
        // All members are const, because endpoints are immutable.
        //
        const InstancePtr _instance;
        const IceInternal::EndpointIPtr _delegate;
    };

    class EndpointFactoryI final : public IceInternal::EndpointFactoryWithUnderlying
    {
    public:
        EndpointFactoryI(const InstancePtr&, std::int16_t);

        [[nodiscard]] IceInternal::EndpointFactoryPtr
        cloneWithUnderlying(const IceInternal::ProtocolInstancePtr&, std::int16_t) const final;

    protected:
        IceInternal::EndpointIPtr
        createWithUnderlying(const IceInternal::EndpointIPtr&, std::vector<std::string>&, bool) const final;
        IceInternal::EndpointIPtr readWithUnderlying(const IceInternal::EndpointIPtr&, Ice::InputStream*) const final;

    private:
        const InstancePtr _sslInstance;
    };
}

#endif
