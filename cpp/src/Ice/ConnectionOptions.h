//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_OPTIONS_H
#define ICE_CONNECTION_OPTIONS_H

#include <chrono>

namespace Ice
{
    /// Represents a property bag used to configure client and server connections.
    struct ConnectionOptions
    {
        std::chrono::seconds connectTimeout = std::chrono::seconds(10);
        std::chrono::seconds closeTimeout = std::chrono::seconds(10);
        std::chrono::seconds idleTimeout = std::chrono::seconds(60);
        bool enableIdleCheck = false; // TODO: switch to true
        std::chrono::seconds inactivityTimeout = std::chrono::seconds(300);
    };
}

#endif
