// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
