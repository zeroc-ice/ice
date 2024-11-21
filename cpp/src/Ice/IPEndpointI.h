//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IP_ENDPOINT_I_H
#define ICE_IP_ENDPOINT_I_H

#include "EndpointI.h"
#include "IPEndpointIF.h"
#include "Ice/Config.h"
#include "Ice/ObserverHelper.h"
#include "Network.h"
#include "ProtocolInstanceF.h"

#include <condition_variable>
#include <deque>
#include <mutex>

namespace IceInternal
{
    class ICE_API IPEndpointInfoI : public Ice::IPEndpointInfo
    {
    public:
        IPEndpointInfoI(const EndpointIPtr&);
        virtual ~IPEndpointInfoI();

        virtual std::int16_t type() const noexcept;
        virtual bool datagram() const noexcept;
        virtual bool secure() const noexcept;

    private:
        const EndpointIPtr _endpoint;
    };

    class ICE_API IPEndpointI : public EndpointI, public std::enable_shared_from_this<IPEndpointI>
    {
    public:
        void streamWriteImpl(Ice::OutputStream*) const override;

        Ice::EndpointInfoPtr getInfo() const noexcept override;
        std::int16_t type() const override;
        const std::string& protocol() const override;
        bool secure() const override;

        const std::string& connectionId() const override;
        EndpointIPtr connectionId(const std::string&) const override;
        void connectorsAsync(
            Ice::EndpointSelectionType,
            std::function<void(std::vector<ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const override;
        std::vector<EndpointIPtr> expandHost() const override;
        bool isLoopbackOrMulticast() const override;
        std::shared_ptr<EndpointI> withPublishedHost(std::string host) const override;
        bool equivalent(const EndpointIPtr&) const override;
        std::size_t hash() const noexcept override;
        std::string options() const override;

        bool operator==(const Ice::Endpoint&) const override;
        bool operator<(const Ice::Endpoint&) const override;

        virtual std::vector<ConnectorPtr> connectors(const std::vector<Address>&, const NetworkProxyPtr&) const;

        virtual void fillEndpointInfo(Ice::IPEndpointInfo*) const;

        using EndpointI::connectionId;

        virtual void initWithOptions(std::vector<std::string>&, bool);

    protected:
        friend class EndpointHostResolver;

        bool checkOption(const std::string&, const std::string&, const std::string&) override;

        virtual ConnectorPtr createConnector(const Address& address, const NetworkProxyPtr&) const = 0;
        virtual IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const = 0;

        IPEndpointI(const ProtocolInstancePtr&, const std::string&, int, const Address&, const std::string&);
        IPEndpointI(const ProtocolInstancePtr&);
        IPEndpointI(const ProtocolInstancePtr&, Ice::InputStream*);

        const ProtocolInstancePtr _instance;
        const std::string _host;
        const int _port;
        const Address _sourceAddr;
        const std::string _connectionId;
    };

    class ICE_API EndpointHostResolver final
    {
    public:
        EndpointHostResolver(const InstancePtr&);

        void resolve(
            const std::string&,
            int,
            Ice::EndpointSelectionType,
            const IPEndpointIPtr&,
            std::function<void(std::vector<ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>);
        void destroy();

        void run();
        void updateObserver();

    private:
        struct ResolveEntry
        {
            std::string host;
            int port;
            Ice::EndpointSelectionType selType;
            IPEndpointIPtr endpoint;
            std::function<void(std::vector<ConnectorPtr>)> response;
            std::function<void(std::exception_ptr)> exception;
            Ice::Instrumentation::ObserverPtr observer;
        };

        const InstancePtr _instance;
        const IceInternal::ProtocolSupport _protocol;
        const bool _preferIPv6;
        bool _destroyed;
        std::deque<ResolveEntry> _queue;
        ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
}

#endif
