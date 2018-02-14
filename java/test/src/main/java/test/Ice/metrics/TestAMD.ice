// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.metrics.AMD"]]
module Test
{

exception UserEx
{
};

sequence<byte> ByteSeq;

interface Metrics
{
    ["amd"] void op();

    ["amd"] idempotent void fail();
    
    ["amd"] void opWithUserException()
        throws UserEx;

    ["amd"] void opWithRequestFailedException();

    ["amd"] void opWithLocalException();

    ["amd"] void opWithUnknownException();

    ["amd"] void opByteS(ByteSeq bs);

    Object* getAdmin();

    void shutdown();
};

interface Controller
{
    void hold();

    void resume();
};

};
