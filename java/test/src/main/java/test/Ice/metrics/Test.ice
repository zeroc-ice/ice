// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.metrics"]]
module Test
{

exception UserEx
{
}

sequence<byte> ByteSeq;

interface Metrics
{
    void op();

    idempotent void fail();

    void opWithUserException()
        throws UserEx;

    void opWithRequestFailedException();

    void opWithLocalException();

    void opWithUnknownException();

    void opByteS(ByteSeq bs);

    Object* getAdmin();

    void shutdown();
}

interface Controller
{
    void hold();

    void resume();
}

}
