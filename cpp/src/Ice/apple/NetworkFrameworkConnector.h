// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_CONNECTOR_H
#define ICE_NETWORK_FRAMEWORK_CONNECTOR_H

#include "../Connector.h"
#include "../Network.h"
#include "../ProtocolInstanceF.h"
#include "../TransceiverF.h"

namespace IceInternal
{
    class NetworkFrameworkConnector final : public Connector
    {
    public:
        NetworkFrameworkConnector(
            ProtocolInstancePtr,
            const std::string& host,
            std::int32_t port,
            const Address& sourceAddr,
            std::int32_t timeout,
            std::string connectionId);
        ~NetworkFrameworkConnector() override;

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
        const std::int32_t _timeout;
        const std::string _connectionId;
    };
}

#endif
