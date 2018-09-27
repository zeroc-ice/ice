// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/SliceChecksumDict.ice>

[["cs:namespace:Ice.checksum"]]
module Test
{

interface Checksum
{
    idempotent Ice::SliceChecksumDict getSliceChecksums();

    void shutdown();
}

}
