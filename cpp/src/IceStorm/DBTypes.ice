// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <SubscriberRecord.ice>
#include <LLURecord.ice>

module IceStormElection
{
dictionary<string, LogUpdate> StringLogUpdateDict;
};

module IceStorm
{
dictionary<SubscriberRecordKey, SubscriberRecord> SubscriberRecordDict;

struct AllData
{
    IceStormElection::StringLogUpdateDict llus;
    IceStorm::SubscriberRecordDict subscribers;
};

};
