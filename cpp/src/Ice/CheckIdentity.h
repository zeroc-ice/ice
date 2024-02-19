//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Identity.h"
#include "Ice/LocalException.h"

namespace Ice
{

// An identity with an empty name is illegal for all Ice APIs: no Ice API returns such an identity, and no Ice API
// accepts such an identity argument.
inline void checkIdentity(const Identity& identity)
{
    if (identity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__, identity);
    }
}

}
