// Copyright (c) ZeroC, Inc.

#include "Ice/Identity.h"
#include "Ice/LocalExceptions.h"

#include <sstream>

namespace Ice
{
    // An identity with an empty name is illegal for all Ice APIs: no Ice API returns such an identity, and no Ice API
    // accepts such an identity argument.
    inline void checkIdentity(const Identity& identity, const char* file, int line)
    {
        if (identity.name.empty())
        {
            std::ostringstream os;
            os << file << ':' << line << ": the name component of an Ice identity cannot be empty";
            throw std::invalid_argument(os.str());
        }
    }
}
