// Copyright (c) ZeroC, Inc.

#ifndef ICE_VALUE_F_H
#define ICE_VALUE_F_H

#include <memory>

namespace Ice
{
    class Value;

    /// A shared pointer to a Value.
    using ValuePtr = std::shared_ptr<Value>;
}

#endif
