// Copyright (c) ZeroC, Inc.

#ifndef ICE_OBJECT_ADAPTER_F_H
#define ICE_OBJECT_ADAPTER_F_H

#include <memory>

namespace Ice
{
    class ObjectAdapter;

    /// A shared pointer to an ObjectAdapter.
    using ObjectAdapterPtr = std::shared_ptr<ObjectAdapter>;
}

#endif
