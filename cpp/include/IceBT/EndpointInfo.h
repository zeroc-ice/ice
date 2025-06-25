// Copyright (c) ZeroC, Inc.

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
    /// Provides access to Bluetooth endpoint information.
    /// @headerfile IceBT/IceBT.h
    class ICEBT_API EndpointInfo final : public Ice::EndpointInfo
    {
    public:
        ~EndpointInfo() final;
        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        [[nodiscard]] std::int16_t type() const noexcept final { return _type; }
        [[nodiscard]] bool secure() const noexcept final { return _secure; }

        /// The address configured with the endpoint.
        const std::string addr;

        /// The UUID configured with the endpoint.
        const std::string uuid;

        /// @private
        EndpointInfo(bool compress, std::string addr, std::string uuid, std::int16_t type, bool secure)
            : Ice::EndpointInfo{compress},
              addr{std::move(addr)},
              uuid{std::move(uuid)},
              _type{type},
              _secure{secure}
        {
        }

    private:
        const std::int16_t _type;
        const bool _secure;
    };

    /// A shared pointer to an EndpointInfo.
    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
