//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_ENDPOINT_I_H
#define ICE_BT_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>
#include <IceBT/Config.h>
#include <IceBT/EndpointInfo.h>
#include <IceBT/Engine.h>
#include <IceBT/InstanceF.h>

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
        IceInternal::AcceptorPtr acceptor(const std::string&) const final;
        std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const final;
        std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const final;
        bool equivalent(const IceInternal::EndpointIPtr&) const final;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

        std::int32_t hash() const final;

        std::string options() const final;

        Ice::EndpointInfoPtr getInfo() const noexcept final;

        void initWithOptions(std::vector<std::string>&, bool);

        EndpointIPtr endpoint(const AcceptorIPtr&) const;

    private:
        void hashInit();
        bool checkOption(const std::string&, const std::string&, const std::string&);

        const InstancePtr _instance;
        const std::string _addr;
        const std::string _uuid;
        const std::string _name;
        const std::int32_t _channel;
        const std::int32_t _timeout;
        const std::string _connectionId;
        const bool _compress;
        const std::int32_t _hashValue;
    };

    class EndpointInfoI final : public EndpointInfo
    {
    public:
        EndpointInfoI(const EndpointIPtr&);
        ~EndpointInfoI();

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
        void destroy();

        IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

    private:
        InstancePtr _instance;
    };

}

#endif
