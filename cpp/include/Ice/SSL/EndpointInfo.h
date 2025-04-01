// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_ENDPOINT_INFO_H
#define ICE_SSL_ENDPOINT_INFO_H

#include "../Endpoint.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace Ice::SSL
{
    /// Provides access to an SSL endpoint information.
    class ICE_API EndpointInfo final : public Ice::EndpointInfo
    {
    public:
        ~EndpointInfo() final;
        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        /// @private
        explicit EndpointInfo(Ice::EndpointInfoPtr underlying) : Ice::EndpointInfo{std::move(underlying)} {}
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
