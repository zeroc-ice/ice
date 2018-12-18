// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/SliceChecksumDict.ice>

[["java:package:test.Ice.checksum.server"]] // For classes with operations

module Test
{

interface Checksum
{
    idempotent Ice::SliceChecksumDict getSliceChecksums();

    void shutdown();
}

}
