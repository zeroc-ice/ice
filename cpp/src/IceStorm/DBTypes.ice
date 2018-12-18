// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h"]]

#include <IceStorm/SubscriberRecord.ice>
#include <IceStorm/LLURecord.ice>

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
