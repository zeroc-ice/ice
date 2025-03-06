// Copyright (c) ZeroC, Inc.

#pragma once

#include "IceGrid/Admin.ice"

[["cpp:header-ext:h"]]

module IceGrid
{
    dictionary<string, long> StringLongDict;

    struct AllData
    {
        ApplicationInfoSeq applications;
        AdapterInfoSeq adapters;
        ObjectInfoSeq objects;
        ObjectInfoSeq internalObjects;
        StringLongDict serials;
    }
}
