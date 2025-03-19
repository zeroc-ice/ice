// Copyright (c) ZeroC, Inc.

#ifndef ICE_UUID_H
#define ICE_UUID_H

#include "Config.h"
#include <string>

namespace Ice
{
    /// Generates a universally unique identifier (UUID).
    /// @return The UUID.
    ICE_API std::string generateUUID();
}

#endif
