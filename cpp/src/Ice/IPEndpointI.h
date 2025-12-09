// Copyright (c) ZeroC, Inc.

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
    class IPEndpointI : public EndpointI, public std::enable_shared_from_this<IPEndpointI>
    {
    public:
        void streamWriteImpl(Ice::OutputStream*) const override;

        [[nodiscard]] std::int16_t type() const override;
        [[nodiscard]] const std::string& protocol() const override;
        [[nodiscard]] bool secure() const override;

        [[nodiscard]] const std::string& connectionId() const override;
        [[nodiscard]] EndpointIPtr connectionId(const std::string&) const override;
        void connectorsAsync(std::function<void(std::vector<ConnectorPtr>)>, std::function<void(std::exception_ptr)>)
            const override;
        [[nodiscard]] std::vector<EndpointIPtr> expandHost() const override;
        [[nodiscard]] bool isLoopbackOrMulticast() const override;
        [[nodiscard]] bool equivalent(const EndpointIPtr&) const override;
        [[nodiscard]] std::size_t hash() const noexcept override;
        [[nodiscard]] std::string options() const override;

        bool operator==(const Ice::Endpoint&) const override;
        bool operator<(const Ice::Endpoint&) const override;

        [[nodiscard]] virtual std::vector<ConnectorPtr>
        connectors(const std::vector<Address>&, const NetworkProxyPtr&) const;

        using EndpointI::connectionId;

        virtual void initWithOptions(std::vector<std::string>&, bool);

    protected:
        friend class EndpointHostResolver;

        bool checkOption(const std::string&, const std::string&, const std::string&) override;

        [[nodiscard]] virtual ConnectorPtr createConnector(const Address& address, const NetworkProxyPtr&) const = 0;
        [[nodiscard]] virtual IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const = 0;

        IPEndpointI(ProtocolInstancePtr, std::string, int, const Address&, std::string);
        IPEndpointI(ProtocolInstancePtr);
        IPEndpointI(ProtocolInstancePtr, Ice::InputStream*);

        const ProtocolInstancePtr _instance;
        const std::string _host;
        const std::string _normalizedHost;
        const int _port;
        const Address _sourceAddr;
        const std::string _connectionId;
    };

    class EndpointHostResolver final
    {
    public:
        EndpointHostResolver(const InstancePtr&);

        void resolve(
            const std::string&,
            int,
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
            IPEndpointIPtr endpoint;
            std::function<void(std::vector<ConnectorPtr>)> response;
            std::function<void(std::exception_ptr)> exception;
            Ice::Instrumentation::ObserverPtr observer;
        };

        const InstancePtr _instance;
        const IceInternal::ProtocolSupport _protocol;
        const bool _preferIPv6;
        bool _destroyed{false};
        std::deque<ResolveEntry> _queue;
        ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
}

#endif
