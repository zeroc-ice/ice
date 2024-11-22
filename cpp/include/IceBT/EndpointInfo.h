//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_ENDPOINT_INFO_H
#define ICE_BT_ENDPOINT_INFO_H

#include "Ice/Endpoint.h"
#include "Types.h"

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

#endif
