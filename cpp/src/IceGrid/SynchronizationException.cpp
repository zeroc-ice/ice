//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SynchronizationException.h"

using namespace std;

const char*
IceGrid::SynchronizationException::ice_id() const
{
    return ice_staticId();
}

const char*
IceGrid::SynchronizationException::ice_staticId() noexcept
{
    return "::IceGrid::SynchronizationException";
}
