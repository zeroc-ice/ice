// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]

#include "LocatorRegistry.ice"

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// A server application managed by a locator implementation such as IceGrid hosts a Process object and registers a
    /// proxy to this object with the locator registry. See {@link LocatorRegistry::setServerProcessProxy}.
    interface Process
    {
        /// Initiates a graceful shutdown of the server application.
        void shutdown();

        /// Writes a message on the server application's stdout or stderr.
        /// @param message The message to write.
        /// @param fd 1 for stdout, 2 for stderr.
        void writeMessage(string message, int fd);
    }
}
