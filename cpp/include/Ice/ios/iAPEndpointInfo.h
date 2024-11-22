//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_ENDPOINT_INFO_H
#define ICE_IAP_ENDPOINT_INFO_H

#include "../Endpoint.h"

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

#endif
