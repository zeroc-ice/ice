// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class EI : Test.EDisp_
{
    public EI()
    {
    }

    public override string callE(Ice.Current current)
    {
        return "E";
    }
}
