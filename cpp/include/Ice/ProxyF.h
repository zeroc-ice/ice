//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include "Config.h"
#include <optional>

namespace Ice
{

    class ObjectPrx;

    // TODO: remove
    using ObjectPrxPtr = std::optional<ObjectPrx>;

}

#endif
