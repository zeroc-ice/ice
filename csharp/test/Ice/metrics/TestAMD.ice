// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    exception UserEx
    {
    }
<<<<<<< Updated upstream

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

=======

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

>>>>>>> Stashed changes
        void resume();
    }
}
