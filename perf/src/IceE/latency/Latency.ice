// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Demo
{
sequence<byte> ByteSeq;

class Latency
{
    ["ami"] void ping();

    //
    // A simple latency test with a payload. The payload size is set
    // programmatically.
    //
    ["ami"] void withData(ByteSeq payLoad);

    void shutdown();
};

};
