//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_CONNECTION_INFO_H
#define ICE_IAP_CONNECTION_INFO_H

#include "../Connection.h"

namespace IceIAP
{
    /**
     * Provides access to the connection details of an IAP connection
     * \headerfile Ice/Ice.h
     */
    class ConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        ConnectionInfo() = default;
        ~ConnectionInfo() final;
        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * The accessory name.
         */
        std::string name;
        /**
         * The accessory manufacturer.
         */
        std::string manufacturer;
        /**
         * The accessory model number.
         */
        std::string modelNumber;
        /**
         * The accessory firmware revision.
         */
        std::string firmwareRevision;
        /**
         * The accessory hardware revision.
         */
        std::string hardwareRevision;
        /**
         * The protocol used by the accessory.
         */
        std::string protocol;
    };

    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;
}

#endif
