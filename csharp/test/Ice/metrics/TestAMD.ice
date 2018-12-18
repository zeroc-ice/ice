// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

module Test
{

exception UserEx
{
}

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
}

interface Controller
{
    void hold();

    void resume();
}

}
