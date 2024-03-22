//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ENDPOINT_INFO_H
#define ICESSL_ENDPOINT_INFO_H

#include "Ice/Endpoint.h"

namespace IceSSL
{
    /**
     * Provides access to an SSL endpoint information.
     * \headerfile IceSSL/IceSSL.h
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
        EndpointInfo(const std::shared_ptr<Ice::EndpointInfo>& underlying, int timeout, bool compress)
            : Ice::EndpointInfo(underlying, timeout, compress)
        {
        }

        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;
    };

    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#endif
