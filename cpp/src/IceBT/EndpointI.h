//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_ENDPOINT_I_H
#define ICE_BT_ENDPOINT_I_H

#include "../Ice/EndpointFactory.h"
#include "../Ice/EndpointI.h"
#include "Config.h"
#include "Engine.h"
#include "IceBT/EndpointInfo.h"
#include "InstanceF.h"

namespace IceBT
{
    class EndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<EndpointI>
    {
    public:
        EndpointI(
            const InstancePtr&,
            const std::string&,
            const std::string&,
            const std::string&,
            std::int32_t,
            std::int32_t,
            const std::string&,
            bool);
        EndpointI(const InstancePtr&);
        EndpointI(const InstancePtr&, Ice::InputStream*);

        void streamWriteImpl(Ice::OutputStream*) const final;
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

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

        std::size_t hash() const noexcept final;

        std::string options() const final;

        Ice::EndpointInfoPtr getInfo() const noexcept final;

        void initWithOptions(std::vector<std::string>&, bool);

        EndpointIPtr endpoint(const AcceptorIPtr&) const;

    private:
        bool checkOption(const std::string&, const std::string&, const std::string&);

        const InstancePtr _instance;
        const std::string _addr;
        const std::string _uuid;
        const std::string _name;
        const std::int32_t _channel;
        const std::int32_t _timeout;
        const std::string _connectionId;
        const bool _compress;
    };

    class EndpointInfoI final : public EndpointInfo
    {
    public:
        EndpointInfoI(const EndpointIPtr&);
        ~EndpointInfoI() = default;

        std::int16_t type() const noexcept final;
        bool datagram() const noexcept final;
        bool secure() const noexcept final;

    private:
        const EndpointIPtr _endpoint;
    };

    class EndpointFactoryI final : public IceInternal::EndpointFactory
    {
    public:
        EndpointFactoryI(const InstancePtr&);
        ~EndpointFactoryI();

        std::int16_t type() const final;
        std::string protocol() const final;
        IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const final;
        IceInternal::EndpointIPtr read(Ice::InputStream*) const final;

        IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

    private:
        const InstancePtr _instance;
    };
}

#endif
