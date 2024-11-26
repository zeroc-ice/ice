//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_CONNECTION_INFO_H
#define ICE_IAP_CONNECTION_INFO_H

#include "../Connection.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace IceIAP
{
    /**
     * Provides access to the connection details of an IAP connection
     * \headerfile Ice/Ice.h
     */
    class ConnectionInfo final : public Ice::ConnectionInfo
    {
    public:
        ~ConnectionInfo() final;
        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * The accessory name.
         */
        const std::string name;

        /**
         * The accessory manufacturer.
         */
        const std::string manufacturer;

        /**
         * The accessory model number.
         */
        const std::string modelNumber;

        /**
         * The accessory firmware revision.
         */
        const std::string firmwareRevision;

        /**
         * The accessory hardware revision.
         */
        const std::string hardwareRevision;

        /**
         * The protocol used by the accessory.
         */
        const std::string protocol;

        // internal constructor
        ConnectionInfo(
            std::string connectionId,
            std::string name,
            std::string manufacturer,
            std::string modelNumber,
            std::string firmwareRevision,
            std::string hardwareRevision,
            std::string protocol)
            : Ice::ConnectionInfo{false, "", std::move(connectionId)},
              name{std::move(name)},
              manufacturer{std::move(manufacturer)},
              modelNumber{std::move(modelNumber)},
              firmwareRevision{std::move(firmwareRevision)},
              hardwareRevision{std::move(hardwareRevision)},
              protocol{std::move(protocol)}
        {
        }
    };

    using ConnectionInfoPtr = std::shared_ptr<ConnectionInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
