//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_CONNECTION_INFO_H
#define ICE_BT_CONNECTION_INFO_H

#include "Ice/Connection.h"
#include "Types.h"

namespace IceBT
{
    /**
     * Provides access to the details of a Bluetooth connection.
     * \headerfile IceBT/IceBT.h
     */
    class ICEBT_API ConnectionInfo : public Ice::ConnectionInfo
    {
    public:
        ConnectionInfo() = default;
        ~ConnectionInfo() final;
        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

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

#endif
