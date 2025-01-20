// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONNECTION_OPTIONS_H
#define ICE_CONNECTION_OPTIONS_H

#include <chrono>

namespace Ice
{
    /// Represents a property bag used to configure client and server connections.
    struct ConnectionOptions
    {
        std::chrono::seconds connectTimeout;
        std::chrono::seconds closeTimeout;
        std::chrono::seconds idleTimeout;
        bool enableIdleCheck;
        std::chrono::seconds inactivityTimeout;
        int maxDispatches;
    };
}

#endif
