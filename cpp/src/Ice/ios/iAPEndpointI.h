// Copyright (c) ZeroC, Inc.

#ifndef ICE_IAP_ENDPOINT_I_H
#define ICE_IAP_ENDPOINT_I_H

#include "../EndpointFactory.h"
#include "../EndpointI.h"
#include "../ProtocolInstanceF.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"

#include <optional>

namespace IceObjC
{
    class iAPEndpointI;
    typedef std::shared_ptr<iAPEndpointI> iAPEndpointIPtr;

    class iAPEndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<iAPEndpointI>
    {
    public:
        iAPEndpointI(
            const IceInternal::ProtocolInstancePtr&,
            const std::string&,
            const std::string&,
            const std::string&,
            const std::string&,
            std::int32_t,
            const std::string&,
            bool);
        iAPEndpointI(const IceInternal::ProtocolInstancePtr&);
        iAPEndpointI(const IceInternal::ProtocolInstancePtr&, Ice::InputStream*);

        void streamWriteImpl(Ice::OutputStream*) const final;

        Ice::EndpointInfoPtr getInfo() const noexcept final;
        std::int16_t type() const final;
        const std::string& protocol() const final;
        bool datagram() const final;
        bool secure() const final;

        std::int32_t timeout() const final;
        IceInternal::EndpointIPtr timeout(std::int32_t) const final;
        const std::string& connectionId() const final;
        IceInternal::EndpointIPtr connectionId(const std::string&) const final;
        bool compress() const final;
        IceInternal::EndpointIPtr compress(bool) const final;

        IceInternal::TransceiverPtr transceiver() const final;
        void connectorsAsync(
            std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const final;
        IceInternal::AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const final;
        std::vector<IceInternal::EndpointIPtr> expandHost() const final;
        bool isLoopbackOrMulticast() const final;
        std::shared_ptr<EndpointI> toPublishedEndpoint(std::string publishedHost) const final;
        bool equivalent(const IceInternal::EndpointIPtr&) const final;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

        std::string options() const final;
        std::size_t hash() const noexcept final;

    private:
        bool checkOption(const std::string&, const std::string&, const std::string&) final;

        //
        // All members are const, because endpoints are immutable.
        //
        const IceInternal::ProtocolInstancePtr _instance;
        const std::string _manufacturer;
        const std::string _modelNumber;
        const std::string _name;
        const std::string _protocol;
        const std::int32_t _timeout;
        const std::string _connectionId;
        const bool _compress;
    };

    class iAPEndpointFactory final : public IceInternal::EndpointFactory
    {
    public:
        iAPEndpointFactory(const IceInternal::ProtocolInstancePtr&);

        ~iAPEndpointFactory();

        std::int16_t type() const final;
        std::string protocol() const final;
        IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const final;
        IceInternal::EndpointIPtr read(Ice::InputStream*) const final;

        IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

    private:
        const IceInternal::ProtocolInstancePtr _instance;
    };
}

#endif
