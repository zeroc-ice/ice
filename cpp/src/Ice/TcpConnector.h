// Copyright (c) ZeroC, Inc.

#ifndef ICE_TCP_CONNECTOR_H
#define ICE_TCP_CONNECTOR_H

#include "Connector.h"
#include "Network.h"
#include "ProtocolInstanceF.h"
#include "TransceiverF.h"

namespace IceInternal
{
    class TcpConnector final : public Connector
    {
    public:
        TcpConnector(ProtocolInstancePtr, const Address&, NetworkProxyPtr, const Address&, std::int32_t, std::string);
        ~TcpConnector() override;
        TransceiverPtr connect() final;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const Connector&) const final;
        bool operator<(const Connector&) const final;

    private:
        const ProtocolInstancePtr _instance;
        const Address _addr;
        const NetworkProxyPtr _proxy;
        const Address _sourceAddr;
        const std::int32_t _timeout;
        const std::string _connectionId;
    };
}

#endif
