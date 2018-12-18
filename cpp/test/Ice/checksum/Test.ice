// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/SliceChecksumDict.ice>

module Test
{

interface Checksum
{
    ["cpp:const"] idempotent Ice::SliceChecksumDict getSliceChecksums();

    void shutdown();
}

}
