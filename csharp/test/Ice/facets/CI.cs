// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class CI : Test.CDisp_
{
    public CI()
    {
    }

    public override string callA(Ice.Current current) => "A";

    public override string callC(Ice.Current current) => "C";
}
