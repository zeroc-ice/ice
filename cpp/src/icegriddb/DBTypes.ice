// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <IceGrid/Admin.ice>

[["ice-prefix", "cpp:header-ext:h"]]

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
