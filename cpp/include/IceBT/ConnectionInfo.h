// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_CONNECTION_INFO_H
#define ICE_BT_CONNECTION_INFO_H

#include "Ice/Connection.h"
#include "Types.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace IceBT
{
    /// Provides access to the details of a Bluetooth connection.
    /// @headerfile IceBT/IceBT.h
    class ICEBT_API ConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        ~ConnectionInfo() final;
        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /// The local Bluetooth address.
        const std::string localAddress;

        /// The local RFCOMM channel.
        const int localChannel;

        /// The remote Bluetooth address.
        const std::string remoteAddress;

        /// The remote RFCOMM channel.
        const int remoteChannel;

        /// The UUID of the service being offered (in a server) or targeted (in a client).
        const std::string uuid;

        /// The connection buffer receive size.
        const int rcvSize;

        /// The connection buffer send size.
        const int sndSize;

        /// @private
        ConnectionInfo(
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            std::string localAddress,
            int localChannel,
            std::string remoteAddress,
            int remoteChannel,
            std::string uuid,
            int rcvSize,
            int sndSize)
            : Ice::ConnectionInfo{incoming, std::move(adapterName), std::move(connectionId)},
              localAddress{std::move(localAddress)},
              localChannel{localChannel},
              remoteAddress{std::move(remoteAddress)},
              remoteChannel{remoteChannel},
              uuid{std::move(uuid)},
              rcvSize{rcvSize},
              sndSize{sndSize}
        {
        }

        /// @private
        ConnectionInfo(bool incoming, std::string adapterName, std::string connectionId)
            : ConnectionInfo{incoming, std::move(adapterName), std::move(connectionId), "", -1, "", -1, "", 0, 0}
        {
        }
    };

    /// A shared pointer to a ConnectionInfo.
    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
