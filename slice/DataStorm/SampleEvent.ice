// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:DATASTORM_API"]]
[["cpp:doxygen:include:DataStorm/DataStorm.h"]]

/// Data-centric, broker-less publish/subscribe framework. C++ only.
module DataStorm
{
    /// Describes the operation used by a data writer to update a data element.
    enum SampleEvent
    {
        /// The data writer added the element.
        Add,

        /// The data writer updated the element.
        Update,

        /// The data writer partially updated the element.
        PartialUpdate,

        /// The data writer removed the element.
        Remove,
    }

    /// A sequence of sample events.
    sequence<SampleEvent> SampleEventSeq;
}
