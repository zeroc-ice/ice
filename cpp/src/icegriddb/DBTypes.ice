//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <IceGrid/Admin.ice>

[[suppress-warning(reserved-identifier)]]

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
