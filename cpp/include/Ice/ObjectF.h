// Copyright (c) ZeroC, Inc.

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <memory>

namespace Ice
{
    class Object;

    /// A shared pointer to an Object.
    using ObjectPtr = std::shared_ptr<Object>;
}

#endif
