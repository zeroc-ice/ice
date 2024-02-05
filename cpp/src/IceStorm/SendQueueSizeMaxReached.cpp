//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SendQueueSizeMaxReached.h"

IceStorm::SendQueueSizeMaxReached::~SendQueueSizeMaxReached()
{
}

const ::std::string&
IceStorm::SendQueueSizeMaxReached::ice_staticId()
{
    static const ::std::string typeId = "::IceStorm::SendQueueSizeMaxReached";
    return typeId;
}
