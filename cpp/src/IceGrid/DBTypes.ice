// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceGrid/Admin.ice>

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
};

};
