// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.metrics.AMD.Test"]
module Test
{
    exception UserEx
    {}

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
