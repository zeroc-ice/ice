// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_CONNECTOR_H
#define ICE_NETWORK_FRAMEWORK_CONNECTOR_H

#include "../Connector.h"
#include "../Network.h"
#include "../NetworkProxyF.h"
#include "../ProtocolInstanceF.h"
#include "../TransceiverF.h"
#include "ObjectRef.h"

#include <Network/Network.h>

#include <memory>

namespace IceInternal
{
    class NetworkFrameworkTransceiver;

    // Establishes the outgoing TCP and TLS connections of Network.framework. Both share one construction path: the
    // destination endpoint, the source address the endpoint binds the connection to, the connection created from the
    // parameters, and the stream transceiver that owns it. The parameters, with or without the TLS protocol, are
    // the caller's: connect() creates plain ones, the SSL connector configures the TLS ones.
    class NetworkFrameworkConnector final : public Connector
    {
    public:
        // The protocol of the connection, which decides how the connection reaches its destination when a network
        // proxy is configured. The two choices are deliberate, they are not interchangeable:
        //
        // - TCP: the connection is established with the proxy, and the transceiver performs Ice's SOCKS4 or HTTP
        //   CONNECT handshake for the destination before Ice uses the connection, as on the other platforms.
        //
        // - TLS: the TLS handshake starts as soon as the transport connection is established, which leaves no room
        //   for an in-band handshake. The connection is established with the destination through the proxy
        //   configured in the parameters, and Network.framework performs the proxy handshake itself (its SOCKS
        //   client interoperates with the SOCKS4 proxy of the Ice test suite). Requires macOS 14 or iOS 17.
        enum class Protocol
        {
            TCP,
            TLS
        };

        NetworkFrameworkConnector(
            ProtocolInstancePtr,
            const Address& addr,
            NetworkProxyPtr proxy,
            const Address& sourceAddr,
            std::int32_t timeout,
            std::string connectionId);
        ~NetworkFrameworkConnector() override;

        // Establishes a plain TCP connection.
        TransceiverPtr connect() final;

        // Establishes the connection with the given parameters, which the protocol must match, and returns the
        // transceiver that owns it.
        std::shared_ptr<NetworkFrameworkTransceiver> connect(Protocol, NetworkRef<nw_parameters_t> parameters) const;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const Connector&) const final;
        bool operator<(const Connector&) const final;

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
