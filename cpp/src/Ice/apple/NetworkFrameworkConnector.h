// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_CONNECTOR_H
#define ICE_NETWORK_FRAMEWORK_CONNECTOR_H

#include "../Connector.h"
#include "../Network.h"
#include "../NetworkProxyF.h"
#include "../ProtocolInstanceF.h"
#include "../TransceiverF.h"

namespace IceInternal
{
    class NetworkFrameworkConnector final : public Connector
    {
    public:
        NetworkFrameworkConnector(
            ProtocolInstancePtr,
            const Address& addr,
            NetworkProxyPtr proxy,
            const Address& sourceAddr,
            std::int32_t timeout,
            std::string connectionId);
        ~NetworkFrameworkConnector() override;

        TransceiverPtr connect() final;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const Connector&) const final;
        bool operator<(const Connector&) const final;

        [[nodiscard]] const std::string& host() const { return _host; }
        [[nodiscard]] std::int32_t port() const { return _port; }
        [[nodiscard]] const Address& address() const { return _addr; }
        [[nodiscard]] const NetworkProxyPtr& proxy() const { return _proxy; }
        [[nodiscard]] const Address& sourceAddress() const { return _sourceAddr; }

    private:
        const ProtocolInstancePtr _instance;
        const Address _addr;
        const NetworkProxyPtr _proxy;
        std::string _host;
        std::int32_t _port;
        const Address _sourceAddr;
        const std::int32_t _timeout;
        const std::string _connectionId;
    };
}

#endif
