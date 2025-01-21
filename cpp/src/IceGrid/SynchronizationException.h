// Copyright (c) ZeroC, Inc.

// TODO: rework this file.

#pragma once

#include "Ice/Ice.h"

namespace IceGrid
{
    class SynchronizationException final : public Ice::LocalException
    {
    public:
        SynchronizationException(const char* file, int line) : Ice::LocalException(file, line, "synchronization error")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };
}
