// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

#include "SubscriberRecord.ice"
#include "LLURecord.ice"

module IceStormElection
{
    dictionary<string, LogUpdate> StringLogUpdateDict;
}

module IceStorm
{
    dictionary<SubscriberRecordKey, SubscriberRecord> SubscriberRecordDict;

    struct AllData
    {
        IceStormElection::StringLogUpdateDict llus;
        IceStorm::SubscriberRecordDict subscribers;
    }
}
