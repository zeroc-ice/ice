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
    class EndpointInfo final : public Ice::EndpointInfo
    {
    public:
        ~EndpointInfo() final;
        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        std::int16_t type() const noexcept final { return _type; }
        bool secure() const noexcept final { return _secure; }

        /**
         * The accessory manufacturer or empty to not match against
         * a manufacturer.
         */
        const std::string manufacturer;
        /**
         * The accessory model number or empty to not match against
         * a model number.
         */
        const std::string modelNumber;
        /**
         * The accessory name or empty to not match against
         * the accessory name.
         */
        const std::string name;
        /**
         * The protocol supported by the accessory.
         */
        const std::string protocol;

        // internal constructor
        EndpointInfo(
            int timeout,
            bool compress,
            std::string manufacturer,
            std::string modelNumber,
            std::string name,
            std::string protocol,
            std::int16_t type,
            bool secure)
            : Ice::EndpointInfo{timeout, compress},
              manufacturer{std::move(manufacturer)},
              modelNumber{std::move(modelNumber)},
              name{std::move(name)},
              protocol{std::move(protocol)},
              _type{type},
              _secure{secure}
        {
        }

    private:
        const std::int16_t _type;
        const bool _secure;
    };

    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#endif
