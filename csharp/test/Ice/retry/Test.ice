// Copyright (c) ZeroC, Inc.

#pragma once

["cs:identifier:Ice.retry.Test"]
module Test
{
    interface Retry
    {
        void op(bool kill);

        idempotent int opIdempotent(int c);
        void opNotIdempotent();

        idempotent void sleep(int delay);

        idempotent void shutdown();
    }
}
