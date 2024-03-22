//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_CONNECTION_INFO_H
#define ICE_IAP_CONNECTION_INFO_H

#include "Ice/Connection.h"

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
     * \headerfile IceIAP/IceIAP.h
     */
    class ConnectionInfo : public Ice::ConnectionInfo
    {
    public:
        ConnectionInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param name The accessory name.
         * @param manufacturer The accessory manufacturer.
         * @param modelNumber The accessory model number.
         * @param firmwareRevision The accessory firmware revision.
         * @param hardwareRevision The accessory hardware revision.
         * @param protocol The protocol used by the accessory.
         */
        ConnectionInfo(
            const std::shared_ptr<Ice::ConnectionInfo>& underlying,
            bool incoming,
            const std::string& adapterName,
            const std::string& connectionId,
            const std::string& name,
            const std::string& manufacturer,
            const std::string& modelNumber,
            const std::string& firmwareRevision,
            const std::string& hardwareRevision,
            const std::string& protocol)
            : Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
              name(name),
              manufacturer(manufacturer),
              modelNumber(modelNumber),
              firmwareRevision(firmwareRevision),
              hardwareRevision(hardwareRevision),
              protocol(protocol)
        {
        }

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

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
