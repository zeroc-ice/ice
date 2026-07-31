// Copyright (c) ZeroC, Inc.

#ifndef ICE_UUID_H
#define ICE_UUID_H

#include "Config.h"
#include <string>

namespace Ice
{
    /// Generates a universally unique identifier (UUID).
    /// @return The UUID.
    /// @throws std::system_error If the operating system's random number generator fails.
    ICE_API std::string generateUUID();
}

#endif
