//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SendQueueSizeMaxReached.h"

IceStorm::SendQueueSizeMaxReached::~SendQueueSizeMaxReached()
{
}

std::string_view
IceStorm::SendQueueSizeMaxReached::ice_staticId()
{
    static constexpr std::string_view typeId = "::IceStorm::SendQueueSizeMaxReached";
    return typeId;
}
