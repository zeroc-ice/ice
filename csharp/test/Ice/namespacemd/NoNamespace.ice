// Copyright (c) ZeroC, Inc.

#pragma once

module NoNamespace
{
    class C1
    {
        int i;
    }

    class C2 extends C1
    {
        long l;
    }

    exception E1
    {
        int i;
    }

    exception E2 extends E1
    {
        long l;
    }

    exception notify /* Test keyword escape. */
    {
        int i;
    }
}
