// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.echo"]]
module Test
{

//
// This object is available with the identity "__echo".
//
interface Echo
{
    void startBatch();
    void flushBatch();
    void shutdown();
};

};
