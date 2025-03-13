// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class DI : Test.DDisp_
{
    public DI()
    {
    }

    public override string callA(Ice.Current current)
    {
        return "A";
    }

    public override string callB(Ice.Current current)
    {
        return "B";
    }

    public override string callC(Ice.Current current)
    {
        return "C";
    }

    public override string callD(Ice.Current current)
    {
        return "D";
    }
}
