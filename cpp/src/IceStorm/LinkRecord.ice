// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

#include "Ice/Identity.ice"
#include "IceStormInternal.ice"

module IceStorm
{
    /// Used to store persistent information for {@link Topic} federation.
    struct LinkRecord
    {
        /// The topic link object.
        TopicLink* obj;
        /// The cost.
        int cost;
        /// The linked topic for {@link Topic::getLinkInfoSeq}.
        Topic* theTopic;
    }
}
