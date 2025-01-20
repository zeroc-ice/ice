// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOCATOR_INFO_F_H
#define ICE_LOCATOR_INFO_F_H

#include <memory>

namespace IceInternal
{
    class LocatorManager;
    using LocatorManagerPtr = std::shared_ptr<LocatorManager>;

    class LocatorInfo;
    using LocatorInfoPtr = std::shared_ptr<LocatorInfo>;

    class LocatorTable;
    using LocatorTablePtr = std::shared_ptr<LocatorTable>;
}

#endif
