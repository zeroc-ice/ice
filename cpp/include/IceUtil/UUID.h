//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_UUID_H
#define ICE_UTIL_UUID_H

#include "Config.h"
#include <string>

namespace IceUtil
{
    /**
     * Generates a universally unique identifier (UUID).
     * @return The UUID.
     */
    ICE_API std::string generateUUID();
}

#endif
