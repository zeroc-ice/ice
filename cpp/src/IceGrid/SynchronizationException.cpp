//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SynchronizationException.h"

IceGrid::SynchronizationException::~SynchronizationException()
{
}

const ::std::string&
IceGrid::SynchronizationException::ice_staticId()
{
    static const ::std::string typeId = "::IceGrid::SynchronizationException";
    return typeId;
}
