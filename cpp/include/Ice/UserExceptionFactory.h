// Copyright (c) ZeroC, Inc.

#ifndef ICE_USER_EXCEPTION_FACTORY_H
#define ICE_USER_EXCEPTION_FACTORY_H

#include <functional>
#include <string_view>

namespace Ice
{
    /// Creates and throws a user exception.
    using UserExceptionFactory = std::function<void(std::string_view)>;
}

#endif
