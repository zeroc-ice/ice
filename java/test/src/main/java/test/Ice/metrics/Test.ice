// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.metrics.Test"]
module Test
{
    exception UserEx
    {}

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
