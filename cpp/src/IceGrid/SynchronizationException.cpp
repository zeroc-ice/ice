//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SynchronizationException.h"

using namespace std;

string IceGrid::SynchronizationException::ice_id() const { return string{ice_staticId()}; }

void IceGrid::SynchronizationException::ice_throw() const { throw *this; }

std::string_view
IceGrid::SynchronizationException::ice_staticId() noexcept
{
    static constexpr std::string_view typeId = "::IceGrid::SynchronizationException";
    return typeId;
}
