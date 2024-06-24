//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#pragma once

#include "Ice/Ice.h"

namespace IceGrid
{
    class SynchronizationException : public Ice::LocalException
    {
    public:
        using LocalException::LocalException;

        const char* ice_id() const noexcept override;
    };
}
