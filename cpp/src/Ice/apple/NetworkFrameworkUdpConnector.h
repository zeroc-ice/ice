// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_UDP_CONNECTOR_H
#define ICE_NETWORK_FRAMEWORK_UDP_CONNECTOR_H

#include "../Connector.h"
#include "../Network.h"
#include "../ProtocolInstanceF.h"
#include "../TransceiverF.h"

namespace IceInternal
{
    class NetworkFrameworkUdpConnector final : public Connector
    {
    public:
        NetworkFrameworkUdpConnector(
            ProtocolInstancePtr,
            const std::string& host,
            std::int32_t port,
            const Address& sourceAddr,
            const std::string& mcastInterface,
            std::int32_t mcastTtl,
            std::string connectionId);
        ~NetworkFrameworkUdpConnector() override;

        TransceiverPtr connect() final;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const Connector&) const final;
        bool operator<(const Connector&) const final;

    private:
        const ProtocolInstancePtr _instance;
        const std::string _host;
        const std::int32_t _port;
        const Address _sourceAddr;
        const std::string _mcastInterface;
        const std::int32_t _mcastTtl;
        const std::string _connectionId;
    };
}

#endif
