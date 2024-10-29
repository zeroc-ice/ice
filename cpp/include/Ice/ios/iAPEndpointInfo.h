//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_ENDPOINT_INFO_H
#define ICE_IAP_ENDPOINT_INFO_H

#include "../Endpoint.h"

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
     * Provides access to an IAP endpoint information.
     * \headerfile Ice/Ice.h
     */
    class EndpointInfo : public Ice::EndpointInfo
    {
    public:
        EndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint of null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param manufacturer The accessory manufacturer or empty to not match against a manufacturer.
         * @param modelNumber The accessory model number or empty to not match against a model number.
         * @param name The accessory name or empty to not match against the accessory name.
         * @param protocol The protocol supported by the accessory.
         */
        EndpointInfo(
            Ice::EndpointInfoPtr underlying,
            int timeout,
            bool compress,
            std::string manufacturer,
            std::string modelNumber,
            std::string name,
            std::string protocol)
            : Ice::EndpointInfo(std::move(underlying), timeout, compress),
              manufacturer(std::move(manufacturer)),
              modelNumber(std::move(modelNumber)),
              name(std::move(name)),
              protocol(std::move(protocol))
        {
        }

        ~EndpointInfo() override;

        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        /**
         * The accessory manufacturer or empty to not match against
         * a manufacturer.
         */
        std::string manufacturer;
        /**
         * The accessory model number or empty to not match against
         * a model number.
         */
        std::string modelNumber;
        /**
         * The accessory name or empty to not match against
         * the accessory name.
         */
        std::string name;
        /**
         * The protocol supported by the accessory.
         */
        std::string protocol;
    };

    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
