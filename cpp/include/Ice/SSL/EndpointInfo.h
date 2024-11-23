//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_ENDPOINT_INFO_H
#define ICE_SSL_ENDPOINT_INFO_H

#include "../Endpoint.h"

namespace Ice::SSL
{
    /**
     * Provides access to an SSL endpoint information.
     */
    class ICE_API EndpointInfo : public Ice::EndpointInfo
    {
    public:
        EndpointInfo() = default;
        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;
    };

    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#endif
