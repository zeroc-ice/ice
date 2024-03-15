//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SynchronizationException.h"

using namespace std;

string
IceGrid::SynchronizationException::ice_id() const
{
    return string{ice_staticId()};
}

std::string_view
IceGrid::SynchronizationException::ice_staticId() noexcept
{
    static constexpr std::string_view typeId = "::IceGrid::SynchronizationException";
    return typeId;
}
