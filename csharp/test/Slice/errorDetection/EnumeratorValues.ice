//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{
    enum Enum1
    {
        A = 0,
        B = 2147483647,
        C,
        D = 2147483649,
        E = -1,
        F = -2147483649
    }

    enum Enum2 : int
    {
        A = 0,
        B = 2147483647,
        C,
        D = 2147483649,
        E = -1,
        F = -2147483649
    }

    enum Enum3 : byte
    {
        A = 0,
        B = -2,
        C = 6,
        D = 255,
        E,
        F
    }
}
