// Copyright (c) ZeroC, Inc.

#ifndef ICE_ROUTER_INFO_F_H
#define ICE_ROUTER_INFO_F_H

#include <memory>

namespace IceInternal
{
    class RouterManager;
    using RouterManagerPtr = std::shared_ptr<RouterManager>;

    class RouterInfo;
    using RouterInfoPtr = std::shared_ptr<RouterInfo>;
}

#endif
