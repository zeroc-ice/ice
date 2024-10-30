//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    /**
     * Provides access to the details of a Bluetooth connection.
     * \headerfile IceBT/IceBT.h
     */
    class ICEBT_API ConnectionInfo : public Ice::ConnectionInfo
    {
    public:
        ConnectionInfo()
            : localAddress(""),
              localChannel(-1),
              remoteAddress(""),
              remoteChannel(-1),
              uuid(""),
              rcvSize(0),
              sndSize(0)
        {
        }

        ~ConnectionInfo() override;

        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param localAddress The local Bluetooth address.
         * @param localChannel The local RFCOMM channel.
         * @param remoteAddress The remote Bluetooth address.
         * @param remoteChannel The remote RFCOMM channel.
         * @param uuid The UUID of the service being offered (in a server) or targeted (in a client).
         * @param rcvSize The connection buffer receive size.
         * @param sndSize The connection buffer send size.
         */
        ConnectionInfo(
            Ice::ConnectionInfoPtr underlying,
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
            : Ice::ConnectionInfo(std::move(underlying), incoming, std::move(adapterName), std::move(connectionId)),
              localAddress(std::move(localAddress)),
              localChannel(localChannel),
              remoteAddress(std::move(remoteAddress)),
              remoteChannel(remoteChannel),
              uuid(std::move(uuid)),
              rcvSize(rcvSize),
              sndSize(sndSize)
        {
        }

        /**
         * The local Bluetooth address.
         */
        std::string localAddress;
        /**
         * The local RFCOMM channel.
         */
        int localChannel = -1;
        /**
         * The remote Bluetooth address.
         */
        std::string remoteAddress;
        /**
         * The remote RFCOMM channel.
         */
        int remoteChannel = -1;
        /**
         * The UUID of the service being offered (in a server) or targeted (in a client).
         */
        std::string uuid;
        /**
         * The connection buffer receive size.
         */
        int rcvSize = 0;
        /**
         * The connection buffer send size.
         */
        int sndSize = 0;
    };

    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
