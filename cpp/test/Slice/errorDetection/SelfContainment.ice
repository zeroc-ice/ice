// Copyright (c) ZeroC, Inc.

// icefmt: ignore-file

module Test
{
    struct x
    {
        int i;
        x j;        // Error
    }

    class y
    {
        int i;
        y j;        // OK
    }
}
