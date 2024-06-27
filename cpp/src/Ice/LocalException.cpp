// Copyright (c) ZeroC, Inc.

#include "Ice/LocalException.h"

const char*
Ice::LocalException::ice_id() const noexcept
{
    return "::Ice::LocalException";
}
