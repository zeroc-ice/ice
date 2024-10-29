//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_ENDPOINT_INFO_H
#define ICE_BT_ENDPOINT_INFO_H

#include "Ice/Endpoint.h"
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
     * Provides access to Bluetooth endpoint information.
     * \headerfile IceBT/IceBT.h
     */
    class ICEBT_API EndpointInfo : public Ice::EndpointInfo
    {
    public:
        EndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param addr The address configured with the endpoint.
         * @param uuid The UUID configured with the endpoint.
         */
        EndpointInfo(
            Ice::EndpointInfoPtr underlying,
            int timeout,
            bool compress,
            std::string addr,
            std::string uuid)
            : Ice::EndpointInfo(std::move(underlying), timeout, compress),
              addr(std::move(addr)),
              uuid(std::move(uuid))
        {
        }

        ~EndpointInfo() override;

        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        /**
         * The address configured with the endpoint.
         */
        std::string addr;
        /**
         * The UUID configured with the endpoint.
         */
        std::string uuid;
    };

    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
