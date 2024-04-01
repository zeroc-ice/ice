//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_WS_ENDPOINT_I_H
#define ICE_WS_ENDPOINT_I_H

#include "EndpointI.h"
#include "IPEndpointIF.h"
#include "EndpointFactory.h"
#include "Ice/InstanceF.h"
#include "Ice/Endpoint.h"
#include "ProtocolInstance.h"

namespace IceInternal
{
    class WSEndpoint final : public EndpointI, public std::enable_shared_from_this<WSEndpoint>
    {
    public:
        WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, const std::string&);
        WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, std::vector<std::string>&);
        WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, Ice::InputStream*);

        void streamWriteImpl(Ice::OutputStream*) const final;

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
            std::function<void(std::vector<ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const final;
        AcceptorPtr acceptor(const std::string&) const final;
        std::vector<EndpointIPtr> expandIfWildcard() const final;
        std::vector<EndpointIPtr> expandHost(EndpointIPtr&) const final;
        bool equivalent(const EndpointIPtr&) const final;
        std::int32_t hash() const final;
        std::string options() const final;

        WSEndpointPtr endpoint(const EndpointIPtr&) const;

        bool operator==(const Ice::Endpoint&) const final;
        bool operator<(const Ice::Endpoint&) const final;

    protected:
        bool checkOption(const std::string&, const std::string&, const std::string&);

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

        EndpointFactoryPtr cloneWithUnderlying(const ProtocolInstancePtr&, std::int16_t) const final;

    protected:
        EndpointIPtr createWithUnderlying(const EndpointIPtr&, std::vector<std::string>&, bool) const final;
        EndpointIPtr readWithUnderlying(const EndpointIPtr&, Ice::InputStream*) const final;
    };
}

#endif
