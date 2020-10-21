//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

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
