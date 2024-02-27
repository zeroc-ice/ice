//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SynchronizationException.h"

IceGrid::SynchronizationException::~SynchronizationException()
{
}

std::string_view
IceGrid::SynchronizationException::ice_staticId()
{
    static constexpr std::string_view typeId = "::IceGrid::SynchronizationException";
    return typeId;
}
