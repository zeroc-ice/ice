//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SendQueueSizeMaxReached.h"

using namespace std;

const char*
IceStorm::SendQueueSizeMaxReached::ice_id() const
{
    return ice_staticId();
}

const char*
IceStorm::SendQueueSizeMaxReached::ice_staticId() noexcept
{
    return "::IceStorm::SendQueueSizeMaxReached";
}
