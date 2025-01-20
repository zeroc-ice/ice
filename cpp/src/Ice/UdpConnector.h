// Copyright (c) ZeroC, Inc.

#ifndef ICE_UDP_CONNECTOR_H
#define ICE_UDP_CONNECTOR_H

#include "Connector.h"
#include "Network.h"
#include "ProtocolInstanceF.h"
#include "TransceiverF.h"

namespace IceInternal
{
    class UdpConnector final : public Connector
    {
    public:
        UdpConnector(ProtocolInstancePtr, const Address&, const Address&, std::string, int, std::string);

        ~UdpConnector() override;
        TransceiverPtr connect() final;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const Connector&) const final;
        bool operator<(const Connector&) const final;

    private:
        const ProtocolInstancePtr _instance;
        const Address _addr;
        const Address _sourceAddr;
        const std::string _mcastInterface;
        const int _mcastTtl;
        const std::string _connectionId;
    };
}
#endif
