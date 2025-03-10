// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

module IceStormElection
{
    /// A struct used for marking the last log update.
    struct LogUpdate
    {
        /// The generation.
        long generation;
        /// The iteration within this generation.
        long iteration;
    }
}
