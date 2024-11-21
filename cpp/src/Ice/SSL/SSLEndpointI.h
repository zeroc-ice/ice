//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        EndpointI(const InstancePtr&, const IceInternal::EndpointIPtr&);

        void streamWriteImpl(Ice::OutputStream*) const final;

        Ice::EndpointInfoPtr getInfo() const noexcept final;
        std::int16_t type() const final;
        const std::string& protocol() const final;

        std::int32_t timeout() const final;
        IceInternal::EndpointIPtr timeout(std::int32_t) const final;
        const std::string& connectionId() const final;
        IceInternal::EndpointIPtr connectionId(const std::string&) const final;
        bool compress() const final;
        IceInternal::EndpointIPtr compress(bool) const final;
        bool datagram() const final;
        bool secure() const final;

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
        std::size_t hash() const noexcept final;
        std::string options() const final;

        EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

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

        IceInternal::EndpointFactoryPtr
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
