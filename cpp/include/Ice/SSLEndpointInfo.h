//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_ENDPOINT_INFO_H
#define ICE_SSL_ENDPOINT_INFO_H

#include "Endpoint.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace IceSSL
{
    /**
     * Provides access to an SSL endpoint information.
     */
    class EndpointInfo : public Ice::EndpointInfo
    {
    public:
        EndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         */
        EndpointInfo(const Ice::EndpointInfoPtr& underlying, int timeout, bool compress)
            : Ice::EndpointInfo(underlying, timeout, compress)
        {
        }

        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;
    };

    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
