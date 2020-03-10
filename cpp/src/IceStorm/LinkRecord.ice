//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["ice-prefix", "cpp:header-ext:h"]]

#include <Ice/Identity.ice>
#include <IceStorm/IceStormInternal.ice>

module IceStorm
{
    /// Used to store persistent information for Topic federation.
    struct LinkRecord
    {
        /// The topic link object.
        TopicLink* obj;
        /// The cost.
        int cost;
        /// The linked topic for getLinkInfoSeq
        Topic* theTopic;
    }
}
