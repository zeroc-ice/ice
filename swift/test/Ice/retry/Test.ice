// Copyright (c) ZeroC, Inc.
#pragma once

module Test
{
    interface Retry
    {
        void op(bool kill);

        idempotent int opIdempotent(int c);
        void opNotIdempotent();

        idempotent void shutdown();
    }
}
