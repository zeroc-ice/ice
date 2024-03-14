//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SendQueueSizeMaxReached.h"

using namespace std;

string
IceStorm::SendQueueSizeMaxReached::ice_id() const
{
    return string{ice_staticId()};
}

std::string_view
IceStorm::SendQueueSizeMaxReached::ice_staticId() noexcept
{
    static constexpr std::string_view typeId = "::IceStorm::SendQueueSizeMaxReached";
    return typeId;
}
